/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>
#include <restinio/websocket/websocket.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

struct state_listener_t
{
	std::atomic< int > m_accepted{ 0 };
	std::atomic< int > m_closed{ 0 };
	std::atomic< int > m_upgraded_to_websocket{ 0 };

	void state_changed(
		const restinio::connection_state::notice_t & notice )
	{
		switch( notice.cause() )
		{
			case restinio::connection_state::cause_t::accepted:
				++m_accepted;
			break;

			case restinio::connection_state::cause_t::closed:
				++m_closed;
			break;

			case restinio::connection_state::cause_t::upgraded_to_websocket:
				++m_upgraded_to_websocket;
			break;
		}
	}
};

TEST_CASE( "ordinary connection" , "[ordinary_connection]" )
{
	std::string endpoint_value;

	struct test_traits : public restinio::traits_t<
			restinio::asio_timer_manager_t,
			utest_logger_t >
	{
		using connection_state_listener_t = state_listener_t;
	};

	using http_server_t = restinio::http_server_t< test_traits >; 

	auto state_listener = std::make_shared< state_listener_t >();

	http_server_t http_server{
		restinio::own_io_context(),
		[&endpoint_value, state_listener]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.connection_state_listener( state_listener )
				.request_handler(
					[&endpoint_value]( auto req ){
						endpoint_value = fmt::format( "{}", req->remote_endpoint() );

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

	other_thread.stop_and_join();

	REQUIRE( "" != endpoint_value );
	REQUIRE( 1 == state_listener->m_accepted.load() );
	REQUIRE( 1 == state_listener->m_closed.load() );
	REQUIRE( 0 == state_listener->m_upgraded_to_websocket.load() );
}

TEST_CASE( "remote_endpoint for WS" , "[remote_endpoint][ws]" )
{
	std::string endpoint_value;
	std::string endpoint_value_ws;

	struct test_traits : public restinio::traits_t<
			restinio::asio_timer_manager_t,
			utest_logger_t >
	{
		using connection_state_listener_t = state_listener_t;
	};

	using http_server_t = restinio::http_server_t< test_traits >; 

	auto state_listener = std::make_shared< state_listener_t >();

	http_server_t http_server{
		restinio::own_io_context(),
		[&endpoint_value, &endpoint_value_ws, state_listener]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.connection_state_listener( state_listener )
				.request_handler(
					[&endpoint_value, &endpoint_value_ws]( auto req ){
						endpoint_value = fmt::format( "{}", req->remote_endpoint() );

						if( restinio::http_connection_header_t::upgrade == req->header().connection() )
						{
							try
							{
								namespace rws = restinio::websocket::basic;
								auto ws =
									rws::upgrade< test_traits >(
										*req,
										rws::activation_t::immediate,
										[]( rws::ws_handle_t,
											rws::message_handle_t ){} );


								endpoint_value_ws = fmt::format( "{}", ws->remote_endpoint() );
								// TODO: write close-message.
								ws->kill();
								return restinio::request_accepted();
							}
							catch( const std::exception & ex )
							{
								std::cout << "UPGRADE FAILED: "
									<< ex.what() << std::endl;
							}
						}
						return restinio::request_rejected();
					} );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::string response;
	const char * request_str =
		"GET /chat HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n"
		"Sec-WebSocket-Protocol: chat\r\n"
		"Sec-WebSocket-Version: 1\r\n"
		"User-Agent: unit-test\r\n"
		"\r\n";


	REQUIRE_NOTHROW( response = do_request( request_str ) );

	other_thread.stop_and_join();

	REQUIRE( "" != endpoint_value );
	REQUIRE( endpoint_value == endpoint_value_ws );

	REQUIRE( 1 == state_listener->m_accepted.load() );
	REQUIRE( 0 == state_listener->m_closed.load() );
	REQUIRE( 1 == state_listener->m_upgraded_to_websocket.load() );
}

