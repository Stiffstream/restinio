/*
	restinio
*/

/*!
	Test upgrade request.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <asio.hpp>

#include <openssl/sha.h>

#include <so_5/all.hpp>
#include <restinio/all.hpp>
#include <restinio/websocket.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>


// using namespace restinio;
// using namespace restinio::impl;

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}
std::string base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

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

using traits_t =
	restinio::traits_t<
		restinio::asio_timer_factory_t,
		utest_logger_t >;

using http_server_t = restinio::http_server_t< traits_t >;


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
};

class a_server_t
	:	public so_5::agent_t
{
		using so_base_type_t = so_5::agent_t;

	public:
		a_server_t(
			context_t ctx,
			so_5::mbox_t client_mbox )
			:	so_base_type_t{ ctx }
			,	m_client_mbox{ std::move(client_mbox) }
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
				.event( &a_server_t::evt_ws_message );
		}

	private:

		void
		evt_ws_message( const msg_ws_message & msg )
		{
			print_ws_message( *(msg.m_msg) );

			auto resp = *(msg.m_msg);

			resp.header().m_masking_key = 0;

			m_ws->send_message( resp );
		}

		restinio::default_request_handler_t m_req_handler =
			[this]( auto req )
			{
				if( restinio::http_connection_header_t::upgrade == req->header().connection() )
				{
					auto ws_key = req->header().get_field("Sec-WebSocket-Key");

					ws_key.append( "258EAFA5-E914-47DA-95CA-C5AB0DC85B11" );

					unsigned char hash[ SHA_DIGEST_LENGTH ];
					SHA1( reinterpret_cast< const unsigned char* >(
						ws_key.data( ) ), ws_key.length( ), hash );

					m_ws =
						restinio::upgrade_to_websocket< traits_t >(
							*req,
							base64_encode( hash, SHA_DIGEST_LENGTH ),
							[this]( restinio::ws_message_handle_t m ){
									so_5::send<msg_ws_message>(
										this->so_direct_mbox(), m );
								},
							[]( std::string reason ){} );

					return restinio::request_accepted();
				}

				return restinio::request_rejected();
			};

		so_5::mbox_t m_client_mbox;

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
			do_with_socket( [ & ]( auto & socket, auto & io_context ){

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

				std::string response{ data.data(), len };
				// std::cout << "RESPONSE: " << response << std::endl;

				// unsigned char msg1[] = { 0x81, 0x05, 'H', 'e', 'l', 'l', 'o' };
				unsigned char msg1[] = {
					0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d, 0x7f, 0x9f, 0x4d, 0x51, 0x58 };

				auto raw_data = restinio::impl::write_message_details(
					to_ws_message_details(m_etalon_request) );

				std::string masked_payload = m_etalon_request.payload();

				restinio::impl::mask_unmask_payload(
					m_etalon_request.header().m_masking_key,
					masked_payload );

				raw_data.append( masked_payload );

				REQUIRE_NOTHROW(
						asio::write(
							socket, asio::buffer( msg1, sizeof( msg1 ) ) )
							// socket, asio::buffer( raw_data, raw_data.size() ) )
						);

				REQUIRE_NOTHROW(
					len = socket.read_some( asio::buffer( data.data(), data.size() ) )
					);

				response = std::string{ data.data(), len };

				restinio::impl::ws_parser_t parser;

				auto parsed = parser.parser_execute( data.data(), len );

				if( parser.header_parsed() )
				{
					restinio::ws_message_t resp{
						parser.current_message().transform_to_header(),
						std::string(
							data.data() + (
								len - parser.current_message().payload_len()),
							parser.current_message().payload_len() )
					};

					print_ws_message( resp );
				}
			} );

			so_environment().stop();
		}

		request_response_context_t & m_result;

		const restinio::ws_message_t m_etalon_request{
			restinio::ws_message_header_t{
				true,
				restinio::opcode_t::text_frame,
				5, 128 },
			"Hello" } ;
};


request_response_context_t
client_server_ws_connection()
{
	request_response_context_t res;

	try
	{

		so_5::launch(
			[&]( auto & env )
			{
				env.introduce_coop(
					so_5::disp::active_obj::create_private_disp( env )->binder(),
					[ & ]( so_5::coop_t & coop ) {

						auto client_mbox =
							coop.make_agent< a_client_t >( res )->so_direct_mbox();
						coop.make_agent< a_server_t >( client_mbox );
					} );
			},
			[]( so_5::environment_params_t & params )
			{
			} );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
	}

	return res;
}

TEST_CASE( "Request/Response comparision" , "[ws_connection]" )
{
	auto ctx = client_server_ws_connection();

	REQUIRE( ctx.m_request == ctx.m_response );
}
