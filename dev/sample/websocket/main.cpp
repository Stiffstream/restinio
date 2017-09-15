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

using traits_t =
	restinio::traits_t<
		restinio::asio_timer_factory_t,
		restinio::single_threaded_ostream_logger_t,
		router_t >;

using http_server_t = restinio::http_server_t< traits_t >;

auto server_handler( restinio::websocket::websocket_unique_ptr_t & websocket )
{
	auto router = std::make_unique< router_t >();

	router->http_get(
		"/chat",
		[&]( auto req, auto ){

			if( restinio::http_connection_header_t::upgrade == req->header().connection() )
			{
				websocket =
					restinio::websocket::upgrade_to_websocket< traits_t >(
						*req,
						[&]( restinio::websocket::ws_message_handle_t m ){
							websocket->send_message( *m );
						},
						[]( std::string reason ){
							std::cout << "Close websocket: " << reason << std::endl;
						} );
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
		restinio::websocket::websocket_unique_ptr_t websocket;

		http_server_t http_server{
			restinio::create_child_io_context( 1 ),
			[&websocket]( auto & settings ){
				settings
					.address( "localhost" )
					.request_handler( server_handler( websocket ) )
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

		websocket->close();
		http_server.close();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
