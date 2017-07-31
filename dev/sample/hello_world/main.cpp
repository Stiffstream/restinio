#include <type_traits>
#include <iostream>
#include <chrono>
#include <memory>

#include <asio.hpp>
#include <asio/ip/tcp.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <restinio/all.hpp>

template < typename RESP >
RESP
init_resp( RESP resp )
{
	resp.append_header( "Server", "RESTinio sample server /v.0.2" );
	resp.append_header_date_field();

	return resp;
};

namespace rr = restinio::router;
using router_t = rr::express_router_t;

auto server_handler()
{
	auto router = std::make_unique< router_t >();

	router->http_get(
		"/",
		[]( auto req, auto ){
				init_resp( req->create_response() )
					.append_header( "Content-Type", "text/plain; charset=utf-8" )
					.set_body( "Hello world!")
					.done();

				return restinio::request_accepted();
		} );

	router->http_get(
		"/json",
		[]( auto req, auto ){
				init_resp( req->create_response() )
					.append_header( "Content-Type", "text/json; charset=utf-8" )
					.set_body( R"-({"message" : "Hello world!"})-")
					.done();

				return restinio::request_accepted();
		} );

	router->http_get(
		"/html",
		[]( auto req, auto ){
				init_resp( req->create_response() )
						.append_header( "Content-Type", "text/html; charset=utf-8" )
						.set_body(
R"-(<html>
<head><title>Hello from RESTinio!</title></head>
<body>
<center><h1>Hello world</h1></center>
</body>
</html>)-" )
						.done();

				return restinio::request_accepted();
		} );

	return router;
}

int main()
{
	using namespace std::chrono;

	try
	{
		using http_server_t =
			restinio::http_server_t<
				restinio::traits_t<
					restinio::asio_timer_factory_t,
					restinio::single_threaded_ostream_logger_t,
					router_t > >;

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
