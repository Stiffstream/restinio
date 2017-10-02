#include <type_traits>
#include <iostream>
#include <chrono>
#include <memory>

#include <asio.hpp>
#include <asio/ip/tcp.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <restinio/all.hpp>
#include <restinio/websocket/websocket.hpp>


namespace rr = restinio::router;
using router_t = rr::express_router_t;

namespace rws = restinio::websocket;

using traits_t =
	restinio::traits_t<
		restinio::asio_timer_factory_t,
		restinio::single_threaded_ostream_logger_t,
		router_t >;

using ws_regestry_t = std::map< std::uint64_t, rws::ws_handle_t >;

auto server_handler()
{
	auto router = std::make_unique< router_t >();

	router->http_get(
		"/chat",
		[ regestry = std::make_shared< ws_regestry_t >() ]( auto req, auto ) mutable {

			if( restinio::http_connection_header_t::upgrade == req->header().connection() )
			{
				auto r = std::weak_ptr< ws_regestry_t >{ regestry };
				auto wsh =
					rws::upgrade< traits_t >(
						*req,
						rws::activation_t::immediate,
						[ r ]( auto wsh, auto m ){
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
								if( auto regestry = r.lock() )
									regestry->erase( wsh->connection_id() );
							}
						} );

				regestry->emplace( wsh->connection_id(), wsh );

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
		restinio::run(
			1,
			restinio::server_settings_t< traits_t >{}
				.address( "localhost" )
				.request_handler( server_handler() )
				.read_next_http_message_timelimit( 10s )
				.write_http_response_timelimit( 1s )
				.handle_request_timeout( 1s ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
