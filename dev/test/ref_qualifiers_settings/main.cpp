/*
	restinio
*/

/*!
	Tests for settings parameters.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

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
	operator () ( restinio::request_handle_t req ) const
	{
		return restinio::request_rejected();
	}
};


struct tester_timer_factory_t
	:	public tagged_object_t
{
	using tagged_object_t::tagged_object_t;

	//! Timer guard for async operations.
	struct timer_guard_t
	{
		// Set new timeout guard.
		template <
				typename EXECUTOR,
				typename CALLBACK_FUNC >
		void
		schedule_operation_timeout_callback(
			const EXECUTOR & ,
			std::chrono::steady_clock::duration ,
			CALLBACK_FUNC && ) const
		{}

		// Cancel timeout guard if any.
		void
		cancel() const
		{}
	};

	using timer_guard_instance_t = std::shared_ptr< timer_guard_t >;

	// Create guard for connection.
	timer_guard_instance_t
	create_timer_guard( asio::io_service & )
	{
		return std::make_shared< timer_guard_t >();
	}
};

using traits_t =
	restinio::traits_t<
		tester_timer_factory_t,
		tester_logger_t,
		tester_req_handler_t >;

using server_settings_t = restinio::server_settings_t< traits_t >;

TEST_CASE( "Ref-qualifiers" , "[settings][ref_qualifiers]" )
{
	auto check_params =
		[ & ]( server_settings_t settings ){
			REQUIRE( 4242 == settings.port() );
			REQUIRE( asio::ip::tcp::v6() == settings.protocol() );
			REQUIRE( std::string{ "127.0.0.1" } == settings.address() );
			REQUIRE( 2017 == settings.buffer_size() );
			REQUIRE( std::chrono::seconds( 120 ) ==
				settings.read_next_http_message_timelimit() );
			REQUIRE( std::chrono::seconds( 121 ) ==
				settings.write_http_response_timelimit() );
			REQUIRE( std::chrono::seconds( 122 ) ==
				settings.handle_request_timeout() );
			REQUIRE( 42 == settings.max_pipelined_requests() );
			REQUIRE( std::string{ "REQUESTHANDLER" } ==
				settings.request_handler()->tag() );
			REQUIRE( std::string{ "TIMERFACTORY" } ==
				settings.timer_factory()->tag() );
			REQUIRE( std::string{ "LOGGER" } ==
				settings.logger()->tag() );
		};

	check_params(
		server_settings_t{}
			.port( 4242 )
			.protocol( asio::ip::tcp::v6() )
			.address( "127.0.0.1" )
			.buffer_size( 2017 )
			.read_next_http_message_timelimit( std::chrono::seconds( 120 ) )
			.write_http_response_timelimit( std::chrono::seconds( 121 ) )
			.handle_request_timeout( std::chrono::seconds( 122 ) )
			.max_pipelined_requests( 42 )
			.request_handler( "REQUESTHANDLER" )
			.timer_factory( "TIMERFACTORY" )
			.logger( "LOGGER" ) );
}
