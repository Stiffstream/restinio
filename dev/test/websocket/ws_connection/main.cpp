/*
	restinio
*/

/*!
	Test upgrade request.
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>
#include <restinio/websocket/websocket.hpp>
#include <restinio/utils/base64.hpp>
#include <restinio/utils/sha1.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>
#include <test/websocket/common/pub.hpp>

#include <so_5/all.hpp>

namespace rws = restinio::websocket::basic;

using traits_t =
	restinio::traits_t<
		restinio::asio_timer_manager_t,
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

struct server_started_t : public so_5::signal_t {};

//
// g_last_close_code
//

std::atomic< std::uint16_t > g_last_close_code{ 0 };
std::atomic< std::uint16_t > g_message_handled{ 0 };

//
// a_server_t
//

//! Agent running ws server logic.
class a_server_t
	:	public so_5::agent_t
{
		using so_base_type_t = so_5::agent_t;

	public:
		a_server_t(
			context_t ctx,
			so_5::mchain_t server_started_mchain )
			:	so_base_type_t{ ctx }
			,	m_server_started_mchain( std::move(server_started_mchain) )
			,	m_http_server{
					restinio::own_io_context(),
					[this]( auto & settings ){
						auto mbox = this->so_direct_mbox();
						settings
							.port( utest_default_port() )
							.address( "127.0.0.1" )
							.request_handler(
								[mbox]( auto req ){
									if( restinio::http_connection_header_t::upgrade ==
										req->header().connection() )
									{
										++g_message_handled;
										so_5::send< upgrade_request_t >( mbox, std::move( req ) );

										return restinio::request_accepted();
									}

									return restinio::request_rejected();
								} );
					} }
			,	m_other_thread{ m_http_server }
		{
			g_last_close_code = 0;
			g_message_handled = 0;
		}

		virtual void
		so_define_agent() override
		{
			so_subscribe_self()
				.event( &a_server_t::evt_upgrade_request )
				.event( &a_server_t::evt_ws_message );
		}

		virtual void
		so_evt_start() override
		{
			m_other_thread.run();
			so_5::send<server_started_t>( m_server_started_mchain );
		}

		virtual void
		so_evt_finish() override
		{
			m_ws.reset();
			m_other_thread.stop_and_join();
		}

	private:
		void
		evt_upgrade_request( const upgrade_request_t & msg )
		{
			auto req = msg.m_req;

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
							rws::final_frame,
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
					g_last_close_code = (std::uint16_t)rws::status_code_from_bin( req.payload() );
					std::cout << "CLOSE FRAME: " << g_last_close_code << std::endl;
					m_ws.reset();
				}
			}
		}

		const so_5::mchain_t m_server_started_mchain;
		http_server_t m_http_server;
		other_work_thread_for_server_t<http_server_t> m_other_thread;
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

class sobj_t
{
	so_5::wrapped_env_t m_sobj;

	static void
	init( so_5::environment_t & env )
	{
		auto server_started_mchain = so_5::create_mchain(env);
		// Launch server as separate coop.
		env.introduce_coop(
			so_5::disp::active_obj::create_private_disp(env)->binder(),
			[&]( so_5::coop_t & coop ) {
				coop.make_agent< a_server_t >(server_started_mchain);
			} );
		// Wait acknowledgement about successful server start.
		so_5::receive(
				server_started_mchain,
				std::chrono::seconds(5),
				[](so_5::mhood_t<server_started_t>) {});
	}

public :
	sobj_t( const sobj_t & ) = delete;
	sobj_t( sobj_t && ) = delete;

	sobj_t()
	{
		init( m_sobj.environment() );
	}

	void
	stop_and_join()
	{
		m_sobj.stop();
		m_sobj.join();
	}
};

template < typename Socket >
void
fragmented_send( Socket & socket, void * buf, std::size_t n )
{
	const auto * b = static_cast< std::uint8_t * >( buf );
	while( n-- )
	{
		restinio::asio_ns::write( socket, restinio::asio_ns::buffer( b++, 1 ) );
		if( 0 < n )
			std::this_thread::sleep_for( std::chrono::milliseconds( n ) );
	}
}

TEST_CASE( "Simple echo" , "[ws_connection][echo][normal_close]" )
{
	sobj_t sobj;

	do_with_socket(
		[&]( auto & socket, auto & /*io_context*/ ){
			REQUIRE_NOTHROW(
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

			std::array< std::uint8_t, 1024 > data;

			std::size_t len{ 0 };
			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);

			std::vector< std::uint8_t > msg_frame =
					{ 0x81, 0x85, 0xAA,0xBB,0xCC,0xDD,
					  0xAA ^ 'H', 0xBB ^ 'e', 0xCC ^ 'l', 0xDD ^ 'l', 0xAA ^ 'o' };
			SECTION( "simple msg_frame")
			{
				REQUIRE_NOTHROW(
					restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), msg_frame.size() ) )
				);
			}
			SECTION( "fragmentated msg_frame")
			{
				REQUIRE_NOTHROW(
					fragmented_send( socket, msg_frame.data(), msg_frame.size() )
				);
			}

			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
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
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( close_frame.data(), close_frame.size() ) )
			);

			REQUIRE_NOTHROW(
					len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
				);
			REQUIRE( 4 == len );
			REQUIRE( 0x88 == data[ 0 ] );
			REQUIRE( 0x02 == data[ 1 ] );
			REQUIRE( 0x03 == data[ 2 ] );
			REQUIRE( 0xe8 == data[ 3 ] );

			restinio::asio_ns::error_code ec;
			len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ), ec );
			REQUIRE( ec );
			REQUIRE( restinio::asio_ns::error::eof == ec.value() );

		} );

	sobj.stop_and_join();

	REQUIRE( 1000 == g_last_close_code );
}

