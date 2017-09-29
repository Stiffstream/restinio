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

using http_server_t = restinio::http_server_t< traits_t >;

struct ws_storage_t
{
	rws::ws_handle_t m_handle;
};

auto server_handler()
{
	auto router = std::make_unique< router_t >();

	router->http_get(
		"/chat",
		[]( auto req, auto ){

			if( restinio::http_connection_header_t::upgrade == req->header().connection() )
			{
				auto ws_storage = std::make_shared< ws_storage_t >();

				ws_storage->m_handle =
					rws::upgrade< traits_t >(
						*req,
						rws::activation_t::immediate,
						[ ws_storage ]( auto wsh, auto m ){
							wsh->send_message( *m );
						});
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
		http_server_t http_server{
			restinio::create_child_io_context( 1 ),
			[]( auto & settings ){
				settings
					.address( "localhost" )
					.request_handler( server_handler() )
					.read_next_http_message_timelimit( 10s )
					.write_http_response_timelimit( 1s )
					.handle_request_timeout( 1s );
			} };

		http_server.open();

		// Wait for quit command.
		std::cout << "Type \"quit\" or \"q\" to quit." << std::endl;

		std::string cmd;
		do
		{
			std::cin >> cmd;
		} while( cmd != "quit" && cmd != "q" );

		http_server.close();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
