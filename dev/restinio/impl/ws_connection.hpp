/*
	restinio
*/

/*!
	WebSocket connection routine.
*/

#pragma once

#include <asio.hpp>

#include <nodejs/http_parser/http_parser.h>

#include <fmt/format.h>

#include <restinio/exception.hpp>
#include <restinio/http_headers.hpp>
#include <restinio/connection_handle.hpp>
#include <restinio/request_handler.hpp>
#include <restinio/impl/header_helpers.hpp>
#include <restinio/impl/response_coordinator.hpp>
#include <restinio/impl/connection_settings.hpp>
#include <restinio/impl/fixed_buffer.hpp>
#include <restinio/impl/raw_resp_output_ctx.hpp>

namespace restinio
{

namespace impl
{

//
// ws_connection_t
//

//! Context for handling websocket connections.
/*
*/
template < typename TRAITS, typename WS_MESSAGE_HANDLER >
class ws_connection_t final
	:	public connection_base_t
{
	public:
		using message_handler_t = WS_MESSAGE_HANDLER;
		using logger_t = typename TRAITS::logger_t;
		using strand_t = typename TRAITS::strand_t;
		using stream_socket_t = typename TRAITS::stream_socket_t;

		ws_connection_t(
			//! Connection id.
			std::uint64_t conn_id,
			//! Connection socket.
			std::unique_ptr< stream_socket_t > socket,
			//! Settings that are common for connections.
			connection_settings_shared_ptr_t< TRAITS > settings,
			message_handler_t msg_handler )
			:	connection_base_t{ conn_id }
			,	m_socket{ std::move( socket ) }
			,	m_strand{ socket_lowest_layer().get_executor() }
			,	m_settings{ std::move( settings ) }
			,	m_input{ m_settings->m_buffer_size }
			,	m_msg_handler{ msg_handler }
			,	m_logger{ *( m_settings->m_logger ) }
		{
			// Notify of a new connection instance.
			m_logger.trace( [&]{
					return fmt::format(
						"[ws_connection:{}] start connection with {}",
						connection_id(),
						socket_lowest_layer().remote_endpoint() );
			} );
		}

		ws_connection_t( const ws_connection_t & ) = delete;
		ws_connection_t( ws_connection_t && ) = delete;
		void operator = ( const ws_connection_t & ) = delete;
		void operator = ( ws_connection_t && ) = delete;

		~ws_connection_t()
		{
			try
			{
				// Notify of a new connection instance.
				m_logger.trace( [&]{
					return fmt::format(
						"[ws_connection:{}] destroyed",
						connection_id() );
				} );
			}
			catch( ... )
			{}
		}

	private:
		//! An executor for callbacks on async operations.
		inline strand_t &
		get_executor()
		{
			return m_strand;
		}

		//! Close connection functions.
		//! \{

		//! Standard close routine.
		void
		close()
		{
			m_logger.trace( [&]{
				return fmt::format(
						"[ws_connection:{}] close",
						connection_id() );
			} );

			asio::error_code ignored_ec;
			socket_lowest_layer().shutdown(
				asio::ip::tcp::socket::shutdown_both,
				ignored_ec );
			socket_lowest_layer().close();
		}

		//! Trigger an error.
		/*!
			Closes the connection and write to log
			an error message.
		*/
		template< typename MSG_BUILDER >
		void
		trigger_error_and_close( MSG_BUILDER && msg_builder )
		{
			m_logger.error( std::move( msg_builder ) );

			close();
		}
		//! \}

		//! Connection
		std::unique_ptr< stream_socket_t > m_socket;

		stream_socket_t &
		socket_ref()
		{
			return *m_socket;
		}

		auto &
		socket_lowest_layer()
		{
			return m_socket->lowest_layer();
		}

		//! Sync object for connection events.
		strand_t m_strand;

		//! Common paramaters for buffer.
		connection_settings_shared_ptr_t< TRAITS > m_settings;

		message_handler_t m_msg_handler;

		//! Input routine.
		connection_input_t m_input;

		//! Write to socket operation context.
		raw_resp_output_ctx_t m_resp_out_ctx;

		//! Logger for operation
		logger_t & m_logger;
};

} /* namespace impl */

} /* namespace restinio */
