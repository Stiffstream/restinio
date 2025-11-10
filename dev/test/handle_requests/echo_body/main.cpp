/*
	restinio
*/

/*!
	Echo server.
*/

#include <catch2/catch_all.hpp>

#include <restinio/core.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

using namespace restinio::tests;

template< typename Traits >
void
perform_test()
{
	using http_server_t = restinio::http_server_t< Traits >;

	const auto perform_checks = [](std::uint16_t port) {
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

		{
			const std::string body = "01234567890123456789";
			REQUIRE_NOTHROW( response = do_request(
					create_request( body ),
					default_ip_addr(),
					port ) );

			REQUIRE_THAT(
				response,
				Catch::Matchers::ContainsSubstring(
					"Content-Length: " + std::to_string( body.size() ) ) );
			REQUIRE_THAT( response, Catch::Matchers::EndsWith( body ) );
		}

		{
			const std::string body =
				"0123456789012345678901234567890123456789\r\n"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n"
				"abcdefghijklmnopqrstuvwxyz\r\n"
				"~!@#$%^&*()_+";

			REQUIRE_NOTHROW( response = do_request(
					create_request( body ),
					default_ip_addr(),
					port ) );

			REQUIRE_THAT(
				response,
				Catch::Matchers::ContainsSubstring(
					"Content-Length: " + std::to_string( body.size() ) ) );
			REQUIRE_THAT( response, Catch::Matchers::EndsWith( body ) );
		}

		{
			const std::string body( 2048, 'a' );

			REQUIRE_NOTHROW( response = do_request(
					create_request( body ),
					default_ip_addr(),
					port ) );

			REQUIRE_THAT(
				response,
				Catch::Matchers::ContainsSubstring(
					"Content-Length: " + std::to_string( body.size() ) ) );
			REQUIRE_THAT( response, Catch::Matchers::EndsWith( body ) );
		}
	};

	const auto request_handler = []( auto req )
	{
		if( restinio::http_method_post() == req->header().method() )
		{
			req->create_response()
				.append_header( "Server", "RESTinio utest server" )
				.append_header_date_field()
				.append_header( "Content-Type", "text/plain; charset=utf-8" )
				.set_body( req->body() )
				.done();
			return restinio::request_accepted();
		}

		return restinio::request_rejected();
	};

	SECTION( "server address as a string" )
	{
		random_port_getter_t port_getter;

		http_server_t http_server{
			restinio::own_io_context(),
			[&request_handler, &port_getter]( auto & settings ){
				settings
					.port( 0 )
					.address( default_ip_addr() )
					.acceptor_post_bind_hook( port_getter.as_post_bind_hook() )
					.request_handler( request_handler );
			}
		};

		other_work_thread_for_server_t<http_server_t> other_thread(http_server);
		other_thread.run();

		perform_checks( port_getter.port() );

		other_thread.stop_and_join();
	}

	SECTION( "server address as an IP-address" )
	{
		random_port_getter_t port_getter;
		http_server_t http_server{
			restinio::own_io_context(),
			[&request_handler, &port_getter]( auto & settings ){
				settings
					.port( 0 )
					.address( restinio::asio_ns::ip::make_address( default_ip_addr() ) )
					.acceptor_post_bind_hook( port_getter.as_post_bind_hook() )
					.request_handler( request_handler );
			}
		};

		other_work_thread_for_server_t<http_server_t> other_thread(http_server);
		other_thread.run();

		perform_checks( port_getter.port() );

		other_thread.stop_and_join();
	}
}

namespace restinio::tests
{

struct no_connection_limiter_traits_t : public restinio::default_traits_t {
	using logger_t = utest_logger_t;
};

} /* namespace restinio::tests */

TEST_CASE( "HTTP echo server (noop_connection_limiter)" , "[echo]" )
{
	perform_test< no_connection_limiter_traits_t >();
}

namespace restinio::tests
{

struct thread_safe_connection_limiter_traits_t : public restinio::default_traits_t {
	using logger_t = utest_logger_t;

	static constexpr bool use_connection_count_limiter = true;
};

} /* namespace restinio::tests */

TEST_CASE( "HTTP echo server (thread_safe_connection_limiter)" , "[echo]" )
{
	perform_test< thread_safe_connection_limiter_traits_t >();
}

namespace restinio::tests
{

struct single_thread_connection_limiter_traits_t : public restinio::default_single_thread_traits_t {
	using logger_t = utest_logger_t;

	static constexpr bool use_connection_count_limiter = true;
};

} /* namespace restinio::tests */

TEST_CASE( "HTTP echo server (single_thread_connection_limiter)" , "[echo]" )
{
	perform_test< single_thread_connection_limiter_traits_t >();
}

