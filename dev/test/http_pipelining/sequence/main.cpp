/*
	restinio
*/

/*!
	Test http pipelining support.
*/

#include <algorithm>
#include <cctype>
#include <sstream>

#include <catch2/catch.hpp>

#include <restinio/all.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

void
send_response_if_needed( restinio::request_handle_t rh )
{
	if( rh )
		rh->create_response()
			.append_header( "Server", "RESTinio utest server" )
			.append_header_date_field()
			.append_header( "Content-Type", "text/plain; charset=utf-8" )
			.set_body( rh->body() )
			.done();
}

template < unsigned int N >
struct req_handler_t
{
	auto
	operator () ( restinio::request_handle_t req )
	{
		if( restinio::http_method_post() == req->header().method() )
		{
			const auto & target = req->header().request_target();

			if( target == "/" + std::to_string( N - 1 ) )
			{
				m_requests.back() = std::move( req );
				std::for_each(
					std::rbegin( m_requests ),
					std::rend( m_requests ),
					[]( auto & req ){
						send_response_if_needed( std::move( req ) );
					} );
			}
			else
			{
				for( unsigned int i = 0; i < N - 1; ++i )
				{
					if( target == "/" + std::to_string( i ) )
					{
						send_response_if_needed( std::move( m_requests[ i ] ) );
						m_requests[ i ] = std::move( req );
						break;
					}
				}
			}

			return restinio::request_accepted();
		}

		return restinio::request_rejected();
	}

	std::array< restinio::request_handle_t, N >  m_requests;
};

const std::string REQ_BODY_STARTER{ "REQUESTBODY#" };

auto
create_request(
	unsigned int req_id,
	const std::string & conn_field_value = "keep-alive" )
{
	const std::string body = REQ_BODY_STARTER + std::to_string( req_id );
	return
		"POST /" + std::to_string( req_id ) + " HTTP/1.0\r\n"
		"From: unit-test\r\n"
		"User-Agent: unit-test\r\n"
		"Content-Length: " + std::to_string( body.size() ) + "\r\n"
		"Connection: " + conn_field_value +"\r\n"
		"\r\n" +
		body;
}

std::vector< unsigned int >
get_response_sequence( const std::string responses )
{
	std::vector< unsigned int > result;
	std::size_t pos = 0;
	while( std::string::npos !=
		(pos = responses.find( REQ_BODY_STARTER, pos ) ) )
	{
		pos += REQ_BODY_STARTER.size();

		unsigned int item = 0;
		while( pos != responses.size() &&
			std::isdigit( responses[ pos ] ) )
		{
			item = item * 10 + ( responses[ pos++ ] - '0' );
		}

		result.push_back( item );
	}

	return result;
}

TEST_CASE( "Simple HTTP piplining " , "[reverse_handling]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t,
				req_handler_t< 3 > > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )

				// Must have notable timeouts:
				.read_next_http_message_timelimit(
					std::chrono::hours( 24 ) )
				.handle_request_timeout( std::chrono::hours( 24 ) )

				.max_pipelined_requests( 10 );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	{
		std::string response;

		const auto pipelinedrequests =
			create_request( 0 ) +
			create_request( 1 ) +
			create_request( 2, "close" );

		REQUIRE_NOTHROW( response = do_request( pipelinedrequests ) );

		const auto resp_seq = get_response_sequence( response );
		REQUIRE( 3 == resp_seq.size() );

		REQUIRE( 0 == resp_seq[ 0 ] );
		REQUIRE( 1 == resp_seq[ 1 ] );
		REQUIRE( 2 == resp_seq[ 2 ] );
	}

	{
		// Check on this thread.

		std::function< void (void ) > final_checks;
		std::thread helper_thread{
			[&](){
				const auto pipelinedrequests =
					create_request( 0 ) +
					create_request( 1 ) +
					create_request( 2 ) +
					create_request( 0 ) +
					create_request( 1, "close" );

				std::string response = do_request( pipelinedrequests );

				final_checks = [ response ](){
					const auto resp_seq =
						get_response_sequence( response );
					REQUIRE( 5 == resp_seq.size() );

					REQUIRE( 0 == resp_seq[ 0 ] );
					REQUIRE( 1 == resp_seq[ 1 ] );
					REQUIRE( 2 == resp_seq[ 2 ] );
					REQUIRE( 0 == resp_seq[ 3 ] );
					REQUIRE( 1 == resp_seq[ 4 ] );
				};

			} };

		// To ensure that requests from aux thread will be send earlier.
		std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

		std::string response;
		// Send 3rd reques through another connection.
		REQUIRE_NOTHROW(
			response = do_request( create_request( 2, "close" ) ) );

		// It must not contain responses on 1st and 2dn request
		// leaved in handler.
		const auto resp_seq =
			get_response_sequence( response );
		REQUIRE( 1 == resp_seq.size() );
		REQUIRE( 2 == resp_seq[ 0 ] );

		helper_thread.join();
		final_checks();
	}

	other_thread.stop_and_join();
}

