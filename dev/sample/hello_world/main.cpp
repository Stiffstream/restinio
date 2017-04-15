#include <type_traits>
#include <iostream>
#include <chrono>
#include <memory>

#include <asio.hpp>
#include <asio/ip/tcp.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <restinio/all.hpp>

#include <sample/common/ostream_logger.hpp>

auto server_handler()
{
	return []( auto req ) {
			auto create_common_resp =
				[&] {
					auto resp = req->create_response();
					resp.append_header( "Server", "RESTinio sample server /v.0.2" );
					resp.append_header_date_field();
					return resp;
				};

			auto result = restinio::request_rejected();

			if( restinio::http_method_get() == req->header().method() )
			{
				if( req->header().request_target() == "/" )
				{
					create_common_resp()
						.append_header( "Content-Type", "text/plain; charset=utf-8" )
						.set_body( "Hello world!")
						.done();

					result = restinio::request_accepted();
				}
				else if( req->header().request_target() == "/json" )
				{
					create_common_resp()
						.append_header( "Content-Type", "text/json; charset=utf-8" )
						.set_body( R"-({"message" : "Hello world!"})-")
						.done();

					result = restinio::request_accepted();
				}
				else if( req->header().request_target() == "/html" )
				{
					create_common_resp()
						.append_header( "Content-Type", "text/html; charset=utf-8" )
						.set_body(
R"-(<html>
<head><title>Hello from RESTinio!</title></head>
<body>
<center><h1>Hello world</h1></center>
</body>
</html>)-")
						.done();

					result = restinio::request_accepted();
				}
			}

			return result;
		};
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
					restinio::sample::single_threaded_ostream_logger_t > >;

		http_server_t http_server{
			restinio::create_child_io_service( 1 ),
			[]( auto & settings ){
				settings
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
