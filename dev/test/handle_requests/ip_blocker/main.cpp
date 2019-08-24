/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>
#include <restinio/websocket/websocket.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

class blocker_t
{
	std::mutex m_lock;
	unsigned m_ordinal{};

public :

	auto inspect(
		const restinio::ip_blocker::incoming_info_t & info ) noexcept
	{
		std::lock_guard< std::mutex > l{ m_lock };
		auto result = (0 == (m_ordinal & 1)) ?
				restinio::ip_blocker::inspection_result_t::allow :
				restinio::ip_blocker::inspection_result_t::deny;
		++m_ordinal;

		return result;
	}
};

TEST_CASE( "no blocker" , "[no_blocker]" )
{
	struct test_traits : public restinio::traits_t<
			restinio::asio_timer_manager_t,
			utest_logger_t >
	{
		using ip_blocker_t = blocker_t;
	};

	using http_server_t = restinio::http_server_t< test_traits >; 

	REQUIRE_THROWS( std::unique_ptr<http_server_t>{
		new http_server_t{
				restinio::own_io_context(),
				[]( auto & settings ){
					settings
						.port( utest_default_port() )
						.address( "127.0.0.1" )
						.request_handler(
							[]( auto ){
								return restinio::request_rejected();
							} );
				} }
	} );
}

TEST_CASE( "ordinary connection" , "[ordinary_connection]" )
{
	struct test_traits : public restinio::traits_t<
			restinio::asio_timer_manager_t,
			utest_logger_t >
	{
		using ip_blocker_t = blocker_t;
	};

	using http_server_t = restinio::http_server_t< test_traits >; 

	auto blocker = std::make_shared< blocker_t >();

	http_server_t http_server{
		restinio::own_io_context(),
		[blocker]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.ip_blocker( blocker )
				.request_handler(
					[]( auto req ){
						req->create_response()
							.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" )
							.set_body(
								restinio::const_buffer( req->header().method().c_str() ) )
							.done();

						return restinio::request_accepted();
					} );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );
	REQUIRE_THAT( response, Catch::Matchers::EndsWith( "GET" ) );

	// This request should be blocked.
	REQUIRE_THROWS( response = do_request( request_str ) );

	// This request shouldn't be blocked.
	REQUIRE_NOTHROW( response = do_request( request_str ) );
	REQUIRE_THAT( response, Catch::Matchers::EndsWith( "GET" ) );

	// This request should be blocked.
	REQUIRE_THROWS( response = do_request( request_str ) );

	other_thread.stop_and_join();
}

