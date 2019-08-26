#include <catch2/catch.hpp>

#include <restinio/all.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>


TEST_CASE(
	"RC & char* & single set & single buf" ,
	"[restinio_controlled_output][const_buffer][single_set][single_buf]" )
{
	const char * resp_message = "RC & char* & single set & single buf";

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
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

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response,
		Catch::Matchers::Contains(
			fmt::format( "Content-Length: {}", std::strlen( resp_message ) ) ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( resp_message ) );

	other_thread.stop_and_join();
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
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
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

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response,
		Catch::Matchers::Contains(
			fmt::format( "Content-Length: {}", std::strlen( resp_message ) ) ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( resp_message ) );

	other_thread.stop_and_join();
}

TEST_CASE(
	"RC & char* & single set & multi buf" ,
	"[restinio_controlled_output][const_buffer][single_set][multi_buf]" )
{
	const char * resp_message = "RC & char* & single set & multi buf";

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
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

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response,
		Catch::Matchers::Contains(
			fmt::format( "Content-Length: {}", std::strlen( resp_message ) ) ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( resp_message ) );

	other_thread.stop_and_join();
}

TEST_CASE(
	"RC & char* & multi set & multi buf" ,
	"[restinio_controlled_output][const_buffer][multi_set][multi_buf]" )
{
	const char * resp_message = "RC & char* & multi set & multi buf";
	const char * resp_message_fake1 = "RC & char* & multi set & single buf------------1";
	const char * resp_message_fake2 = "RC & char* & multi set & single buf------------2";

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
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

						{
							resp.set_body( "" );
							const char * resp_msg = resp_message_fake1;
							std::size_t n = 1, remaining_resp_size = std::strlen( resp_message_fake1 );

							while( 0 != remaining_resp_size )
							{
								auto sz = std::min( remaining_resp_size, n++ );
								resp.append_body( restinio::const_buffer( resp_msg, sz ) );
								remaining_resp_size -= sz;
								resp_msg += sz;
							}
						}
						{
							resp.set_body( "" );
							const char * resp_msg = resp_message_fake2;
							std::size_t n = 1, remaining_resp_size = std::strlen( resp_message_fake2 );

							while( 0 != remaining_resp_size )
							{
								auto sz = std::min( remaining_resp_size, n++ );
								resp.append_body( restinio::const_buffer( resp_msg, sz ) );
								remaining_resp_size -= sz;
								resp_msg += sz;
							}
						}

						{
							resp.set_body( "" );
							const char * resp_msg = resp_message;
							std::size_t n = 1, remaining_resp_size = std::strlen( resp_msg );

							while( 0 != remaining_resp_size )
							{
								auto sz = std::min( remaining_resp_size, n++ );
								resp.append_body( restinio::const_buffer( resp_msg, sz ) );
								remaining_resp_size -= sz;
								resp_msg += sz;
							}
						}

						return resp.done();
					} );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response,
		Catch::Matchers::Contains(
			fmt::format( "Content-Length: {}", std::strlen( resp_message ) ) ) );

	// Add "\r\n\r\n" to ensure that resp goes right after header.
	REQUIRE_THAT( response, Catch::Matchers::EndsWith( std::string( "\r\n\r\n" ) + resp_message ) );

	other_thread.stop_and_join();
}

TEST_CASE(
	"UC & char* & single set & single buf" ,
	"[user_controlled_output][const_buffer][single_set][single_buf]" )
{
	const char * resp_message = "UC & char* single set single buf";

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[ = ]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[ = ]( const restinio::request_handle_t& req ){
						using output_type_t = restinio::user_controlled_output_t;

						return
							req->create_response< output_type_t >()
								.append_header( "Server", "RESTinio utest server" )
								.append_header_date_field()
								.append_header( "Content-Type", "text/plain; charset=utf-8" )
								.set_content_length( std::strlen( resp_message ) )
								.set_body( restinio::const_buffer( resp_message ) )
								.done();
					} );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response,
		Catch::Matchers::Contains(
			fmt::format( "Content-Length: {}", std::strlen( resp_message ) ) ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( resp_message ) );

	other_thread.stop_and_join();
}

TEST_CASE(
	"UC & char* & multi set & single buf" ,
	"[user_controlled_output][const_buffer][multi_set][single_buf]" )
{
	const char * resp_message = "UC & char* & multi set & single buf";
	const char * resp_message_fake1 = "UC & char* & multi set & single buf------------1";
	const char * resp_message_fake2 = "UC & char* & multi set & single buf------------2";

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[ = ]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[ = ]( const restinio::request_handle_t& req ){
						using output_type_t = restinio::user_controlled_output_t;

						auto resp = req->create_response< output_type_t >();

						resp.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" )
							.set_content_length( std::strlen( resp_message ) );

						resp.set_body( restinio::const_buffer( resp_message_fake1 ) );
						resp.set_body( restinio::const_buffer( resp_message_fake2 ) );

						return resp.set_body( std::string{ resp_message } ).done();
					} );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response,
		Catch::Matchers::Contains(
			fmt::format( "Content-Length: {}", std::strlen( resp_message ) ) ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( resp_message ) );

	other_thread.stop_and_join();
}

