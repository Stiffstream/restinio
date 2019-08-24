/*
	restinio
*/

/*!
	Test slow client.
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

#if defined(__GNUG__)
#pragma GCC diagnostic ignored "-Wparentheses"
#endif

const std::string RESP_BODY{ "-=UNIT-TEST=-" };

TEST_CASE( "Slow transmit" , "[slow_trunsmit]" )
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
				.read_next_http_message_timelimit( std::chrono::seconds( 5 ) )
				.request_handler( []( auto req ){
					if( restinio::http_method_get() == req->header().method() )
					{
						req->create_response()
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

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	do_with_socket( [ & ]( auto & socket, auto & io_context ){

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
				restinio::asio_ns::write( socket, restinio::asio_ns::buffer( &c, 1 ) )
				);
			std::this_thread::sleep_for( std::chrono::milliseconds( 2 ) );
		}

		std::array< char, 1024 > data{};

		socket.async_read_some(
			restinio::asio_ns::buffer( data.data(), data.size() ),
			[ & ]( auto ec, std::size_t length ){

				REQUIRE( 0 != length );
				REQUIRE_FALSE( ec );

				const std::string response{ data.data(), length };

				REQUIRE_THAT( response, Catch::Matchers::EndsWith( RESP_BODY ) );
			} );

		io_context.run();
	} );

	other_thread.stop_and_join();
}
