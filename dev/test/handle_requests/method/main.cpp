/*
	restinio
*/

/*!
	Tests for settings parameters that have default constructor.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <asio.hpp>

#include <restinio/all.hpp>

#include "../common/pub.hpp"

TEST_CASE( "HTTP method" , "[method]" )
{
	using http_server_t = restinio::http_server_t<>;

	http_server_t http_server{
		restinio::create_child_io_service( 1 ),
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
							.set_body( method_to_string( req->header().method() ) )
							.done();

						return restinio::request_accepted();
					} );
		} };

	http_server.open();

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

	http_server.close();
}
