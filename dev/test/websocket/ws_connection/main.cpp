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
#include <restinio/websocket/websocket.hpp>
#include <restinio/utils/base64.hpp>
#include <restinio/utils/sha1.hpp>
#include <restinio/websocket/impl/utf8.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>
#include <test/websocket/common/pub.hpp>

namespace rws = restinio::websocket;

using traits_t =
	restinio::traits_t<
		restinio::asio_timer_factory_t,
		utest_logger_t >;

using http_server_t = restinio::http_server_t< traits_t >;

struct upgrade_request_t : public so_5::message_t
{
	upgrade_request_t( restinio::request_handle_t req )
		:	m_req{ std::move( req ) }
	{}

	restinio::request_handle_t m_req;
};

struct msg_ws_message_t : public so_5::message_t
{
	msg_ws_message_t( rws::message_handle_t msg )
		:	m_msg{ msg }
	{}

	rws::message_handle_t m_msg;
};

class a_server_t
	:	public so_5::agent_t
{
		using so_base_type_t = so_5::agent_t;

	public:
		a_server_t(
			context_t ctx )
			:	so_base_type_t{ ctx }
			,	m_http_server{
					restinio::create_child_io_context( 1 ),
					[this]( auto & settings ){
						auto mbox = so_direct_mbox();
						settings
							.port( utest_default_port() )
							.address( "127.0.0.1" )
							.request_handler( [mbox]( auto req )
								{
									if( restinio::http_connection_header_t::upgrade ==
										req->header().connection() )
									{
										so_5::send< upgrade_request_t >( mbox, req );

										return restinio::request_accepted();
									}

									return restinio::request_rejected();
								} );
					} }
		{
			m_http_server.open();
		}

		virtual void
		so_define_agent() override
		{
			so_subscribe_self()
				.event( &a_server_t::evt_upgrade_request )
				.event( &a_server_t::evt_ws_message );
		}

		virtual void
		so_evt_finish() override
		{
			if( m_ws )
			{
				m_ws->send_message( true, rws::opcode_t::connection_close_frame, "" );
				m_ws->shutdown();
				m_ws.reset();
			}
			m_http_server.close();
		}

	private:
		void
		evt_upgrade_request( const upgrade_request_t & msg )
		{
			auto req = msg.m_req;
			auto ws_key = req->header().get_field("Sec-WebSocket-Key");

			ws_key.append( "258EAFA5-E914-47DA-95CA-C5AB0DC85B11" );

			auto digest = restinio::utils::sha1::make_digest( ws_key );

			m_ws =
				rws::upgrade< traits_t >(
					*req,
					rws::activation_t::immediate,
					[mbox = so_direct_mbox()]( auto /* ws_handle*/, rws::message_handle_t m ){
						so_5::send< msg_ws_message_t >( mbox, m );
					} );
		}

		void
		evt_ws_message( const msg_ws_message_t & msg )
		{
			if( m_ws )
			{
				auto & req = *(msg.m_msg);

				if( rws::opcode_t::text_frame == req.opcode() ||
					rws::opcode_t::binary_frame == req.opcode() )
				{
					if( req.payload() == "close" )
					{
						m_ws->send_message(
							true,
							rws::opcode_t::connection_close_frame,
							rws::status_code_to_bin( rws::status_code_t::normal_closure ) );
					}
					else if( req.payload() == "shutdown" )
					{
						m_ws->shutdown();
						m_ws.reset();
					}
					else if( req.payload() == "kill" )
					{
						m_ws->kill();
						m_ws.reset();
					}
					else
					{
						auto resp = req;
						m_ws->send_message( resp );
					}
				}
				else if( rws::opcode_t::ping_frame == req.opcode() )
				{
					auto resp = req;
					resp.set_opcode( rws::opcode_t::pong_frame );
					m_ws->send_message( resp );
				}
				// else if( rws::opcode_t::pong_frame == req.opcode() )
				// {
				// 	// ?
				// }
				else if( rws::opcode_t::connection_close_frame == req.opcode() )
				{
					// TODO: print status code.
					m_ws.reset();
				}
			}
		}

		http_server_t m_http_server;
		rws::ws_handle_t m_ws;
};

const std::string upgrade_request{
	"GET /chat HTTP/1.1\r\n"
	"Host: 127.0.0.1\r\n"
	"Upgrade: websocket\r\n"
	"Connection: Upgrade\r\n"
	"Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
	"Sec-WebSocket-Protocol: chat\r\n"
	"Sec-WebSocket-Version: 1\r\n"
	"User-Agent: unit-test\r\n"
	"\r\n" };

class soenv_t : public so_5::environment_t
{
	public:
	using base_type_t = so_5::environment_t;