TEST_CASE( "Ping" , "[ws_connection][ping][normal_close]" )
{
	sobj_t sobj;

	do_with_socket(
		[&]( auto & socket, auto & /*io_context*/ ){
			REQUIRE_NOTHROW(
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

			std::array< std::uint8_t, 1024 > data;

			std::size_t len{ 0 };
			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);

			std::vector< std::uint8_t > msg_frame =
					{ 0x89, 0x84, 0x0A,0xB0,0x0C,0xD0,
					  0x0A ^ 'P', 0xB0 ^ 'i', 0x0C ^ 'n', 0xD0 ^ 'g' };

			SECTION( "simple msg_frame")
			{
				REQUIRE_NOTHROW(
					restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), msg_frame.size() ) )
				);
			}
			SECTION( "fragmentated msg_frame")
			{
				REQUIRE_NOTHROW(
					fragmented_send( socket, msg_frame.data(), msg_frame.size() )
				);
			}

			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
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
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( close_frame.data(), close_frame.size() ) )
			);

			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);
			REQUIRE( 4 == len );
			REQUIRE( 0x88 == data[ 0 ] );
			REQUIRE( 0x02 == data[ 1 ] );
			REQUIRE( 0x03 == data[ 2 ] );
			REQUIRE( 0xe8 == data[ 3 ] );

			restinio::asio_ns::error_code ec;
			len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ), ec );
			REQUIRE( ec );
			REQUIRE( restinio::asio_ns::error::eof == ec.value() );
		} );

	sobj.stop_and_join();

	REQUIRE( 1000 == g_last_close_code );
}

