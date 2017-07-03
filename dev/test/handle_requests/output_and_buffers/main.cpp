/*
	restinio
*/

/*!
	Test method detection.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <asio.hpp>

#include <restinio/all.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

TEST_CASE(
	"RC & char* & single set & single buf" ,
	"[restinio_controlled_output][const_buffer][single_set][single_buf]" )
{
	const char * resp_message = "RC & char* single set single buf";

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_factory_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::create_child_io_service( 1 ),
		[ = ]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[ = ]( auto req ){
						return
							req->create_response()
								.append_header( "Server", "RESTinio utest server" )
								.append_header_date_field()
								.append_header( "Content-Type", "text/plain; charset=utf-8" )
								.set_body( restinio::const_buffer( resp_message ) )
								.done();
					} );
		} };

	http_server.open();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( resp_message ) );

	http_server.close();
}

TEST_CASE(
	"RC & char* & multi set & single buf" ,
	"[restinio_controlled_output][const_buffer][multi_set][single_buf]" )
{
	const char * resp_message = "RC & char* & multi set & single buf";
	const char * resp_message_fake1 = "RC & char* & multi set & single buf------------1";
	const char * resp_message_fake2 = "RC & char* & multi set & single buf------------2";

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_factory_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::create_child_io_service( 1 ),
		[ = ]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[ = ]( auto req ){
						auto resp = req->create_response();

						resp.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" );

						resp.set_body( restinio::const_buffer( resp_message_fake1 ) );
						resp.set_body( restinio::const_buffer( resp_message_fake2 ) );

						return resp.set_body( restinio::const_buffer( resp_message ) ).done();
					} );
		} };

	http_server.open();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( resp_message ) );

	http_server.close();
}

TEST_CASE(
	"RC & char* & single set & multi buf" ,
	"[restinio_controlled_output][const_buffer][single_set][multi_buf]" )
{
	const char * resp_message = "RC & char* & single set & multi buf";

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_factory_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::create_child_io_service( 1 ),
		[ = ]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[ = ]( auto req ){
						const char * resp_msg = resp_message;
						auto resp = req->create_response();

						resp.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" );

						std::size_t n = 1, remaining_resp_size = std::strlen( resp_msg );

						while( 0 != remaining_resp_size )
						{
							auto sz = std::min( remaining_resp_size, n++ );
							resp.append_body( restinio::const_buffer( resp_msg, sz ) );
							remaining_resp_size -= sz;
							resp_msg += sz;
						}

						return resp.done();
					} );
		} };

	http_server.open();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( resp_message ) );

	http_server.close();
}

TEST_CASE( "FAKE" , "[fake]" )
{
	REQUIRE( false );
}
