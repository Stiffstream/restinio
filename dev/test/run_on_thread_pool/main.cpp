/*
	restinio
*/

/*!
	Test method detection.
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>
#include <restinio/websocket/websocket.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

TEST_CASE( "on thread pool with break signals" , "[with_break_signal]" )
{
	std::string endpoint_value;

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[&endpoint_value]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[&endpoint_value]( auto req ){
						endpoint_value = fmt::format( "{}", req->remote_endpoint() );

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

	std::thread other_thread{ [&http_server] {
		run( restinio::on_thread_pool(
				2,
				restinio::use_break_signal_handling(),
				http_server ) );
	} };

	// Wait for HTTP server to run.
	std::promise<void> started;
	http_server.io_context().post( [&started] {
			started.set_value();
		} );
	started.get_future().get();

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

	http_server.io_context().post( [&http_server] {
			http_server.close_sync();
			http_server.io_context().stop();
		} );
	other_thread.join();

	REQUIRE( "" != endpoint_value );
}

TEST_CASE( "on thread pool without break signals" , "[without_break_signal]" )
{
	std::string endpoint_value;

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[&endpoint_value]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[&endpoint_value]( auto req ){
						endpoint_value = fmt::format( "{}", req->remote_endpoint() );

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

	std::thread other_thread{ [&http_server] {
		run( restinio::on_thread_pool(
				2,
				restinio::skip_break_signal_handling(),
				http_server ) );
	} };

	// Wait for HTTP server to run.
	std::promise<void> started;
	http_server.io_context().post( [&started] {
			started.set_value();
		} );
	started.get_future().get();

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

	http_server.io_context().post( [&http_server] {
			http_server.close_sync();
			http_server.io_context().stop();
		} );
	other_thread.join();

	REQUIRE( "" != endpoint_value );
}