TEST_CASE( "Long sequesnces HTTP piplining" , "[long_sequences]" )
{
	std::srand( static_cast<unsigned int>(std::time( nullptr )) );

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t,
				req_handler_t< 128 > > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )

				// Must have notable timeouts:
				.read_next_http_message_timelimit(
					std::chrono::hours( 24 ) )
				.handle_request_timeout( std::chrono::hours( 24 ) )

				.max_pipelined_requests( 128 );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	SECTION( "simple order" )
	{
		std::ostringstream sout;
		for( auto i = 0; i < 127; ++i )
		{
			sout << create_request( i );
		}

		sout << create_request( 127, "close" );

		std::string response;
		REQUIRE_NOTHROW( response = do_request( sout.str() ) );

		const auto resp_seq = get_response_sequence( response );
		REQUIRE( 128 == resp_seq.size() );

		for( auto i = 0; i < 128; ++i )
		{
			REQUIRE( i == resp_seq[ i ] );
		}
	}

	SECTION( "not direct order" )
	{
		std::ostringstream sout;
		sout << create_request( 0 );

		std::vector< unsigned int > seq;
		seq.push_back( 0 );
		for( auto i = 0; i < 120; ++i )
		{
			seq.push_back( 1 + std::rand()%100 );
			sout << create_request( seq.back() );
		}

		sout << create_request( 127, "close" );

		seq.push_back( 127 );

		std::string response;
		REQUIRE_NOTHROW( response = do_request( sout.str() ) );

		const auto resp_seq = get_response_sequence( response );
		REQUIRE( 122 == resp_seq.size() );

		for( auto i = 0; i < 122; ++i )
		{
			REQUIRE( seq[ i ] == resp_seq[ i ] );
		}
	}

	other_thread.stop_and_join();
}

TEST_CASE( "Interrupt sequesnces HTTP piplining" , "[long_sequences][interrupt]" )
{
	std::srand( static_cast<unsigned int>(std::time( nullptr )) );

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t,
				req_handler_t< 20 > > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )

				// Must have notable timeouts:
				.read_next_http_message_timelimit(
					std::chrono::hours( 24 ) )
				.handle_request_timeout( std::chrono::hours( 24 ) )

				.max_pipelined_requests( 20 );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::ostringstream sout;
	for( auto i = 0; i < 20; ++i )
	{
		if( i < 9 )
			sout << create_request( i );
		else
			sout << create_request( i, "close" );
	}

	std::string response;
	REQUIRE_NOTHROW( response = do_request( sout.str() ) );

	const auto resp_seq = get_response_sequence( response );
	REQUIRE( 10 == resp_seq.size() );

	for( auto i = 0; i < 10; ++i )
	{
		REQUIRE( i == resp_seq[ i ] );
	}

	other_thread.stop_and_join();
}

