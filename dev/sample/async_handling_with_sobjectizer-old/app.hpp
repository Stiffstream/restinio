#pragma once

#include <memory>
#include <string>
#include <map>
#include <chrono>

#include <so_5/all.hpp>
#include <restinio/all.hpp>

//
// msg_request_t
//

// A message with request, that must be handled by server agent.
struct msg_request_t final : public so_5::message_t
{
	msg_request_t(
		restinio::request_handle_t req )
		:	m_req{ std::move( req ) }
	{}

	restinio::request_handle_t m_req;
};

auto now_string_utc()
{
	const auto t = std::time( nullptr );
	const auto tpoint = *gmtime( &t );

	return fmt::format( "{:%Y-%m-%d %H:%M:%S GMT}", tpoint );
}

//
// a_target_t
//

// Agent that handles a single target.
class a_target_t final : public so_5::agent_t
{
	public:
		a_target_t(
			context_t ctx,
			const std::string & target )
			:	so_5::agent_t{ std::move( ctx ) }
			,	m_response_segments{ target }
		{
			so_subscribe_self()
				.event( &a_target_t::evt_req )
				.event< signal_timeout_check_t >(
						&a_target_t::evt_check_timeout );
		}

	private:
		void evt_req( const msg_request_t & req )
		{
			// Target was requested, so timeout must be refreshed.
			reschedule_timeout_check();

			++m_req_count;

			req.m_req->create_response()
				.append_header( "Server", "RESTinio sample server /v.0.2" )
				.append_header_date_field()
				.append_header( "Content-Type", "text/html; charset=utf-8" )
				.append_body( m_response_segments.m_begin_segment )
				.append_body( fmt::format( "{}", m_req_count ) )
				.append_body( m_response_segments.m_end_segment )
				.done();
		}

		// Check timeout signal.
		struct signal_timeout_check_t : public so_5::signal_t {};

		// Check if timeout occured.
		void evt_check_timeout()
		{
			if( std::chrono::steady_clock::now() >= m_timeout_point )
			{
				// Yes timeut occured.
				so_deregister_agent_coop_normally();
			}
		}

		// Response segments with 3 types of buffer optimization
		struct response_segments_t
		{
			response_segments_t( const std::string & target )
				:	m_begin_segment{
						std::make_shared< std::string >(
							fmt::format(
								"<html>\r\n"
								"<head><title>Target: {target}</title></head>\r\n"
								"<body>\r\n"
								"\t<h1>{target}</h1>\r\n"
								"\tTarget was requested ",
								fmt::arg( "target", target ) ) ) }
				,	m_end_segment{
						std::make_shared< std::string >(
							fmt::format(
								" time(s) since {}.\r\n"
								"</body>\r\n"
								"</html>\r\n",
								now_string_utc() ) ) }
			{}

			//! Cached response segments.
			std::shared_ptr< std::string > m_begin_segment;
			std::shared_ptr< std::string > m_end_segment;
		};

		const response_segments_t m_response_segments;

		// How many request were directed to this target since its creation.
		std::uint32_t m_req_count{ 0 };

		// TTL stuff.
		std::chrono::steady_clock::time_point m_timeout_point;
		so_5::timer_id_t m_timeout_timer_id;

		// Schedule new timeout check.
		void reschedule_timeout_check()
		{
			constexpr auto timeout = std::chrono::minutes( 5 );

			m_timeout_point = std::chrono::steady_clock::now() + timeout;

			m_timeout_timer_id = so_5::send_periodic< signal_timeout_check_t >(
					*this, timeout, std::chrono::steady_clock::duration::zero() );
		}
};

// Get a target name without params followed by '?'.
auto clarify_target( const std::string & target )
{
	std::string result;
	const auto pos = target.find( '?');

	if( std::string::npos == pos )
		result = target;
	else
		result = target.substr( 0, pos );

	return result;
}

//
// a_main_handler_t
//

// Default target handler.
class a_main_handler_t final : public so_5::agent_t
{
	public:
		a_main_handler_t( context_t ctx )
			:	so_5::agent_t{ std::move( ctx ) }
		{
			so_subscribe_self()
				.event( &a_main_handler_t::evt_forget_target )
				.event( &a_main_handler_t::evt_req );
		}

	private:
		// A message sent when target coop is deregistered.
		struct msg_forget_target_t final : public so_5::message_t
		{
			msg_forget_target_t( std::string target )
				:	m_target{ std::move( target ) }
			{}

			const std::string m_target;
		};

		// Handle timedout target.
		void evt_forget_target( const msg_forget_target_t & msg )
		{
			// Remove it from table of available targets.
			m_available_targets.erase( msg.m_target );
		}

		// Handle request.
		void evt_req( const msg_request_t & req )
		{
			const auto target =
				clarify_target( req.m_req->header().request_target() );

			if( target == "/" )
			{
				handle_request( req );
			}
			else
			{
				redirect_to_target( target, req );
			}
		}

		void handle_request( const msg_request_t & req )
		{
			auto response = req.m_req->create_response< restinio::chunked_output_t >();

			// Build main page:
			response
				.append_header( "Server", "RESTinio sample server /v.0.2" )
				.append_header_date_field()
				.append_header( "Content-Type", "text/html; charset=utf-8" )
				.append_chunk(
					restinio::const_buffer(
						"<html>"
						"<head><title>Targets list</title></head>"
						"<body>"
						"<h1>Available targets</h1>"
						"<ul>" ) );

			response.flush();
			// List of available targets.
			for( const auto & t : m_available_targets )
			{
				response.append_chunk(
					fmt::format(
						R"-(<li><a href="{0}">{0}</a></li>)-",
						t.first ) );
			}

			response.flush();

			response
				.append_chunk(
					"</ul>"
					"<br/>"
					"total: " + std::to_string( m_available_targets.size() ) +
					" target(s) available."
					"</body>"
					"</html>" )
				.done();
		}

		// Choose a target and send request to it.
		void redirect_to_target(
			const std::string & target,
			const msg_request_t & req )
		{
			const auto it = m_available_targets.find( target );

			so_5::mbox_t target_mbox;
			if( m_available_targets.end() == it )
			{
				// It is a new target, so create it.
				so_5::introduce_child_coop( *this,
					[&]( so_5::coop_t & coop )
					{
						//Set target mbox to send a request to it.
						target_mbox = coop.make_agent< a_target_t >( target )
								->so_direct_mbox();

						// Remember target.
						m_available_targets[ target ] = target_mbox;

						coop.set_exception_reaction(
							so_5::deregister_coop_on_exception );

						coop.add_dereg_notificator(
							[parent_mbox = so_direct_mbox(), target ](
								so_5::rt::environment_t & ,
								const std::string & ,
								const so_5::rt::coop_dereg_reason_t & )
							{
								so_5::send< msg_forget_target_t >(
									parent_mbox,
									std::move( target ) );
							} );
					} );

			}
			else
			{
				target_mbox = it->second;
			}

			so_5::send< msg_request_t >( target_mbox, req.m_req );
		}

	std::map< std::string, so_5::mbox_t > m_available_targets;
};

// Create request handler that sends a request to a specified mbox.
auto create_request_handler( so_5::mbox_t mbox )
{
	return [mbox]( restinio::request_handle_t req ){
			if( restinio::http_method_get() == req->header().method() )
			{
				so_5::send< msg_request_t >( mbox, std::move( req ) );
				return restinio::request_accepted();
			}

			return restinio::request_rejected();
		};
}

