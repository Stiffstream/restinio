/*
	restinio
*/

/*!
	Test upgrade request.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <asio.hpp>

#include <restinio/all.hpp>
#include <restinio/websocket.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

TEST_CASE( "Upgrade" , "[upgrade]" )
{
	using traits_t =
		restinio::traits_t<
			restinio::asio_timer_factory_t,
			utest_logger_t >;

	using http_server_t = restinio::http_server_t< traits_t >;

	http_server_t http_server{
		restinio::create_child_io_context( 1 ),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[]( auto req ){
						if( restinio::http_connection_header_t::upgrade == req->header().connection() )
						{
							auto ws =
								restinio::upgrade_to_websocket< traits_t >(
									*req,
									// TODO: make sec_websocket_accept_field_value
									std::string{ "sec_websocket_accept_field_value" },
									[]( restinio::ws_message_handle_t ){},
									[]( std::string ){} );

							ws->close();

							return restinio::request_accepted();
						}

						return restinio::request_rejected();
					} );
		} };

	http_server.open();

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

	REQUIRE_THAT( response, Catch::StartsWith( "HTTP/1.1 101 Switching Protocols" ) );
	REQUIRE_THAT( response, Catch::Contains( "Connection: Upgrade" ) );
	REQUIRE_THAT( response, Catch::Contains( "Sec-WebSocket-Accept:" ) );
	REQUIRE_THAT( response, Catch::Contains( "Upgrade: websocket" ) );

	http_server.close();
}