TEST_CASE( "Close" , "[ws_connection][close][normal_close]" )
{
	sobj_t sobj;

	do_with_socket(
		[&]( auto & socket, auto & /*io_context*/ ){

			REQUIRE_NOTHROW(
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

			std::array< std::uint8_t, 1024 > data;

			std::size_t len{ 0 };
			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);

			std::vector< std::uint8_t > msg_frame =
					{ 0x81, 0x85, 0x0A,0xB0,0x0C,0xD0,
					  0x0A ^ 'c', 0xB0 ^ 'l', 0x0C ^ 'o', 0xD0 ^ 's', 0x0A ^ 'e' };

			SECTION( "simple msg_frame")
			{
				REQUIRE_NOTHROW(
					restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), msg_frame.size() ) )
				);
			}
			SECTION( "fragmentated msg_frame")
			{
				REQUIRE_NOTHROW(
					fragmented_send( socket, msg_frame.data(), msg_frame.size() )
				);
			}

			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);

			REQUIRE( 4 == len );
			REQUIRE( 0x88 == data[ 0 ] );
			REQUIRE( 0x02 == data[ 1 ] );
			REQUIRE( 0x03 == data[ 2 ] );
			REQUIRE( 0xe8 == data[ 3 ] );

			std::vector< std::uint8_t > close_frame =
				{0x88, 0x82, 0xFF,0xFF,0xFF,0xFF, 0xFF ^ 0x03, 0xFF ^ 0xe8 };

			REQUIRE_NOTHROW(
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( close_frame.data(), close_frame.size() ) )
			);

			restinio::asio_ns::error_code ec;
			len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ), ec );
			REQUIRE( 0 == len );
			REQUIRE( ec );
			REQUIRE( restinio::asio_ns::error::eof == ec.value() );
		} );

	sobj.stop_and_join();

	// User initiates close.
	REQUIRE( 0 == g_last_close_code );
}

TEST_CASE( "Shutdown" , "[ws_connection][shutdown][normal_close]" )
{
	sobj_t sobj;

	do_with_socket(
		[&]( auto & socket, auto & /*io_context*/ ){
			REQUIRE_NOTHROW(
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

			std::array< std::uint8_t, 1024 > data;

			std::size_t len{ 0 };
			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);

			std::vector< std::uint8_t > msg_frame =
					{ 0x81, 0x88, 0x0A,0xB0,0x0C,0xD0,
					  0x0A ^ 's', 0xB0 ^ 'h', 0x0C ^ 'u', 0xD0 ^ 't',
					  0x0A ^ 'd', 0xB0 ^ 'o', 0x0C ^ 'w', 0xD0 ^ 'n' };

			REQUIRE_NOTHROW(
				restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), msg_frame.size() ) )
			);

			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);

			REQUIRE( 4 == len );
			REQUIRE( 0x88 == data[ 0 ] );
			REQUIRE( 0x02 == data[ 1 ] );
			REQUIRE( 0x03 == data[ 2 ] );
			REQUIRE( 0xe8 == data[ 3 ] );

			std::vector< std::uint8_t > close_frame =
				{0x88, 0x82, 0xFF,0xFF,0xFF,0xFF, 0xFF ^ 0x03, 0xFF ^ 0xe8 };

			SECTION( "simple close_frame")
			{
				REQUIRE_NOTHROW(
					restinio::asio_ns::write(
						socket, restinio::asio_ns::buffer( close_frame.data(), close_frame.size() ) )
				);
			}
			SECTION( "fragmentated close_frame")
			{
				REQUIRE_NOTHROW(
					fragmented_send( socket, close_frame.data(), close_frame.size() )
				);
			}

			restinio::asio_ns::error_code ec;
			len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ), ec );
			REQUIRE( 0 == len );
			REQUIRE( ec );
			REQUIRE( restinio::asio_ns::error::eof == ec.value() );
		} );

	sobj.stop_and_join();

	// User initiates close via shutdown.
	REQUIRE( 0 == g_last_close_code );
}

