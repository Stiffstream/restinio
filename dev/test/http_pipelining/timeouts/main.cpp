/*
	restinio
*/

/*!
	Test timeouts with http pipelining.
*/

#include <algorithm>
#include <cctype>
#include <sstream>

#include <catch2/catch.hpp>

#include <restinio/all.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

void
send_response_if_needed( restinio::request_handle_t rh )
{
	if( rh )
		rh->create_response()
			.append_header( "Server", "RESTinio utest server" )
			.append_header_date_field()
			.append_header( "Content-Type", "text/plain; charset=utf-8" )
			.set_body( rh->body() )
			.done();
}

struct req_handler_t
{
	auto
	operator () ( restinio::request_handle_t req ) const
	{
		if( m_request )
			throw std::runtime_error{ "second request must never come" };

		m_request = std::move( req );

		return restinio::request_accepted();
	}

	static restinio::request_handle_t m_request;
};

restinio::request_handle_t req_handler_t::m_request;

TEST_CASE( "HTTP piplining timout" , "[timeout]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t,
				req_handler_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.timer_manager( std::chrono::milliseconds( 50 ) )
				.read_next_http_message_timelimit( std::chrono::seconds( 5 ) )
				.handle_request_timeout( std::chrono::milliseconds( 100 ) )
				.max_pipelined_requests( 2 );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	do_with_socket( [ & ]( auto & socket, auto & io_context ){
		const std::string pipelinedrequests{
			"GET / HTTP/1.1\r\n"
			"Host: 127.0.0.1\r\n"
			"User-Agent: unit-test\r\n"
			"Accept: */*\r\n"
			"Connection: keep-alive\r\n"
			"\r\n"
			"GET / HTTP/1.1\r\n"
			"Host: 127.0.0.1\r\n"
			"User-Agent: unit-test\r\n"
			"Accept: */*\r\n"
			"Connection: keep-alive\r\n"
			"\r" }; // No \n for second request.

		const auto started_at = std::chrono::steady_clock::now();

		REQUIRE_NOTHROW(
			restinio::asio_ns::write( socket, restinio::asio_ns::buffer( pipelinedrequests ) )
			);

		std::array< char, 1024 > data;

		socket.async_read_some(
			restinio::asio_ns::buffer( data ),
			[ & ]( auto ec, std::size_t length ){

				REQUIRE( 0 == length );
				REQUIRE( ec );
				REQUIRE( ec == restinio::asio_ns::error::eof );
			} );
		io_context.run();

		const auto finished_at = std::chrono::steady_clock::now();

		const auto timeout =
			std::chrono::duration_cast< std::chrono::milliseconds >(
				finished_at - started_at );

		// Timeout is about 200 msec.
		REQUIRE( 100 <= timeout.count() );
		// NOTE: there is no upper limit because if that unit-test
		// is run under virtual machine there can be very slow execution
		// speed and async_read_some can take more that 250ms.
	} );

	other_thread.stop_and_join();

	req_handler_t::m_request.reset();
}
