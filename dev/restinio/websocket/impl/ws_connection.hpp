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

#include <restinio/all.hpp>
#include <restinio/websocket/ws_message.hpp>
#include <restinio/websocket/impl/ws_parser.hpp>
#include <restinio/websocket/impl/utf8.hpp>

namespace restinio
{

namespace websocket
{

namespace impl
{

constexpr size_t WEBSOCKET_HEADER_MAX_SIZE = 14;

//
// ws_outgoing_data_t
//

//! A queue for outgoing buffers.
class ws_outgoing_data_t
{
	public:
		//! Add buffers to queue.
		void
		append( buffers_container_t bufs )
		{
			assert( !m_close_when_done );

			if( m_awaiting_buffers.empty() )
			{
				m_awaiting_buffers = std::move( bufs );
			}
			else
			{
				m_awaiting_buffers.reserve( m_awaiting_buffers.size() + bufs.size() );
				for( auto & buf : bufs )
					m_awaiting_buffers.emplace_back( std::move( buf ) );
			}
		}

		void
		pop_ready_buffers(
			std::size_t max_buf_count,
			buffers_container_t & bufs )
		{
			if( max_buf_count >= m_awaiting_buffers.size() )
				bufs = std::move( m_awaiting_buffers );
			else
			{
				const auto begin_of_bunch = m_awaiting_buffers.begin();
				const auto end_of_bunch = begin_of_bunch + max_buf_count;
				bufs.reserve( max_buf_count );
				for( auto it = begin_of_bunch; it != end_of_bunch; ++it )
				{
					bufs.emplace_back( std::move( *it ) );
				}

				m_awaiting_buffers.erase( begin_of_bunch, end_of_bunch );
			}
		}

		bool
		close_when_done() const
		{
			return m_close_when_done;
		}

		void
		set_close_when_done()
		{
			m_close_when_done = true;
		}

	private:
		//! Flag is set when user initiates close.
		/*!
			If flag is switched on, then after sending all the buffers
			the socket mus be closed.
		*/
		bool m_close_when_done{ false };