TEST_CASE( "Kill" , "[ws_connection][kill][abnormal_close]" )
{
	sobj_t sobj;

	do_with_socket(
		[&]( auto & socket, auto & /*io_context*/ ){
			REQUIRE_NOTHROW(
					restinio::asio_ns::write(
						socket, restinio::asio_ns::buffer( upgrade_request.data(), upgrade_request.size() ) )
				);

			std::array< std::uint8_t, 1024 > data;

			std::size_t len{ 0 };
			REQUIRE_NOTHROW(
					len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
				);

			std::vector< std::uint8_t > msg_frame =
					{ 0x81, 0x84, 0x0A,0xB0,0x0C,0xD0,
					  0x0A ^ 'k', 0xB0 ^ 'i', 0x0C ^ 'l', 0xD0 ^ 'l' };

			SECTION( "simple msg_frame")
			{
				REQUIRE_NOTHROW(
						restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), msg_frame.size() ) )
					);
			}
			SECTION( "fragmentated msg_frame")
			{
				REQUIRE_NOTHROW(
					fragmented_send( socket, msg_frame.data(), msg_frame.size() )
				);
			}

			restinio::asio_ns::error_code ec;
			len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ), ec );
			REQUIRE( 0 == len );
			REQUIRE( ec );
			REQUIRE( restinio::asio_ns::error::eof == ec.value() );
		} );

	sobj.stop_and_join();

	REQUIRE( 0 == g_last_close_code );
}

TEST_CASE( "Invalid header", "[ws_connection][error_close]" )
{
	sobj_t sobj;

	do_with_socket(
		[&]( auto & socket, auto & /*io_context*/ ){
			REQUIRE_NOTHROW(
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

			std::array< std::uint8_t, 1024 > data;

			std::size_t len{ 0 };
			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);

			std::vector< std::uint8_t > msg_frame;
			SECTION( "No mask" )
			{
				msg_frame.assign( { 0x81, 0x05, 'H', 'e', 'l', 'l', 'o'} );
			}
			SECTION( "No rsv1" )
			{
				msg_frame.assign(
					{ 0x81 + 0x40, 0x85, 0xAA,0xBB,0xCC,0xDD,
					  0xAA ^ 'H', 0xBB ^ 'e', 0xCC ^ 'l', 0xDD ^ 'l', 0xAA ^ 'o'} );
			}
			SECTION( "No rsv2" )
			{
				msg_frame.assign(
					{ 0x81 + 0x20, 0x85, 0xAA,0xBB,0xCC,0xDD,
					  0xAA ^ 'H', 0xBB ^ 'e', 0xCC ^ 'l', 0xDD ^ 'l', 0xAA ^ 'o'} );
			}
			SECTION( "No rsv3" )
			{
				msg_frame.assign(
					{ 0x81 + 0x10, 0x85, 0xAA,0xBB,0xCC,0xDD,
					  0xAA ^ 'H', 0xBB ^ 'e', 0xCC ^ 'l', 0xDD ^ 'l', 0xAA ^ 'o'} );
			}

			REQUIRE_NOTHROW(
				restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), msg_frame.size() ) )
			);

			// Validation would fail, close frame in return.
			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);
			REQUIRE( 4 == len );
			REQUIRE( 0x88 == data[ 0 ] );
			REQUIRE( 0x02 == data[ 1 ] );
			REQUIRE( (1002 >> 8) == data[ 2 ] );
			REQUIRE( (1002 & 0xFF) == data[ 3 ] );

			restinio::asio_ns::error_code ec;
			len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ), ec );
			REQUIRE( ec );
			REQUIRE( restinio::asio_ns::error::eof == ec.value() );
		} );

	sobj.stop_and_join();

	REQUIRE( 1002 == g_last_close_code );
}

