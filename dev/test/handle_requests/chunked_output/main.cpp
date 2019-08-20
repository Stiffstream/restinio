/*
	restinio
*/

/*!
	Tests for chunked output.
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
			using output_type_t = restinio::chunked_output_t;
			auto resp = req->create_response< output_type_t >();

			resp.append_header( "Server", "RESTinio utest server" )
				.append_header_date_field()
				.append_header( "Content-Type", "text/plain; charset=utf-8" );

			resp.flush();

			auto pause = std::chrono::milliseconds( 1 );

			if( req->header().request_target() == "/0" )
			{
				pause = std::chrono::milliseconds( 0 );
			}
			else if( req->header().request_target() == "/5" )
			{
				pause = std::chrono::milliseconds( 2 );
			}
			else if( req->header().request_target() == "/9" )
			{
				pause = std::chrono::milliseconds( 3 );
			}

			const auto & body = req->body();
			std::string::size_type chunk_begin = 0;
			std::string::size_type chunk_end = body.find( "\r\n", chunk_begin );

			auto next_flush_after_n_chunks = 1;
			auto not_flushed_chunks = 0;

			while( std::string::npos != chunk_end )
			{
				resp.append_chunk( body.substr( chunk_begin, chunk_end - chunk_begin ) );
				if( next_flush_after_n_chunks == ++not_flushed_chunks )
				{
					resp.flush();
					next_flush_after_n_chunks *= 2;
					not_flushed_chunks = 0;
				}

				chunk_begin = chunk_end + 2;
				chunk_end = body.find( "\r\n", chunk_begin );

				std::this_thread::sleep_for( pause );
			}

			if( chunk_begin < body.size() )
				resp.append_chunk( body.substr( chunk_begin ) );

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

const std::string standard_body =
	"0\r\n"
	"01\r\n"
	"012\r\n"
	"0123\r\n"
	"01234\r\n"
	"012345\r\n"
	"0123456\r\n"
	"01234567\r\n"
	"012345678\r\n"
	"0123456789\r\n"
	"0123456789A\r\n"
	"0123456789AB\r\n"
	"0123456789ABC\r\n"
	"0123456789ABCD\r\n"
	"0123456789ABCDE\r\n"
	"0123456789ABCDEF\r\n"
	"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";

const std::string standard_response =
	"1\r\n"
	"0\r\n"

	"2\r\n"
	"01\r\n"

	"3\r\n"
	"012\r\n"

	"4\r\n"
	"0123\r\n"

	"5\r\n"
	"01234\r\n"

	"6\r\n"
	"012345\r\n"

	"7\r\n"
	"0123456\r\n"

	"8\r\n"
	"01234567\r\n"

	"9\r\n"
	"012345678\r\n"

	"A\r\n"
	"0123456789\r\n"

	"B\r\n"
	"0123456789A\r\n"

	"C\r\n"
	"0123456789AB\r\n"
	"D\r\n"
	"0123456789ABC\r\n"
	"E\r\n"
	"0123456789ABCD\r\n"
	"F\r\n"
	"0123456789ABCDE\r\n"
	"10\r\n"
	"0123456789ABCDEF\r\n"

	"40\r\n"
	"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\r\n"

	"0\r\n"
	"\r\n";

TEST_CASE( "Using user chunked output response builder" , "[chunked_output]" )
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
				.max_pipelined_requests( 16 );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	start_request_handler_pool();

	SECTION( "single request" )
	{
		std::string request =
			create_request( 0, standard_body, "close" );
		std::string response;

		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::EndsWith( standard_response ) );
	}

	SECTION( "ten requests" )
	{
		const auto pipelinedrequests =
			create_request( 1, "FIRST REQUEST\r\n" + standard_body )+
			create_request( 2, "SECOND REQUEST\r\n" + standard_body )+
			create_request( 3, "THIRD REQUEST\r\n" + standard_body )+
			create_request( 4, "FOUTH REQUEST\r\n" + standard_body )+
			create_request( 5, "FIFTH REQUEST\r\n" + standard_body )+
			create_request( 6, "SIXTH REQUEST\r\n" + standard_body )+
			create_request( 7, "SEVENTH REQUEST\r\n" + standard_body )+
			create_request( 8, "EIGHTH REQUEST\r\n" + standard_body )+
			create_request( 9, "NINTH REQUEST\r\n" + standard_body )+
			create_request( 0, "LAST REQUEST\r\n" + standard_body, "close" );

		std::string response;
		REQUIRE_NOTHROW( response = do_request( pipelinedrequests ) );
		std::vector < std::string::size_type > resp_bodies_start_positions;

		resp_bodies_start_positions.emplace_back( response.find( "D\r\nFIRST REQUEST\r\n" ) );
		resp_bodies_start_positions.emplace_back( response.find( "E\r\nSECOND REQUEST\r\n" ) );
		resp_bodies_start_positions.emplace_back( response.find( "D\r\nTHIRD REQUEST\r\n" ) );
		resp_bodies_start_positions.emplace_back( response.find( "D\r\nFOUTH REQUEST\r\n" ) );
		resp_bodies_start_positions.emplace_back( response.find( "D\r\nFIFTH REQUEST\r\n" ) );
		resp_bodies_start_positions.emplace_back( response.find( "D\r\nSIXTH REQUEST\r\n" ) );
		resp_bodies_start_positions.emplace_back( response.find( "F\r\nSEVENTH REQUEST\r\n" ) );
		resp_bodies_start_positions.emplace_back( response.find( "E\r\nEIGHTH REQUEST\r\n" ) );
		resp_bodies_start_positions.emplace_back( response.find( "D\r\nNINTH REQUEST\r\n" ) );
		resp_bodies_start_positions.emplace_back( response.find( "C\r\nLAST REQUEST\r\n" ) );

		REQUIRE( resp_bodies_start_positions[ 0 ] != std::string::npos );
		REQUIRE( resp_bodies_start_positions[ 1 ] != std::string::npos );
		REQUIRE( resp_bodies_start_positions[ 2 ] != std::string::npos );
		REQUIRE( resp_bodies_start_positions[ 3 ] != std::string::npos );
		REQUIRE( resp_bodies_start_positions[ 4 ] != std::string::npos );
		REQUIRE( resp_bodies_start_positions[ 5 ] != std::string::npos );
		REQUIRE( resp_bodies_start_positions[ 6 ] != std::string::npos );
		REQUIRE( resp_bodies_start_positions[ 7 ] != std::string::npos );
		REQUIRE( resp_bodies_start_positions[ 8 ] != std::string::npos );
		REQUIRE( resp_bodies_start_positions[ 9 ] != std::string::npos );

		REQUIRE( resp_bodies_start_positions[ 0 ] < resp_bodies_start_positions[ 1 ] );
		REQUIRE( resp_bodies_start_positions[ 1 ] < resp_bodies_start_positions[ 2 ] );
		REQUIRE( resp_bodies_start_positions[ 2 ] < resp_bodies_start_positions[ 3 ] );
		REQUIRE( resp_bodies_start_positions[ 3 ] < resp_bodies_start_positions[ 4 ] );
		REQUIRE( resp_bodies_start_positions[ 4 ] < resp_bodies_start_positions[ 5 ] );
		REQUIRE( resp_bodies_start_positions[ 5 ] < resp_bodies_start_positions[ 6 ] );
		REQUIRE( resp_bodies_start_positions[ 6 ] < resp_bodies_start_positions[ 7 ] );
		REQUIRE( resp_bodies_start_positions[ 7 ] < resp_bodies_start_positions[ 8 ] );
		REQUIRE( resp_bodies_start_positions[ 8 ] < resp_bodies_start_positions[ 9 ] );
	}

	stop_request_handler_pool();

	other_thread.stop_and_join();
}

