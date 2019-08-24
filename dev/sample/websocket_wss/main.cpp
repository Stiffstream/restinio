#include <iostream>

#include <restinio/all.hpp>
#include <restinio/tls.hpp>
#include <restinio/websocket/websocket.hpp>

#include <fmt/format.h>

using router_t = restinio::router::express_router_t<>;

namespace rws = restinio::websocket::basic;

using traits_t =
	restinio::single_thread_tls_traits_t<
		restinio::asio_timer_manager_t,
		restinio::single_threaded_ostream_logger_t,
		router_t >;

using ws_registry_t = std::map< std::uint64_t, rws::ws_handle_t >;

auto server_handler( ws_registry_t & registry )
{
	auto router = std::make_unique< router_t >();

	router->http_get(
		"/chat",
		[ &registry ]( auto req, auto ) mutable {

			if( restinio::http_connection_header_t::upgrade == req->header().connection() )
			{
				auto wsh =
					rws::upgrade< traits_t >(
						*req,
						rws::activation_t::immediate,
						[ &registry ]( auto wsh, auto m ){
							if( rws::opcode_t::text_frame == m->opcode() ||
								rws::opcode_t::binary_frame == m->opcode() ||
								rws::opcode_t::continuation_frame == m->opcode() )
							{
								wsh->send_message( *m );
							}
							else if( rws::opcode_t::ping_frame == m->opcode() )
							{
								auto resp = *m;
								resp.set_opcode( rws::opcode_t::pong_frame );
								wsh->send_message( resp );
							}
							else if( rws::opcode_t::connection_close_frame == m->opcode() )
							{
								registry.erase( wsh->connection_id() );
							}
						} );

				registry.emplace( wsh->connection_id(), wsh );

				return restinio::request_accepted();
			}

			return restinio::request_rejected();
		} );

	return router;
}

int main( int argc, char const *argv[] )
{
	using namespace std::chrono;

	std::string certs_dir = ".";

	if( 2 == argc )
	{
		certs_dir = argv[ 1 ];
	}

	try
	{
		// Since RESTinio supports both stand-alone ASIO and boost::ASIO
		// we specify an alias for a concrete asio namesace.
		// That's makes it possible to compile the code in both cases.
		// Typicaly only one of ASIO variants would be used,
		// and so only asio::* or only boost::asio::* would be applied.
		namespace asio_ns = restinio::asio_ns;

		asio_ns::ssl::context tls_context{ asio_ns::ssl::context::sslv23 };
		tls_context.set_options(
			asio_ns::ssl::context::default_workarounds
			| asio_ns::ssl::context::no_sslv2
			| asio_ns::ssl::context::single_dh_use );

		tls_context.use_certificate_chain_file( certs_dir + "/wss_server.pem" );
		tls_context.use_private_key_file(
			certs_dir + "/wss_key.pem",
			asio_ns::ssl::context::pem );
		tls_context.use_tmp_dh_file( certs_dir + "/wss_dh2048.pem" );

		ws_registry_t registry;
		restinio::run(
			restinio::on_this_thread<traits_t>()
				.address( "localhost" )
				.request_handler( server_handler( registry ) )
				.read_next_http_message_timelimit( 10s )
				.write_http_response_timelimit( 1s )
				.handle_request_timeout( 1s )
				.tls_context( std::move( tls_context ) )
				.cleanup_func( [&]{ registry.clear(); } ) );

	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