TEST_CASE( "Invalid payload" , "[ws_connection][error_close]" )
{
	sobj_t sobj;

	do_with_socket(
		[&]( auto & socket, auto & /*io_context*/ ){

			REQUIRE_NOTHROW(
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

			std::array< std::uint8_t, 1024 > data;

			std::size_t len{ 0 };
			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);

			std::vector< std::uint8_t > msg_frame =
					{ 0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d, 'H', 'e', 'l', 'l', 'o' };

			SECTION( "simple msg_frame")
			{
				REQUIRE_NOTHROW(
					restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), msg_frame.size() ) )
				);
			}
			SECTION( "fragmentated msg_frame")
			{
				REQUIRE_NOTHROW(
					fragmented_send( socket, msg_frame.data(), msg_frame.size() )
				);
			}

			// Validation would fail, so no data in return.
			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);
			REQUIRE( 4 == len );

			// TODO: какие должны быть байты?
			REQUIRE( 0x88 == data[ 0 ] );
			REQUIRE( 0x02 == data[ 1 ] );
			REQUIRE( (1007 >> 8) == data[ 2 ] );
			REQUIRE( (1007 & 0xFF) == data[ 3 ] );

			std::vector< std::uint8_t > close_frame =
				{ 0x88, 0x82, 0xFF,0xFF,0xFF,0xFF, 0xFF ^ 0x03, 0xFF ^ 0xef };

			REQUIRE_NOTHROW(
				restinio::asio_ns::write( socket, restinio::asio_ns::buffer( close_frame.data(), close_frame.size() ) )
			);

			restinio::asio_ns::error_code ec;
			len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ), ec );
			REQUIRE( 0 == len );
			REQUIRE( ec );
			REQUIRE( restinio::asio_ns::error::eof == ec.value() );
		} );

	sobj.stop_and_join();

	REQUIRE( 1007 == g_last_close_code );
}

TEST_CASE( "Connection lost" , "[ws_connection][error_close][connection_lost]" )
{
	sobj_t sobj;

	do_with_socket(
		[&]( auto & socket, auto & /*io_context*/ ){

			REQUIRE_NOTHROW(
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

			std::array< std::uint8_t, 1024 > data;

			std::size_t len{ 0 };
			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);

			std::vector< std::uint8_t > msg_frame =
					{ 0x81, 0x85, 0xAA,0xBB,0xCC,0xDD,
					  0xAA ^ 'H', 0xBB ^ 'e', 0xCC ^ 'l', 0xDD ^ 'l', 0xAA ^ 'o' };

			SECTION( "reset before msg")
			{
				std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
				// Do nothing.
			}
			SECTION( "after header first byte")
			{
				REQUIRE_NOTHROW(
					restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), 1 ) )
				);
			}
			SECTION( "after header second byte")
			{
				REQUIRE_NOTHROW(
					restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), 2 ) )
				);
			}
			SECTION( "in the middle of mask")
			{
				REQUIRE_NOTHROW(
					restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), 4 ) )
				);
			}
			SECTION( "after mask before payload")
			{
				REQUIRE_NOTHROW(
					restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), 6 ) )
				);
			}
			SECTION( "in the middle of the payload")
			{
				REQUIRE_NOTHROW(
					restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), 8 ) )
				);
			}

			socket.close();

		} );

	// Give sobjectizer some time to run.
	std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
	sobj.stop_and_join();


	REQUIRE( 1006 == g_last_close_code );
}