		//! A queue of buffers.
		buffers_container_t m_awaiting_buffers;
};

//
// ws_connection_input_t
//

struct ws_connection_input_t
{
	ws_connection_input_t( std::size_t buffer_size )
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

inline ws_message_t
create_close_msg(
	status_code_t code,
	const std::string & desc )
{
	raw_data_t payload{
		restinio::websocket::impl::status_code_to_bin( code ) + desc };

	return restinio::websocket::ws_message_t(
		true, restinio::websocket::opcode_t::connection_close_frame, payload );
}

//
// ws_connection_t
//

//! Context for handling websocket connections.
/*
*/
template <
		typename TRAITS,
		typename WS_MESSAGE_HANDLER,
		typename WS_CLOSE_HANDLER >
class ws_connection_t final
	:	public ws_connection_base_t
{
	public:
		using message_handler_t = WS_MESSAGE_HANDLER;
		using close_handler_t = WS_CLOSE_HANDLER;

		using timer_factory_t = typename TRAITS::timer_factory_t;
		using timer_guard_instance_t = typename timer_factory_t::timer_guard_instance_t;
		using logger_t = typename TRAITS::logger_t;
		using strand_t = typename TRAITS::strand_t;
		using stream_socket_t = typename TRAITS::stream_socket_t;

		ws_connection_t(
			//! Connection id.
			std::uint64_t conn_id,
			//! Connection socket.
			stream_socket_t socket,
			strand_t strand,
			timer_guard_instance_t timer_guard,
			//! Settings that are common for connections.
			restinio::impl::connection_settings_shared_ptr_t< TRAITS > settings,
			message_handler_t msg_handler,
			close_handler_t close_handler )
			:	ws_connection_base_t{ conn_id }
			,	m_socket{ std::move( socket ) }
			,	m_strand{ std::move( strand ) }
			,	m_settings{ std::move( settings ) }
			,	m_timer_guard{ std::move( timer_guard ) }
			,	m_input{ WEBSOCKET_HEADER_MAX_SIZE }
			,	m_msg_handler{ std::move( msg_handler ) }
			,	m_close_handler{ std::move( close_handler ) }
			,	m_logger{ *( m_settings->m_logger ) }
		{
			// Notify of a new connection instance.
			m_logger.trace( [&]{
					return fmt::format(
						"[ws_connection:{}] start connection with {}",
						connection_id(),
						m_socket.remote_endpoint() );
			} );
		}

		ws_connection_t( const ws_connection_t & ) = delete;
		ws_connection_t( ws_connection_t && ) = delete;
		const ws_connection_t & operator = ( const ws_connection_t & ) = delete;
		ws_connection_t & operator = ( ws_connection_t && ) = delete;

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

		virtual void
		close() override
		{
			//! Run write message on io_context loop if possible.
			asio::dispatch(
				get_executor(),
				[ this, ctx = shared_from_this() ](){
					try
					{
						m_logger.trace( [&]{
							return fmt::format(
								"[ws_connection:{}] close",
								connection_id() );
						} );
						graceful_close(
							status_code_t::normal_closure, std::string{} );
					}
					catch( const std::exception & ex )
					{
						m_logger.error( [&]{
							return fmt::format(
								"[ws_connection:{}] close operation error: {}",
								connection_id(),
								ex.what() );
						} );
					}
			} );
		}

		//! Start reading ws-messages.
		void
		init_read(
			ws_weak_handle_t ws_wh ) override
		{
			//! Run write message on io_context loop if possible.
			asio::dispatch(
				get_executor(),
				[ this, ctx = shared_from_this(), ws_wh = std::move( ws_wh ) ](){
					try
					{
						m_websocket_weak_handle = std::move( ws_wh );
						start_read_header();
					}
					catch( const std::exception & ex )
					{
						trigger_error_and_close(
							ex.what(),
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
		write_data( buffers_container_t bufs ) override
		{
			//! Run write message on io_context loop if possible.
			asio::dispatch(
				get_executor(),
				[ this,
					actual_bufs = std::move( bufs ),
					ctx = shared_from_this() ]() mutable {
						try
						{
							write_data_impl( std::move( actual_bufs ) );
						}
						catch( const std::exception & ex )
						{
							trigger_error_and_close(
								ex.what(),
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
				consume_message();
			}
			else
			{
				// Has something to read from m_input.m_buf.
				consume_header_from_buffer(
					m_input.m_buf.bytes(), m_input.m_buf.length() );
			}
		}

		inline void
		consume_message()
		{
			m_logger.trace( [&]{
				return fmt::format(
						"[ws_connection:{}] continue reading message",
						connection_id() );
			} );

			m_socket.async_read_some(
				m_input.m_buf.make_asio_buffer(),
				asio::bind_executor(
					get_executor(),
					[ this, ctx = shared_from_this() ](
						const asio::error_code & ec,
						std::size_t length ){
							try
							{
								after_read_header( ec, length );
							}
							catch( const std::exception & ex )
							{
								trigger_error_and_close(
									ex.what(),
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
		handle_read_error( const std::error_code & ec )
		{
			trigger_error_and_close(
				ec.message(),
				[&]{
					return fmt::format(
						"[ws_connection:{}] read error: {}",
						connection_id(),
						ec.message() );
				} );
		}

		//! Handle read operation result, when reading header.
		void
		after_read_header(
			const std::error_code & ec,
			std::size_t length )
		{
			if( !ec )
			{
				m_input.m_buf.obtained_bytes( length );
				consume_header_from_buffer( m_input.m_buf.bytes(), length );
			}
			else
			{
				handle_read_error( ec );
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
				if( !validate_current_ws_message_header() )
				{
					graceful_close(
						restinio::websocket::status_code_t::protocol_error );

					return;
				}

				auto payload_length = m_input.m_parser.current_message().payload_len();
				m_input.m_payload.resize( payload_length );

				if( 0 < m_input.m_buf.length() )
				{
					const auto payload_part_size =
						std::min(
							m_input.m_buf.length(),
							payload_length );

					std::memcpy(
						&m_input.m_payload.front(),
						m_input.m_buf.bytes(),
						payload_part_size );

					m_input.m_buf.consumed_bytes( payload_part_size );

					if( payload_part_size == payload_length )
					{
						// All message is obtained.
						call_handler_on_current_message();
					}
					else
					{
						// Read the rest of payload:
						start_read_payload(
							&m_input.m_payload.front() + payload_part_size,
							payload_length - payload_part_size );
					}
				}
				else
				{
					// callback for message with 0-size payload.
					call_handler_on_current_message();
				}
			}
			else
			{
				consume_message();
			}
		}

		//! Start reading message payload.
		void
		start_read_payload(
			//! A pointer to the remainder of unfetched payload.
			char * payload_data,
			//! The size of the remainder of unfetched payload.
			std::size_t length_remaining )
		{
			m_socket.async_read_some(
				asio::buffer( payload_data, length_remaining ),
				asio::bind_executor(
					get_executor(),
					[ this,
						ctx = shared_from_this(),
						payload_data,
						length_remaining ](
						const asio::error_code & ec,
						std::size_t length ){

							try
							{
								after_read_payload(
									payload_data,
									length_remaining,
									ec,
									length );
							}
							catch( const std::exception & ex )
							{
								trigger_error_and_close(
									ex.what(),
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
			const std::error_code & ec,
			std::size_t length )
		{
			if( !ec )
			{
				if( length < length_remaining )
				{
					//Here: not all payload is obtained,
					// so inintiate read once again:
					this->start_read_payload(
						payload_data + length,
						length_remaining - length );
				}
				else
				{
					// Here: all the payload is ready.
					assert( length == length_remaining );

					// All message is obtained.
					call_handler_on_current_message();
				}
			}
			else
			{
				handle_read_error( ec );
			}
		}

		//! Implementation of writing data performed on the asio io context.
		void
		write_data_impl( buffers_container_t bufs )
		{
			if( !m_socket.is_open() )
			{
				m_logger.warn( [&]{
					return fmt::format(
							"[ws_connection:{}] try to write response, "
							"while socket is closed",
							connection_id() );
				} );
				return;
			}
			else if( m_awaiting_buffers.close_when_done() )
			{
				// User closed ws-connection before.

				//TODO: it might be the case to leave only an assert here
				// because there should be no way to init write
				// after websocket_t object is closed.
				// Depends on whether it is considered to be used in parallel
				// wnen `close()` call and `message_send()` call
				// can happen in parallel threads.

				m_logger.warn( [&]{
					return fmt::format(
							"[ws_connection:{}] try to write response "
							"after sebsocket was closed",
							connection_id() );
				} );
				return;
			}

			// Push buffers to queue.
			m_awaiting_buffers.append( std::move( bufs ) );

			init_write_if_necessary();
		}

		//! Checks if there is something to write,
		//! and if so starts write operation.
		void
		init_write_if_necessary()
		{
			if( !m_resp_out_ctx.transmitting() )
			{
				// Here: not writing anything to socket, so
				// write operation can be initiated.
				if( m_resp_out_ctx.obtain_bufs( m_awaiting_buffers ) )
				{
					// Here: and there is smth to write.

					// Asio buffers (param for async write):
					auto & bufs = m_resp_out_ctx.create_bufs();

					m_logger.trace( [&]{
						return fmt::format(
							"[ws_connection:{}] sending resp data, "
							"buf count: {}",
							connection_id(),
							bufs.size() ); } );

					// There is somethig to write.
					asio::async_write(
						m_socket,
						bufs,
						asio::bind_executor(
							get_executor(),
							[ this,
								ctx = shared_from_this() ]
								( const asio::error_code & ec, std::size_t written ){
									try
									{
										after_write( ec, written );
									}
									catch( const std::exception & ex )
									{
										trigger_error_and_close(
											ex.what(),
											[&]{
												return fmt::format(
													"[ws_connection:{}] after write callback error: {}",
													connection_id(),
													ex.what() );
											} );
									}
							} ) );

					//guard_write_operation();
				}
				else if ( m_awaiting_buffers.close_when_done() )
				{
					call_close_handler( "user initiated" );
					close_impl();
				}
			}
		}

		//! Handle write response finished.
		inline void
		after_write(
			const std::error_code & ec,
			std::size_t written )
		{
			if( !ec )
			{
				// Release buffers.
				m_resp_out_ctx.done( );

				m_logger.trace( [&]{
					return fmt::format(
							"[ws_connection:{}] outgoing data was sent: {}b",
							connection_id(),
							written );
				} );

				if( m_socket.is_open() )
				{
					// Start another write opertion
					// if there is something to send.
					init_write_if_necessary();
				}
			}
			else
			{
				trigger_error_and_close(
					ec.message(),
					[&]{
						return fmt::format(
							"[ws_connection:{}] unable to write: {}",
							connection_id(),
							ec.message() );
					} );
			}
		}

		//! Close WebSocket connection in a graceful manner
		//! sending a close-message
		void
		graceful_close(
			status_code_t code,
			std::string desc = std::string{} )
		{
			if( !m_awaiting_buffers.close_when_done() )
			{
				auto close_msg = create_close_msg( code, desc );

				buffers_container_t bufs;
				bufs.reserve( 2 );

				bufs.emplace_back(
					impl::write_message_details(
						ws_message_details_t{ close_msg } ) );

				bufs.emplace_back( std::move( close_msg.payload() ) );
				m_awaiting_buffers.append( std::move( bufs ) );

				m_awaiting_buffers.set_close_when_done();
				init_write_if_necessary();
			}
		}


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
		close_impl()
		{
			m_logger.trace( [&]{
				return fmt::format(
						"[ws_connection:{}] close socket",
						connection_id() );
			} );

			asio::error_code ignored_ec;
			m_socket.shutdown(
				asio::ip::tcp::socket::shutdown_both,
				ignored_ec );
			m_socket.close();
		}

		//! Trigger an error.
		/*!
			Closes the connection and write to log
			an error message.
		*/
		template< typename MSG_BUILDER >
		void
		trigger_error_and_close(
			const std::string & reason,
			MSG_BUILDER && msg_builder )
		{
			m_logger.error( std::move( msg_builder ) );
			call_close_handler( reason );
		}
		//! \}

		void
		call_handler_on_current_message()
		{
			if( !validate_current_ws_message_body() )
			{
				graceful_close(
					restinio::websocket::status_code_t::invalid_message_data );

				return;
			}

			const auto & current_header = m_input.m_parser.current_message();
			const auto & current_payload = m_input.m_payload;

			m_msg_handler(
				m_websocket_weak_handle,
				std::make_shared< ws_message_t >(
					current_header.transform_to_header(),
					current_payload ) );

			start_read_header();
		}

		void
		call_close_handler( const std::string & reason )
		{
			if( !m_close_handler_was_called )
			{
				m_close_handler( reason );
				m_close_handler_was_called = true;
			}
		}

		//! Start guard write operation if necessary.
		void
		guard_write_operation()
		{
			std::weak_ptr< ws_connection_base_t > weak_ctx = shared_from_this();

			m_timer_guard
				->schedule_operation_timeout_callback(
					get_executor(),
					m_settings->m_write_http_response_timelimit,
					[ this, weak_ctx = std::move( weak_ctx ) ](){
						if( auto ctx = weak_ctx.lock() )
						{
							m_logger.trace( [&]{
								return fmt::format(
										"[wd_connection:{}] write operation timed out",
										this->connection_id() );
								} );
							close();
						}
					} );
		}

		//! Check current websocket message header has correct flags and fields.
		bool
		validate_current_ws_message_header() const
		{
			const auto & current_header = m_input.m_parser.current_message();

			return current_header.m_masking_key != 0;
		}

		//! Check current websocket message body is correct.
		bool
		validate_current_ws_message_body()
		{
			const auto & current_header = m_input.m_parser.current_message();
			auto & current_payload = m_input.m_payload;

			if( current_header.m_mask_flag == true )
			{
				impl::mask_unmask_payload(
					current_header.m_masking_key, current_payload );
			}

			if( current_header.m_opcode == opcode_t::text_frame)
			{

				return check_utf8_is_correct( current_payload );
			}

			return true;
		}

		//! Connection.
		stream_socket_t m_socket;

		//! Sync object for connection events.
		strand_t m_strand;

		//! Common paramaters of a connection.
		restinio::impl::connection_settings_shared_ptr_t< TRAITS > m_settings;

		//! Operation timeout guard.
		timer_guard_instance_t m_timer_guard;

		//! Input routine.
		ws_connection_input_t m_input;

		message_handler_t m_msg_handler;

		bool m_close_handler_was_called{ false };
		close_handler_t m_close_handler;

		ws_weak_handle_t m_websocket_weak_handle;
		//! Write to socket operation context.
		restinio::impl::raw_resp_output_ctx_t m_resp_out_ctx;

		//! Output buffers queue.
		ws_outgoing_data_t m_awaiting_buffers;

		//! Logger for operation
		logger_t & m_logger;
};

} /* namespace impl */

} /* namespace websocket */

} /* namespace restinio */
