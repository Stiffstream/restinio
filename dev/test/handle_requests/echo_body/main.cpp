/*
	restinio
*/

/*!
	Echo server.
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

TEST_CASE( "HTTP echo server" , "[echo]" )
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
					[]( auto req ){
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
					} );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

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
		REQUIRE_NOTHROW( response = do_request( create_request( body ) ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains(
				"Content-Length: " + std::to_string( body.size() ) ) );
		REQUIRE_THAT( response, Catch::Matchers::EndsWith( body ) );
	}

	{
		const std::string body =
			"0123456789012345678901234567890123456789\r\n"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n"
			"abcdefghijklmnopqrstuvwxyz\r\n"
			"~!@#$%^&*()_+";

		REQUIRE_NOTHROW( response = do_request( create_request( body ) ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains(
				"Content-Length: " + std::to_string( body.size() ) ) );
		REQUIRE_THAT( response, Catch::Matchers::EndsWith( body ) );
	}

	{
		const std::string body( 2048, 'a' );

		REQUIRE_NOTHROW( response = do_request( create_request( body ) ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains(
				"Content-Length: " + std::to_string( body.size() ) ) );
		REQUIRE_THAT( response, Catch::Matchers::EndsWith( body ) );
	}

	other_thread.stop_and_join();
}
