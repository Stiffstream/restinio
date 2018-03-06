/*
	restinio
*/

/*!
	Connection settings.
*/

#pragma once

#include <memory>
#include <chrono>

#include <nodejs/http_parser/http_parser.h>

namespace restinio
{

namespace impl
{

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
{
	using timer_manager_t = typename Traits::timer_manager_t;
	using timer_manager_handle_t = std::shared_ptr< timer_manager_t >;
	using request_handler_t = typename Traits::request_handler_t;
	using logger_t = typename Traits::logger_t;

	connection_settings_t( const connection_settings_t & ) = delete;
	connection_settings_t( const connection_settings_t && ) = delete;
	void operator = ( const connection_settings_t & ) = delete;
	void operator = ( const connection_settings_t && ) = delete;

	template < typename Settings >
	connection_settings_t(
		Settings && settings,
		http_parser_settings parser_settings,
		timer_manager_handle_t timer_manager )
		:	m_request_handler{ settings.request_handler() }
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

