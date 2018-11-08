
/*
	restinio
*/

/*!
	Test upgrade request.
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>
#include <restinio/websocket/websocket.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

TEST_CASE( "Upgrade" , "[upgrade]" )
{
	using traits_t =
		restinio::traits_t<
			restinio::asio_timer_manager_t,
			utest_logger_t >;

	using http_server_t = restinio::http_server_t< traits_t >;
	namespace rws = restinio::websocket;

	http_server_t http_server{
		restinio::own_io_context(),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[]( auto req ){
						if( restinio::http_connection_header_t::upgrade == req->header().connection() )
						{
							try
							{
								namespace rws = restinio::websocket::basic;
								auto ws =
									rws::upgrade< traits_t >(
										*req,
										rws::activation_t::immediate,
										[]( rws::ws_handle_t,
											rws::message_handle_t ){} );

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

	other_work_thread_for_server_t<http_server_t> other_thread{http_server};

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

	REQUIRE_THAT( response, Catch::StartsWith( "HTTP/1.1 101 Switching Protocols" ) );
	REQUIRE_THAT( response, Catch::Contains( "Connection: Upgrade" ) );
	REQUIRE_THAT( response, Catch::Contains( "Sec-WebSocket-Accept:" ) );
	REQUIRE_THAT( response, Catch::Contains( "Upgrade: websocket" ) );

	other_thread.stop_and_join();
}
