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

TEST_CASE( "start-stop" , "[stop]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_factory_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::create_child_io_context( 1 ),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[]( auto req ){ return restinio::request_rejected(); } );
		} };

	REQUIRE_NOTHROW( http_server.start() );
	REQUIRE_NOTHROW( http_server.stop() );
}

TEST_CASE( "start-stop-stop" , "[stop]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_factory_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::create_child_io_context( 1 ),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[]( auto req ){ return restinio::request_rejected(); } );
		} };

	REQUIRE_NOTHROW( http_server.start() );
	REQUIRE_NOTHROW( http_server.stop() );
	REQUIRE_NOTHROW( http_server.stop() );
}

TEST_CASE( "start-start-stop" , "[stop]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_factory_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::create_child_io_context( 1 ),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[]( auto req ){ return restinio::request_rejected(); } );
		} };

	REQUIRE_NOTHROW( http_server.start() );
	REQUIRE_NOTHROW( http_server.start() );
	REQUIRE_NOTHROW( http_server.stop() );
}

TEST_CASE( "start-dtor" , "[stop]" )
{
	auto runner = []{
		using http_server_t =
			restinio::http_server_t<
				restinio::traits_t<
					restinio::asio_timer_factory_t,
					utest_logger_t > >;

		http_server_t http_server{
			restinio::create_child_io_context( 1 ),
			[]( auto & settings ){
				settings
					.port( utest_default_port() )
					.address( "127.0.0.1" )
					.request_handler(
						[]( auto req ){ return restinio::request_rejected(); } );
			} };

		REQUIRE_NOTHROW( http_server.start() );
	};

	REQUIRE_NOTHROW( runner() );
}
