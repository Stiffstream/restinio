/*
	restinio
*/

/*!
	Socket options tls.
*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <restinio/all.hpp>
#include <restinio/tls.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

TEST_CASE( "Socket_options TLS" , "[socket][options][tls]" )
{
	bool socket_options_setter_was_called = false;
	using http_server_t =
		restinio::http_server_t<
			restinio::tls_traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
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
						restinio::asio_ns::ip::tcp::no_delay no_delay{ true };
						options.set_option( no_delay );
						socket_options_setter_was_called = true;
					} );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	// TODO: when tls client will be available use it instead of the following code:
	do_with_socket(
		[]( auto & socket, auto &  ){
			// Ensure we get connected:
			std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );

			socket.shutdown( restinio::asio_ns::ip::tcp::socket::shutdown_both );
			socket.close(); // Close without doing anything.

			//! Ensure we closed.
			std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
		} );

	REQUIRE( socket_options_setter_was_called );

	other_thread.stop_and_join();
}

TEST_CASE( "Usage of asio_ssl_stream() method", "[socket][tls][asio_ssl_stream]" )
{
	restinio::asio_ns::io_context ctx;
	auto ssl_context = std::make_shared< restinio::asio_ns::ssl::context >(
			restinio::asio_ns::ssl::context::sslv23 );

	restinio::tls_socket_t socket{ ctx, ssl_context };

	REQUIRE( nullptr != socket.asio_ssl_stream().native_handle() );
}

