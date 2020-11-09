/*
	restinio
*/

/*!
	Echo server.
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>

#include <restinio/helpers/string_algo.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

namespace details
{

class handlers_counter_t
{
	std::mutex m_lock;
	std::size_t m_current{ 0u };
	std::size_t m_max{ 0u };

public:
	handlers_counter_t() = default;

	void inc() noexcept
	{
		std::lock_guard< std::mutex > lock{ m_lock };

		++m_current;
		if( m_current > m_max ) m_max = m_current;
	}

	void dec() noexcept
	{
		std::lock_guard< std::mutex > lock{ m_lock };

		--m_current;
	}

	std::size_t result() noexcept
	{
		std::lock_guard< std::mutex > lock{ m_lock };

		return m_max;
	}
};

struct client_load_t
{
	std::size_t m_threads_count;
	std::size_t m_iterations_per_thread;

	client_load_t(
		std::size_t threads_count,
		std::size_t iterations_per_thread )
		:	m_threads_count{ threads_count }
		,	m_iterations_per_thread{ iterations_per_thread }
	{}
};

} /* namespace details */

void
ensure_or_die( bool condition, const char * description )
{
	if( !condition )
	{
		std::cerr << "FAILURE: " << description << std::endl;
		std::abort();
	}
}

#define MY_REQUIRE(statement) ensure_or_die( (statement), #statement )

template< typename Traits >
void
perform_test(
	std::size_t max_parallel_connections,
	bool separate_accept_and_create_connection,
	std::size_t server_threads_count,
	details::client_load_t client_load )
{
	const auto perform_checks = []( std::size_t iterations ) {
		std::string response;
		auto create_request = []( const std::string & body ){
			return
				"POST /data HTTP/1.0\r\n"
				"From: unit-test\r\n"
				"User-Agent: unit-test\r\n"
				"Content-Type: application/x-www-form-urlencoded\r\n"
				"Content-Length: " + std::to_string( body.size() ) + "\r\n"
				"Connection: close\r\n"
				"\r\n" +
				body;
		};

		using namespace restinio::string_algo;
		for( std::size_t i = 0; i != iterations; ++i )
		{
			{
				const std::string body = "01234567890123456789";
				response = do_request( create_request( body ) );

				MY_REQUIRE( std::string::npos != response.find(
						"Content-Length: " + std::to_string( body.size() ) ) );
				MY_REQUIRE( ends_with( response, body ) );
			}

			{
				const std::string body =
					"0123456789012345678901234567890123456789\r\n"
					"ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n"
					"abcdefghijklmnopqrstuvwxyz\r\n"
					"~!@#$%^&*()_+";

				response = do_request( create_request( body ) );

				MY_REQUIRE( std::string::npos != response.find(
						"Content-Length: " + std::to_string( body.size() ) ) );
				MY_REQUIRE( ends_with( response, body ) );
			}

			{
				const std::string body( 2048, 'a' );

				response = do_request( create_request( body ) );

				MY_REQUIRE( std::string::npos != response.find(
						"Content-Length: " + std::to_string( body.size() ) ) );
				MY_REQUIRE( ends_with( response, body ) );
			}
		}
	};

	restinio::asio_ns::io_context ioctx;

	details::handlers_counter_t counter;

	const auto request_handler = [&counter, &ioctx]( auto req )
	{
		counter.inc();

		// Delay request processing to 0.25s
		auto timer = std::make_shared<restinio::asio_ns::steady_timer>( ioctx );
		timer->expires_after( std::chrono::milliseconds(25) );
		timer->async_wait( [&counter, timer, req](const auto & ec) {
				// Call decrement right now because if we call it
				// after create_response()...done() chain then there could
				// be a possibility that the connection will be dropped
				// just before a call to dec().
				counter.dec();

				if( !ec ) {
					req->create_response()
						.append_header( "Server", "RESTinio utest server" )
						.append_header_date_field()
						.append_header( "Content-Type", "text/plain; charset=utf-8" )
						.set_body( req->body() )
						.done();
				}
			} );

		return restinio::request_accepted();
	};

	auto server = restinio::run_async(
			restinio::external_io_context( ioctx ),
			restinio::server_settings_t< Traits >{}
					.port( utest_default_port() )
					.address( "127.0.0.1" )
					.request_handler( request_handler )
					.concurrent_accepts_count( server_threads_count )
					.separate_accept_and_create_connect(
							separate_accept_and_create_connection )
					.max_parallel_connections( max_parallel_connections ),
			server_threads_count );

	std::vector< std::thread > clients;
	clients.reserve( client_load.m_threads_count );

	for( std::size_t i = 0u; i != client_load.m_threads_count; ++i )
	{
		clients.emplace_back(
				perform_checks, client_load.m_iterations_per_thread );
	}

	for( auto & t : clients )
		t.join();

	server->stop();
	server->wait();

	REQUIRE( counter.result() <= max_parallel_connections );
}

struct thread_safe_connection_limiter_traits_t : public restinio::default_traits_t {
	static constexpr bool use_connection_count_limiter = true;
};

TEST_CASE( "thread_safe_connection_limiter, no accept-connect separation" , "[thread_safe][no_separate_accept]" )
{
	perform_test< thread_safe_connection_limiter_traits_t >(
			8,
			false,
			5,
			details::client_load_t{ 16, 40 } );
}

TEST_CASE( "thread_safe_connection_limiter, accept-connect separation" , "[thread_safe][separate_accept]" )
{
	perform_test< thread_safe_connection_limiter_traits_t >(
			8,
			true,
			5,
			details::client_load_t{ 16, 40 } );
}

struct single_thread_connection_limiter_traits_t : public restinio::default_single_thread_traits_t {
	static constexpr bool use_connection_count_limiter = true;
};

TEST_CASE( "single_thread_connection_limiter, no accept-connect separation" , "[single_thread][no_separate_accept]" )
{
	perform_test< single_thread_connection_limiter_traits_t >(
			8,
			false,
			1,
			details::client_load_t{ 10, 40 } );
}

TEST_CASE( "single_thread_connection_limiter, accept-connect separation" , "[single_thread][separate_accept]" )
{
	perform_test< single_thread_connection_limiter_traits_t >(
			8,
			false,
			1,
			details::client_load_t{ 10, 40 } );
}

