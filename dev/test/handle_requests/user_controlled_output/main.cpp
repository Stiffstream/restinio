/*
	restinio
*/

/*!
	Test for user controlled output.
*/

#include <cstdlib>
#include <thread>

#include <catch2/catch.hpp>

#include <restinio/all.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/request_handler_pool.hpp>
#include <test/common/pub.hpp>

#if defined(__GNUG__)
#pragma GCC diagnostic ignored "-Wparentheses"
#endif

// Global request handlers pool.
request_handler_pool_t< 10 > g_request_handler_pool;

void
start_request_handler_pool()
{
	g_request_handler_pool.start(
		[]( restinio::request_handle_t req ){
			using output_type_t = restinio::user_controlled_output_t;
			auto resp = req->create_response< output_type_t >();

			resp.append_header( "Server", "RESTinio utest server" )
				.append_header_date_field()
				.append_header( "Content-Type", "text/plain; charset=utf-8" )
				.append_header( "UTest-req-target", req->header().request_target() )
				.set_content_length( req->body().size() );

			resp.flush();

			auto pause = std::chrono::milliseconds( 1 );

			if( req->header().request_target() == "/5" )
			{
				pause = std::chrono::milliseconds( 2 );
			}
			else if( req->header().request_target() == "/9" )
			{
				pause = std::chrono::milliseconds( 3 );
			}

			for( const char c : req->body() )
			{
				std::string s;
				s += c;
				resp.set_body( std::move( s ) );
				resp.flush();

				std::this_thread::sleep_for( pause );
			}

			resp.done();
		} );
}

void
stop_request_handler_pool()
{
	g_request_handler_pool.stop();
}

struct req_handler_t
{
	auto
	operator () ( restinio::request_handle_t req )
	{
		if( restinio::http_method_post() == req->header().method() )
		{
			g_request_handler_pool.enqueue( std::move( req ) );

			return restinio::request_accepted();
		}

		return restinio::request_rejected();
	}
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
}

TEST_CASE( "Using user controlled output response builder" , "[user_controlled_output]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t,
				req_handler_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.read_next_http_message_timelimit( std::chrono::hours( 24 ) )
				.handle_request_timeout( std::chrono::hours( 24 ) )
				.max_pipelined_requests( 20 );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	start_request_handler_pool();

	SECTION( "Simple sequence" )
	{
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
	}

	SECTION( "Interrupted sequence" )
	{
		std::string response;

		const auto pipelinedrequests =
			create_request( 0, "0:0123456789:0" ) +
			create_request( 1, "1:0123456789:1" ) +
			create_request( 2, "2:0123456789:2" ) +
			create_request( 3, "3:0123456789:3" ) +
			create_request( 4, "4:0123456789:4" ) +
			create_request( 5, "5:0123456789:5", "close" ) + // Interrupt
			create_request( 6, "NOWAY" ) +
			create_request( 7, "NOWAY" ) +
			create_request( 8, "NOWAY" ) +
			create_request( 9, "NOWAY" );

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
			Catch::Matchers::EndsWith( "5:0123456789:5" ) );
		REQUIRE(
			std::string::npos == response.find( "NOWAY" ) );
	}

	SECTION( "Interrupted sequence with slow before" )
	{
		std::string response;

		const auto pipelinedrequests =
			create_request( 0, "0:0123456789:0" ) +
			create_request( 1, "1:0123456789:1" ) +
			create_request( 2, "2:0123456789:2" ) +
			create_request( 3, "3:0123456789:3" ) +
			create_request( 4, "4:0123456789:4" ) +

			create_request( 9, "9:MUST_BE_1:9" ) +
			create_request( 9, "9:MUST_BE_2:9" ) +
			create_request( 9, "9:MUST_BE_3:9" ) +
			create_request( 9, "9:MUST_BE_4:9" ) +

			create_request( 5, "5:0123456789:5", "close" ) + // Interrupt

			create_request( 0, "NOWAY" ) +
			create_request( 1, "NOWAY" ) +

			create_request( 2, "NOWAY" ) +
			create_request( 3, "NOWAY" ) +
			create_request( 4, "NOWAY" ) +

			create_request( 6, "NOWAY" ) +
			create_request( 7, "NOWAY" ) +
			create_request( 8, "NOWAY" ) +
			create_request( 9, "NOWAY" );

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
			Catch::Matchers::Contains( "9:MUST_BE_1:9" ) );
		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains( "9:MUST_BE_2:9" ) );
		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains( "9:MUST_BE_3:9" ) );
		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains( "9:MUST_BE_4:9" ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::EndsWith( "5:0123456789:5" ) );

		REQUIRE(
			std::string::npos == response.find( "NOWAY" ) );
	}

	SECTION( "sequence with mixed order" )
	{
		std::srand( static_cast<unsigned int>(std::time( nullptr )) );

		auto create_body = []( auto i ){
			return "====" + std::to_string( i ) + "====";
		};

		std::string pipelinedrequests;
		for( int i = 0; i < 40; ++i )
		{
			if( i != 20 )
				pipelinedrequests +=
					create_request( std::rand() % 10, create_body( i ) );
			else
				pipelinedrequests +=
					create_request( i % 10, "CLOSECLOSECLOSE", "close" );
		}

		std::string response;
		REQUIRE_NOTHROW( response = do_request( pipelinedrequests ) );

		for( int i = 0; i < 20; ++i )
		{
			REQUIRE_THAT(
				response,
				Catch::Matchers::Contains( create_body( i ) ) );
		}

		REQUIRE_THAT(
			response,
			Catch::Matchers::EndsWith( "CLOSECLOSECLOSE" ) );

		REQUIRE(
			std::string::npos == response.find( "NOWAY" ) );
	}

	stop_request_handler_pool();

	other_thread.stop_and_join();
}
