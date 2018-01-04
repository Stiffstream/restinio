/*
	restinio
*/

/*!
	Test method detection.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <restinio/all.hpp>

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
