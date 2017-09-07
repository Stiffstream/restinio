/*
	restinio
*/

/*!
	Test upgrade request.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <asio.hpp>

#include <so_5/all.hpp>
#include <restinio/all.hpp>
#include <restinio/websocket.hpp>
#include <restinio/impl/base64.hpp>
#include <restinio/impl/sha1.hpp>
#include <restinio/impl/utf8.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

char
to_char( int val )
{
	return static_cast<char>(val);
};

restinio::raw_data_t
to_char_each( std::vector< int > source )
{
	restinio::raw_data_t result;
	result.reserve( source.size() );

	for( const auto & val : source )
	{
		result.push_back( to_char(val) );
	}

	return result;
}

restinio::impl::ws_message_details_t
to_ws_message_details( const restinio::ws_message_t & msg )
{
	return restinio::impl::ws_message_details_t{
		msg.header().m_is_final,
		msg.header().m_opcode,
		msg.payload().size(),
		msg.header().m_masking_key };
}

bool
operator==(
	const restinio::ws_message_header_t & lhs,
	const restinio::ws_message_header_t & rhs )
{
	return
		lhs.m_is_final == rhs.m_is_final &&
		lhs.m_opcode == rhs.m_opcode &&
		lhs.m_payload_len == rhs.m_payload_len &&
		lhs.m_masking_key == rhs.m_masking_key;
}

bool
operator==(
	const restinio::ws_message_t & lhs,
	const restinio::ws_message_t & rhs )
{
	return
		lhs.header() == rhs.header() &&
		lhs.payload() == rhs.payload();
}

void
print_ws_header( const restinio::ws_message_header_t & header )
{
	std::cout <<
		"final: " << header.m_is_final <<
		", opcode: " << static_cast<int>(header.m_opcode) <<
		", payload_len: " << header.m_payload_len <<
		", masking_key: " << header.m_masking_key
		<< std::endl;
}

void
print_ws_message( const restinio::ws_message_t & msg )
{
	std::cout << "header: {";

	print_ws_header(msg.header());

	std::cout << "}, payload: '" << msg.payload() << "' (";

	for( const auto ch : msg.payload() )
	{
		std::cout << std::hex << static_cast< int >(ch) << " ";
	}

	std::cout << ")" << std::endl;
}

restinio::ws_message_t
parse_bin_data( const char* data, size_t len )
{
	restinio::impl::ws_parser_t parser;

	auto parsed = parser.parser_execute( data, len );

	if( parser.header_parsed() )
	{
		restinio::ws_message_t result{
			parser.current_message().transform_to_header(),
			std::string(
				data + (
					len - parser.current_message().payload_len()),
				parser.current_message().payload_len() )
		};

		print_ws_message( result );

		return result;

	}
	else
	{
		throw std::runtime_error("Invalid bin data");
	}
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
	restinio::raw_data_t payload{status_code_to_bin( code ) + desc };

	restinio::ws_message_t close_msg(
		true, restinio::opcode_t::connection_close_frame, payload );

	return close_msg;
}

using traits_t =
	restinio::traits_t<
		restinio::asio_timer_factory_t,
		utest_logger_t >;

using http_server_t = restinio::http_server_t< traits_t >;


struct upgrade_request_t : public so_5::message_t
{
    upgrade_request_t( restinio::request_handle_t req )
        :    m_req{ std::move( req ) }
    {}

    restinio::request_handle_t m_req;
};

struct msg_ws_message : public so_5::message_t
{
	msg_ws_message( restinio::ws_message_handle_t msg )
	:	m_msg{ msg }
	{
	}

	restinio::ws_message_handle_t m_msg;
};

struct
request_response_context_t
{
	restinio::ws_message_t m_request;
	restinio::ws_message_t m_response;
	std::string m_request_bin;
	std::string m_response_bin;

	int close_code = 0;
	std::string m_close_description;
};

std::array< char, 20>
digest_to_char_array( const restinio::impl::sha1::digest_t & digest )
{
	std::array< char, 20> result;

	size_t i = 0;

	for( const auto c : digest )
	{
		result[i++] = ( (c >>  24) & 0xFF );
		result[i++] = ( (c >>  16) & 0xFF );
		result[i++] = ( (c >>  8) & 0xFF );
		result[i++] = ( (c) & 0xFF );
	}

	return result;
}

class a_server_t
	:	public so_5::agent_t
{
		using so_base_type_t = so_5::agent_t;

	public:
		a_server_t(
			context_t ctx )
			:	so_base_type_t{ ctx }
			,	http_server{
					restinio::create_child_io_context( 1 ),
					[this]( auto & settings ){
						settings
							.port( utest_default_port() )
							.address( "127.0.0.1" )
							.request_handler( m_req_handler );
					} }
		{
			http_server.open();
		}

		virtual void
		so_define_agent() override
		{
			so_subscribe_self()
				.event( &a_server_t::evt_upgrade_request )
				.event( &a_server_t::evt_ws_message );
		}

	private:

		void
		evt_upgrade_request( const upgrade_request_t & msg )
		{
			auto req = msg.m_req;
			auto ws_key = req->header().get_field("Sec-WebSocket-Key");

			ws_key.append( "258EAFA5-E914-47DA-95CA-C5AB0DC85B11" );

			auto digest = restinio::impl::sha1::make_digest( ws_key );

			m_ws =
				restinio::upgrade_to_websocket< traits_t >(
					*req,
					restinio::impl::base64::encode(
						std::string{
							digest_to_char_array(digest).data(), 20
						} ),
					[this]( restinio::ws_message_handle_t m ){
							so_5::send<msg_ws_message>(
								this->so_direct_mbox(), m );
						},
					[]( std::string /*reason*/ ){} );
		}

		void
		evt_ws_message( const msg_ws_message & msg )
		{
			auto req = *(msg.m_msg);

			if( req.header().m_masking_key )
			{
				if( !restinio::impl::check_utf8_is_correct( req.payload() ) )
				{
					m_ws->send_message( create_close_msg(
						restinio::status_code_t::invalid_message_data ) );
				}

				auto resp = req;

				resp.header().m_masking_key = 0;

				m_ws->send_message( resp );
			}
			else
			{
				m_ws->send_message( create_close_msg(
					restinio::status_code_t::protocol_error ) );

				m_ws->close();
			}
		}

		restinio::default_request_handler_t m_req_handler =
			[this]( restinio::request_handle_t req )
			{
				if( restinio::http_connection_header_t::upgrade == req->header().connection() )
				{
					so_5::send< upgrade_request_t >( this->so_direct_mbox(), req );

					return restinio::request_accepted();
				}

				return restinio::request_rejected();
			};

		http_server_t http_server;

		restinio::websocket_unique_ptr_t m_ws;
};