TEST_CASE(
	"UC & char* & single set & multi buf" ,
	"[user_controlled_output][const_buffer][single_set][multi_buf]" )
{
	const char * resp_message = "UC & char* & single set & multi buf";

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[ = ]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[ = ]( const restinio::request_handle_t& req ){
						using output_type_t = restinio::user_controlled_output_t;

						auto resp = req->create_response< output_type_t >();

						resp.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" )
							.set_content_length( std::strlen( resp_message ) );

						const char * resp_msg = resp_message;
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

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response,
		Catch::Matchers::Contains(
			fmt::format( "Content-Length: {}", std::strlen( resp_message ) ) ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( resp_message ) );

	other_thread.stop_and_join();
}

TEST_CASE(
	"UC & char* & multi set & multi buf" ,
	"[user_controlled_output][const_buffer][multi_set][multi_buf]" )
{
	const char * resp_message = "UC & char* & multi set & multi buf";
	const char * resp_message_fake1 = "UC & char* & multi set & single buf------------1";
	const char * resp_message_fake2 = "UC & char* & multi set & single buf------------2";

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[ = ]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[ = ]( const restinio::request_handle_t& req ){
						using output_type_t = restinio::user_controlled_output_t;

						auto resp = req->create_response< output_type_t >();

						resp.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" )
							.set_content_length( std::strlen( resp_message ) );

						{
							resp.set_body( "" );
							const char * resp_msg = resp_message_fake1;
							std::size_t n = 1, remaining_resp_size = std::strlen( resp_message_fake1 );

							while( 0 != remaining_resp_size )
							{
								auto sz = std::min( remaining_resp_size, n++ );
								resp.append_body( restinio::const_buffer( resp_msg, sz ) );
								remaining_resp_size -= sz;
								resp_msg += sz;
							}
						}
						{
							resp.set_body( "" );
							const char * resp_msg = resp_message_fake2;
							std::size_t n = 1, remaining_resp_size = std::strlen( resp_message_fake2 );

							while( 0 != remaining_resp_size )
							{
								auto sz = std::min( remaining_resp_size, n++ );
								resp.append_body( restinio::const_buffer( resp_msg, sz ) );
								remaining_resp_size -= sz;
								resp_msg += sz;
							}
						}

						{
							resp.set_body( "" );
							const char * resp_msg = resp_message;
							std::size_t n = 1, remaining_resp_size = std::strlen( resp_msg );

							while( 0 != remaining_resp_size )
							{
								auto sz = std::min( remaining_resp_size, n++ );
								resp.append_body( restinio::const_buffer( resp_msg, sz ) );
								remaining_resp_size -= sz;
								resp_msg += sz;
							}
						}

						return resp.done();
					} );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response,
		Catch::Matchers::Contains(
			fmt::format( "Content-Length: {}", std::strlen( resp_message ) ) ) );

	// Add "\r\n\r\n" to ensure that resp goes right after header.
	REQUIRE_THAT( response, Catch::Matchers::EndsWith( std::string( "\r\n\r\n" ) + resp_message ) );

	other_thread.stop_and_join();
}

TEST_CASE(
	"Chunked & char*",
	"[user_controlled_output][const_buffer]" )
{
	const char * resp_message = "Chunked & char* single_/multi_ set/buf not applied";
	const char * chunked_resp_message =
	"1\r\n"
	"C\r\n"
	"2\r\n"
	"hu\r\n"
	"3\r\n"
	"nke\r\n"
	"4\r\n"
	"d & \r\n"
	"5\r\n"
	"char*\r\n"
	"6\r\n"
	" singl\r\n"
	"7\r\n"
	"e_/mult\r\n"
	"8\r\n"
	"i_ set/b\r\n"
	"9\r\n"
	"uf not ap\r\n"
	"5\r\n"
	"plied\r\n"
	"0\r\n"
	"\r\n";

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[ = ]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[ = ]( const restinio::request_handle_t& req ){
						using output_type_t = restinio::chunked_output_t;

						auto resp = req->create_response< output_type_t >();

						resp.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" );

						const char * resp_msg = resp_message;
						std::size_t n = 1, remaining_resp_size = std::strlen( resp_msg );

						while( 0 != remaining_resp_size )
						{
							auto sz = std::min( remaining_resp_size, n++ );
							resp.append_chunk( restinio::const_buffer( resp_msg, sz ) );
							remaining_resp_size -= sz;
							resp_msg += sz;
						}

						return resp.done();
					} );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response, !Catch::Matchers::Contains( "Content-Length" ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( chunked_resp_message ) );

	other_thread.stop_and_join();
}

