/*
	restinio
*/

/*!
	WebSocket connection routine.
*/

#pragma once

#include <queue>

#include <restinio/asio_include.hpp>

#include <http_parser.h>

#include <restinio/impl/include_fmtlib.hpp>

#include <restinio/all.hpp>
#include <restinio/impl/executor_wrapper.hpp>
#include <restinio/impl/write_group_output_ctx.hpp>
#include <restinio/websocket/message.hpp>
#include <restinio/websocket/impl/ws_parser.hpp>
#include <restinio/websocket/impl/ws_protocol_validator.hpp>

#include <restinio/utils/impl/safe_uint_truncate.hpp>

#include <restinio/compiler_features.hpp>

namespace restinio
{

namespace websocket
{

namespace basic
{

namespace impl
{

using write_groups_queue_t = std::queue< write_group_t >;

//! Max possible size of websocket frame header (a part before payload).
constexpr size_t
websocket_header_max_size()
{
	return 14;
}

//
// ws_outgoing_data_t
//

//! A queue for outgoing buffers.
class ws_outgoing_data_t
{
	public:
		//! Add buffers to queue.
		void
		append( write_group_t wg )
		{
			m_awaiting_write_groups.emplace( std::move( wg ) );
		}

		optional_t< write_group_t >
		pop_ready_buffers()
		{
			optional_t< write_group_t > result;

			if( !m_awaiting_write_groups.empty() )
			{
				result = std::move( m_awaiting_write_groups.front() );
				m_awaiting_write_groups.pop();
			}

			return result;
		}

	private:
		//! A queue of buffers.
		write_groups_queue_t m_awaiting_write_groups;
};

//
// connection_input_t
//

//! Websocket input stuff.
struct connection_input_t
{
	connection_input_t( std::size_t buffer_size )
		:	m_buf{ buffer_size }
	{}

	//! websocket parser.
	ws_parser_t m_parser;

	//! Input buffer.
	restinio::impl::fixed_buffer_t m_buf;

	//! Current payload.
	std::string m_payload;

