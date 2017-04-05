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

TEST_CASE( "Slow transmit" , "[slow_trunsmit]" )
{
	using http_server_t = restinio::http_server_t<>;

	http_server_t http_server{
		restinio::create_child_io_service( 1 ),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.read_next_http_message_timelimit( std::chrono::seconds( 5 ) )
				.request_handler( []( auto req, auto conn ){
					if( restinio::http_method_get() == req->m_header.method() )
					{
						restinio::response_builder_t{ req->m_header, std::move( conn ) }
							.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" )
							.set_body( RESP_BODY )
							.done();

						return restinio::request_accepted();
					}

					return restinio::request_rejected();
				} );
		}
	};

	http_server.open();

	do_with_socket( [ & ]( auto & socket, auto & io_service ){

		const std::string request{
			"GET / HTTP/1.1\r\n"
			"Host: 127.0.0.1\r\n"
			"User-Agent: unit-test\r\n"
			"Accept: */*\r\n"
			"Connection: close\r\n"
			"\r\n" };

		for( const auto c : request )
		{
			REQUIRE_NOTHROW(
				asio::write( socket, asio::buffer( &c, 1 ) )
				);
			std::this_thread::sleep_for( std::chrono::milliseconds( 2 ) );
		}

		std::array< char, 1024 > data;

		socket.async_read_some(
			asio::buffer( data.data(), data.size() ),
			[ & ]( auto ec, std::size_t length ){

				REQUIRE( 0 != length );
				REQUIRE_FALSE( ec );

				const std::string response{ data.data(), length };

				REQUIRE_THAT( response, Catch::Matchers::EndsWith( RESP_BODY ) );
			} );

		io_service.run();
	} );

	http_server.close();
}
