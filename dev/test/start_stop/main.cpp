/*
	restinio
*/

/*!
	Test method detection.
*/

#include <catch2/catch_all.hpp>

#include <restinio/core.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

TEST_CASE( "start-stop" , "[stop]" )
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
				.request_handler(
					[]( auto ){ return restinio::request_rejected(); } );
		} };

	restinio::asio_ns::post( http_server.io_context(),
		[&] {
			REQUIRE_NOTHROW( http_server.open_sync() );
			REQUIRE_NOTHROW( http_server.close_sync() );
		} );
	http_server.io_context().run();
}

TEST_CASE( "start-stop-stop" , "[stop]" )
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
				.request_handler(
					[]( auto ){ return restinio::request_rejected(); } );
		} };

	restinio::asio_ns::post( http_server.io_context(),
		[&] {
			REQUIRE_NOTHROW( http_server.open_sync() );
			REQUIRE_NOTHROW( http_server.close_sync() );
			REQUIRE_NOTHROW( http_server.close_sync() );
		} );
	http_server.io_context().run();
}

TEST_CASE( "start-start-stop" , "[stop]" )
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
				.request_handler(
					[]( auto ){ return restinio::request_rejected(); } );
		} };

	restinio::asio_ns::post( http_server.io_context(),
		[&] {
			REQUIRE_NOTHROW( http_server.open_sync() );
			REQUIRE_NOTHROW( http_server.open_sync() );
			REQUIRE_NOTHROW( http_server.close_sync() );
		} );
	http_server.io_context().run();
}

TEST_CASE( "Don't move std::function by lvalue reference in open-async",
	"[open-async-lvalue-ref-std-func]")
{
	using http_server_t = restinio::http_server_t<
			restinio::traits_t< restinio::asio_timer_manager_t, utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[]( auto ){ return restinio::request_rejected(); } );
		} };

	// A big dummy object to prevent small object optimization in std::function.
	std::array<char, 128> big_data{ 0 };

	std::function< void() > open_ok_cb = [big_data]{ std::cout << "OK"; };
	std::function< void(std::exception_ptr) > open_error_cb =
			[big_data](std::exception_ptr){ std::cout << "ERR"; };

	REQUIRE( open_ok_cb );
	REQUIRE( open_error_cb );

	http_server.open_async( open_ok_cb, open_error_cb );

	REQUIRE( open_ok_cb );
	REQUIRE( open_error_cb );

	restinio::asio_ns::post( http_server.io_context(),
		[&] {
			REQUIRE_NOTHROW( http_server.close_sync() );
		} );
	http_server.io_context().run();
}

TEST_CASE( "std::function by rvalue reference in open-async",
	"[open-async-rvalue-ref-std-func]")
{
	using http_server_t = restinio::http_server_t<
			restinio::traits_t< restinio::asio_timer_manager_t, utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[]( auto ){ return restinio::request_rejected(); } );
		} };

	// A big dummy object to prevent small object optimization in std::function.
	std::array<char, 128> big_data{ 0 };

	std::function< void() > open_ok_cb = [big_data]{ std::cout << "OK"; };
	std::function< void(std::exception_ptr) > open_error_cb =
			[big_data](std::exception_ptr){ std::cout << "ERR"; };

	REQUIRE( open_ok_cb );
	REQUIRE( open_error_cb );

	http_server.open_async( std::move(open_ok_cb), std::move(open_error_cb) );

	REQUIRE( !open_ok_cb );
	REQUIRE( !open_error_cb );

	restinio::asio_ns::post( http_server.io_context(),
		[&] {
			REQUIRE_NOTHROW( http_server.close_sync() );
		} );
	http_server.io_context().run();
}

TEST_CASE( "Don't move std::function by lvalue reference in close-async",
	"[close-async-lvalue-ref-std-func]")
{
	using http_server_t = restinio::http_server_t<
			restinio::traits_t< restinio::asio_timer_manager_t, utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[]( auto ){ return restinio::request_rejected(); } );
		} };

	// A big dummy object to prevent small object optimization in std::function.
	std::array<char, 128> big_data{ 0 };

	std::function< void() > close_ok_cb = [big_data]{ std::cout << "OK"; };
	std::function< void(std::exception_ptr) > close_error_cb =
			[big_data](std::exception_ptr){ std::cout << "ERR"; };

	REQUIRE( close_ok_cb );
	REQUIRE( close_error_cb );

	restinio::asio_ns::post( http_server.io_context(),
		[&] {
			REQUIRE_NOTHROW( http_server.open_sync() );
			REQUIRE_NOTHROW( http_server.close_async( close_ok_cb, close_error_cb ) );
		} );
	http_server.io_context().run();

	REQUIRE( close_ok_cb );
	REQUIRE( close_error_cb );

}

TEST_CASE( "std::function by rvalue reference in close-async",
	"[close-async-rvalue-ref-std-func]")
{
	using http_server_t = restinio::http_server_t<
			restinio::traits_t< restinio::asio_timer_manager_t, utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[]( auto ){ return restinio::request_rejected(); } );
		} };

	// A big dummy object to prevent small object optimization in std::function.
	std::array<char, 128> big_data{ 0 };

	std::function< void() > close_ok_cb = [big_data]{ std::cout << "OK"; };
	std::function< void(std::exception_ptr) > close_error_cb =
			[big_data](std::exception_ptr){ std::cout << "ERR"; };

	REQUIRE( close_ok_cb );
	REQUIRE( close_error_cb );

	restinio::asio_ns::post( http_server.io_context(),
		[&] {
			REQUIRE_NOTHROW( http_server.open_sync() );
			REQUIRE_NOTHROW( http_server.close_async(
					std::move(close_ok_cb),
					std::move(close_error_cb) ) );
		} );
	http_server.io_context().run();

	REQUIRE( !close_ok_cb );
	REQUIRE( !close_error_cb );

}

TEST_CASE( "failed-start-restart" , "[start][stop]" )
{
	unsigned short test_port = 8096;

	restinio::asio_ns::io_context io_svc;
	restinio::asio_ns::ip::tcp::socket dummy_socket( io_svc,
			restinio::asio_ns::ip::tcp::endpoint(
						restinio::asio_ns::ip::make_address( "127.0.0.1" ),
						test_port ) );

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::external_io_context( io_svc ),
		[test_port]( auto & settings ){
			settings
				.port( test_port )
				.protocol( restinio::asio_ns::ip::tcp::v4() )
				.address( "127.0.0.1" )
				.request_handler(
					[]( auto ){ return restinio::request_rejected(); } );
		} };

	restinio::asio_ns::post( http_server.io_context(),
		[&] {
			REQUIRE_THROWS( http_server.open_sync() );

			dummy_socket.close();

			REQUIRE_NOTHROW( http_server.open_sync() );

			// Now server should be started and should accept
			// incoming connections.
			restinio::asio_ns::ip::tcp::socket socket( io_svc );
			auto do_connect = [&socket, test_port]() {
				socket.connect( restinio::asio_ns::ip::tcp::endpoint(
						restinio::asio_ns::ip::make_address( "127.0.0.1" ),
						test_port ) );
			};
			REQUIRE_NOTHROW( do_connect() );
			REQUIRE_NOTHROW( socket.close() );

			REQUIRE_NOTHROW( http_server.close_sync() );
		} );
	http_server.io_context().run();
}

