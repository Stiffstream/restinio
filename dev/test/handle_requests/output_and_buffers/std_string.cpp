#include <catch2/catch.hpp>

#include <restinio/all.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

TEST_CASE(
	"RC & std::string & single set & single buf" ,
	"[restinio_controlled_output][std::string][single_set][single_buf]" )
{
	const std::string resp_message = "RC & std::string & single set & single buf";

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
								.set_body( std::string{ resp_message } )
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
			fmt::format( "Content-Length: {}", resp_message.size() ) ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( resp_message ) );

	other_thread.stop_and_join();
}

TEST_CASE(
	"RC & std::string & multi set & single buf" ,
	"[restinio_controlled_output][std::string][multi_set][single_buf]" )
{
	const std::string resp_message = "RC & std::string & multi set & single buf";
	const std::string resp_message_fake1 = "RC & std::string & multi set & single buf------------1";
	const std::string resp_message_fake2 = "RC & std::string & multi set & single buf------------2";

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

						resp.set_body( resp_message_fake1 );
						resp.set_body( resp_message_fake2 );

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
			fmt::format( "Content-Length: {}", resp_message.size() ) ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( resp_message ) );

	other_thread.stop_and_join();
}

TEST_CASE(
	"RC & std::string & single set & multi buf" ,
	"[restinio_controlled_output][std::string][single_set][multi_buf]" )
{
	const std::string resp_message = "RC & std::string & single set & multi buf";

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
						const char * resp_msg = resp_message.data();
						auto resp = req->create_response();

						resp.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" );

						std::size_t n = 1, remaining_resp_size = resp_message.size();

						while( 0 != remaining_resp_size )
						{
							auto sz = std::min( remaining_resp_size, n++ );
							resp.append_body( std::string{ resp_msg, sz } );
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
			fmt::format( "Content-Length: {}", resp_message.size() ) ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( resp_message ) );

	other_thread.stop_and_join();
}

TEST_CASE(
	"RC & std::string & multi set & multi buf" ,
	"[restinio_controlled_output][std::string][multi_set][multi_buf]" )
{
	const std::string resp_message = "RC & std::string & multi set & multi buf";
	const std::string resp_message_fake1 = "RC & std::string & multi set & single buf------------1";
	const std::string resp_message_fake2 = "RC & std::string & multi set & single buf------------2";

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
							const char * resp_msg = resp_message_fake1.data();
							std::size_t n = 1, remaining_resp_size = resp_message_fake1.size();

							while( 0 != remaining_resp_size )
							{
								auto sz = std::min( remaining_resp_size, n++ );
								resp.append_body( std::string{ resp_msg, sz } );
								remaining_resp_size -= sz;
								resp_msg += sz;
							}
						}
						{
							resp.set_body( "" );
							const char * resp_msg = resp_message_fake2.data();
							std::size_t n = 1, remaining_resp_size = resp_message_fake2.size();

							while( 0 != remaining_resp_size )
							{
								auto sz = std::min( remaining_resp_size, n++ );
								resp.append_body( std::string{ resp_msg, sz } );
								remaining_resp_size -= sz;
								resp_msg += sz;
							}
						}

						{
							resp.set_body( "" );
							const char * resp_msg = resp_message.data();
							std::size_t n = 1, remaining_resp_size = resp_message.size();

							while( 0 != remaining_resp_size )
							{
								auto sz = std::min( remaining_resp_size, n++ );
								resp.append_body( std::string{ resp_msg, sz } );
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
			fmt::format( "Content-Length: {}", resp_message.size() ) ) );

	// Add "\r\n\r\n" to ensure that resp goes right after header.
	REQUIRE_THAT( response, Catch::Matchers::EndsWith( std::string( "\r\n\r\n" ) + resp_message ) );

	other_thread.stop_and_join();
}

TEST_CASE(
	"UC & std::string & single set & single buf" ,
	"[user_controlled_output][std::string][single_set][single_buf]" )
{
	const std::string resp_message = "UC & std::string &ingle set single buf";

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
					[ = ]( restinio::request_handle_t req ){
						using output_type_t = restinio::user_controlled_output_t;

						return
							req->create_response< output_type_t >()
								.append_header( "Server", "RESTinio utest server" )
								.append_header_date_field()
								.append_header( "Content-Type", "text/plain; charset=utf-8" )
								.set_content_length( resp_message.size() )
								.set_body( std::string{ resp_message } )
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
			fmt::format( "Content-Length: {}", resp_message.size() ) ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( resp_message ) );

	other_thread.stop_and_join();
}

