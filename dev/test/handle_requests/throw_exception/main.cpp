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

#if defined(__GNUG__)
#pragma GCC diagnostic ignored "-Wparentheses"
#endif

const std::string RESP_BODY{ "-=UNIT-TEST=-" };

TEST_CASE( "Throw exception" , "[exception]" )
{
	using http_server_t = restinio::http_server_t<>;

	http_server_t http_server{
		restinio::create_child_io_service( 1 ),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.read_next_http_message_timelimit( std::chrono::milliseconds( 5 ) )
				.request_handler( []( auto /*req*/, auto /*conn*/ ){
					throw std::runtime_error( "unit test exception" );
					return restinio::request_accepted();
				} );
		}
	};

	http_server.open();

	do_with_socket( [ & ]( auto & socket, auto & /*io_service*/ ){

		const std::string request{
			"GET / HTTP/1.1\r\n"
			"Host: 127.0.0.1\r\n"
			"User-Agent: unit-test\r\n"
			"Accept: */*\r\n"
			"Connection: close\r\n"
			"\r\n" };

		REQUIRE_NOTHROW(
			asio::write( socket, asio::buffer( request ) )
			);

		std::array< char, 64 > data;
		asio::error_code error;

		size_t length = // sock.read_some(asio::buffer(data), error);
			asio::read( socket, asio::buffer(data), error );

		REQUIRE( 0 == length );
		REQUIRE( error == asio::error::eof );
		} );

	http_server.close();
}