class a_client_t
	:	public so_5::agent_t
{
		using so_base_type_t = so_5::agent_t;

	public:
		a_client_t(
			context_t ctx,
			request_response_context_t & result )
			:	so_base_type_t{ ctx }
			,	m_result{ result }
		{}

		virtual void
		so_evt_start() override
		{
			init_connection_with_srv();
		}

	private:

		void init_connection_with_srv()
		{
			do_with_socket( [ & ]( auto & socket, auto & /*io_context*/ ){

				const std::string request{
					"GET /chat HTTP/1.1\r\n"
					"Host: 127.0.0.1\r\n"
					"Upgrade: websocket\r\n"
					"Connection: Upgrade\r\n"
					"Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
					"Sec-WebSocket-Protocol: chat\r\n"
					"Sec-WebSocket-Version: 1\r\n"
					"User-Agent: unit-test\r\n"
					"\r\n" };


				REQUIRE_NOTHROW(
					asio::write(
						socket, asio::buffer( &request.front(), request.size() ) )
					);

				std::array< char, 1024 > data;

				std::size_t len{ 0 };
				REQUIRE_NOTHROW(
					len = socket.read_some( asio::buffer( data.data(), data.size() ) )
					);

				REQUIRE_NOTHROW(
						asio::write(
							socket, asio::buffer(
								m_result.m_request_bin.data(),
								m_result.m_request_bin.size() ) )
						);

				REQUIRE_NOTHROW(
					len = socket.read_some( asio::buffer( data.data(), data.size() ) )
					);

				m_result.m_response = parse_bin_data( data.data(), len );
				m_result.m_response_bin = std::string( data.data(), len ) ;
			} );

			so_environment().stop();
		}

		request_response_context_t & m_result;
};


request_response_context_t
client_server_ws_connection( const std::string & req_bin )
{
	request_response_context_t rr_ctx;

	rr_ctx.m_request_bin = req_bin;
	rr_ctx.m_request = parse_bin_data(req_bin.data(), req_bin.size());

	try
	{

		so_5::launch(
			[&]( auto & env )
			{
				env.introduce_coop(
					so_5::disp::active_obj::create_private_disp( env )->binder(),
					[ & ]( so_5::coop_t & coop ) {
						coop.make_agent< a_client_t >( rr_ctx );
						coop.make_agent< a_server_t >( );
					} );
			},
			[]( so_5::environment_params_t & /*params*/ )
			{
			} );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
	}

	return rr_ctx;
}

TEST_CASE( "Request/Response echo" , "[ws_connection]" )
{
	restinio::raw_data_t bin_data{ to_char_each(
			{0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d, 0x7f, 0x9f, 0x4d, 0x51, 0x58}) };

	auto ctx = client_server_ws_connection( bin_data );

	auto request_payload = ctx.m_request.payload();
	restinio::impl::mask_unmask_payload(
			ctx.m_request.header().m_masking_key, request_payload );

	REQUIRE( request_payload == ctx.m_response.payload() );
}

TEST_CASE( "Request/Response close without masking key" , "[ws_connection]" )
{
	restinio::raw_data_t bin_data{ to_char_each(
			{0x81, 0x05, 'H', 'e', 'l', 'l', 'o'}) };

	auto ctx = client_server_ws_connection( bin_data );

	REQUIRE( ctx.m_response.header().m_opcode ==
		restinio::opcode_t::connection_close_frame );


}

TEST_CASE( "Request/Response close with non utf-8 payload" , "[ws_connection]" )
{
	restinio::raw_data_t bin_data{ to_char_each(
			{0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d, 'H', 'e', 'l', 'l', 'o'} ) };

	auto ctx = client_server_ws_connection( bin_data );

	REQUIRE( ctx.m_response.header().m_opcode ==
		restinio::opcode_t::connection_close_frame );


}
