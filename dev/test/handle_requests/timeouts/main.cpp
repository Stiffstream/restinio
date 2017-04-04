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

const std::string RESP_BODY{ "-=UNIT-TEST=-" };

TEST_CASE( "Timeout on reading requests" , "[timeout][read]" )
{
	using http_server_t = restinio::http_server_t<>;

	http_server_t http_server{
		restinio::create_child_io_service( 1 ),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.read_next_http_message_timelimit( std::chrono::milliseconds( 5 ) )
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

	SECTION( "write nothing" )
	{
		do_with_socket( [ & ]( auto & socket, auto & io_service ){
			std::this_thread::sleep_for( std::chrono::milliseconds( 6 ) );

			std::array< char, 64 > data;

			asio::error_code error;

			size_t length = // sock.read_some(asio::buffer(data), error);
				asio::read( socket, asio::buffer(data), error );

			REQUIRE( 0 == length );
			REQUIRE( error == asio::error::eof );
		} );
	}

	SECTION( "write a little" )
	{
		do_with_socket( [ & ]( auto & socket, auto & io_service ){

			const std::string a_part_of_request{ "GET / HTT" };

			REQUIRE_NOTHROW(
				asio::write( socket, asio::buffer( a_part_of_request ) )
				);

			std::this_thread::sleep_for( std::chrono::milliseconds( 6 ) );

			std::array< char, 64 > data;
			asio::error_code error;

			size_t length = // sock.read_some(asio::buffer(data), error);
				asio::read( socket, asio::buffer(data), error );

			REQUIRE( 0 == length );
			REQUIRE( error == asio::error::eof );
		} );
	}

	SECTION( "write almost all" )
	{
		do_with_socket( [ & ]( auto & socket, auto & io_service ){

			const std::string a_part_of_request{
				"GET / HTTP/1.1\r\n"
				"Host: 127.0.0.1\r\n"
				"User-Agent: unit-test\r\n"
				"Accept: */*\r\n"
				"Connection: close\r\n"
				"\r" }; // '\n' is missing

			REQUIRE_NOTHROW(
				asio::write( socket, asio::buffer( a_part_of_request ) )
				);

			std::this_thread::sleep_for( std::chrono::milliseconds( 6 ) );

			std::array< char, 64 > data;
			asio::error_code error;

			size_t length = // sock.read_some(asio::buffer(data), error);
				asio::read( socket, asio::buffer(data), error );

			REQUIRE( 0 == length );
			REQUIRE( error == asio::error::eof );
		} );
	}

	http_server.close();
}

TEST_CASE( "Timeout on handling request" , "[timeout][handle_request]" )
{
	using http_server_t = restinio::http_server_t<>;

	restinio::connection_handle_t conn_to_store;

	http_server_t http_server{
		restinio::create_child_io_service( 1 ),
		[ &conn_to_store ]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.handle_request_timeout( std::chrono::milliseconds( 5 ) )
				.request_handler( [ &conn_to_store ]( auto req, auto conn ){

					// Store connection.
					conn_to_store = std::move( conn );

					// Signal that request is going to be handled.
					return restinio::request_accepted();
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

		REQUIRE_NOTHROW(
			asio::write( socket, asio::buffer( request ) )
			);

		std::array< char, 1024 > data;

		socket.async_read_some(
			asio::buffer( data.data(), data.size() ),
			[ & ]( auto ec, std::size_t length ){

				REQUIRE( 0 != length );
				REQUIRE_FALSE( ec );

				const std::string response{ data.data(), length };

				REQUIRE_THAT( response, Catch::Matchers::StartsWith( "HTTP/1.1 504" ) );
			} );

		io_service.run();

	} );

	conn_to_store.reset();

	http_server.close();
}