TEST_CASE( "Invalid opcode" , "[ws_connection][error_close]" )
{
	sobj_t sobj;

	do_with_socket(
		[&]( auto & socket, auto & /*io_context*/ ){
			REQUIRE_NOTHROW(
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

			std::array< std::uint8_t, 1024 > data;

			std::size_t len{ 0 };
			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);

			std::vector< std::uint8_t > msg_frame;
			SECTION( "opcode 0x03" )
			{
				msg_frame.assign(
					{ 0x83, 0x85, 0xAA,0xBB,0xCC,0xDD,
					  0xAA ^ 'H', 0xBB ^ 'e', 0xCC ^ 'l', 0xDD ^ 'l', 0xAA ^ 'o'} );
			}
			SECTION( "opcode 0x04" )
			{
				msg_frame.assign(
					{ 0x84, 0x85, 0xAA,0xBB,0xCC,0xDD,
					  0xAA ^ 'H', 0xBB ^ 'e', 0xCC ^ 'l', 0xDD ^ 'l', 0xAA ^ 'o'} );
			}
			SECTION( "opcode 0x05" )
			{
				msg_frame.assign(
					{ 0x85, 0x85, 0xAA,0xBB,0xCC,0xDD,
					  0xAA ^ 'H', 0xBB ^ 'e', 0xCC ^ 'l', 0xDD ^ 'l', 0xAA ^ 'o'} );
			}
			SECTION( "opcode 0x06" )
			{
				msg_frame.assign(
					{ 0x86, 0x85, 0xAA,0xBB,0xCC,0xDD,
					  0xAA ^ 'H', 0xBB ^ 'e', 0xCC ^ 'l', 0xDD ^ 'l', 0xAA ^ 'o'} );
			}
			SECTION( "opcode 0x07" )
			{
				msg_frame.assign(
					{ 0x87, 0x85, 0xAA,0xBB,0xCC,0xDD,
					  0xAA ^ 'H', 0xBB ^ 'e', 0xCC ^ 'l', 0xDD ^ 'l', 0xAA ^ 'o'} );
			}
			SECTION( "opcode 0x0B" )
			{
				msg_frame.assign(
					{ 0x8B, 0x85, 0xAA,0xBB,0xCC,0xDD,
					  0xAA ^ 'H', 0xBB ^ 'e', 0xCC ^ 'l', 0xDD ^ 'l', 0xAA ^ 'o'} );
			}
			SECTION( "opcode 0x0C" )
			{
				msg_frame.assign(
					{ 0x8C, 0x85, 0xAA,0xBB,0xCC,0xDD,
					  0xAA ^ 'H', 0xBB ^ 'e', 0xCC ^ 'l', 0xDD ^ 'l', 0xAA ^ 'o'} );
			}
			SECTION( "opcode 0x0D" )
			{
				msg_frame.assign(
					{ 0x8D, 0x85, 0xAA,0xBB,0xCC,0xDD,
					  0xAA ^ 'H', 0xBB ^ 'e', 0xCC ^ 'l', 0xDD ^ 'l', 0xAA ^ 'o'} );
			}
			SECTION( "opcode 0x0E" )
			{
				msg_frame.assign(
					{ 0x8E, 0x85, 0xAA,0xBB,0xCC,0xDD,
					  0xAA ^ 'H', 0xBB ^ 'e', 0xCC ^ 'l', 0xDD ^ 'l', 0xAA ^ 'o'} );
			}
			SECTION( "opcode 0x0F" )
			{
				msg_frame.assign(
					{ 0x8F, 0x85, 0xAA,0xBB,0xCC,0xDD,
					  0xAA ^ 'H', 0xBB ^ 'e', 0xCC ^ 'l', 0xDD ^ 'l', 0xAA ^ 'o'} );
			}

			REQUIRE_NOTHROW(
				restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), msg_frame.size() ) )
			);

			// Validation would fail, close frame in return.
			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);
			REQUIRE( 4 == len );
			REQUIRE( 0x88 == data[ 0 ] );
			REQUIRE( 0x02 == data[ 1 ] );
			REQUIRE( (1002 >> 8) == data[ 2 ] );
			REQUIRE( (1002 & 0xFF) == data[ 3 ] );

			restinio::asio_ns::error_code ec;
			len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ), ec );
			REQUIRE( ec );
			REQUIRE( restinio::asio_ns::error::eof == ec.value() );
		} );

	sobj.stop_and_join();

	REQUIRE( 1002 == g_last_close_code );
}

