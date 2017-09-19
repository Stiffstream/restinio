#include <type_traits>
#include <iostream>
#include <chrono>
#include <memory>

#include <asio.hpp>
#include <asio/ip/tcp.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <restinio/all.hpp>
#include <restinio/websocket/websocket.hpp>

namespace rr = restinio::router;
using router_t = rr::express_router_t;

namespace rws = restinio::websocket;

using traits_t =
	restinio::traits_t<
		restinio::asio_timer_factory_t,
		restinio::single_threaded_ostream_logger_t,
		router_t >;

using http_server_t = restinio::http_server_t< traits_t >;

void
print_ws_header( const rws::ws_message_header_t & header )
{
	std::cout <<
		"final: " << header.m_is_final <<
		", opcode: " << static_cast<int>(header.m_opcode) <<
		", payload_len: " << header.m_payload_len <<
		", masking_key: " << header.m_masking_key
		<< std::endl;
}

void
print_ws_message( const rws::ws_message_t & msg )
{
	std::cout << "header: {";

	print_ws_header(msg.header());

	std::cout << "}, payload: '" << msg.payload() << "' (";

	for( const auto ch : msg.payload() )
	{
		std::cout << std::hex << (static_cast< int >(ch) & 0xFF) << " ";
	}

	std::cout << ")" << std::endl;
}

auto server_handler( rws::ws_handle_t & websocket )
{
	auto router = std::make_unique< router_t >();

	router->http_get(
		"/",
		[&]( auto req, auto ){

			if( restinio::http_connection_header_t::upgrade == req->header().connection() )
			{
				websocket =
					rws::upgrade< traits_t >(
						*req,
						[]( auto wsh, auto m ){
							// print_ws_message( *m );
							// if( m->header().)

							if( m->header().m_opcode == rws::opcode_t::ping_frame )
							{
								if( m->header().m_payload_len > 125)
								{
									wsh->send_message(
										true,
										rws::opcode_t::ping_frame,
										rws::impl::status_code_to_bin(
											rws::status_code_t::protocol_error ) );
									wsh->close();
								}

								auto pong = *m;
								pong.header().m_opcode = rws::opcode_t::pong_frame;
								wsh->send_message( pong );
							}
							else if( m->header().m_opcode == rws::opcode_t::pong_frame )
							{
							}
							else if( m->header().m_opcode == rws::opcode_t::connection_close_frame )
							{
								wsh->close();
							}
							else
							{
								wsh->send_message( *m );
							}
						},
						[]( std::string reason ){
							std::cout << "Close websocket: " << reason << std::endl;
						} );

				return restinio::request_accepted();
			}

			return restinio::request_rejected();
		} );

	return router;
}

int main()
{
	using namespace std::chrono;

	try
	{
		rws::ws_handle_t websocket;

		http_server_t http_server{
			restinio::create_child_io_context( 1 ),
			[&websocket]( auto & settings ){
				settings
					.address( "localhost" )
					.port( 9001 )
					.request_handler( server_handler( websocket ) )
					.read_next_http_message_timelimit( 10s )
					.write_http_response_timelimit( 1s )
					.handle_request_timeout( 1s );
			} };

		http_server.open();

		// Wait for quit command.
		std::cout << "Type \"quit\" or \"q\" to quit." << std::endl;

		std::string cmd;
		do
		{
			std::cin >> cmd;
		} while( cmd != "quit" && cmd != "q" );

		websocket->close();
		http_server.close();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
