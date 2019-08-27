/*
	restinio
*/

/*!
	Test method detection.
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>
#include <restinio/websocket/websocket.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

TEST_CASE( "remote_endpoint extraction" , "[remote_endpoint]" )
{
	std::string endpoint_value;

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[&endpoint_value]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
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

	REQUIRE( !endpoint_value.empty() );
}


TEST_CASE( "remote_endpoint for WS" , "[remote_endpoint][ws]" )
{
	std::string endpoint_value;
	std::string endpoint_value_ws;

	using traits_t =
		restinio::traits_t<
			restinio::asio_timer_manager_t,
			utest_logger_t >;

	using http_server_t =
		restinio::http_server_t< traits_t >;

	http_server_t http_server{
		restinio::own_io_context(),
		[&endpoint_value,&endpoint_value_ws]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[&endpoint_value, &endpoint_value_ws]( auto req ){
						endpoint_value = fmt::format( "{}", req->remote_endpoint() );

						if( restinio::http_connection_header_t::upgrade == req->header().connection() )
						{
							try
							{
								namespace rws = restinio::websocket::basic;
								auto ws =
									rws::upgrade< traits_t >(
										*req,
										rws::activation_t::immediate,
										[]( const rws::ws_handle_t&,
											const rws::message_handle_t& ){} );


								endpoint_value_ws = fmt::format( "{}", ws->remote_endpoint() );
								
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

	REQUIRE( !endpoint_value.empty() );
	REQUIRE( endpoint_value == endpoint_value_ws );
}
