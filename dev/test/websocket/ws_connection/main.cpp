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

rws::impl::ws_message_details_t
to_ws_message_details( const rws::message_t & msg )
{
	return rws::impl::ws_message_details_t{
		msg.is_final(),
		msg.opcode(),
		msg.payload().size() };
}

bool
operator==(
	const rws::message_t & lhs,
	const rws::message_t & rhs )
{
	return
		lhs.is_final() == rhs.is_final() &&
		lhs.opcode() == rhs.opcode() &&
		lhs.payload() == rhs.payload();
}

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

struct msg_ws_message : public so_5::message_t
{
	msg_ws_message( rws::message_handle_t msg )
		:	m_msg{ msg }
	{}

	rws::message_handle_t m_msg;
};

struct
request_response_context_t
{
	rws::message_t m_request;
	rws::message_t m_response;
	std::string m_request_bin;
	std::string m_response_bin;

	int close_code = 0;
	std::string m_close_description;
};

std::array< char, 20>
digest_to_char_array( const restinio::utils::sha1::digest_t & digest )
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
			,	m_http_server{
					restinio::create_child_io_context( 1 ),
					[this]( auto & settings ){
						settings
							.port( utest_default_port() )
							.address( "127.0.0.1" )
							.request_handler( [ mbox = this->so_direct_mbox() ]( auto req )
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
					restinio::utils::base64::encode(
						std::string{
							digest_to_char_array(digest).data(), 20
						} ),
					[this]( auto /* ws_weak_handle*/, rws::message_handle_t m ){
						so_5::send< msg_ws_message >(
							this->so_direct_mbox(), m );
					},
					[]( std::string reason ){
						std::cout << "Close ws reason: " << reason << std::endl;
					} );
		}

		void
		evt_ws_message( const msg_ws_message & msg )
		{
			auto req = *(msg.m_msg);

			auto resp = req;

			m_ws->send_message( resp );
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


TEST_CASE( "Request/Response echo" , "[ws_connection]" )
{
	restinio::raw_data_t bin_ws_msg_data{ to_char_each(
			{0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d, 0x7f, 0x9f, 0x4d, 0x51, 0x58}) };

	soenv_t soenv{ so_5::environment_params_t{} };
	auto soenv_thread = start_soenv_in_separate_thread( soenv );

	do_with_socket( [ & ]( auto & socket, auto & /*io_context*/ ){

		REQUIRE_NOTHROW(
				asio::write(
					socket, asio::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

		std::array< char, 1024 > data;

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

		auto response_bin = std::string( data.data(), len );
		// TODO: check response
		// auto request_payload = ctx.m_request.payload();
		// rws::impl::mask_unmask_payload(
		// 		ctx.m_request.header().m_masking_key, request_payload );

		// REQUIRE( request_payload == ctx.m_response.payload() );
		// REQUIRE( ctx.m_response.header().m_masking_key == 0 );

	} );

	soenv.stop();
	soenv_thread.join();
}

TEST_CASE( "Request/Response close without masking key" , "[ws_connection]" )
{
	restinio::raw_data_t bin_ws_msg_data{ to_char_each(
			{0x81, 0x05, 'H', 'e', 'l', 'l', 'o'}) };

	soenv_t soenv{ so_5::environment_params_t{} };
	auto soenv_thread = start_soenv_in_separate_thread( soenv );

	do_with_socket( [ & ]( auto & socket, auto & /*io_context*/ ){

		REQUIRE_NOTHROW(
				asio::write(
					socket, asio::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

		std::array< char, 1024 > data;

		std::size_t len{ 0 };
		REQUIRE_NOTHROW(
				len = socket.read_some( asio::buffer( data.data(), data.size() ) )
			);

		REQUIRE_NOTHROW(
				asio::write( socket, asio::buffer( bin_ws_msg_data.data(), bin_ws_msg_data.size() ) )
			);

		// Validation would fail, so no data in return.
		asio::error_code ec;
		len = socket.read_some( asio::buffer( data.data(), data.size() ), ec );
		REQUIRE( 0 == len );
		REQUIRE( ec );
		REQUIRE( asio::error::eof == ec.value() );
	} );

	soenv.stop();
	soenv_thread.join();

}

TEST_CASE( "Request/Response close with non utf-8 payload" , "[ws_connection]" )
{
	restinio::raw_data_t bin_ws_msg_data{ to_char_each(
			{0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d, 'H', 'e', 'l', 'l', 'o'} ) };

	soenv_t soenv{ so_5::environment_params_t{} };
	auto soenv_thread = start_soenv_in_separate_thread( soenv );

	do_with_socket( [ & ]( auto & socket, auto & /*io_context*/ ){

		REQUIRE_NOTHROW(
				asio::write(
					socket, asio::buffer( upgrade_request.data(), upgrade_request.size() ) )
			);

		std::array< char, 1024 > data;

		std::size_t len{ 0 };
		REQUIRE_NOTHROW(
				len = socket.read_some( asio::buffer( data.data(), data.size() ) )
			);

		REQUIRE_NOTHROW(
				asio::write( socket, asio::buffer( bin_ws_msg_data.data(), bin_ws_msg_data.size() ) )
			);

		// Validation would fail, so no data in return.
		asio::error_code ec;
		len = socket.read_some( asio::buffer( data.data(), data.size() ), ec );
		REQUIRE( 0 == len );
		REQUIRE( ec );
		REQUIRE( asio::error::eof == ec.value() );
	} );

	soenv.stop();
	soenv_thread.join();
}