	using base_type_t::base_type_t;

	private:
		virtual void
		init() override
		{
			introduce_coop(
				so_5::disp::active_obj::create_private_disp( *this )->binder(),
				[ & ]( so_5::coop_t & coop ) {
					coop.make_agent< a_server_t >();
				} );
		}
};

std::thread
start_soenv_in_separate_thread( so_5::environment_t & env )
{
	std::thread soenv_thread{ [&](){
		try
		{
			env.run();
		}
		catch( const std::exception & ex )
		{
			std::cerr << "Error running sobjectizer: " << ex.what() << std::endl;
		}
	} };

	// Give some time for server to start.
	std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );

	return soenv_thread;
}

TEST_CASE( "Simple echo" , "[ws_connection][echo][normal_close]" )
{

	soenv_t soenv{ so_5::environment_params_t{} };
	auto soenv_thread = start_soenv_in_separate_thread( soenv );

	do_with_socket( [ & ]( auto & socket, auto & /*io_context*/ ){
		std::vector< std::uint8_t > bin_ws_msg_data =
				{ 0x81, 0x85, 0xAA,0xBB,0xCC,0xDD,
				  0xAA ^ 'H', 0xBB ^ 'e', 0xCC ^ 'l', 0xDD ^ 'l', 0xAA ^ 'o' };

		REQUIRE_NOTHROW(
				asio::write(
					socket, asio::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

		std::array< std::uint8_t, 1024 > data;

		std::size_t len{ 0 };
		REQUIRE_NOTHROW(
				len = socket.read_some( asio::buffer( data.data(), data.size() ) )
			);

		REQUIRE_NOTHROW(
				asio::write( socket, asio::buffer( bin_ws_msg_data.data(), bin_ws_msg_data.size() ) )
			);

		REQUIRE_NOTHROW(
				len = socket.read_some( asio::buffer( data.data(), data.size() ) )
			);

		REQUIRE( 7 == len );
		REQUIRE( 0x81 == data[ 0 ] );
		REQUIRE( 0x05 == data[ 1 ] );
		REQUIRE( 'H' == data[ 2 ] );
		REQUIRE( 'e' == data[ 3 ] );
		REQUIRE( 'l' == data[ 4 ] );
		REQUIRE( 'l' == data[ 5 ] );
		REQUIRE( 'o' == data[ 6 ] );

		std::vector< std::uint8_t > close_frame =
			{0x88, 0x82, 0xFF,0xFF,0xFF,0xFF, 0xFF ^ 0x03, 0xFF ^ 0xe8 };

		REQUIRE_NOTHROW(
				asio::write(
					socket, asio::buffer( close_frame.data(), close_frame.size() ) )
			);

		REQUIRE_NOTHROW(
				len = socket.read_some( asio::buffer( data.data(), data.size() ) )
			);
		REQUIRE( 4 == len );
		REQUIRE( 0x88 == data[ 0 ] );
		REQUIRE( 0x02 == data[ 1 ] );
		REQUIRE( 0x03 == data[ 2 ] );
		REQUIRE( 0xe8 == data[ 3 ] );

		asio::error_code ec;
		len = socket.read_some( asio::buffer( data.data(), data.size() ), ec );
		REQUIRE( ec );
		REQUIRE( asio::error::eof == ec.value() );
	} );

	soenv.stop();
	soenv_thread.join();
}

TEST_CASE( "Ping" , "[ws_connection][ping][normal_close]" )
{

	soenv_t soenv{ so_5::environment_params_t{} };
	auto soenv_thread = start_soenv_in_separate_thread( soenv );

	do_with_socket( [ & ]( auto & socket, auto & /*io_context*/ ){
		std::vector< std::uint8_t > bin_ws_msg_data =
				{ 0x89, 0x84, 0x0A,0xB0,0x0C,0xD0,
				  0x0A ^ 'P', 0xB0 ^ 'i', 0x0C ^ 'n', 0xD0 ^ 'g' };

		REQUIRE_NOTHROW(
				asio::write(
					socket, asio::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

		std::array< std::uint8_t, 1024 > data;

		std::size_t len{ 0 };
		REQUIRE_NOTHROW(
				len = socket.read_some( asio::buffer( data.data(), data.size() ) )
			);

		REQUIRE_NOTHROW(
				asio::write( socket, asio::buffer( bin_ws_msg_data.data(), bin_ws_msg_data.size() ) )
			);

		REQUIRE_NOTHROW(
				len = socket.read_some( asio::buffer( data.data(), data.size() ) )
			);

		REQUIRE( 6 == len );
		REQUIRE( 0x8A == data[ 0 ] );
		REQUIRE( 0x04 == data[ 1 ] );
		REQUIRE( 'P' == data[ 2 ] );
		REQUIRE( 'i' == data[ 3 ] );
		REQUIRE( 'n' == data[ 4 ] );
		REQUIRE( 'g' == data[ 5 ] );

		std::vector< std::uint8_t > close_frame =
			{0x88, 0x82, 0xFF,0xFF,0xFF,0xFF, 0xFF ^ 0x03, 0xFF ^ 0xe8 };

		REQUIRE_NOTHROW(
				asio::write(
					socket, asio::buffer( close_frame.data(), close_frame.size() ) )
			);

		REQUIRE_NOTHROW(
				len = socket.read_some( asio::buffer( data.data(), data.size() ) )
			);
		REQUIRE( 4 == len );
		REQUIRE( 0x88 == data[ 0 ] );
		REQUIRE( 0x02 == data[ 1 ] );
		REQUIRE( 0x03 == data[ 2 ] );
		REQUIRE( 0xe8 == data[ 3 ] );

		asio::error_code ec;
		len = socket.read_some( asio::buffer( data.data(), data.size() ), ec );
		REQUIRE( ec );
		REQUIRE( asio::error::eof == ec.value() );
	} );

	soenv.stop();
	soenv_thread.join();
}

TEST_CASE( "Close" , "[ws_connection][close][normal_close]" )
{
	soenv_t soenv{ so_5::environment_params_t{} };
	auto soenv_thread = start_soenv_in_separate_thread( soenv );

	do_with_socket( [ & ]( auto & socket, auto & /*io_context*/ ){

		std::vector< std::uint8_t > bin_ws_msg_data =
				{ 0x81, 0x85, 0x0A,0xB0,0x0C,0xD0,
				  0x0A ^ 'c', 0xB0 ^ 'l', 0x0C ^ 'o', 0xD0 ^ 's', 0x0A ^ 'e' };

		REQUIRE_NOTHROW(
				asio::write(
					socket, asio::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

		std::array< std::uint8_t, 1024 > data;

		std::size_t len{ 0 };
		REQUIRE_NOTHROW(
				len = socket.read_some( asio::buffer( data.data(), data.size() ) )
			);

		REQUIRE_NOTHROW(
				asio::write( socket, asio::buffer( bin_ws_msg_data.data(), bin_ws_msg_data.size() ) )
			);

		REQUIRE_NOTHROW(
				len = socket.read_some( asio::buffer( data.data(), data.size() ) )
			);

		REQUIRE( 4 == len );
		REQUIRE( 0x88 == data[ 0 ] );
		REQUIRE( 0x02 == data[ 1 ] );
		REQUIRE( 0x03 == data[ 2 ] );
		REQUIRE( 0xe8 == data[ 3 ] );

		std::vector< std::uint8_t > close_frame =
			{0x88, 0x82, 0xFF,0xFF,0xFF,0xFF, 0xFF ^ 0x03, 0xFF ^ 0xe8 };

		REQUIRE_NOTHROW(
				asio::write(
					socket, asio::buffer( close_frame.data(), close_frame.size() ) )
			);

		asio::error_code ec;
		len = socket.read_some( asio::buffer( data.data(), data.size() ), ec );
		REQUIRE( 0 == len );
		REQUIRE( ec );
		REQUIRE( asio::error::eof == ec.value() );
	} );

	soenv.stop();
	soenv_thread.join();
}

TEST_CASE( "Shutdown" , "[ws_connection][shutdown][normal_close]" )
{
	soenv_t soenv{ so_5::environment_params_t{} };
	auto soenv_thread = start_soenv_in_separate_thread( soenv );

	do_with_socket( [ & ]( auto & socket, auto & /*io_context*/ ){

		std::vector< std::uint8_t > bin_ws_msg_data =
				{ 0x81, 0x88, 0x0A,0xB0,0x0C,0xD0,
				  0x0A ^ 's', 0xB0 ^ 'h', 0x0C ^ 'u', 0xD0 ^ 't',
				  0x0A ^ 'd', 0xB0 ^ 'o', 0x0C ^ 'w', 0xD0 ^ 'n' };

		REQUIRE_NOTHROW(
				asio::write(
					socket, asio::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

		std::array< std::uint8_t, 1024 > data;

		std::size_t len{ 0 };
		REQUIRE_NOTHROW(
				len = socket.read_some( asio::buffer( data.data(), data.size() ) )
			);

		REQUIRE_NOTHROW(
				asio::write( socket, asio::buffer( bin_ws_msg_data.data(), bin_ws_msg_data.size() ) )
			);

		REQUIRE_NOTHROW(
				len = socket.read_some( asio::buffer( data.data(), data.size() ) )
			);

		REQUIRE( 4 == len );
		REQUIRE( 0x88 == data[ 0 ] );
		REQUIRE( 0x02 == data[ 1 ] );
		REQUIRE( 0x03 == data[ 2 ] );
		REQUIRE( 0xe8 == data[ 3 ] );

		std::vector< std::uint8_t > close_frame =
			{0x88, 0x82, 0xFF,0xFF,0xFF,0xFF, 0xFF ^ 0x03, 0xFF ^ 0xe8 };

		REQUIRE_NOTHROW(
				asio::write(
					socket, asio::buffer( close_frame.data(), close_frame.size() ) )
			);

		asio::error_code ec;
		len = socket.read_some( asio::buffer( data.data(), data.size() ), ec );
		REQUIRE( 0 == len );
		REQUIRE( ec );
		REQUIRE( asio::error::eof == ec.value() );
	} );

	soenv.stop();
	soenv_thread.join();
}

TEST_CASE( "Invalid header" , "[ws_connection][error_close]" )
{
	std::vector< std::uint8_t > bin_ws_msg_data =
		{ 0x81, 0x05, 'H', 'e', 'l', 'l', 'o'};

	soenv_t soenv{ so_5::environment_params_t{} };
	auto soenv_thread = start_soenv_in_separate_thread( soenv );

	do_with_socket( [ & ]( auto & socket, auto & /*io_context*/ ){

		REQUIRE_NOTHROW(
				asio::write(
					socket, asio::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

		std::array< std::uint8_t, 1024 > data;

		std::size_t len{ 0 };
		REQUIRE_NOTHROW(
				len = socket.read_some( asio::buffer( data.data(), data.size() ) )
			);

		REQUIRE_NOTHROW(
				asio::write( socket, asio::buffer( bin_ws_msg_data.data(), bin_ws_msg_data.size() ) )
			);

		// Validation would fail, so no data in return.
		REQUIRE_NOTHROW(
				len = socket.read_some( asio::buffer( data.data(), data.size() ) )
			);
		REQUIRE( 4 == len );
		REQUIRE( 0x88 == data[ 0 ] );
		REQUIRE( 0x02 == data[ 1 ] );
		REQUIRE( (1002 >> 8) == data[ 2 ] );
		REQUIRE( (1002 & 0xFF) == data[ 3 ] );

		asio::error_code ec;
		len = socket.read_some( asio::buffer( data.data(), data.size() ), ec );
		REQUIRE( ec );
		REQUIRE( asio::error::eof == ec.value() );
	} );

	soenv.stop();
	soenv_thread.join();

}

TEST_CASE( "Invalid payload" , "[ws_connection][error_close]" )
{
	std::vector< std::uint8_t > bin_ws_msg_data =
			{ 0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d, 'H', 'e', 'l', 'l', 'o' };

	soenv_t soenv{ so_5::environment_params_t{} };
	auto soenv_thread = start_soenv_in_separate_thread( soenv );

	do_with_socket( [ & ]( auto & socket, auto & /*io_context*/ ){

		REQUIRE_NOTHROW(
				asio::write(
					socket, asio::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

		std::array< std::uint8_t, 1024 > data;

		std::size_t len{ 0 };
		REQUIRE_NOTHROW(
				len = socket.read_some( asio::buffer( data.data(), data.size() ) )
			);

		REQUIRE_NOTHROW(
				asio::write( socket, asio::buffer( bin_ws_msg_data.data(), bin_ws_msg_data.size() ) )
			);

		// Validation would fail, so no data in return.
		REQUIRE_NOTHROW(
				len = socket.read_some( asio::buffer( data.data(), data.size() ) )
			);
		REQUIRE( 4 == len );

		// TODO: какие должны быть байты?
		REQUIRE( 0x88 == (unsigned)data[ 0 ] );
		REQUIRE( 0x02 == data[ 1 ] );
		REQUIRE( (1007 >> 8) == data[ 2 ] );
		REQUIRE( (1007 & 0xFF) == data[ 3 ] );

		std::vector< std::uint8_t > close_frame =
			{ 0x88, 0x82, 0xFF,0xFF,0xFF,0xFF, 0xFF ^ 0x03, 0xFF ^ 0xef };

		REQUIRE_NOTHROW(
				asio::write( socket, asio::buffer( close_frame.data(), close_frame.size() ) )
			);

		asio::error_code ec;
		len = socket.read_some( asio::buffer( data.data(), data.size() ), ec );
		REQUIRE( 0 == len );
		REQUIRE( ec );
		REQUIRE( asio::error::eof == ec.value() );
	} );

	soenv.stop();
	soenv_thread.join();
}