TEST_CASE( "Invalid payload, close on first err 1" , "[ws_connection][echo][normal_close]" )
{
	sobj_t sobj;

	do_with_socket(
		[&]( auto & socket, auto & /*io_context*/ ){
			REQUIRE_NOTHROW(
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

			std::array< std::uint8_t, 1024 > data;

			std::size_t len{ 0 };
			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);

			std::vector< std::uint8_t > msg_frame =
					{ 0x81, 0x80 | 0x10, 0xAA,0xAA,0xAA,0xAA,
					  0xAA ^ '1', 0xAA ^ '2', 0xAA ^ '3', 0xAA ^ '4',
					  0xAA ^ '5', 0xAA ^ '6', 0xAA ^ '7', 0xAA ^ '8',
					  0xAA ^ '9', 0xAA ^ 'A', 0xAA ^ 'B', 0xAA ^ 'C',
					  0xAA ^ 'D', 0xAA ^ 'E', 0xAA ^ 'F', 0xAA ^ '0' };

			std::size_t indx = 6;
			SECTION( "pos 0")
			{
				indx += 0;
			}
			SECTION( "pos 2")
			{
				indx += 2;
			}
			SECTION( "pos 4")
			{
				indx += 4;
			}
			SECTION( "pos 8")
			{
				indx += 8;
			}
			SECTION( "pos 15")
			{
				indx += 15;
			}
			msg_frame[ indx ] = 0xFF ^ 0xAA;

			REQUIRE_NOTHROW(
				restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), indx ) )
			);

			// Now error:
			REQUIRE_NOTHROW(
				restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data() + indx, 1 ) )
			);

			REQUIRE_NOTHROW(
					len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
				);
			REQUIRE( 4 == len );
			REQUIRE( 0x88 == data[ 0 ] );
			REQUIRE( 0x02 == data[ 1 ] );
			REQUIRE( 0x03 == data[ 2 ] );
			REQUIRE( 0xef == data[ 3 ] );

			if( indx != msg_frame.size() )
			{
				REQUIRE_NOTHROW(
					restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data() + indx + 1, msg_frame.size() - indx - 1 ) )
				);
			}

			std::vector< std::uint8_t > close_frame =
				{0x88, 0x82, 0xFF,0xFF,0xFF,0xFF, 0xFF ^ 0x03, 0xFF ^ 0xef };

			REQUIRE_NOTHROW(
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( close_frame.data(), close_frame.size() ) )
			);

			restinio::asio_ns::error_code ec;
			len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ), ec );
			REQUIRE( ec );
			REQUIRE( restinio::asio_ns::error::eof == ec.value() );
		} );

	sobj.stop_and_join();

	REQUIRE( 1007 == g_last_close_code );
	REQUIRE( 1 == g_message_handled );
}

TEST_CASE( "Invalid payload, close on first err 2", "[ws_connection][echo][normal_close]" )
{
	sobj_t sobj;

	do_with_socket(
		[&]( auto & socket, auto & /*io_context*/ ){
			REQUIRE_NOTHROW(
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

			std::array< std::uint8_t, 1024 > data;

			std::size_t len{ 0 };
			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);

			std::vector< std::uint8_t > msg_frame =
					{ 0x81, 0x80 | 0x10, 0xAA,0xAA,0xAA,0xAA,
					  0xAA ^ '1', 0xAA ^ '2', 0xAA ^ '3', 0xAA ^ '4',
					  0xAA ^ '5', 0xAA ^ '6', 0xAA ^ '7', 0xAA ^ '8',
					  0xAA ^ '9', 0xAA ^ 'A', 0xAA ^ 'B', 0xAA ^ 'C',
					  0xAA ^ 'D', 0xAA ^ 'E', 0xAA ^ 'F', 0xAA ^ '0' };

			std::size_t indx = 6;
			SECTION( "pos 0")
			{
				indx += 0;
			}
			SECTION( "pos 2")
			{
				indx += 2;
			}
			SECTION( "pos 4")
			{
				indx += 4;
			}
			SECTION( "pos 8")
			{
				indx += 8;
			}
			SECTION( "pos 15")
			{
				indx += 15;
			}
			unsigned char c = 0xFF ^ 0xAA;

			REQUIRE_NOTHROW(
				restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), indx ) )
			);

			// Now error:
			REQUIRE_NOTHROW(
				restinio::asio_ns::write( socket, restinio::asio_ns::buffer( &c, 1 ) )
			);

			REQUIRE_NOTHROW(
					len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
				);
			REQUIRE( 4 == len );
			REQUIRE( 0x88 == data[ 0 ] );
			REQUIRE( 0x02 == data[ 1 ] );
			REQUIRE( 0x03 == data[ 2 ] );
			REQUIRE( 0xef == data[ 3 ] );

			if( indx != msg_frame.size() )
			{
				REQUIRE_NOTHROW(
					restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data() + indx + 1, msg_frame.size() - indx - 1 ) )
				);
			}

			// Send one more text-frame (valid one).
			REQUIRE_NOTHROW(
				restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), msg_frame.size() ) )
			);

			std::vector< std::uint8_t > close_frame =
				{0x88, 0x82, 0xFF,0xFF,0xFF,0xFF, 0xFF ^ 0x03, 0xFF ^ 0xef };

			REQUIRE_NOTHROW(
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( close_frame.data(), close_frame.size() ) )
			);

			restinio::asio_ns::error_code ec;
			len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ), ec );
			REQUIRE( ec );
			REQUIRE( restinio::asio_ns::error::eof == ec.value() );
		} );

	sobj.stop_and_join();

	REQUIRE( 1007 == g_last_close_code );
	REQUIRE( 1 == g_message_handled );
}


