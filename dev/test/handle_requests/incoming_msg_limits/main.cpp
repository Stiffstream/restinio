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
				.incoming_http_msg_limits(
						restinio::incoming_http_msg_limits_t{}
							.max_url_size( 20 )
							.max_field_name_size( 30 )
							.max_field_value_size( 40 )
							.max_field_count( 16 ) 
							.max_body_size( 40 )
					)
				.request_handler(
					[]( auto req ){
						std::cout << "### BODY: '" << req->body() << "'" << std::endl;
						return req->create_response()
							.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" )
							.set_body( "Ok" )
							.done();
					} );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	SECTION( "small URL" )
	{
		std::string request{
			"GET /123456789/123456789 HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Connection: close\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"Ok") );
	}

	SECTION( "long URL" )
	{
		std::string request{
			"GET /123456789/123456789/ HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Connection: close\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_THROWS( response = do_request( request ) );
	}

	SECTION( "small field name" )
	{
		std::string request{
			"GET /123456789/123456789 HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Connection: close\r\n"
			"123456789-123456789-1234567890: bla-bla-bla\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"Ok") );
	}

	SECTION( "long field name" )
	{
		std::string request{
			"GET /123456789/123456789 HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Connection: close\r\n"
			"123456789-123456789-123456789-1: bla-bla-bla\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_THROWS( response = do_request( request ) );
	}

	SECTION( "small field value" )
	{
		std::string request{
			"GET /123456789/123456789 HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Connection: close\r\n"
			"123456789-123456789-1234567890: "
				"123456789-123456789-123456789-123456789\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"Ok") );
	}

	SECTION( "long field value" )
	{
		std::string request{
			"GET /123456789/123456789 HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Connection: close\r\n"
			"123456789-123456789-1234567890: "
				"123456789-123456789-123456789-123456789-\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_THROWS( response = do_request( request ) );
	}

	SECTION( "small body with Content-Length" )
	{
		std::string request{
			"POST /123456789/123456789 HTTP/1.1\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: 40\r\n"
			"Connection: close\r\n"
			"\r\n"
			"123456789-123456789-1234567890-1234567890"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"Ok") );
	}

	SECTION( "large body with Content-Length" )
	{
		std::string request{
			"POST /123456789/222222222 HTTP/1.1\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: 41\r\n"
			"Connection: close\r\n"
			"\r\n"
			"123456789-123456789-1234567890-123456789-1"
		};

		std::string response;
		REQUIRE_THROWS( response = do_request( request ) );
	}

	other_thread.stop_and_join();
}

