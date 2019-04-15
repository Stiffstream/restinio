#include <iostream>

#include <restinio/all.hpp>
#include <restinio/websocket/websocket.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

using namespace std::chrono_literals;

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

namespace rws = restinio::websocket::basic;

using traits_t =
	restinio::traits_t<
		restinio::asio_timer_manager_t,
		restinio::single_threaded_ostream_logger_t,
		router_t >;

using http_server_t = restinio::http_server_t< traits_t >;

// Will be defined later, but we have to have that type right now.
class actual_handler_t;

using actual_handler_shptr_t = std::shared_ptr< actual_handler_t >;

using ws_registry_t = std::map< std::uint64_t, actual_handler_shptr_t >;

class actual_handler_t final
	: public std::enable_shared_from_this< actual_handler_t >
{
private :
	asio::steady_timer m_timer;
	ws_registry_t & m_registry;

	// Will be set manually after successful upgrade.
	rws::ws_handle_t m_wsh;

	rws::opcode_t m_continued_frame_opcode;
	std::string m_continuation_frame_buffer;

public :
	actual_handler_t(
		asio::io_context & io_ctx,
		ws_registry_t & registry )
		:	m_timer{ io_ctx }
		,	m_registry{ registry }
	{
		std::cout << this << ": actual_handler created!" << std::endl;
	}
	~actual_handler_t()
	{
		std::cout << this << ": actual_handler destroyed!" << std::endl;
	}

	void start( rws::ws_handle_t wsh )
	{
		m_wsh = std::move(wsh);
		activate( *m_wsh );

		schedule_next_ping();
	}

	void on_message( rws::ws_handle_t wsh, rws::message_handle_t m )
	{
		if( m->opcode() == rws::opcode_t::continuation_frame )
		{
			m_continuation_frame_buffer.append( m->payload() );

			if( m->is_final() )
				wsh->send_message(
					rws::final_frame,
					m_continued_frame_opcode,
					m_continuation_frame_buffer );
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
			// Have to store ID because it will be invalidated in shutdown().
			const auto id = wsh->connection_id();

			wsh->send_message( *m );
			wsh->shutdown();

			// WS-instance and its handler should be removed from registry.
			// This will lead to destruction of actual_handler_t instance.
			//
			// NOTE: the timer will be cancelled automatically in the
			// destructor of actual_handler_t.
			m_registry.erase( id );
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

private :

	void schedule_next_ping()
	{
		m_timer.expires_after( 5s );
		m_timer.async_wait( [this]( auto ec ) {
				if( ec ) return;

				// Send new ping message.
				m_wsh->send_message(
						rws::final_frame,
						rws::opcode_t::ping_frame,
						restinio::writable_item_t{} );

				// Restart the timer.
				schedule_next_ping();
			} );
	}
};

// This proxy will be used by RESTinio for calling actual_handler.
struct handler_proxy_t
{
	actual_handler_t & m_handler;

	handler_proxy_t(
		actual_handler_t & handler )
		:	m_handler{ std::move(handler) }
	{}

	void operator()( rws::ws_handle_t wsh, rws::message_handle_t m )
	{
		m_handler.on_message( std::move(wsh), std::move(m) );
	}
};

auto server_handler( asio::io_context & io_ctx, ws_registry_t & registry )
{
	auto router = std::make_unique< router_t >();

	router->http_get(
		"/",
		[&io_ctx, &registry]( auto req, auto ){

			if( restinio::http_connection_header_t::upgrade == req->header().connection() )
			{
				auto handler = std::make_shared< actual_handler_t >(
						io_ctx, registry );

				auto wsh = rws::upgrade< traits_t >(
						*req,
						rws::activation_t::delayed,
						handler_proxy_t{ *handler } );

				registry.emplace( wsh->connection_id(), handler );

				handler->start( wsh );

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
		asio::io_context io_ctx;

		ws_registry_t registry;

		restinio::run(
			io_ctx,
			restinio::on_this_thread<traits_t>()
				.address( "localhost" )
				.port( 9001 )
				.request_handler( server_handler( io_ctx, registry ) )
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