TEST_CASE( "Invalid payload, close on first err 3", "[ws_connection][echo][normal_close]" )
{
	sobj_t sobj;

	do_with_socket(
		[&]( auto & socket, auto & /*io_context*/ ){
			REQUIRE_NOTHROW(
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

			std::array< std::uint8_t, 1024 > data;

			std::size_t len{ 0 };
			REQUIRE_NOTHROW(
				len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
			);

			std::vector< std::uint8_t > msg_frame =
					{ 0x81, 0x80 | 0x10, 0xAA,0xAA,0xAA,0xAA,
					  0xAA ^ '1', 0xAA ^ '2', 0xAA ^ '3', 0xAA ^ '4',
					  0xAA ^ '5', 0xAA ^ '6', 0xAA ^ '7', 0xAA ^ '8',
					  0xAA ^ '9', 0xAA ^ 'A', 0xAA ^ 'B', 0xAA ^ 'C',
					  0xAA ^ 'D', 0xAA ^ 'E', 0xAA ^ 'F', 0xAA ^ '0' };

			std::size_t indx = 6;
			SECTION( "pos 0")
			{
				indx += 0;
			}
			SECTION( "pos 2")
			{
				indx += 2;
			}
			SECTION( "pos 4")
			{
				indx += 4;
			}
			SECTION( "pos 8")
			{
				indx += 8;
			}
			SECTION( "pos 15")
			{
				indx += 15;
			}
			msg_frame[ indx ] = 0xFF ^ 0xAA;

			REQUIRE_NOTHROW(
				restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), indx ) )
			);

			// Now error:
			REQUIRE_NOTHROW(
				restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data() + indx, 1 ) )
			);

			REQUIRE_NOTHROW(
					len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ) )
				);
			REQUIRE( 4 == len );
			REQUIRE( 0x88 == data[ 0 ] );
			REQUIRE( 0x02 == data[ 1 ] );
			REQUIRE( 0x03 == data[ 2 ] );
			REQUIRE( 0xef == data[ 3 ] );

			if( indx != msg_frame.size() )
			{
				REQUIRE_NOTHROW(
					restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data() + indx + 1, msg_frame.size() - indx - 1 ) )
				);
			}

			// Send one more text-frame (invalid one).
			REQUIRE_NOTHROW(
				restinio::asio_ns::write( socket, restinio::asio_ns::buffer( msg_frame.data(), msg_frame.size() ) )
			);

			std::vector< std::uint8_t > close_frame =
				{0x88, 0x82, 0xFF,0xFF,0xFF,0xFF, 0xFF ^ 0x03, 0xFF ^ 0xef };

			REQUIRE_NOTHROW(
				restinio::asio_ns::write(
					socket, restinio::asio_ns::buffer( close_frame.data(), close_frame.size() ) )
			);

			restinio::asio_ns::error_code ec;
			len = socket.read_some( restinio::asio_ns::buffer( data.data(), data.size() ), ec );
			REQUIRE( ec );
			REQUIRE( restinio::asio_ns::error::eof == ec.value() );
		} );

	sobj.stop_and_join();

	REQUIRE( 1007 == g_last_close_code );
	REQUIRE( 1 == g_message_handled ); // close frame only.
}

