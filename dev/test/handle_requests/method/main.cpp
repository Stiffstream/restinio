/*
	restinio
*/

/*!
	Test method detection.
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

TEST_CASE( "HTTP method" , "[method]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[]( auto req ){
						req->create_response()
							.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" )
							.set_body(
								restinio::const_buffer( req->header().method().c_str() ) )
							.done();

						return restinio::request_accepted();
					} );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	SECTION( "GET" )
	{
		std::string response;
		const char * request_str =
			"GET / HTTP/1.1\r\n"
			"Host: 127.0.0.1\r\n"
			"User-Agent: unit-test\r\n"
			"Accept: */*\r\n"
			"Connection: close\r\n"
			"\r\n";

		REQUIRE_NOTHROW( response = do_request( request_str ) );

		REQUIRE_THAT( response, Catch::Matchers::EndsWith( "GET" ) );
	}

	SECTION( "POST" )
	{
		std::string response;
		const char * request_str =
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Content-Length: 16\r\n"
			"Connection: close\r\n"
			"\r\n"
			"DATADATADATADATA";

		REQUIRE_NOTHROW( response = do_request( request_str ) );

		REQUIRE_THAT( response, Catch::Matchers::EndsWith( "POST" ) );
	}

	SECTION( "HEAD" )
	{
		std::string response;
		const char * request_str =
			"HEAD / HTTP/1.1\r\n"
			"Host: 127.0.0.1\r\n"
			"User-Agent: unit-test\r\n"
			"Accept: */*\r\n"
			"Connection: close\r\n"
			"\r\n";

		REQUIRE_NOTHROW( response = do_request( request_str ) );

		REQUIRE_THAT( response, Catch::Matchers::EndsWith( "HEAD" ) );
	}

	SECTION( "PUT" )
	{
		std::string response;
		const char * request_str =
			"PUT /data HTTP/1.1\r\n"
			"Host: 127.0.0.1\r\n"
			"User-Agent: unit-test\r\n"
			"Accept: */*\r\n"
			"Connection: close\r\n"
			"Content-Length: 32\r\n"
			"\r\n"
			"DATADATADATADATA"
			"DATADATADATADATA";

		REQUIRE_NOTHROW( response = do_request( request_str ) );

		REQUIRE_THAT( response, Catch::Matchers::EndsWith( "PUT" ) );
	}

	SECTION( "DELETE" )
	{
		std::string response;
		const char * request_str =
			"DELETE /data HTTP/1.1\r\n"
			"Host: 127.0.0.1\r\n"
			"User-Agent: unit-test\r\n"
			"Accept: */*\r\n"
			"Connection: close\r\n"
			"\r\n";

		REQUIRE_NOTHROW( response = do_request( request_str ) );

		REQUIRE_THAT( response, Catch::Matchers::EndsWith( "DELETE" ) );
	}

	other_thread.stop_and_join();
}
