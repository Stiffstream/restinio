/*
	restinio
*/

/*!
	Test throwing exception in handler.
*/

#include <catch2/catch_all.hpp>

#include <restinio/core.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

#if defined(__GNUG__)
#pragma GCC diagnostic ignored "-Wparentheses"
#endif

using namespace restinio::tests;

const std::string RESP_BODY{ "-=UNIT-TEST=-" };

TEST_CASE( "Throw exception" , "[exception]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	random_port_getter_t port_getter;

	http_server_t http_server{
		restinio::own_io_context(),
		[&port_getter]( auto & settings ){
			settings
				.port( 0 )
				.address( default_ip_addr() )
				.acceptor_post_bind_hook( port_getter.as_post_bind_hook() )
				.read_next_http_message_timelimit( std::chrono::milliseconds( 5 ) )
				.request_handler( []( auto /*req*/ ){
					throw std::runtime_error( "unit test exception" );
					return restinio::request_accepted();
				} );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	do_with_socket( [ & ]( auto & socket, auto & /*io_context*/ )
		{
			const std::string request{
				"GET / HTTP/1.1\r\n"
				"Host: 127.0.0.1\r\n"
				"User-Agent: unit-test\r\n"
				"Accept: */*\r\n"
				"Connection: close\r\n"
				"\r\n" };

			REQUIRE_NOTHROW(
				restinio::asio_ns::write(
						socket,
						restinio::asio_ns::buffer( request ) )
				);

			std::array< char, 64 > data{};
			restinio::asio_ns::error_code error;

			size_t length = restinio::asio_ns::read(
						socket,
						restinio::asio_ns::buffer(data), error );

			REQUIRE( 0 == length );
			REQUIRE( error == restinio::asio_ns::error::eof );
		},
		default_ip_addr(),
		port_getter.port() );

	other_thread.stop_and_join();
}

