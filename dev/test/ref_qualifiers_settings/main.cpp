/*
	restinio
*/

/*!
	Tests for settings parameters.
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>

class tagged_object_t
{
	public:
		tagged_object_t( std::string tag )
			:	m_tag{ std::move( tag ) }
		{}

		virtual ~tagged_object_t()
		{}

		const std::string &
		tag() const
		{
			return m_tag;
		}

	private:
		const std::string m_tag;
};

struct tester_logger_t
	:	public tagged_object_t
{
	using tagged_object_t::tagged_object_t;

	template< typename MSG_BUILDER >
	constexpr void
	trace( MSG_BUILDER && ) const
	{}

	template< typename MSG_BUILDER >
	constexpr void
	info( MSG_BUILDER && ) const
	{}

	template< typename MSG_BUILDER >
	constexpr void
	warn( MSG_BUILDER && ) const
	{}

	template< typename MSG_BUILDER >
	constexpr void
	error( MSG_BUILDER && ) const
	{}
};

struct tester_req_handler_t
	:	public tagged_object_t
{
	using tagged_object_t::tagged_object_t;

	auto
	operator () ( restinio::request_handle_t ) const
	{
		return restinio::request_rejected();
	}
};


struct tester_timer_manager_t
{
	//! Timer guard for async operations.
	struct timer_guard_t
	{
		// Set new timeout guard.
		template <typename... Args >
		constexpr void
		schedule_operation_timeout_callback( Args &&... ) const
		{}

		// Cancel timeout guard if any.
		constexpr void
		cancel() const
		{}
	};

	// Create guard for connection.
	timer_guard_t
	create_timer_guard()
	{
		return timer_guard_t();
	}

	//! Start/stop timer manager.
	//! \{
	void start() const {}
	void stop() const {}
	//! \}

	struct factory_t
		:	public tagged_object_t
	{
		using tagged_object_t::tagged_object_t;

		auto
		create( restinio::asio_ns::io_context & )
		{
			return std::make_shared< tester_timer_manager_t >();
		}
	};
};

using traits_t =
	restinio::traits_t<
		tester_timer_manager_t,
		tester_logger_t,
		tester_req_handler_t >;

using server_settings_t = restinio::server_settings_t< traits_t >;

TEST_CASE( "Ref-qualifiers" , "[settings][ref_qualifiers]" )
{
	bool acceptor_options_lambda_was_called = false;
	bool socket_options_lambda_was_called = false;

	auto check_params =
		[ & ]( server_settings_t settings ){
			REQUIRE( 4242 == settings.port() );
			REQUIRE( restinio::asio_ns::ip::tcp::v6() == settings.protocol() );
			REQUIRE( std::string{ "127.0.0.1" } == settings.address() );
			REQUIRE( 2017 == settings.buffer_size() );
			REQUIRE( std::chrono::seconds( 120 ) == settings.read_next_http_message_timelimit() );
			REQUIRE( std::chrono::seconds( 121 ) == settings.write_http_response_timelimit() );
			REQUIRE( std::chrono::seconds( 122 ) == settings.handle_request_timeout() );
			REQUIRE( 42 == settings.max_pipelined_requests() );
			REQUIRE( std::string{ "REQUESTHANDLER" } == settings.request_handler()->tag() );
			REQUIRE( std::string{ "TIMERFACTORY" } == settings.timer_factory()->tag() );
			REQUIRE( std::string{ "LOGGER" } == settings.logger()->tag() );
			REQUIRE( 4 == settings.concurrent_accepts_count() );

			{
				auto acceptor_option_setter = settings.acceptor_options_setter();
				restinio::acceptor_options_t
					acceptor_options{ *static_cast< restinio::asio_ns::ip::tcp::acceptor * >( nullptr ) };

				(*acceptor_option_setter)( acceptor_options );
			}

			REQUIRE( acceptor_options_lambda_was_called );

			{
				auto socket_option_setter = settings.socket_options_setter();
				restinio::socket_options_t
					socket_options{ *static_cast< restinio::asio_ns::ip::tcp::socket * >( nullptr ) };

				(*socket_option_setter)( socket_options );
			}

			REQUIRE( socket_options_lambda_was_called );


			REQUIRE( settings.separate_accept_and_create_connect() );
		};

	check_params(
		server_settings_t{}
			.port( 4242 )
			.protocol( restinio::asio_ns::ip::tcp::v6() )
			.address( "127.0.0.1" )
			.buffer_size( 2017 )
			.read_next_http_message_timelimit( std::chrono::seconds( 120 ) )
			.write_http_response_timelimit( std::chrono::seconds( 121 ) )
			.handle_request_timeout( std::chrono::seconds( 122 ) )
			.max_pipelined_requests( 42 )
			.request_handler( "REQUESTHANDLER" )
			.timer_manager( "TIMERFACTORY" )
			.logger( "LOGGER" )
			.concurrent_accepts_count( 4 )
			.acceptor_options_setter(
				[&]( auto & ){
					acceptor_options_lambda_was_called = true;
				} )
			.socket_options_setter(
				[&]( auto & ){
					socket_options_lambda_was_called = true;
				} )
			.separate_accept_and_create_connect( true ) );
}
