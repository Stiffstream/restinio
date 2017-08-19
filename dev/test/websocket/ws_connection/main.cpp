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

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>


// using namespace restinio;
// using namespace restinio::impl;

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

struct srv_started : public so_5::signal_t {};

struct msg_ws_message : public so_5::message_t
{
	msg_ws_message( restinio::ws_message_handle_t msg )
	:	m_msg{ msg }
	{
	}

	restinio::ws_message_handle_t m_msg;
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
							.request_handler(
								[this]( auto req ){
									if( restinio::http_connection_header_t::upgrade == req->header().connection() )
									{
										m_ws =
											restinio::upgrade_to_websocket< traits_t >(
												*req,
												// TODO: make sec_websocket_accept_field_value
												std::string{ "sec_websocket_accept_field_value" },
												[this]( restinio::ws_message_handle_t m ){
														so_5::send<msg_ws_message>(
															this->so_direct_mbox(), m );
													},
												[]( std::string reason ){} );

										return restinio::request_accepted();
									}

									return restinio::request_rejected();
								} );
					} }
		{}

		virtual void
		so_define_agent() override
		{
			so_subscribe_self()
				.event( &a_server_t::evt_ws_message );
		}

		virtual void
		so_evt_start() override
		{
			std::cout << "SERVER\n";
			http_server.open();

			so_5::send<srv_started>(m_client_mbox);
		}

	private:

		void
		evt_ws_message( const msg_ws_message & msg )
		{
			std::cout << "WS MESSAGE\n";

			// REQUIRE( *(msg.m_msg) == m_etalon_request );

			print_ws_message( *(msg.m_msg) );

			m_ws->send_message(
				true, restinio::opcode_t::text_frame, restinio::buffer_storage_t("Hi"));

			// so_environment().stop();
		}

		const restinio::ws_message_t m_etalon_request{
			restinio::ws_message_header_t{
				true,
				restinio::opcode_t::text_frame,
				5 },
			"Hello" } ;

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
			context_t ctx )
			:	so_base_type_t{ ctx }
		{}

		virtual void
		so_define_agent() override
		{
			so_subscribe_self()
				.event< srv_started >( &a_client_t::evt_srv_started );
		}

		virtual void
		so_evt_start() override
		{
			std::cout << "CLIENT\n";
		}

	private:

		void evt_srv_started()
		{
			std::cout << "SRV STARTED\n";

			do_with_socket( [ & ]( auto & socket, auto & io_context ){

				const std::string request{
					"GET /chat HTTP/1.1\r\n"
					"Host: 127.0.0.1\r\n"
					"Upgrade: websocket\r\n"
					"Connection: Upgrade\r\n"
					"Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n"
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
				std::cout << "RESPONSE: " << response << std::endl;

				unsigned char msg1[] = { 0x81, 0x05, 'H', 'e', 'l', 'l', 'o' };

				REQUIRE_NOTHROW(
						asio::write(
							socket, asio::buffer( msg1, sizeof( msg1 ) ) )
						);

				REQUIRE_NOTHROW(
					len = socket.read_some( asio::buffer( data.data(), data.size() ) )
					);

				response = std::string{ data.data(), len };
				std::cout << "RESPONSE: " << response << std::endl;
			} );
		}
};

TEST_CASE( "Websocket" , "[ws_connection]" )
{
	try
	{
		so_5::launch(
			[&]( auto & env )
			{
				env.introduce_coop(
					so_5::disp::active_obj::create_private_disp( env )->binder(),
					[ & ]( so_5::coop_t & coop ) {

						auto client_mbox =
							coop.make_agent< a_client_t >()->so_direct_mbox();
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
}