	//! Prepare parser for reading new http-message.
	void
	reset_parser_and_payload()
	{
		m_parser.reset();
		m_payload.clear();
	}
};

//
// ws_connection_t
//

//! Context for handling websocket connections.
template <
		typename Traits,
		typename WS_Message_Handler >
class ws_connection_t final
	:	public ws_connection_base_t
	,	public restinio::impl::executor_wrapper_t< typename Traits::strand_t >
{
		using executor_wrapper_base_t = restinio::impl::executor_wrapper_t< typename Traits::strand_t >;

	public:
		using message_handler_t = WS_Message_Handler;

		using timer_manager_t = typename Traits::timer_manager_t;
		using timer_manager_handle_t = std::shared_ptr< timer_manager_t >;
		using timer_guard_t = typename timer_manager_t::timer_guard_t;
		using logger_t = typename Traits::logger_t;
		using strand_t = typename Traits::strand_t;
		using stream_socket_t = typename Traits::stream_socket_t;

		using ws_weak_handle_t = std::weak_ptr< ws_t >;

		ws_connection_t(
			//! Connection id.
			connection_id_t conn_id,
			//! Data inherited from http-connection.
			//! \{
			restinio::impl::connection_settings_handle_t< Traits > settings,
			stream_socket_t socket,
			//! \}
			message_handler_t msg_handler )
			:	ws_connection_base_t{ conn_id }
			,	executor_wrapper_base_t{ socket.get_executor() }
			,	m_settings{ std::move( settings ) }
			,	m_socket{ std::move( socket ) }
			,	m_timer_guard{ m_settings->create_timer_guard() }
			,	m_input{ websocket_header_max_size() }
			,	m_msg_handler{ std::move( msg_handler ) }
			,	m_logger{ *( m_settings->m_logger ) }
		{
			// Notify of a new connection instance.
			m_logger.trace( [&]{
					return fmt::format(
						"[connection:{}] move socket to [ws_connection:{}]",
						connection_id(),
						connection_id() );
			} );

			m_logger.trace( [&]{
					return fmt::format(
						"[ws_connection:{}] start connection with {}",
						connection_id(),
						m_socket.remote_endpoint() );
			} );

			// Inform state listener if it used.
			m_settings->call_state_listener( [this]() noexcept {
					return connection_state::notice_t {
							connection_id(),
							m_socket.remote_endpoint(),
							connection_state::upgraded_to_websocket_t{}
						};
				} );
		}

		ws_connection_t( const ws_connection_t & ) = delete;
		ws_connection_t( ws_connection_t && ) = delete;
		ws_connection_t & operator = ( const ws_connection_t & ) = delete;
		ws_connection_t & operator = ( ws_connection_t && ) = delete;

		~ws_connection_t() override
		{
			try
			{
				// Notify of a new connection instance.
				m_logger.trace( [&]{
					return fmt::format(
						"[ws_connection:{}] destructor called",
						connection_id() );
				} );
			}
			catch( ... )
			{}
		}

		//! Shutdown websocket.
		virtual void
		shutdown() override
		{
			asio_ns::dispatch(
				this->get_executor(),
				[ this, ctx = shared_from_this() ]
				// NOTE: this lambda is noexcept since v.0.6.0.
				() noexcept {
					try
					{
						// An exception from logger shouldn't prevent
						// main shutdown actions.
						restinio::utils::log_trace_noexcept( m_logger,
							[&]{
								return fmt::format(
									"[ws_connection:{}] shutdown",
									connection_id() );
							} );

						m_close_frame_to_user.disable();
						graceful_close();
					}
					catch( const std::exception & ex )
					{
						restinio::utils::log_error_noexcept( m_logger,
							[&]{
								return fmt::format(
									"[ws_connection:{}] shutdown operation error: {}",
									connection_id(),
									ex.what() );
							} );
					}
			} );
		}

		//! Kill websocket.
		virtual void
		kill() override
		{
			asio_ns::dispatch(
				this->get_executor(),
				[ this, ctx = shared_from_this() ]
				// NOTE: this lambda is noexcept since v.0.6.0.
				() noexcept
				{
					try
					{
						// An exception from logger shouldn't prevent
						// main kill actions.
						restinio::utils::log_trace_noexcept( m_logger,
							[&]{
								return fmt::format(
									"[ws_connection:{}] kill",
									connection_id() );
							} );

						m_close_frame_to_user.disable();
						m_close_frame_to_peer.disable();

						close_impl();
					}
					catch( const std::exception & ex )
					{
						restinio::utils::log_error_noexcept( m_logger,
							[&]{
								return fmt::format(
									"[ws_connection:{}] kill operation error: {}",
									connection_id(),
									ex.what() );
							} );
					}
			} );
		}

		//! Start reading ws-messages.
		void
		init_read( ws_handle_t wsh ) override
		{
			ws_weak_handle_t wswh{ wsh };

			// Run write message on io_context loop (direct invocation if possible).
			asio_ns::dispatch(
				this->get_executor(),
				[ this, ctx = shared_from_this(), wswh = std::move( wswh ) ]
				// NOTE: this lambda is noexcept since v.0.6.0.
				() noexcept
				{
					try
					{
						// Start timeout checking.
						m_prepared_weak_ctx = shared_from_this();
						init_next_timeout_checking();

						m_websocket_weak_handle = std::move( wswh );
						m_read_state = read_state_t::read_any_frame;
						start_read_header();
					}
					catch( const std::exception & ex )
					{
						trigger_error_and_close(
							status_code_t::unexpected_condition,
							[&]{
								return fmt::format(
									"[ws_connection:{}] unable to init read: {}",
									connection_id(),
									ex.what() );
							} );
					}
			} );
		}

		//! Write pieces of outgoing data.
		virtual void
		write_data(
			write_group_t wg,
			bool is_close_frame ) override
		{
			//! Run write message on io_context loop if possible.
			asio_ns::dispatch(
				this->get_executor(),
				[ this,
					actual_wg = std::move( wg ),
					ctx = shared_from_this(),
					is_close_frame ]
				// NOTE: this lambda is noexcept since v.0.6.0.
				() mutable noexcept
				{
					try
					{
						if( write_state_t::write_enabled == m_write_state )
							write_data_impl(
								std::move( actual_wg ),
								is_close_frame );
						else
						{
							m_logger.warn( [&]{
								return fmt::format(
										"[ws_connection:{}] cannot write to websocket: "
										"write operations disabled",
										connection_id() );
							} );
						}
					}
					catch( const std::exception & ex )
					{
						trigger_error_and_close(
							status_code_t::unexpected_condition,
							[&]{
								return fmt::format(
									"[ws_connection:{}] unable to write data: {}",
									connection_id(),
									ex.what() );
							} );
					}
				} );
		}
	private:
		//! Standard close routine.
		/*!
		 * @note
		 * This method is noexcept since v.0.6.0.
		 */
		void
		close_impl() noexcept
		{
			m_close_impl.run_if_first(
				[&]() noexcept {
					restinio::utils::log_trace_noexcept( m_logger,
						[&]{
							return fmt::format(
									"[ws_connection:{}] close socket",
									connection_id() );
						} );

					// This actions can throw and because of that we have
					// to wrap them...
					restinio::utils::suppress_exceptions(
							m_logger,
							"ws_connection.close_impl.socket.shutdown",
							[&] {
								asio_ns::error_code ignored_ec;
								m_socket.shutdown(
									asio_ns::ip::tcp::socket::shutdown_both,
									ignored_ec );
							} );

					restinio::utils::suppress_exceptions(
							m_logger,
							"ws_connection.close_impl.socket.close",
							[&] {
								m_socket.close();
							} );
				} );
		}

		//! Start waiting for close-frame.
		void
		start_waiting_close_frame_only()
		{
			m_read_state = read_state_t::read_only_close_frame;
			guard_close_frame_from_peer_operation();
		}

		//! Close WebSocket connection in a graceful manner.
		void
		graceful_close()
		{
			m_close_frame_to_peer.run_if_first(
				[&]{
					send_close_frame_to_peer( status_code_t::normal_closure );
					start_waiting_close_frame_only();
				} );
		}

		//! Send close frame to peer.
		void
		send_close_frame_to_peer( std::string payload )
		{
			writable_items_container_t bufs;
			bufs.reserve( 2 );

			bufs.emplace_back(
				impl::write_message_details(
					final_frame,
					opcode_t::connection_close_frame,
					payload.size() ) );

			bufs.emplace_back( std::move( payload ) );
			m_outgoing_data.append( write_group_t{ std::move( bufs ) } );

			init_write_if_necessary();

			// No more data must be written.
			m_write_state = write_state_t::write_disabled;
		}

		//! Send close frame to peer.
		void
		send_close_frame_to_peer(
			status_code_t code,
			std::string desc = std::string{} )
		{
			send_close_frame_to_peer( std::string{ status_code_to_bin( code ) + desc } );
		}

		//! Trigger an error.
		/*!
			Writes error message to log,
			closes socket,
			and sends close frame to user if necessary.

			@note
			This method is noexcept since v.0.6.0
		*/
		template< typename MSG_BUILDER >
		void
		trigger_error_and_close(
			status_code_t status,
			MSG_BUILDER msg_builder ) noexcept
		{
			// An exception in logger shouldn't prevent the main actions.
			restinio::utils::log_error_noexcept(
					m_logger, std::move( msg_builder ) );

			// This can throw but we have to suppress any exceptions.
			restinio::utils::suppress_exceptions(
					m_logger, "ws_connection.call_close_handler_if_necessary",
					[this, status] {
						call_close_handler_if_necessary( status );
					} );

			RESTINIO_ENSURE_NOEXCEPT_CALL( close_impl() );
		}


		//! Start the process of reading ws messages from socket.
		void
		start_read_header()
		{
			m_logger.trace( [&]{
				return fmt::format(
						"[ws_connection:{}] start reading header",
						connection_id() );
			} );

			// Prepare parser for consuming new message.
			m_input.reset_parser_and_payload();

			if( 0 == m_input.m_buf.length() )
			{
				consume_header_from_socket();
			}
			else
			{
				// Has something to read from m_input.m_buf.
				consume_header_from_buffer(
					m_input.m_buf.bytes(), m_input.m_buf.length() );
			}
		}

		//! Initiate read operation on socket to receive bytes for header.
		void
		consume_header_from_socket()
		{
			m_logger.trace( [&]{
				return fmt::format(
						"[ws_connection:{}] continue reading message",
						connection_id() );
			} );

			m_socket.async_read_some(
				m_input.m_buf.make_asio_buffer(),
				asio_ns::bind_executor(
					this->get_executor(),
					[ this, ctx = shared_from_this() ]
					// NOTE: this lambda is noexcept since v.0.6.0.
					( const asio_ns::error_code & ec, std::size_t length ) noexcept
					{
						try
						{
							after_read_header( ec, length );
						}
						catch( const std::exception & ex )
						{
							trigger_error_and_close(
								status_code_t::unexpected_condition,
								[&]{
									return fmt::format(
										"[ws_connection:{}] after read header callback error: {}",
										connection_id(),
										ex.what() );
								} );
						}
					} ) );
		}

		//! Handle read error (reading header or payload)
		void
		handle_read_error( const char * desc, const asio_ns::error_code & ec )
		{
			// Assume that connection is lost.
			trigger_error_and_close(
				status_code_t::connection_lost,
				[&]{
					return fmt::format(
						"[ws_connection:{}] {}: {}",
						connection_id(),
						desc,
						ec.message() );
				} );
		}

		//! Handle read operation result, when reading header.
		void
		after_read_header(
			const asio_ns::error_code & ec,
			std::size_t length )
		{
			if( !ec )
			{
				m_logger.trace( [&]{
					return fmt::format(
							"[ws_connection:{}] received {} bytes",
							this->connection_id(),
							length );
				} );

				m_input.m_buf.obtained_bytes( length );
				consume_header_from_buffer( m_input.m_buf.bytes(), length );
			}
			else
			{
				handle_read_error( "reading message header error", ec );
			}
		}

		//! Parse header from internal buffer.
		void
		consume_header_from_buffer( const char * data, std::size_t length )
		{
			const auto nparsed = m_input.m_parser.parser_execute( data, length );

			m_input.m_buf.consumed_bytes( nparsed );

			if( m_input.m_parser.header_parsed() )
			{
				handle_parsed_header( m_input.m_parser.current_message() );
			}
			else
			{
				assert( nparsed == length );
				consume_header_from_socket();
			}
		}

		//! Handle parsed header.
		void
		handle_parsed_header( const message_details_t & md )
		{
			m_logger.trace( [&]{
				return fmt::format(
						"[ws_connection:{}] start handling {} ({:#x})",
						connection_id(),
						opcode_to_string( md.m_opcode ),
						static_cast<std::uint16_t>(md.m_opcode) );
			} );

			const auto validation_result =
				m_protocol_validator.process_new_frame( md );

			if( validation_state_t::frame_header_is_valid != validation_result )
			{
				m_logger.error( [&]{
					return fmt::format(
							"[ws_connection:{}] invalid header",
							connection_id() );
				} );

				if( read_state_t::read_any_frame == m_read_state )
				{
					m_close_frame_to_peer.run_if_first(
						[&]{
							send_close_frame_to_peer( status_code_t::protocol_error );
							// Do not wait anything in return, because
							// protocol is violated.
						} );

					call_close_handler_if_necessary( status_code_t::protocol_error );
				}
				else if( read_state_t::read_only_close_frame == m_read_state )
				{
					// Wait for close frame cannot be done.
					close_impl();
				}

				return;
			}

			handle_parsed_and_valid_header( md );
		}

		//! Handle parsed and valid header.
		void
		handle_parsed_and_valid_header( const message_details_t & md )
		{
			const auto payload_length =
					restinio::utils::impl::uint64_to_size_t(md.payload_len());

			m_input.m_payload.resize( payload_length );

			if( payload_length == 0 )
			{
				// Callback for message with 0-size payload.
				call_handler_on_current_message();
			}
			else
			{
				const auto payload_part_size =
							std::min( m_input.m_buf.length(), payload_length );

				std::memcpy(
					&m_input.m_payload.front(),
					m_input.m_buf.bytes(),
					payload_part_size );

				m_input.m_buf.consumed_bytes( payload_part_size );

				const std::size_t length_remaining =
					payload_length - payload_part_size;

				if( validate_payload_part(
						&m_input.m_payload.front(),
						payload_part_size,
						length_remaining ) )
				{
					if( 0 == length_remaining )
					{
						// All message is obtained.
						call_handler_on_current_message();
					}
					else
					{
						// Read the rest of payload:
						start_read_payload(
							&m_input.m_payload.front() + payload_part_size,
							length_remaining );
					}
				}
				// Else payload is invalid and validate_payload_part()
				// has handled the case so do nothing.
			}
		}

		//! Start reading message payload.
		void
		start_read_payload(
			//! A pointer to the remainder of unfetched payload.
			char * payload_data,
			//! The size of the remainder of unfetched payload.
			std::size_t length_remaining,
			//! Validate payload and call handler.
			bool do_validate_payload_and_call_msg_handler = true )
		{
			m_socket.async_read_some(
				asio_ns::buffer( payload_data, length_remaining ),
				asio_ns::bind_executor(
					this->get_executor(),
					[ this,
						ctx = shared_from_this(),
						payload_data,
						length_remaining,
						do_validate_payload_and_call_msg_handler ]
						// NOTE: this lambda is noexcept since v.0.6.0.
						( const asio_ns::error_code & ec, std::size_t length ) noexcept
						{
							try
							{
								after_read_payload(
									payload_data,
									length_remaining,
									ec,
									length,
									do_validate_payload_and_call_msg_handler );
							}
							catch( const std::exception & ex )
							{
								trigger_error_and_close(
									status_code_t::unexpected_condition,
									[&]{
										return fmt::format(
											"[ws_connection:{}] after read payload callback error: {}",
											connection_id(),
											ex.what() );
									} );
							}
						} ) );
		}

		//! Handle read operation result, when reading payload.
		void
		after_read_payload(
			char * payload_data,
			std::size_t length_remaining,
			const asio_ns::error_code & ec,
			std::size_t length,
			bool do_validate_payload_and_call_msg_handler = true )
		{
			if( !ec )
			{
				m_logger.trace( [&]{
					return fmt::format(
							"[ws_connection:{}] received {} bytes",
							this->connection_id(),
							length );
				} );

				assert( length <= length_remaining );

				const std::size_t next_length_remaining =
					length_remaining - length;

				if( do_validate_payload_and_call_msg_handler )
				{
					if( validate_payload_part( payload_data, length, next_length_remaining ) )
					{
						if( 0 == next_length_remaining )
						{
							// Here: all the payload is ready.

							// All message is obtained.
							call_handler_on_current_message();
						}
						else
						{
							//Here: not all payload is obtained,
							// so inintiate read once again:
							start_read_payload(
								payload_data + length,
								next_length_remaining,
								do_validate_payload_and_call_msg_handler );
						}
					}
					// Else payload is invalid and validate_payload_part()
					// has handled the case so do nothing.
				}
				else
				{
					if( 0 == next_length_remaining )
					{
						start_read_header();
					}
					else
					{
						start_read_payload(
							payload_data + length,
							length_remaining - length,
							do_validate_payload_and_call_msg_handler );
					}
				}
			}
			else
			{
				handle_read_error( "reading message payload error", ec );
			}
		}

		//! Call user message handler with current message.
		void
		call_message_handler( message_handle_t close_frame )
		{
			if( auto wsh = m_websocket_weak_handle.lock() )
			{
				try
				{
					m_msg_handler(
						std::move( wsh ),
						std::move( close_frame ) );
				}
				catch( const std::exception & ex )
				{
					m_logger.error( [&]{
						return fmt::format(
								"[ws_connection:{}] execute handler error: {}",
								connection_id(),
								ex.what() );
					} );
				}
			}
		}

		//! Validates a part of received payload.
		bool
		validate_payload_part(
			char * payload_data,
			std::size_t length,
			std::size_t next_length_remaining )
		{
			const auto validation_result =
				m_protocol_validator.process_and_unmask_next_payload_part( payload_data, length );

			if( validation_state_t::payload_part_is_valid != validation_result )
			{
				handle_invalid_payload( validation_result );

				if( validation_state_t::incorrect_utf8_data == validation_result )
				{
					// Can skip this payload because it was not a bad close frame.

					// It is the case we are expecting close frame
					// so validator must be ready to receive more headers
					// and payloads after this frame.
					m_protocol_validator.reset();

					if( 0 == next_length_remaining )
					{
						start_read_header();
					}
					else
					{
						// Skip checking payload for this frame:
						const bool do_validate_payload_and_call_msg_handler = false;
						start_read_payload(
								payload_data + length,
								next_length_remaining,
								do_validate_payload_and_call_msg_handler );
					}
				}
				return false;
			}

			return true;
		}

		//! Handle payload errors.
		void
		handle_invalid_payload( validation_state_t validation_result )
		{
			m_logger.error( [&]{
					return fmt::format(
							"[ws_connection:{}] invalid paload",
							connection_id() );
				} );

			if( validation_state_t::invalid_close_code == validation_result  )
			{
				// A corner case: invalid payload in close frame.

				if( read_state_t::read_any_frame == m_read_state )
				{
					// Case: close frame was not expected.

					// This actually must be executed:
					m_close_frame_to_peer.run_if_first(
						[&]{
							send_close_frame_to_peer( status_code_t::protocol_error );
							// Do not wait anything in return, because
							// protocol is violated.
						} );

					// Notify user of a close but use a correct close code.
					call_close_handler_if_necessary( status_code_t::protocol_error );
				}
				else if( read_state_t::read_only_close_frame == m_read_state )
				{
					// Case: close frame was expected.

					// We got a close frame but it is incorrect,
					// so just close (there is not too much we can do).
					close_impl();
				}
			}
			else
			{
				if( read_state_t::read_any_frame == m_read_state )
				{
					m_close_frame_to_peer.run_if_first(
						[&]{
							send_close_frame_to_peer( status_code_t::invalid_message_data );
							start_waiting_close_frame_only();
						} );

					call_close_handler_if_necessary( status_code_t::invalid_message_data );
				}
			}
		}

		void
		call_handler_on_current_message()
		{
			auto & md = m_input.m_parser.current_message();

			const auto validation_result = m_protocol_validator.finish_frame();
			if( validation_state_t::frame_is_valid == validation_result )
			{
				if( read_state_t::read_any_frame == m_read_state )
				{
					if( opcode_t::connection_close_frame == md.m_opcode )
					{
						m_logger.trace( [&]{
							return fmt::format(
									"[ws_connection:{}] got close frame from peer, status: {}",
									connection_id(),
									static_cast<std::uint16_t>(
											status_code_from_bin( m_input.m_payload )) );
						} );

						m_close_frame_to_user.disable();
						m_close_frame_to_peer.run_if_first(
							[&]{
								send_close_frame_to_peer( m_input.m_payload );
							} );

						m_read_state = read_state_t::read_nothing;
					}

					call_message_handler(
						std::make_shared< message_t >(
							md.m_final_flag ? final_frame : not_final_frame,
							md.m_opcode,
							std::move( m_input.m_payload ) ) );

					if( read_state_t::read_nothing != m_read_state )
						start_read_header();
				}
				else
				{
					assert( read_state_t::read_only_close_frame == m_read_state );

					if( opcode_t::connection_close_frame == md.m_opcode )
					{
						// Got it!
						m_timer_guard.cancel();

						close_impl();

						m_logger.trace( [&]{
							return fmt::format(
									"[ws_connection:{}] expected close frame came",
									connection_id() );
						} );
					}
					else
					{
						// Wait for next frame.
						start_read_header();
					}
				}
			}
			else
			{
				handle_invalid_payload( validation_result );
			}
		}

		void
		call_close_handler_if_necessary( status_code_t status )
		{
			m_close_frame_to_user.run_if_first(
				[&]{
					call_message_handler(
						std::make_shared< message_t >(
							final_frame,
							opcode_t::connection_close_frame,
							status_code_to_bin( status ) ) );
				} );
		}

		//! Implementation of writing data performed on the asio_ns::io_context.
		void
		write_data_impl( write_group_t wg, bool is_close_frame )
		{
			if( m_socket.is_open() )
			{
				if( is_close_frame )
				{
					m_logger.trace( [&]{
						return fmt::format(
								"[ws_connection:{}] user sends close frame",
								connection_id() );
					} );

					m_close_frame_to_peer.disable(); // It is formed and sent by user
					m_close_frame_to_user.disable(); // And user knows that websocket is closed.
					// No more writes.
					m_write_state = write_state_t::write_disabled;

					// Start waiting only close-frame.
					start_waiting_close_frame_only();
				}

				// Push write_group to queue.
				m_outgoing_data.append( std::move( wg ) );

				init_write_if_necessary();
			}
			else
			{
				m_logger.warn( [&]{
					return fmt::format(
							"[ws_connection:{}] try to write while socket is closed",
							connection_id() );
				} );

				try
				{
					wg.invoke_after_write_notificator_if_exists(
						make_asio_compaible_error(
							asio_convertible_error_t::write_was_not_executed ) );
				}
				catch( ... )
				{}
			}
		}

		//! Checks if there is something to write,
		//! and if so starts write operation.
		void
		init_write_if_necessary()
		{
			if( !m_write_output_ctx.transmitting() )
			{
				init_write();
			}
		}

		//! Initiate write operation.
		void
		init_write()
		{
			// Here: not writing anything to socket, so
			// write operation can be initiated.
			auto next_write_group = m_outgoing_data.pop_ready_buffers();

			if( next_write_group )
			{
				m_logger.trace( [&]{
					return fmt::format(
						"[ws_connection:{}] start next write group, "
						"size: {}",
						this->connection_id(),
						next_write_group->items_count() );
				} );

				// Initialize write context with a new write group.
				m_write_output_ctx.start_next_write_group(
					std::move( next_write_group ) );

				// Start the loop of sending data from current write group.
				handle_current_write_ctx();
			}
		}

		// Use aliases for shorter names.
		using none_write_operation_t = ::restinio::impl::write_group_output_ctx_t::none_write_operation_t;
		using trivial_write_operation_t = ::restinio::impl::write_group_output_ctx_t::trivial_write_operation_t;
		using file_write_operation_t = ::restinio::impl::write_group_output_ctx_t::file_write_operation_t;

		void
		handle_current_write_ctx()
		{
			try
			{
				auto wo = m_write_output_ctx.extract_next_write_operation();

				if( holds_alternative< trivial_write_operation_t >( wo ) )
				{
					handle_trivial_write_operation( get< trivial_write_operation_t >( wo ) );
				}
				else if( holds_alternative< none_write_operation_t >( wo ) )
				{
					finish_handling_current_write_ctx();
				}
				else
				{
					assert( holds_alternative< file_write_operation_t >( wo ) );
					throw exception_t{ "sendfile write operation not implemented" };
				}
			}
			catch( const std::exception & ex )
			{
				trigger_error_and_close(
					status_code_t::unexpected_condition,
					[&]{
						return fmt::format(
							"[ws_connection:{}] handle_current_write_ctx failed: {}",
							connection_id(),
							ex.what() );
					} );
			}
		}

		void
		handle_trivial_write_operation( const trivial_write_operation_t & op )
		{
			// Asio buffers (param for async write):
			auto & bufs = op.get_trivial_bufs();

			m_logger.trace( [&]{
				return fmt::format(
					"[ws_connection:{}] sending data with "
					"buf count: {}, "
					"total size: {}",
					connection_id(),
					bufs.size(),
					op.size() ); } );

			guard_write_operation();

			// There is somethig to write.
			asio_ns::async_write(
				m_socket,
				bufs,
				asio_ns::bind_executor(
					this->get_executor(),
					[ this,
						ctx = shared_from_this() ]
					// NOTE: this lambda is noexcept since v.0.6.0.
					( const asio_ns::error_code & ec, std::size_t written ) noexcept
					{
						try
						{
							if( !ec )
							{
								m_logger.trace( [&]{
									return fmt::format(
											"[ws_connection:{}] outgoing data was sent: {} bytes",
											connection_id(),
											written );
								} );
							}

							after_write( ec );
						}
						catch( const std::exception & ex )
						{
							trigger_error_and_close(
								status_code_t::unexpected_condition,
								[&]{
									return fmt::format(
										"[ws_connection:{}] after write callback error: {}",
										connection_id(),
										ex.what() );
								} );
						}
				} ) );
		}

		//! Do post write actions for current write group.
		void
		finish_handling_current_write_ctx()
		{
			// Finishing writing this group.
			m_logger.trace( [&]{
				return fmt::format(
						"[ws_connection:{}] finishing current write group",
						this->connection_id() );
			} );

			// Group notificators are called from here (if exist):
			m_write_output_ctx.finish_write_group();

			// Start another write opertion
			// if there is something to send.
			init_write_if_necessary();
		}

		//! Handle write response finished.
		void
		after_write( const asio_ns::error_code & ec )
		{
			if( !ec )
			{
				handle_current_write_ctx();
			}
			else
			{
				trigger_error_and_close(
					status_code_t::connection_lost,
					[&]{
						return fmt::format(
							"[ws_connection:{}] unable to write: {}",
							connection_id(),
							ec.message() );
					} );

				try
				{
					m_write_output_ctx.fail_write_group( ec );
				}
				catch( const std::exception & ex )
				{
					m_logger.error( [&]{
						return fmt::format(
							"[ws_connection:{}] notificator error: {}",
							connection_id(),
							ex.what() );
					} );
				}
			}
		}

		//! Common paramaters of a connection.
		restinio::impl::connection_settings_handle_t< Traits > m_settings;

		//! Connection.
		stream_socket_t m_socket;

		//! Timers.
		//! \{
		static ws_connection_t &
		cast_to_self( tcp_connection_ctx_base_t & base )
		{
			return static_cast< ws_connection_t & >( base );
		}

		virtual void
		check_timeout( tcp_connection_ctx_handle_t & self ) override
		{
			asio_ns::dispatch(
				this->get_executor(),
				[ ctx = std::move( self ) ]
				// NOTE: this lambda is noexcept since v.0.6.0.
				() noexcept
				{
					auto & conn_object = cast_to_self( *ctx );
					// If an exception will be thrown we can only
					// close the connection.
					try
					{
						conn_object.check_timeout_impl();
					}
					catch( const std::exception & x )
					{
						conn_object.trigger_error_and_close(
							status_code_t::unexpected_condition,
							[&] {
								return fmt::format( "[connection: {}] unexpected "
										"error during timeout handling: {}",
										conn_object.connection_id(),
										x.what() );
							} );
					}
				} );
		}

		std::chrono::steady_clock::time_point m_write_operation_timeout_after;
		std::chrono::steady_clock::time_point m_close_frame_from_peer_timeout_after =
			std::chrono::steady_clock::time_point::max();
		tcp_connection_ctx_weak_handle_t m_prepared_weak_ctx;
		timer_guard_t m_timer_guard;

		void
		check_timeout_impl()
		{
			const auto now = std::chrono::steady_clock::now();
			if( m_write_output_ctx.transmitting() && now > m_write_operation_timeout_after )
			{
				m_logger.trace( [&]{
					return fmt::format(
							"[wd_connection:{}] write operation timed out",
							connection_id() );
					} );
				m_close_frame_to_peer.disable();
				call_close_handler_if_necessary( status_code_t::unexpected_condition );
				close_impl();
			}
			else if( now > m_close_frame_from_peer_timeout_after )
			{
				m_logger.trace( [&]{
					return fmt::format(
							"[wd_connection:{}] waiting for close-frame from peer timed out",
							connection_id() );
					} );
				close_impl();
			}
			else
			{
				init_next_timeout_checking();
			}
		}

		//! schedule next timeout checking.
		void
		init_next_timeout_checking()
		{
			m_timer_guard.schedule( m_prepared_weak_ctx );
		}

		//! Start guard write operation if necessary.
		void
		guard_write_operation()
		{
			m_write_operation_timeout_after =
				std::chrono::steady_clock::now() + m_settings->m_write_http_response_timelimit;
		}

		void
		guard_close_frame_from_peer_operation()
		{
			m_close_frame_from_peer_timeout_after =
				std::chrono::steady_clock::now() + m_settings->m_read_next_http_message_timelimit;
		}
		//! \}

		//! Input routine.
		connection_input_t m_input;

		//! Helper for validating protocol.
		ws_protocol_validator_t m_protocol_validator{ true };

		//! Websocket message handler provided by user.
		message_handler_t m_msg_handler;

		//! Logger for operation
		logger_t & m_logger;

		//! Write to socket operation context.
		restinio::impl::write_group_output_ctx_t m_write_output_ctx;

		//! Output buffers queue.
		ws_outgoing_data_t m_outgoing_data;

		//! A waek handler for owning ws_t to use it when call message handler.
		ws_weak_handle_t m_websocket_weak_handle;

		//! Websocket output states.
		enum class write_state_t
		{
			//! Able to append outgoing data.
			write_enabled,
			//! No more outgoing data can be added (e.g. close-frame was sent).
			write_disabled
		};

		//! A state of a websocket output.
		write_state_t m_write_state = write_state_t::write_enabled;

		//! Websocket input states.
		enum class read_state_t
		{
			//! Reads any type of frame and serve it to user.
			read_any_frame,
			//! Reads only close frame: skip all frames until close-frame.
			read_only_close_frame,
			//! Do not read anything (before activation).
			read_nothing
		};

		//! A state of a websocket input.
		read_state_t m_read_state = read_state_t::read_nothing;

		//! A helper class for running exclusive action.
		//! Only a first action will run.
		class one_shot_action_t
		{
			public:
				template < typename Action >
				void
				run_if_first( Action && action ) noexcept(noexcept(action()))
				{
					if( m_not_executed_yet )
					{
						m_not_executed_yet = false;
						action();
					}
				}

				//! Disable ation: action will not be executed even on a first shot.
				void
				disable()
				{
					m_not_executed_yet = false;
				}

			private:
				bool m_not_executed_yet{ true };
		};

		one_shot_action_t m_close_frame_to_user;
		one_shot_action_t m_close_frame_to_peer;
		one_shot_action_t m_close_impl;
};

} /* namespace impl */

} /* namespace basic */

} /* namespace websocket */

} /* namespace restinio */
