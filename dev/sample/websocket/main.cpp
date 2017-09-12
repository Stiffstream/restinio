#include <type_traits>
#include <iostream>
#include <chrono>
#include <memory>

#include <asio.hpp>
#include <asio/ip/tcp.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <restinio/all.hpp>
#include <restinio/websocket.hpp>
#include <restinio/impl/base64.hpp>
#include <restinio/impl/sha1.hpp>
#include <restinio/impl/utf8.hpp>


namespace rr = restinio::router;
using router_t = rr::express_router_t;

using traits_t =
	restinio::traits_t<
		restinio::asio_timer_factory_t,
		restinio::single_threaded_ostream_logger_t,
		router_t >;

using http_server_t = restinio::http_server_t< traits_t >;

restinio::raw_data_t
to_char_each( std::vector< int > source )
{
	restinio::raw_data_t result;
	result.reserve( source.size() );

	for( const auto & val : source )
	{
		result.push_back( static_cast<char>(val) );
	}

	return result;
}

restinio::raw_data_t
status_code_to_bin( restinio::status_code_t code )
{
	return restinio::raw_data_t{
		to_char_each(
			{
				(static_cast<std::uint16_t>(code) >> 8) & 0xFF ,
				static_cast<std::uint16_t>(code) & 0xFF
			}
		) };
}

restinio::ws_message_t
create_close_msg(
	restinio::status_code_t code,
	const std::string & desc = std::string() )
{
	restinio::raw_data_t payload{
		restinio::impl::status_code_to_bin( code ) + desc };

	restinio::ws_message_t close_msg(
		true, restinio::opcode_t::connection_close_frame, payload );

	return close_msg;
}

void
ws_msg_handler(
	restinio::websocket_unique_ptr_t & websocket,
	restinio::ws_message_handle_t m )
{
	auto req = *m;
		auto opcode = req.header().m_opcode;

	if( opcode ==
		restinio::opcode_t::connection_close_frame )
	{
		websocket->close();
	}
	else if( opcode ==
		restinio::opcode_t::text_frame )
	{
		if( req.header().m_masking_key )
		{
			if( !restinio::impl::check_utf8_is_correct(
				req.payload() ) )
			{
				websocket->send_message( create_close_msg(
					restinio::status_code_t::invalid_message_data ) );
			}

			auto resp = req;

			resp.header().m_masking_key = 0;

			websocket->send_message( resp );
		}
		else
		{
			websocket->send_message( create_close_msg(
				restinio::status_code_t::protocol_error ) );

			websocket->close();
		}
	}
}

auto server_handler( restinio::websocket_unique_ptr_t & websocket )
{
	auto router = std::make_unique< router_t >();

	router->http_get(
		"/chat",
		[&]( auto req, auto ){

				if( restinio::http_connection_header_t::upgrade == req->header().connection() )
				{

					websocket =
						restinio::upgrade_to_websocket< traits_t >(
							*req,
							[&]( restinio::ws_message_handle_t m ){

								ws_msg_handler( websocket, m );
							},
							[]( std::string reason ){
								std::cout << "Close websocket: " << reason << std::endl;
							} );
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
		restinio::websocket_unique_ptr_t websocket;

		http_server_t http_server{
			restinio::create_child_io_context( 1 ),
			[&websocket]( auto & settings ){
				settings
					.address( "localhost" )
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