TEST_CASE(
	"UC & std::string & multi set & single buf" ,
	"[user_controlled_output][std::string][multi_set][single_buf]" )
{
	const std::string resp_message = "UC & std::string & multi set & single buf";
	const std::string resp_message_fake1 = "UC & std::string & multi set & single buf------------1";
	const std::string resp_message_fake2 = "UC & std::string & multi set & single buf------------2";

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
					[ = ]( restinio::request_handle_t req ){
						using output_type_t = restinio::user_controlled_output_t;

						auto resp = req->create_response< output_type_t >();

						resp.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" )
							.set_content_length( resp_message.size() );

						resp.set_body( std::string{ resp_message_fake1 } );
						resp.set_body( std::string{ resp_message_fake2 } );

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
			fmt::format( "Content-Length: {}", resp_message.size() ) ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( resp_message ) );

	other_thread.stop_and_join();
}

TEST_CASE(
	"UC & std::string & single set & multi buf" ,
	"[user_controlled_output][std::string][single_set][multi_buf]" )
{
	const std::string resp_message = "UC & std::string & single set & multi buf";

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
					[ = ]( restinio::request_handle_t req ){
						using output_type_t = restinio::user_controlled_output_t;

						auto resp = req->create_response< output_type_t >();

						resp.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" )
							.set_content_length( resp_message.size() );

						const char * resp_msg = resp_message.data();
						std::size_t n = 1, remaining_resp_size = resp_message.size();

						while( 0 != remaining_resp_size )
						{
							auto sz = std::min( remaining_resp_size, n++ );
							resp.append_body( std::string{ resp_msg, sz } );
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
			fmt::format( "Content-Length: {}", resp_message.size() ) ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( resp_message ) );

	other_thread.stop_and_join();
}

TEST_CASE(
	"UC & std::string & multi set & multi buf" ,
	"[user_controlled_output][std::string][multi_set][multi_buf]" )
{
	const std::string resp_message = "UC & std::string & multi set & multi buf";
	const std::string resp_message_fake1 = "UC & std::string & multi set & single buf------------1";
	const std::string resp_message_fake2 = "UC & std::string & multi set & single buf------------2";

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
					[ = ]( restinio::request_handle_t req ){
						using output_type_t = restinio::user_controlled_output_t;

						auto resp = req->create_response< output_type_t >();

						resp.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" )
							.set_content_length( resp_message.size() );

						{
							resp.set_body( "" );
							const char * resp_msg = resp_message_fake1.data();
							std::size_t n = 1, remaining_resp_size = resp_message_fake1.size();

							while( 0 != remaining_resp_size )
							{
								auto sz = std::min( remaining_resp_size, n++ );
								resp.append_body( std::string{ resp_msg, sz } );
								remaining_resp_size -= sz;
								resp_msg += sz;
							}
						}
						{
							resp.set_body( "" );
							const char * resp_msg = resp_message_fake2.data();
							std::size_t n = 1, remaining_resp_size = resp_message_fake2.size();

							while( 0 != remaining_resp_size )
							{
								auto sz = std::min( remaining_resp_size, n++ );
								resp.append_body( std::string{ resp_msg, sz } );
								remaining_resp_size -= sz;
								resp_msg += sz;
							}
						}

						{
							resp.set_body( "" );
							const char * resp_msg = resp_message.data();
							std::size_t n = 1, remaining_resp_size = resp_message.size();

							while( 0 != remaining_resp_size )
							{
								auto sz = std::min( remaining_resp_size, n++ );
								resp.append_body( std::string{ resp_msg, sz } );
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
			fmt::format( "Content-Length: {}", resp_message.size() ) ) );

	// Add "\r\n\r\n" to ensure that resp goes right after header.
	REQUIRE_THAT( response, Catch::Matchers::EndsWith( std::string( "\r\n\r\n" ) + resp_message ) );

	other_thread.stop_and_join();
}

TEST_CASE(
	"Chunked & std::string",
	"[user_controlled_output][std::string]" )
{
	const std::string resp_message = "Chunked & std::string & single_/multi_ set/buf not applied";
	const std::string chunked_resp_message =
		"1\r\n"
		"C\r\n"
		"2\r\n"
		"hu\r\n"
		"3\r\n"
		"nke\r\n"
		"4\r\n"
		"d & \r\n"
		"5\r\n"
		"std::\r\n"
		"6\r\n"
		"string\r\n"
		"7\r\n"
		" & sing\r\n"
		"8\r\n"
		"le_/mult\r\n"
		"9\r\n"
		"i_ set/bu\r\n"
		"A\r\n"
		"f not appl\r\n"
		"3\r\n"
		"ied\r\n"
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
					[ = ]( restinio::request_handle_t req ){
						using output_type_t = restinio::chunked_output_t;

						auto resp = req->create_response< output_type_t >();

						resp.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" );

						const char * resp_msg = resp_message.data();
						std::size_t n = 1, remaining_resp_size = resp_message.size();

						while( 0 != remaining_resp_size )
						{
							auto sz = std::min( remaining_resp_size, n++ );
							resp.append_chunk( std::string{ resp_msg, sz } );
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

