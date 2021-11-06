/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>
#include <restinio/websocket/websocket.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

TEST_CASE( "zero as server port" , "[acceptor_post_bind_hook]" )
{
	struct test_traits : public restinio::traits_t<
			restinio::asio_timer_manager_t,
			utest_logger_t >
	{
	};

	using http_server_t = restinio::http_server_t< test_traits >; 

	std::promise<unsigned short> server_port_promise;
	auto server_port_future = server_port_promise.get_future();

	http_server_t http_server{
		restinio::own_io_context(),
		[&server_port_promise]( auto & settings ){
			namespace asio_ns = restinio::asio_ns;

			settings
				.port( 0u )
				.address( "127.0.0.1" )
				.acceptor_post_bind_hook(
					[&server_port_promise]( asio_ns::ip::tcp::acceptor & acceptor ) {
						server_port_promise.set_value(
								acceptor.local_endpoint().port() );
					} )
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

	REQUIRE_NOTHROW( response = do_request(
			request_str, "127.0.0.1", server_port_future.get() ) );
	REQUIRE_THAT( response, Catch::Matchers::EndsWith( "GET" ) );

	other_thread.stop_and_join();
}

