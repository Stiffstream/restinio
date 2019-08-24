/*
	restinio
*/

/*!
	Notificators.
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

const std::string g_raw_request{
	"GET /1 HTTP/1.0\r\n"
	"From: unit-test\r\n"
	"User-Agent: unit-test\r\n"
	"Content-Type: application/x-www-form-urlencoded\r\n"
	"Connection: close\r\n"
	"\r\n" };


TEST_CASE( "notificators simple 1" , "[restinio_controlled_output]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	std::atomic< bool > notificator_was_called{ false };

	http_server_t http_server{
		restinio::own_io_context(),
		[&notificator_was_called]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[&notificator_was_called]( auto req ){
						return
							req->create_response()
								.append_header( "Server", "RESTinio utest server" )
								.append_header_date_field()
								.append_header( "Content-Type", "text/plain; charset=utf-8" )
								.set_body( "0123456789" )
								.done( [& ]( const auto & ) mutable{
									notificator_was_called = true;
								} );
					} );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread{ http_server };
	other_thread.run();

	std::string response;
	REQUIRE_NOTHROW( response = do_request( g_raw_request ) );

	other_thread.stop_and_join();

	REQUIRE( notificator_was_called );
}

TEST_CASE( "notificators simple 2" , "[user_controlled_output]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	std::atomic< bool > flush_notificator_was_called{ false };
	std::atomic< bool > done_notificator_was_called{ false };

	http_server_t http_server{
		restinio::own_io_context(),
		[&]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[&]( const restinio::request_handle_t& req ){
						using output_type_t = restinio::user_controlled_output_t;
						auto resp = req->create_response< output_type_t >();

						resp.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" )
							.set_content_length( 12 );

						resp.flush(
							[& ]( const auto & ) mutable{
								flush_notificator_was_called = true;
							} );

						return resp.set_body( "0123456789\r\n" )
							.done(
								[& ]( const auto & ) mutable{
									done_notificator_was_called = true;
								} );
					} );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread{ http_server };
	other_thread.run();

	std::string response;
	REQUIRE_NOTHROW( response = do_request( g_raw_request ) );

	other_thread.stop_and_join();

	REQUIRE( flush_notificator_was_called );
	REQUIRE( done_notificator_was_called );
}

TEST_CASE( "notificators simple 3" , "[chunked_output]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	std::atomic< bool > flush1_notificator_was_called{ false };
	std::atomic< bool > flush2_notificator_was_called{ false };
	std::atomic< bool > done_notificator_was_called{ false };

	http_server_t http_server{
		restinio::own_io_context(),
		[&]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[&]( const restinio::request_handle_t& req ){
						using output_type_t = restinio::chunked_output_t;
						auto resp = req->create_response< output_type_t >();

						resp.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" );

						resp.flush(
							[& ]( const auto & ) mutable{
								flush1_notificator_was_called = true;
							} );

						resp.append_chunk( restinio::const_buffer( "0123456" ) );
						resp.append_chunk( restinio::const_buffer( "789" ) );
						resp.append_chunk( restinio::const_buffer( "ABCDEF" ) );

						resp.flush(
							[& ]( const auto & ) mutable{
								flush2_notificator_was_called = true;
							} );

						return resp.done(
							[& ]( const auto & ) mutable{
								done_notificator_was_called = true;
							} );
					} );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread{ http_server };
	other_thread.run();

	std::string response;
	REQUIRE_NOTHROW( response = do_request( g_raw_request ) );

	other_thread.stop_and_join();

	REQUIRE( flush1_notificator_was_called );
	REQUIRE( flush2_notificator_was_called );
	REQUIRE( done_notificator_was_called );
}


TEST_CASE( "notificators user_controlled_output flush-flush" , "[user_controlled_output][flush]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	std::atomic< int > counter{0};
	std::atomic< int > flush1_notificator_was_called{ -1 };
	std::atomic< int > flush2_notificator_was_called{ -1 };
	std::atomic< int > done_notificator_was_called{ -1 };

	http_server_t http_server{
		restinio::own_io_context(),
		[&]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[&]( const restinio::request_handle_t& req ){
						using output_type_t = restinio::user_controlled_output_t;
						auto resp = req->create_response< output_type_t >();

						resp.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" )
							.set_content_length( 12 );

						resp.flush(
							[& ]( const auto & ) mutable{
								flush1_notificator_was_called = ++counter;
							} );

						resp.flush(
							[& ]( const auto & ) mutable{
								flush2_notificator_was_called = ++counter;
							} );

						return resp.set_body( "0123456789\r\n" )
							.done(
								[& ]( const auto & ) mutable{
									done_notificator_was_called = ++counter;
								} );
					} );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread{ http_server };
	other_thread.run();

	std::string response;
	REQUIRE_NOTHROW( response = do_request( g_raw_request ) );

	other_thread.stop_and_join();

	REQUIRE( flush1_notificator_was_called == 1 );
	REQUIRE( flush2_notificator_was_called == 2 );
	REQUIRE( done_notificator_was_called == 3 );
}

TEST_CASE( "notificators chunked_output flush-flush" , "[chunked_output][flush]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	std::atomic< int > counter{0};
	std::atomic< int > flush1_notificator_was_called{ -1 };
	std::atomic< int > flush2_notificator_was_called{ -1 };
	std::atomic< int > flush3_notificator_was_called{ -1 };
	std::atomic< int > done_notificator_was_called{ -1 };

	http_server_t http_server{
		restinio::own_io_context(),
		[&]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[&]( const restinio::request_handle_t& req ){
						using output_type_t = restinio::chunked_output_t;
						auto resp = req->create_response< output_type_t >();

						resp.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" );

						resp.flush(
							[& ]( const auto & ) mutable{
								flush1_notificator_was_called = ++counter;
							} );

						resp.append_chunk( restinio::const_buffer( "0123456" ) );
						resp.append_chunk( restinio::const_buffer( "789" ) );
						resp.append_chunk( restinio::const_buffer( "ABCDEF" ) );

						resp.flush(
							[& ]( const auto & ) mutable{
								flush2_notificator_was_called = ++counter;
							} );

						resp.flush(
							[& ]( const auto & ) mutable{
								flush3_notificator_was_called = ++counter;
							} );

						return resp.done(
							[& ]( const auto & ) mutable{
								done_notificator_was_called = ++counter;
							} );
					} );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread{ http_server };
	other_thread.run();

	std::string response;
	REQUIRE_NOTHROW( response = do_request( g_raw_request ) );

	other_thread.stop_and_join();

	REQUIRE( flush1_notificator_was_called == 1 );
	REQUIRE( flush2_notificator_was_called == 2 );
	REQUIRE( flush3_notificator_was_called == 3 );
	REQUIRE( done_notificator_was_called );
}

TEST_CASE( "notificators error" , "[error]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	std::promise< restinio::asio_ns::error_code > ec_promise;

	http_server_t http_server{
		restinio::own_io_context(),
		[&]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[&]( const restinio::request_handle_t& req ){
						auto resp = req->create_response();

						resp
							.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" )
							.set_body( "0123456789" );

						for( auto i = 0; i< 64*10 +1; ++i )
						{
							resp.append_body( "****" );
						}

						return resp.done( [& ]( const auto & ec ) mutable{
									ec_promise.set_value( ec );
								} );
					} );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread{ http_server };
	other_thread.run();

	REQUIRE_NOTHROW(
		do_with_socket(
			[]( auto & socket, auto & /*io_context*/ ){
				restinio::asio_ns::streambuf b;
				std::ostream req_stream(&b);
				req_stream << g_raw_request;
				restinio::asio_ns::write( socket, b );
				// Send req and close without reading.
			} )
	);

	auto ec = ec_promise.get_future();
	ec.wait();
	REQUIRE( ec.get() );

	other_thread.stop_and_join();
}

