/*
	restinio
*/

/*!
	Tests for settings parameters that have default constructor.
*/

#define CATCH_CONFIG_MAIN
#include <thread>

#include <catch/catch.hpp>

#include <asio.hpp>

#include <restinio/all.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/request_handler_pool.hpp>
#include <test/common/pub.hpp>

#if defined(__GNUG__)
#pragma GCC diagnostic ignored "-Wparentheses"
#endif

class req_handler_t
{
	public:
		req_handler_t()
		{
			m_request_handler_pool.start(
				[]( restinio::request_handle_t req ){
					using output_type_t = restinio::user_controlled_output_t;
					auto resp = req->create_response< output_type_t >();

					resp.append_header( "Server", "RESTinio utest server" )
						.append_header_date_field()
						.append_header( "Content-Type", "text/plain; charset=utf-8" )
						.set_content_length( req->body().size() );

					resp.flush();

					for( const char c : req->body() )
					{
						std::string s;
						s += c;
						resp.set_body( std::move( s ) );
						resp.flush();
						std::this_thread::sleep_for(
							std::chrono::milliseconds( 1 ) );
					}

					resp.done();
				} );
		}

		~req_handler_t()
		{
			m_request_handler_pool.stop();
		}

	auto
	operator () ( restinio::request_handle_t req )
	{
		if( restinio::http_method_post() == req->header().method() )
		{
			m_request_handler_pool.enqueue( std::move( req ) );

			return restinio::request_accepted();
		}

		return restinio::request_rejected();
	}

	request_handler_pool_t< 10 > m_request_handler_pool;
};

auto
create_request(
	unsigned int req_id,
	const std::string & body,
	const std::string & conn_field_value = "keep-alive" )
{
	return
		"POST /" + std::to_string( req_id ) + " HTTP/1.0\r\n"
		"From: unit-test\r\n"
		"User-Agent: unit-test\r\n"
		"Content-Length: " + std::to_string( body.size() ) + "\r\n"
		"Connection: " + conn_field_value +"\r\n"
		"\r\n" +
		body;
};

TEST_CASE( "Using user controlled output response builder" , "[user_controlled_output]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_factory_t,
				utest_logger_t,
				req_handler_t > >;

	http_server_t http_server{
		restinio::create_child_io_service( 1 ),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.read_next_http_message_timelimit( std::chrono::seconds( 5 ) )
				.max_pipelined_requests( 20 );
		}
	};

	http_server.open();

	std::string response;

	const auto pipelinedrequests =
		create_request( 0, "0:0123456789:0" ) +
		create_request( 1, "1:0123456789:1" ) +
		create_request( 2, "2:0123456789:2" ) +
		create_request( 3, "3:0123456789:3" ) +
		create_request( 4, "4:0123456789:4" ) +
		create_request( 5, "5:0123456789:5" ) +
		create_request( 6, "6:0123456789:6" ) +
		create_request( 7, "7:0123456789:7" ) +
		create_request( 8, "8:0123456789:8" ) +
		create_request( 9, "9:0123456789:9", "close" );

	REQUIRE_NOTHROW( response = do_request( pipelinedrequests ) );
	REQUIRE_THAT(
		response,
		Catch::Matchers::Contains( "0:0123456789:0" ) );
	REQUIRE_THAT(
		response,
		Catch::Matchers::Contains( "1:0123456789:1" ) );
	REQUIRE_THAT(
		response,
		Catch::Matchers::Contains( "2:0123456789:2" ) );
	REQUIRE_THAT(
		response,
		Catch::Matchers::Contains( "3:0123456789:3" ) );
	REQUIRE_THAT(
		response,
		Catch::Matchers::Contains( "4:0123456789:4" ) );
	REQUIRE_THAT(
		response,
		Catch::Matchers::Contains( "5:0123456789:5" ) );
	REQUIRE_THAT(
		response,
		Catch::Matchers::Contains( "6:0123456789:6" ) );
	REQUIRE_THAT(
		response,
		Catch::Matchers::Contains( "7:0123456789:7" ) );
	REQUIRE_THAT(
		response,
		Catch::Matchers::Contains( "8:0123456789:8" ) );
	REQUIRE_THAT(
		response,
		Catch::Matchers::EndsWith( "9:0123456789:9" ) );

	http_server.close();
}
