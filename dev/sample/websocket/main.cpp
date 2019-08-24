#include <iostream>

#include <restinio/all.hpp>
#include <restinio/websocket/websocket.hpp>

#include <fmt/format.h>

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

namespace rws = restinio::websocket::basic;

using traits_t =
	restinio::traits_t<
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

int main()
{
	using namespace std::chrono;

	try
	{
		ws_registry_t registry;
		restinio::run(
			restinio::on_this_thread<traits_t>()
				.address( "localhost" )
				.request_handler( server_handler( registry ) )
				.read_next_http_message_timelimit( 10s )
				.write_http_response_timelimit( 1s )
				.handle_request_timeout( 1s )
				.cleanup_func( [&]{ registry.clear(); } ) );

	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
