/*
	restinio
*/

/*!
	Echo server.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <asio.hpp>

#include <restinio/all.hpp>
#include <restinio/tls.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

TEST_CASE( "Socket options" , "[socket][options]" )
{
	bool socket_options_setter_was_called = false;
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_factory_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::create_child_io_context( 1 ),
		[&socket_options_setter_was_called]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[]( auto req ){
						if( restinio::http_method_post() == req->header().method() )
						{
							req->create_response()
								.append_header( "Server", "RESTinio utest server" )
								.append_header_date_field()
								.append_header( "Content-Type", "text/plain; charset=utf-8" )
								.set_body( req->body() )
								.done();
							return restinio::request_accepted();
						}

						return restinio::request_rejected();
					} )
				.socket_options_setter(
					[&socket_options_setter_was_called]( auto options ){
						asio::ip::tcp::no_delay no_delay{ true };
						options.set_option( no_delay );
						socket_options_setter_was_called = true;
					} );
		}
	};

	http_server.open();

	std::string response;
	auto create_request = []( const std::string & body ){
		return
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Content-Length: " + std::to_string( body.size() ) + "\r\n"
			"Connection: close\r\n"
			"\r\n" +
			body;
	};

	{
		const std::string body = "01234567890123456789";
		REQUIRE_NOTHROW( response = do_request( create_request( body ) ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains(
				"Content-Length: " + std::to_string( body.size() ) ) );
		REQUIRE_THAT( response, Catch::Matchers::EndsWith( body ) );

		REQUIRE( socket_options_setter_was_called );
	}

	http_server.close();
}

TEST_CASE( "Socket_options TLS" , "[socket][options][tls]" )
{
	bool socket_options_setter_was_called = false;
	using http_server_t =
		restinio::http_server_t<
			restinio::tls_traits_t<
				restinio::asio_timer_factory_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::create_child_io_context( 1 ),
		[&socket_options_setter_was_called]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[]( auto req ){
						if( restinio::http_method_post() == req->header().method() )
						{
							req->create_response()
								.append_header( "Server", "RESTinio utest server" )
								.append_header_date_field()
								.append_header( "Content-Type", "text/plain; charset=utf-8" )
								.set_body( req->body() )
								.done();
							return restinio::request_accepted();
						}

						return restinio::request_rejected();
					} )
				.socket_options_setter(
					[&socket_options_setter_was_called]( auto options ){
						asio::ip::tcp::no_delay no_delay{ true };
						options.set_option( no_delay );
						socket_options_setter_was_called = true;
					} );
		}
	};

	http_server.open();

	// TODO: when tls client will be available use it instead of the following code:
	do_with_socket(
		[]( auto & socket, auto &  ){
			// Ensure we get connected:
			std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
			socket.close(); // Close without doing anything.
		} );

	REQUIRE( socket_options_setter_was_called );

	http_server.close();
}
