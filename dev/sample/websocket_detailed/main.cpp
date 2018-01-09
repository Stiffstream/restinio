#include <iostream>

#include <restinio/all.hpp>
#include <restinio/websocket/websocket.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

namespace rws = restinio::websocket::basic;

using traits_t =
	restinio::traits_t<
		restinio::asio_timer_manager_t,
		restinio::single_threaded_ostream_logger_t,
		router_t >;

using http_server_t = restinio::http_server_t< traits_t >;

using ws_registry_t = std::map< std::uint64_t, rws::ws_handle_t >;

struct ws_message_handler_t
{
	ws_message_handler_t( ws_registry_t & registry )
	:	m_registry{ registry }
	{
	}

	void
	operator()( rws::ws_handle_t wsh, rws::message_handle_t m )
	{
		if( m->opcode() == rws::opcode_t::continuation_frame )
		{
			m_continuation_frame_buffer.append( m->payload() );

			if( m->is_final() )
				wsh->send_message(
					true, m_continued_frame_opcode, m_continuation_frame_buffer );
		}
		else if( m->opcode() == rws::opcode_t::ping_frame )
		{
			auto pong = *m;
			pong.set_opcode( rws::opcode_t::pong_frame );
			wsh->send_message( pong );
		}
		else if( m->opcode() == rws::opcode_t::pong_frame )
		{
		}
		else if( m->opcode() == rws::opcode_t::connection_close_frame )
		{
			wsh->send_message( *m );
			wsh->shutdown();

			m_registry.erase( wsh->connection_id() );
		}
		else
		{
			if( m->is_final() )
				wsh->send_message( *m );
			else
			{
				m_continued_frame_opcode = m->opcode();
				m_continuation_frame_buffer.append( m->payload() );
			}
		}
	}

	ws_registry_t & m_registry;

	rws::opcode_t m_continued_frame_opcode;
	std::string m_continuation_frame_buffer;
};

auto server_handler( ws_registry_t & registry )
{
	auto router = std::make_unique< router_t >();

	router->http_get(
		"/",
		[&registry]( auto req, auto ){

			if( restinio::http_connection_header_t::upgrade == req->header().connection() )
			{
				auto wsh =
					rws::upgrade< traits_t >(
						*req,
						rws::activation_t::immediate,
						ws_message_handler_t{registry} );

				registry.emplace( wsh->connection_id(), wsh );

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
		ws_registry_t registry;

		restinio::run(
			restinio::on_this_thread<traits_t>()
				.address( "localhost" )
				.port( 9001 )
				.request_handler( server_handler( registry ) )
				.read_next_http_message_timelimit( 10s )
				.write_http_response_timelimit( 1s )
				.handle_request_timeout( 1s )
				.cleanup_func( [&]{ registry.clear(); } ) );

	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