TEST_CASE( "notificators on not written data" , "[error]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	std::atomic< bool > notificator_was_called{ false };
	std::atomic< bool > was_error{ false };

	// Control response generation order.
	std::promise<void> resp_order_barrier;
	auto resp_order_barrier_future = resp_order_barrier.get_future();

	http_server_t http_server{
		restinio::own_io_context(),
		[&]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.max_pipelined_requests( 2 )
				.request_handler(
					[&]( restinio::request_handle_t req ){

						std::thread t{
							[&, req = std::move( req )]() mutable {

								auto resp = req->create_response();
								resp
									.append_header( "Server", "RESTinio utest server" )
									.append_header_date_field()
									.append_header( "Content-Type", "text/plain; charset=utf-8" )
									.set_body( "0123456789" );

								if( req->header().request_target() == "/1")
								{
									resp_order_barrier_future.wait();
									resp.connection_close().done();
								}
								else
								{
									resp.done(
										[&]( const auto & ec ) mutable{
											notificator_was_called = true;
											if( ec ) was_error = true;
										} );
									resp_order_barrier.set_value();
								}
							} };

						t.detach();

						return restinio::request_accepted();
					} );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread{ http_server };
	other_thread.run();

	const std::string request{ g_raw_request +
		"GET /2 HTTP/1.0\r\n"
		"From: unit-test\r\n"
		"User-Agent: unit-test\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Connection: close\r\n"
		"\r\n" };

	std::string response;
	REQUIRE_NOTHROW( response = do_request( request ) );

	other_thread.stop_and_join();

	REQUIRE( notificator_was_called );
	REQUIRE( was_error );
}

TEST_CASE( "notificators on already closed connection" , "[error]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	std::atomic< bool > notificator_was_called{ false };
	std::atomic< bool > was_error{ false };

	// Control response generation order.
	std::promise< void > close_notificator_barrier;
	auto close_notificator_future = close_notificator_barrier.get_future();

	std::promise< void > done_notificator_barrier;
	auto done_notificator_future = done_notificator_barrier.get_future();

	http_server_t http_server{
		restinio::own_io_context(),
		[&]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.max_pipelined_requests( 2 )
				.request_handler(
					[&]( restinio::request_handle_t req ){

						std::thread t{
							[&, req = std::move( req )]() mutable {

								auto resp = req->create_response();
								resp
									.append_header( "Server", "RESTinio utest server" )
									.append_header_date_field()
									.append_header( "Content-Type", "text/plain; charset=utf-8" )
									.set_body( "0123456789" );

								if( req->header().request_target() == "/1")
								{
									resp.connection_close().done(
										[&]( const auto & ){
											close_notificator_barrier.set_value();
											done_notificator_future.wait();
										} );
								}
								else
								{
									close_notificator_future.wait();
									resp.done(
										[&]( const auto & ec ) mutable{
											notificator_was_called = true;
											if( ec ) was_error = true;
										} );
									done_notificator_barrier.set_value();
								}
							} };

						t.detach();

						return restinio::request_accepted();
					} );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread{ http_server };
	other_thread.run();

	const std::string request{ g_raw_request +
		"GET /2 HTTP/1.0\r\n"
		"From: unit-test\r\n"
		"User-Agent: unit-test\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Connection: close\r\n"
		"\r\n" };

	std::string response;
	REQUIRE_NOTHROW( response = do_request( request ) );

	other_thread.stop_and_join();

	REQUIRE( notificator_was_called );
	REQUIRE( was_error );
}


TEST_CASE( "notificators throw 1" , "[error][throw]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	std::promise< restinio::asio_ns::error_code > ec1;
	std::promise< restinio::asio_ns::error_code > ec2;
	std::promise< restinio::asio_ns::error_code > ec3;

	http_server_t http_server{
		restinio::own_io_context(),
		[&]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.max_pipelined_requests( 2 )
				.request_handler(
					[&]( const restinio::request_handle_t& req ){
						auto resp = req->create_response();

						resp
							.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" )
							.set_body( "0123456789" );

						for( auto i = 0; i< 64*10 +1; ++i )
						{
							resp.append_body( "****" );
						}

						std::promise< restinio::asio_ns::error_code > * ec_promise_ptr{ nullptr };
						if( req->header().request_target() == "/X" )
						{
							ec_promise_ptr = &ec1;
						}
						else if( req->header().request_target() == "/Y" )
						{
							ec_promise_ptr = &ec2;
						}
						else if( req->header().request_target() == "/Z" )
						{
							ec_promise_ptr = &ec3;
						}

						return resp.done( [&, ec_promise_ptr]( const auto & ec ){
									if( ec_promise_ptr )
									{
										ec_promise_ptr->set_value( ec );
									}

									throw std::runtime_error{ "notificator error" };
								} );
					} );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread{ http_server };
	other_thread.run();

	std::string response;
	REQUIRE_NOTHROW( response = do_request(
		"GET /X HTTP/1.0\r\n"
		"From: unit-test\r\n"
		"User-Agent: unit-test\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Connection: keep-alive\r\n"
		"\r\n" ) );

	{
		auto ec = ec1.get_future();
		ec.wait();
		REQUIRE( ec.get() == restinio::asio_ns::error_code{} );
	}

	REQUIRE_NOTHROW(
		do_with_socket(
			[]( auto & socket, auto & /*io_context*/ ){
				restinio::asio_ns::streambuf b;
				std::ostream req_stream(&b);
				req_stream <<
				"GET /Y HTTP/1.0\r\n"
				"From: unit-test\r\n"
				"User-Agent: unit-test\r\n"
				"Content-Type: application/x-www-form-urlencoded\r\n"
				"Connection: keep-alive\r\n"
				"\r\n";
				restinio::asio_ns::write( socket, b );
				// Send req and close without reading.
			} )
	);

	{
		auto ec = ec2.get_future().get();
		REQUIRE( ec );
		REQUIRE( ec.category() == restinio::asio_ec::system_category() );
	}

	REQUIRE_NOTHROW( response = do_request(
		"GET /z HTTP/1.0\r\n"
		"From: unit-test\r\n"
		"User-Agent: unit-test\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Connection: keep-alive\r\n"
		"\r\n"
		"GET /Z HTTP/1.0\r\n"
		"From: unit-test\r\n"
		"User-Agent: unit-test\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Connection: keep-alive\r\n"
		"\r\n" ) );

	{
		auto ec = ec3.get_future().get();
		REQUIRE( ec );
		REQUIRE( ec.category() == restinio::restinio_err_category() );
	}

	other_thread.stop_and_join();
}
