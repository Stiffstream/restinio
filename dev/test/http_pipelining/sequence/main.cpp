/*
	restinio
*/

/*!
	Tests for settings parameters that have default constructor.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <asio.hpp>

#include <restinio/all.hpp>

#include "../../handle_requests/common/pub.hpp"


struct req_handler_t
{
	auto
	operator () ( restinio::request_handle_t req )
	{
		if( restinio::http_method_post() == req->header().method() )
		{
			if( req->header().request_target() == "/first" )
			{
				m_first_request = std::move( req );
				return restinio::request_accepted();
			}
			else if( req->header().request_target() == "/second" )
			{
				m_second_request = std::move( req );
				return restinio::request_accepted();
			}
			else if( req->header().request_target() == "/third" )
			{
				req->create_response()
					.append_header( "Server", "RESTinio utest server" )
					.append_header_date_field()
					.append_header( "Content-Type", "text/plain; charset=utf-8" )
					.set_body( req->body() )
					.done();

				if( m_second_request )
				{
					m_second_request->create_response()
						.append_header( "Server", "RESTinio utest server" )
						.append_header_date_field()
						.append_header( "Content-Type", "text/plain; charset=utf-8" )
						.set_body( m_second_request->body() )
						.done();

					m_second_request.reset();
				}

				if( m_first_request )
				{
					m_first_request->create_response()
						.append_header( "Server", "RESTinio utest server" )
						.append_header_date_field()
						.append_header( "Content-Type", "text/plain; charset=utf-8" )
						.set_body( m_first_request->body() )
						.done();

					m_first_request.reset();
				}

				return restinio::request_accepted();
			}
		}

		return restinio::request_rejected();
	}

	restinio::request_handle_t m_first_request;
	restinio::request_handle_t m_second_request;
};

TEST_CASE( "HTTP piplining" , "[reverse_handling]" )
{
	using http_server_t = restinio::http_server_t<>;

	http_server_t http_server{
		restinio::create_child_io_service( 1 ),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.read_next_http_message_timelimit( std::chrono::seconds( 5 ) )
				.handle_request_timeout( std::chrono::seconds( 1 ) )
				.max_pipelined_requests( 5 )
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

	http_server.open();

	std::string response;
	auto create_request = [](
		const std::string & path,
		const std::string & body,
		const std::string & conn_field_value = "keep-alive" ){
		return
			"POST /" + path + " HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Content-Length: " + std::to_string( body.size() ) + "\r\n"
			"Connection: " + conn_field_value +"\r\n"
			"\r\n" +
			body;
	};

	{
		const auto pipelinedrequests =
			create_request( "first", "FIRST" ) +
			create_request( "second", "SECOND" ) +
			create_request( "third", "THIRD", "close" );

		REQUIRE_NOTHROW( response = do_request( pipelinedrequests ) );

		const auto first_pos = response.find( "FIRST" );
		const auto second_pos = response.find( "SECOND" );
		const auto third_pos = response.find( "THIRD" );

		REQUIRE_FALSE( std::string::npos == first_pos );
		REQUIRE_FALSE( std::string::npos == second_pos );
		REQUIRE_FALSE( std::string::npos == third_pos );

		REQUIRE( first_pos < second_pos );
		REQUIRE( second_pos < third_pos );
	}

	{
		const auto pipelinedrequests =
			create_request( "first", "FIRST" ) +
			create_request( "second", "SECOND" ) +
			create_request( "third", "THIRD" ) +
			create_request( "first", "FIRST" ) +
			create_request( "second", "SECOND" );

		REQUIRE_NOTHROW( response = do_request( pipelinedrequests ) );

		{
			const auto first_pos = response.find( "FIRST" );
			const auto second_pos = response.find( "SECOND" );
			const auto third_pos = response.find( "THIRD" );

			REQUIRE_FALSE( std::string::npos == first_pos );
			REQUIRE_FALSE( std::string::npos == second_pos );
			REQUIRE_FALSE( std::string::npos == third_pos );

			REQUIRE( first_pos < second_pos );
			REQUIRE( second_pos < third_pos );
		}

		// Send 3rd reques through another connection.
		REQUIRE_NOTHROW( response = do_request( create_request( "third", "THIRD", "close" ) ) );

		// It must not contain responses on 1st and 2dn request
		// leaved in handler.

		{
			const auto first_pos = response.find( "FIRST" );
			const auto second_pos = response.find( "SECOND" );
			const auto third_pos = response.find( "THIRD" );

			REQUIRE( std::string::npos == first_pos );
			REQUIRE( std::string::npos == second_pos );
			REQUIRE_FALSE( std::string::npos == third_pos );
		}
	}

	http_server.close();
}
