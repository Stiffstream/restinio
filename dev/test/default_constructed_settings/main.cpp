/*
	restinio
*/

/*!
	Tests for settings parameters that have default constructor.
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>

using namespace restinio;

#define RESTINIO_REQHANDLER_UTEST_INTERNALS \
auto operator () ( request_handle_t ) const \
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
					asio_timer_manager_t,
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
					asio_timer_manager_t,
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
					asio_timer_manager_t,
					null_logger_t,
					req_handler_t > >;

		settings_t s{};

		REQUIRE_THROWS_WITH(
			s.request_handler(),
			Catch::Matchers::Contains( "request handler" ) );

		s.request_handler(
			[]( request_handle_t ){
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
					asio_timer_manager_t,
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
					asio_timer_manager_t,
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
		template <typename... Args > \
		constexpr void \
		schedule_operation_timeout_callback( Args &&... ) const \
		{} \
		constexpr void \
		cancel() const \
		{}\
}; \
	void start() const {} \
	void stop() const {}


struct timer_manager1_t
{
	const bool m_is_default_constructed;

	timer_manager1_t( bool is_default_constructed )
		:	m_is_default_constructed{ is_default_constructed }
	{}

	RESTINIO_TIMERFACTORY_UTEST_INTERNALS

	struct factory_t
	{
		const bool m_is_default_constructed;

		factory_t()
			:	m_is_default_constructed{ true }
		{}

		factory_t( int )
			:	m_is_default_constructed{ false }
		{}

		auto create( asio_ns::io_context & )
		{
			return std::make_shared< timer_manager1_t >( m_is_default_constructed );
		}
	};
};

struct timer_manager2_t
{
	RESTINIO_TIMERFACTORY_UTEST_INTERNALS

	struct factory_t
	{
		factory_t( int )
		{}

		auto create( asio_ns::io_context & )
		{
			return std::make_shared< timer_manager2_t >();
		}
	};
};

TEST_CASE( "Timer factory" , "[settings][timer_factory]" )
{
	SECTION( "Has default ctor" )
	{
		using settings_t =
			server_settings_t<
				traits_t<
					timer_manager1_t,
					logger1_t > >;

		settings_t s{};

		asio_ns::io_context io_context;

		REQUIRE( s.timer_factory()->create(io_context)->m_is_default_constructed );

		s.timer_manager( 42 );
		REQUIRE_FALSE( s.timer_factory()->create(io_context)->m_is_default_constructed );
	}

	SECTION( "No default ctor" )
	{

		using settings_t =
			server_settings_t<
				traits_t<
					timer_manager2_t,
					logger2_t > >;

		settings_t s{};

		REQUIRE_THROWS_WITH(
			s.timer_factory(),
			Catch::Matchers::Contains( "timer manager" ) );

		s.timer_manager( 42 );
		REQUIRE_NOTHROW( s.timer_factory() );
	}
}

TEST_CASE( "Acceptor options" , "[settings][acceptor_options]" )
{
	using settings_t = server_settings_t< restinio::default_traits_t >;

	settings_t s{};

	auto acceptor_option_setter = s.acceptor_options_setter();
	REQUIRE( acceptor_option_setter );

	bool lambda_was_called = false;
	s.acceptor_options_setter(
		[&]( auto & ){
			lambda_was_called = true;
		} );

	acceptor_option_setter = s.acceptor_options_setter();
	restinio::acceptor_options_t
		acceptor_options{ *static_cast< asio_ns::ip::tcp::acceptor * >( nullptr ) };

	(*acceptor_option_setter)( acceptor_options );

	REQUIRE( lambda_was_called );
}

TEST_CASE( "Socket options" , "[settings][socket_options]" )
{
	using settings_t = server_settings_t< restinio::default_traits_t >;

	settings_t s{};

	auto socket_option_setter = s.socket_options_setter();
	REQUIRE( socket_option_setter );

	bool lambda_was_called = false;
	s.socket_options_setter(
		[&]( auto & ){
			lambda_was_called = true;
		} );

	socket_option_setter = s.socket_options_setter();
	restinio::socket_options_t
		socket_options{ *static_cast< asio_ns::ip::tcp::socket * >( nullptr ) };

	(*socket_option_setter)( socket_options );

	REQUIRE( lambda_was_called );
}
