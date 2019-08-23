/*
	restinio
*/

/*!
	Connection settings.
*/

#pragma once

#include <memory>
#include <chrono>

#include <http_parser.h>

#include <restinio/connection_state_listener.hpp>

#include <restinio/utils/suppress_exceptions.hpp>

namespace restinio
{

namespace impl
{

namespace connection_settings_details
{

/*!
 * @brief A class for holding actual state listener.
 *
 * This class holds shared pointer to actual state listener object and
 * provides actual call_state_listener() and
 * call_state_listener_suppressing_exceptions() implementations.
 *
 * @since v.0.5.1
 */
template< typename Listener >
struct state_listener_holder_t
{
	std::shared_ptr< Listener > m_connection_state_listener;

	template< typename Settings >
	state_listener_holder_t(
		const Settings & settings )
		:	m_connection_state_listener{ settings.connection_state_listener() }
	{}

	template< typename Lambda >
	void
	call_state_listener( Lambda && lambda ) const
	{
		m_connection_state_listener->state_changed( lambda() );
	}

	template< typename Lambda >
	void
	call_state_listener_suppressing_exceptions(
		Lambda && lambda ) const noexcept
	{
		restinio::utils::suppress_exceptions_quietly( [&] {
				m_connection_state_listener->state_changed( lambda() );
			} );
	}
};

/*!
 * @brief A specialization of state_listener_holder for case of
 * noop_listener.
 *
 * This class doesn't hold anything and doesn't do anything.
 *
 * @since v.0.5.1
 */
template<>
struct state_listener_holder_t< connection_state::noop_listener_t >
{
	template< typename Settings >
	state_listener_holder_t( const Settings & ) { /* nothing to do */ }

	template< typename Lambda >
	void
	call_state_listener( Lambda && /*lambda*/ ) const noexcept
	{
		/* nothing to do */
	}

	template< typename Lambda >
	void
	call_state_listener_suppressing_exceptions(
		Lambda && /*lambda*/ ) const noexcept
	{
		/* nothing to do */
	}
};

} /* namespace connection_settings_details */

//
// connection_settings_t
//

//! Parameters shared between connections.
/*!
	Each connection has access to common params and
	server-agent throught this object.
*/
template < typename Traits >
struct connection_settings_t final
	:	public std::enable_shared_from_this< connection_settings_t< Traits > >
	,	public connection_settings_details::state_listener_holder_t<
				typename Traits::connection_state_listener_t >
{
	using timer_manager_t = typename Traits::timer_manager_t;
	using timer_manager_handle_t = std::shared_ptr< timer_manager_t >;
	using request_handler_t = typename Traits::request_handler_t;
	using logger_t = typename Traits::logger_t;

	using connection_state_listener_holder_t =
			connection_settings_details::state_listener_holder_t<
					typename Traits::connection_state_listener_t >;

	connection_settings_t( const connection_settings_t & ) = delete;
	connection_settings_t( const connection_settings_t && ) = delete;
	connection_settings_t & operator = ( const connection_settings_t & ) = delete;
	connection_settings_t & operator = ( connection_settings_t && ) = delete;

	template < typename Settings >
	connection_settings_t(
		Settings && settings,
		http_parser_settings parser_settings,
		timer_manager_handle_t timer_manager )
		:	connection_state_listener_holder_t{ settings }
		,	m_request_handler{ settings.request_handler() }
		,	m_parser_settings{ parser_settings }
		,	m_buffer_size{ settings.buffer_size() }
		,	m_read_next_http_message_timelimit{
				settings.read_next_http_message_timelimit() }
		,	m_write_http_response_timelimit{
				settings.write_http_response_timelimit() }
		,	m_handle_request_timeout{
				settings.handle_request_timeout() }
		,	m_max_pipelined_requests{ settings.max_pipelined_requests() }
		,	m_logger{ settings.logger() }
		,	m_timer_manager{ std::move( timer_manager ) }
	{
		if( !m_timer_manager )
			throw exception_t{ "timer manager not set" };
	}

	//! Request handler factory.
	std::unique_ptr< request_handler_t > m_request_handler;

	//! Parser settings.
	/*!
		Parsing settings are common for each connection.
	*/
	const http_parser_settings m_parser_settings;

	//! Params from server_settings_t.
	//! \{
	std::size_t m_buffer_size;

	std::chrono::steady_clock::duration
		m_read_next_http_message_timelimit{ std::chrono::seconds( 60 ) };

	std::chrono::steady_clock::duration
		m_write_http_response_timelimit{ std::chrono::seconds( 5 ) };

	std::chrono::steady_clock::duration
		m_handle_request_timeout{ std::chrono::seconds( 10 ) };

	std::size_t m_max_pipelined_requests;

	const std::unique_ptr< logger_t > m_logger;
	//! \}

	//! Create new timer guard.
	auto
	create_timer_guard()
	{
		return m_timer_manager->create_timer_guard();
	}

private:
	//! Timer factory for timout guards.
	timer_manager_handle_t m_timer_manager;
};

template < typename Traits >
using connection_settings_handle_t =
	std::shared_ptr< connection_settings_t< Traits > >;

} /* namespace impl */

} /* namespace restinio */

