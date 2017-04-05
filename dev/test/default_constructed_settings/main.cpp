/*
	restinio
*/

/*!
	Tests for settings parameters that have default constructor.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <restinio/all.hpp>

using namespace restinio;

#define RESTINIO_REQHANDLER_UTEST_INTERNALS \
auto operator () (http_request_handle_t, connection_handle_t) const \
{	return restinio::request_rejected(); }

TEST_CASE( "Request handler" , "[settings][request_handler]" )
{
	SECTION( "Has default ctor" )
	{
		struct req_handler_t
		{
			const bool m_is_default_constructed;

			req_handler_t()
				:	m_is_default_constructed{ true }
			{}
			req_handler_t( int )
				:	m_is_default_constructed{ false }
			{}

			RESTINIO_REQHANDLER_UTEST_INTERNALS
		};

		using settings_t =
			server_settings_t<
				traits_t<
					asio_timer_factory_t,
					null_logger_t,
					req_handler_t > >;

		settings_t s{};

		REQUIRE( s.request_handler()->m_is_default_constructed );

		s.request_handler( 42 );
		REQUIRE_FALSE( s.request_handler()->m_is_default_constructed );
	}

	SECTION( "No default ctor" )
	{
		struct req_handler_t
		{
			req_handler_t( int )
			{}

			RESTINIO_REQHANDLER_UTEST_INTERNALS
		};

		using settings_t =
			server_settings_t<
				traits_t<
					asio_timer_factory_t,
					null_logger_t,
					req_handler_t > >;

		settings_t s{};

		REQUIRE_THROWS_WITH(
			s.request_handler(),
			Catch::Matchers::Contains( "request handler" ) );

		s.request_handler( 42 );
		REQUIRE_NOTHROW( s.request_handler() );
	}

	SECTION( "std::function" )
	{
		using req_handler_t = default_request_handler_t;

		using settings_t =
			server_settings_t<
				traits_t<
					asio_timer_factory_t,
					null_logger_t,
					req_handler_t > >;

		settings_t s{};

		REQUIRE_THROWS_WITH(
			s.request_handler(),
			Catch::Matchers::Contains( "request handler" ) );

		s.request_handler(
			[](http_request_handle_t, connection_handle_t ){
				return restinio::request_rejected();
			} );

		REQUIRE_NOTHROW( s.request_handler() );
	}
}

#define RESTINIO_LOGGER_UTEST_INTERNALS \
template < typename MSG_BUILDER > \
constexpr void trace( MSG_BUILDER && ) const {} \
template < typename MSG_BUILDER > \
constexpr void info( MSG_BUILDER && ) const {} \
template < typename MSG_BUILDER > \
constexpr void warn( MSG_BUILDER && ) const {} \
template < typename MSG_BUILDER > \
constexpr void error( MSG_BUILDER && ) const {}

struct logger1_t
{
	const bool m_is_default_constructed;

	logger1_t()
		:	m_is_default_constructed{ true }
	{}
	logger1_t( int )
		:	m_is_default_constructed{ false }
	{}

	RESTINIO_LOGGER_UTEST_INTERNALS
};

struct logger2_t
{
	logger2_t( int )
	{}

	RESTINIO_LOGGER_UTEST_INTERNALS
};

TEST_CASE( "Logger" , "[settings][logger]" )
{
	SECTION( "Has default ctor" )
	{



		using settings_t =
			server_settings_t<
				traits_t<
					asio_timer_factory_t,
					logger1_t > >;

		settings_t s{};

		REQUIRE( s.logger()->m_is_default_constructed );

		s.logger( 42 );
		REQUIRE_FALSE( s.logger()->m_is_default_constructed );
	}

	SECTION( "No default ctor" )
	{

		using settings_t =
			server_settings_t<
				traits_t<
					asio_timer_factory_t,
					logger2_t > >;

		settings_t s{};

		REQUIRE_THROWS_WITH(
			s.logger(),
			Catch::Matchers::Contains( "logger" ) );

		s.logger( 42 );
		REQUIRE_NOTHROW( s.logger() );
	}
}

#define RESTINIO_TIMERFACTORY_UTEST_INTERNALS \
struct timer_guard_t { \
	template < typename EXECUTOR, typename CALLBACK_FUNC > \
	void schedule_operation_timeout_callback( \
		const EXECUTOR & , \
		std::chrono::steady_clock::duration, \
		CALLBACK_FUNC && ) const \
	{} \
	void cancel() const {} \
}; \
using timer_guard_instance_t = std::shared_ptr< timer_guard_t >; \
timer_guard_instance_t create_timer_guard( asio::io_service & ) \
{ return std::make_shared< timer_guard_t >(); }

struct timer_factory1_t
{
	const bool m_is_default_constructed;

	timer_factory1_t()
		:	m_is_default_constructed{ true }
	{}
	timer_factory1_t( int )
		:	m_is_default_constructed{ false }
	{}

	RESTINIO_TIMERFACTORY_UTEST_INTERNALS
};

struct timer_factory2_t
{
	timer_factory2_t( int )
	{}

	RESTINIO_TIMERFACTORY_UTEST_INTERNALS
};

TEST_CASE( "Timer factory" , "[settings][timer_factory]" )
{
	SECTION( "Has default ctor" )
	{
		using settings_t =
			server_settings_t<
				traits_t<
					timer_factory1_t,
					logger1_t > >;

		settings_t s{};

		REQUIRE( s.timer_factory()->m_is_default_constructed );

		s.timer_factory( 42 );
		REQUIRE_FALSE( s.timer_factory()->m_is_default_constructed );
	}

	SECTION( "No default ctor" )
	{

		using settings_t =
			server_settings_t<
				traits_t<
					timer_factory2_t,
					logger2_t > >;

		settings_t s{};

		REQUIRE_THROWS_WITH(
			s.timer_factory(),
			Catch::Matchers::Contains( "timer factory" ) );

		s.timer_factory( 42 );
		REQUIRE_NOTHROW( s.timer_factory() );
	}
}
