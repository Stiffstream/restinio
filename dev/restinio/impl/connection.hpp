/*
	restinio
*/

/*!
	HTTP-connection routine.
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
// http_parser_ctx_t
//

//! Parsing result context for using in parser callbacks.
/*!
	All data is used as temps, and is usable only
	after parsing completes new requests then it is moved out.
*/
struct http_parser_ctx_t
{
	//! Request data.
	//! \{
	http_request_header_t m_header;
	std::string m_body;
	//! \}

	//! Parser context temp values and flags.
	//! \{
	std::string m_current_field_name;
	bool m_last_was_value{ true };
	//! \}

	//! Flag: is http message parsed completely.
	bool m_message_complete{ false };

	//! Prepare context to handle new request.
	void
	reset()
	{
		m_header = http_request_header_t{};
		m_body.clear();
		m_current_field_name.clear();
		m_last_was_value = true;
		m_message_complete = false;
	}
};

//! Include parser callbacks.
#include "parser_callbacks.inl"

//
// create_parser_settings()
//

//! Helper for setting parser settings.
/*!
	Is used to initialize const value in connection_settings_t ctor.
*/
inline http_parser_settings
create_parser_settings()
{
	http_parser_settings parser_settings;
	http_parser_settings_init( &parser_settings );

	parser_settings.on_url =
		[]( http_parser * parser, const char * at, size_t length ) -> int {
			return restinio_url_cb( parser, at, length );
		};

	parser_settings.on_header_field =
		[]( http_parser * parser, const char * at, size_t length ) -> int {
			return restinio_header_field_cb( parser, at, length );
		};

	parser_settings.on_header_value =
			[]( http_parser * parser, const char * at, size_t length ) -> int {
			return restinio_header_value_cb( parser, at, length );
		};

	parser_settings.on_headers_complete =
		[]( http_parser * parser ) -> int {
			return restinio_headers_complete_cb( parser );
		};

	parser_settings.on_body =
		[]( http_parser * parser, const char * at, size_t length ) -> int {
			return restinio_body_cb( parser, at, length );
		};

	parser_settings.on_message_complete =
		[]( http_parser * parser ) -> int {
			return restinio_message_complete_cb( parser );
		};

	return parser_settings;
}

enum class connection_upgrade_stage_t : std::uint8_t
{
	//! No connection request in progress
	none,
	//! Request with connection-upgrade header came and waits for
	//! request handler to be called in non pipelined fashion
	//! (it must be the only request that is handled at the moment).
	pending_upgrade_handling,
	//! Handler for request with connection-upgrade header was called
	//! so any response data comming is for that request.
	//! If connection transforms to websocket connection
	//! then no further operations are expected.
	wait_for_upgrade_handling_result_or_nothing
};

//! Data associated with connection read routine.
struct connection_input_t
{
	connection_input_t( std::size_t buffer_size )
		:	m_buf{ buffer_size }
	{}

	//! HTTP-parser.
	//! \{
	http_parser m_parser;
	http_parser_ctx_t m_parser_ctx;
	//! \}

	//! Input buffer.
	fixed_buffer_t m_buf;

	//! Connection upgrade request stage.
	connection_upgrade_stage_t m_connection_upgrade_stage{
		connection_upgrade_stage_t::none };

	//! Prepare parser for reading new http-message.
	void
	reset_parser()
	{
		// Reinit parser.
		http_parser_init( &m_parser, HTTP_REQUEST);

		// Reset context and attach it to parser.
		m_parser_ctx.reset();
		m_parser.data = &m_parser_ctx;
	}
};

template < typename CONNECTION, typename START_READ_CB, typename FAILED_CB >
void
prepare_connection_and_start_read(
	asio::ip::tcp::socket & ,
	CONNECTION & ,
	START_READ_CB start_read_cb,
	FAILED_CB )
{
	// No preparation is needed, start
	start_read_cb();
}

//
// connection_t
//

//! Context for handling http connections.
/*
	Working circle consists of the following steps:
	* wait for request -- reading from socket and parsing header and body;
	* handling request -- once the request is completely obtained it's handling
	is deligated to a handler chosen by handler factory;
	* writing response -- writing response to socket;
	* back to first step o close connection -- depending on keep-alive property
	of the last response the connection goes back to first step or
	shutdowns.

	Each step is controlled by timer (\see schedule_operation_timeout_callback())

	In case of errors connection closes itself.
*/
template < typename TRAITS >
class connection_t final
	:	public connection_base_t
{
	public:
		using timer_factory_t = typename TRAITS::timer_factory_t;
		using timer_guard_instance_t = typename timer_factory_t::timer_guard_instance_t;
		using request_handler_t = typename TRAITS::request_handler_t;
		using logger_t = typename TRAITS::logger_t;
		using strand_t = typename TRAITS::strand_t;
		using stream_socket_t = typename TRAITS::stream_socket_t;

		connection_t(
			//! Connection id.
			std::uint64_t conn_id,
			//! Connection socket.
			stream_socket_t && socket,
			//! Settings that are common for connections.
			connection_settings_shared_ptr_t< TRAITS > settings,
			//! Operation timeout guard.
			timer_guard_instance_t timer_guard )
			:	connection_base_t{ conn_id }
			,	m_socket{ std::move( socket ) }
			,	m_strand{ m_socket.get_executor() }
			,	m_settings{ std::move( settings ) }
			,	m_input{ m_settings->m_buffer_size }
			,	m_response_coordinator{ m_settings->m_max_pipelined_requests }
			,	m_timer_guard{ std::move( timer_guard ) }
			,	m_request_handler{ *( m_settings->m_request_handler ) }
			,	m_logger{ *( m_settings->m_logger ) }
		{
			// Notify of a new connection instance.
			m_logger.trace( [&]{
					return fmt::format(
						"[connection:{}] start connection with {}",
						connection_id(),
						m_socket.remote_endpoint() );
			} );
		}

		connection_t( const connection_t & ) = delete;
		connection_t( connection_t && ) = delete;
		void operator = ( const connection_t & ) = delete;
		void operator = ( connection_t && ) = delete;

		~connection_t()
		{
			try
			{
				// Notify of a new connection instance.
				m_logger.trace( [&]{
					return fmt::format(
						"[connection:{}] destroyed",
						connection_id() );
				} );
			}
			catch( ... )
			{}
		}

		void
		init()
		{
			prepare_connection_and_start_read(
				m_socket,
				*this,
				[ & ]{ wait_for_http_message(); },
				[ & ]( const asio::error_code & ec ){
					trigger_error_and_close( [&]{
						return fmt::format(
								"[connection:{}] prepare connection error: {}",
								connection_id(),
								ec.message() );
					} );
				} );
		}

		//! Start reading next htttp-message.
		void
		wait_for_http_message()
		{
			m_logger.trace( [&]{
				 return fmt::format(
						"[connection:{}] start waiting for request",
						connection_id() );
			} );

			// Prepare parser for consuming new request message.
			m_input.reset_parser();

			// Guard total time for a request to be read.
			// guarding here makes the total read process
			// to run in read_next_http_message_timelimit.
			guard_read_operation();

			if( 0 != m_input.m_buf.length() )
			{
				// If a pipeline requests were sent by client
				// then the biginning (or even entire request) of it
				// is in the buffer obtained from socket in previous
				// read operation.
				consume_data( m_input.m_buf.bytes(), m_input.m_buf.length() );
			}
			else
			{
				// Next request (if any) must be obtained from socket.
				consume_message();
			}
		}

		//! Move socket out of connection.
		auto
		move_socket()
		{
			auto res = std::move( m_socket );
		}

		//! Пуее
		auto
		get_settings() const
		{
			return m_settings;
		}

	private:
		//! An executor for callbacks on async operations.
		inline strand_t &
		get_executor()
		{
			return m_strand;
		}

		//! Start (continue) a chain of read-parse-read-... operations.
		inline void
		consume_message()
		{
			m_logger.trace( [&]{
				return fmt::format(
						"[connection:{}] continue reading request",
						connection_id() );
			} );

			m_socket.async_read_some(
				m_input.m_buf.make_asio_buffer(),
				asio::bind_executor(
					get_executor(),
					[ this, ctx = shared_from_this() ]( auto ec, std::size_t length ){
						this->after_read( ec, length );
					} ) );
		}

		inline void
		after_read( const std::error_code & ec, std::size_t length )
		{
			if( !ec )
			{
				m_logger.trace( [&]{
					return fmt::format(
							"[connection:{}] received {} bytes",
							this->connection_id(),
							length );
				} );

				m_input.m_buf.obtained_bytes( length );

				consume_data( m_input.m_buf.bytes(), length );
			}
			else
			{
				// Well, if it is actually an error
				// then close connection.
				if( ec != asio::error::operation_aborted )
				{
					if ( ec != asio::error::eof || 0 != m_input.m_parser.nread )
						trigger_error_and_close( [&]{
							return fmt::format(
									"[connection:{}] read socket error: {}; "
									"parsed bytes: {}",
									connection_id(),
									ec.message(),
									m_input.m_parser.nread );
						} );
					else
					{
						// A case that is not such an  error:
						// on a connection (most probably keeped alive
						// after previous request, but a new also applied)
						// no bytes were consumed and remote peer closes connection.
						m_logger.trace( [&]{
							return fmt::format(
									"[connection:{}] EOF and no request, "
									"close connection",
									connection_id() );
						} );

						close();
					}
				}
				// else: read operation was cancelled.
			}
		}

		//! Parse some data.
		void
		consume_data( const char * data, std::size_t length )
		{
			auto & parser = m_input.m_parser;

			const auto nparsed =
				http_parser_execute(
					&parser,
					&( m_settings->m_parser_settings ),
					data,
					length );

			// If entire http-message was obtained,
			// parser is stopped and the might be a part of consecutive request
			// left in buffer, so we mark how many bytes were obtained.
			// and next message read (if any) will be started from already existing
			// data left in buffer.
			m_input.m_buf.consumed_bytes( nparsed );

			if( HPE_OK != parser.http_errno &&
				HPE_PAUSED != parser.http_errno )
			{
				// PARSE ERROR:
				auto err = HTTP_PARSER_ERRNO( &parser );

				// TODO: handle case when there are some request in process.
				trigger_error_and_close( [&]{
					return fmt::format(
							"[connection:{}] parser error {}: {}",
							connection_id(),
							http_errno_name( err ),
							http_errno_description( err ) );
				} );

				// nothing to do.
				return;
			}

			if( m_input.m_parser_ctx.m_message_complete )
			{
				on_request_message_complete();
			}
			else
				consume_message();
		}

		//! Handle a given request message.
		void
		on_request_message_complete()
		{
			try
			{
				auto & parser = m_input.m_parser;
				auto & parser_ctx = m_input.m_parser_ctx;

				if( m_input.m_parser.upgrade )
				{
					// Start upgrade connection operation.

					// The first thing is to make sure
					// that upgrade request will be handled in
					// a non pipelined fashion.
					m_input.m_connection_upgrade_stage =
						connection_upgrade_stage_t::pending_upgrade_handling;
				}

				if( connection_upgrade_stage_t::none ==
					m_input.m_connection_upgrade_stage )
				{
					// Run ordinary HTTP logic.
					const auto request_id = m_response_coordinator.register_new_request();

					m_logger.trace( [&]{
						return fmt::format(
								"[connection:{}] request received (#{}): {} {}",
								connection_id(),
								request_id,
								http_method_str(
									static_cast<http_method>( parser.method ) ),
								parser_ctx.m_header.request_target() );
					} );

					// TODO: mb there is a way to
					// track if response was emmited immediately in handler
					// or it was delegated
					// so it is possible to omit this timer scheduling.
					guard_request_handling_operation();

					if( request_rejected() ==
						m_request_handler(
							std::make_shared< request_t >(
								request_id,
								std::move( parser_ctx.m_header ),
								std::move( parser_ctx.m_body ),
								shared_from_this() ) ) )
					{
						// If handler refused request, say not implemented.
						write_response_parts_impl(
							request_id,
							response_output_flags_t{
								response_parts_attr_t::final_parts,
								response_connection_attr_t::connection_close },
							create_not_implemented_resp() );
					}
					else if( m_response_coordinator.is_able_to_get_more_messages() )
					{
						// Request was accepted,
						// didn't create immediate response that closes connection after,
						// and it is possible to receive more requests
						// then start consuming yet another request.
						wait_for_http_message();
					}
				}
				else
				{
					m_logger.trace( [&]{
						const std::string default_value{};

						return fmt::format(
								"[connection:{}] upgrade request received: {} {}; "
								"Upgrade: '{}';",
								connection_id(),
								http_method_str(
									static_cast<http_method>( parser.method ) ),
								parser_ctx.m_header.request_target(),
								parser_ctx.m_header.get_field( http_field::upgrade, default_value ) );
					} );

					if( m_response_coordinator.empty() )
					{
						// There are no requests in handling
						// So the current request with upgrade
						// is the only one and can be handled directly.
						// It is safe to call a handler for it.
						handle_upgrade_request();
					}
					else
					{
						// There are pipelined request
						m_logger.trace( [&]{
							return fmt::format(
									"[connection:{}] upgrade request happened to be a pipelined one, "
									"and will be handled after previous requests are handled",
									connection_id() );
						} );
					}

					// No further actions (like continue reading) in both cases are needed.
				}

			}
			catch( const std::exception & ex )
			{
				trigger_error_and_close( [&]{
					return fmt::format(
							"[connection:{}] error while handling request: {}",
							this->connection_id(),
							ex.what() );
				} );
			}
		}

		//! Calls handler for upgrade request.
		/*!
			Request data must be in input context (m_input).
		*/
		void
		handle_upgrade_request()
		{
			auto & parser = m_input.m_parser;
			auto & parser_ctx = m_input.m_parser_ctx;

			// If user responses with error
			// then connection must be able to send
			// (hence to receive) response.

			const auto request_id = m_response_coordinator.register_new_request();

			m_logger.info( [&]{
				return fmt::format(
						"[connection:{}] handle upgrade request (#{}): {} {}",
						connection_id(),
						request_id,
						http_method_str(
							static_cast<http_method>( parser.method ) ),
						parser_ctx.m_header.request_target() );
			} );

			guard_request_handling_operation();

			// After calling handler we expect the results or
			// no further operations with connection
			m_input.m_connection_upgrade_stage =
				connection_upgrade_stage_t::wait_for_upgrade_handling_result_or_nothing;

			if( request_rejected() ==
				m_request_handler(
					std::make_shared< request_t >(
						request_id,
						std::move( parser_ctx.m_header ),
						std::move( parser_ctx.m_body ),
						shared_from_this() ) ) )
			{
				if( m_socket.is_open() )
				{
					// Request is rejected, so our socket
					// must not be moved out to websocket connection.

					// If handler refused request, say not implemented.
					write_response_parts_impl(
						request_id,
						response_output_flags_t{
							response_parts_attr_t::final_parts,
							response_connection_attr_t::connection_close },
						create_not_implemented_resp() );
				}
				else
				{
					// Request is rejected, but the socket
					// was moved out to somewhere else???

					m_logger.error( [&]{
						return fmt::format(
								"[connection:{}] upgrade request handler rejects "
								"request, but socket was moved out from connection",
								connection_id() );
					} );
				}
			}
			// Else 2 cases:
			// 1. request is handled asynchronously, so
			// what happens next depends on handling.
			// 2. handling was immediate, so further destiny
			// of a connection was already determined.
			//
			// In both cases: here do nothing.
		}

		//! Write parts for specified request.
		virtual void
		write_response_parts(
			//! Request id.
			request_id_t request_id,
			//! Resp output flag.
			response_output_flags_t response_output_flags,
			//! parts of a response.
			buffers_container_t bufs ) override
		{
			// TODO: Avoid heap allocation:

			// Unfortunately ASIO tends to call a copy ctor for bufs
			// If pass it to lambda as `bufs = std::move( bufs ),`
			// so bufs_transmit_instance workaround is used.
			auto bufs_transmit_instance =
				std::make_unique< buffers_container_t >( std::move( bufs ) );

			//! Run write message on io_service loop if possible.
			asio::dispatch(
				get_executor(),
				[ this,
					request_id,
					response_output_flags,
					bufs = std::move( bufs_transmit_instance ),
					ctx = shared_from_this() ](){
						try
						{
							write_response_parts_impl(
								request_id,
								response_output_flags,
								std::move( *bufs ) );
						}
						catch( const std::exception & ex )
						{
							trigger_error_and_close( [&]{
								return fmt::format(
									"[connection:{}] unable to handle response: {}",
									connection_id(),
									ex.what() );
							} );
						}
				} );
		}

		//! Write parts for specified request.
		void
		write_response_parts_impl(
			//! Request id.
			request_id_t request_id,
			//! Resp output flag.
			response_output_flags_t response_output_flags,
			//! parts of a response.
			buffers_container_t bufs )
		{
			assert( m_socket );
			if( !m_socket.is_open() )
			{
				m_logger.warn( [&]{
					return fmt::format(
							"[connection:{}] try to write response, "
							"while socket is closed",
							connection_id() );
				} );
				return;
			}

			if( connection_upgrade_stage_t::
					wait_for_upgrade_handling_result_or_nothing ==
				m_input.m_connection_upgrade_stage )
			{
				// It is response for a connection-upgrade request.
				// If we receive it here then it is constructed via
				// message builder and so connection was not transformed
				// to websocket connection.
				// So it is necessary to resume pipeline logic that was stopped
				// for upgrade-request to be handled as the only request
				// on the connection for that moment.
				if( !m_response_coordinator.is_full() )
				{
					wait_for_http_message();
				}
			}

			if( !m_response_coordinator.closed() )
			{
				m_logger.trace( [&]{
					return fmt::format(
							"[connection:{}] append response (#{}), "
							"flags: {}, bufs count: {}",
							connection_id(),
							request_id,
							response_output_flags,
							bufs.size() );
				} );

				m_response_coordinator.append_response(
					request_id,
					response_output_flags,
					std::move( bufs ) );

				init_write_if_necessary();
			}
			else
			{
				m_logger.warn( [&]{
					return fmt::format(
							"[connection:{}] receive response parts for "
							"request (#{}), but response with connection-close "
							"attribute happened before",
							connection_id(),
							request_id );
				} );
			}

		}

		// Check if there is something to write,
		// and if so starts write operation.
		void
		init_write_if_necessary()
		{
			assert( !m_response_coordinator.closed() );

			// Remember if all response cells were busy.
			const auto full_before = m_response_coordinator.is_full();

			if( !m_resp_out_ctx.transmitting() )
			{
				if( m_resp_out_ctx.obtain_bufs( m_response_coordinator ) )
				{
					// Remember if all response cells were busy.
					const auto full_after = m_response_coordinator.is_full();

					auto & bufs = m_resp_out_ctx.create_bufs();

					if( m_response_coordinator.closed() )
					{
						m_logger.trace( [&]{
							return fmt::format(
									"[connection:{}] sending resp data with "
									"connection-close attribute "
									"buf count: {}",
									connection_id(),
									bufs.size() );
						} );

						// Reading new requests is useless.
						asio::error_code ignored_ec;
						m_socket.cancel( ignored_ec );
					}
					else
					{
						m_logger.trace( [&]{
							return fmt::format(
								"[connection:{}] sending resp data, "
								"buf count: {}",
								connection_id(),
								bufs.size() ); } );
					}

					// There is somethig to write.
					asio::async_write(
						m_socket,
						bufs,
						asio::bind_executor(
							get_executor(),
							[ this,
								ctx = shared_from_this(),
								should_keep_alive = !m_response_coordinator.closed(),
								init_read_after_this_write =
									full_before && !full_after ]
								( auto ec, std::size_t written ){
									this->after_write(
										ec,
										written,
										should_keep_alive,
										init_read_after_this_write );
							} ) );

					guard_write_operation();
				}
				else if( m_response_coordinator.closed() )
				{
					// Bufs empty but there happened to
					// be a response context marked as complete
					// (final_parts) and having connection-close attr.
					// It is because `init_write_if_necessary()`
					// is called only under `!m_response_coordinator.closed()`
					// conditio, so if no bufs were obtained
					// and response coordinator is closed means
					// that a first response stored by
					// response coordinator was marked as complete
					// without data.

					m_logger.trace( [&]{
						return fmt::format(
							"[connection:{}] last sent response was marked "
							"as complete",
							connection_id() ); } );
					close();
				}
				else
				{
					// Not writing anything, so need to deal with timouts.
					if( m_response_coordinator.empty() )
					{
						// No requests in processing.
						// So set read next request timeout.
						guard_read_operation();
					}
					else
					{
						// Have requests in process.
						// So take control over request handling.
						guard_request_handling_operation();
					}
				}
			}
		}

		//! Handle write response finished.
		inline void
		after_write(
			const std::error_code & ec,
			std::size_t /*written*/,
			bool should_keep_alive,
			bool should_init_read_after_this_write )
		{
			if( !ec )
			{
				// Release buffers.
				m_resp_out_ctx.done();

				m_logger.trace( [&]{
					return fmt::format(
							"[connection:{}] outgoing data was sent",
							connection_id() );
				} );

				if( should_keep_alive )
				{
					m_logger.trace( [&]{
						return fmt::format(
								"[connection:{}] should keep alive",
								this->connection_id() );
					} );

					if( connection_upgrade_stage_t::none ==
						m_input.m_connection_upgrade_stage )
					{
						// Run ordinary HTTP logic.
						if( should_init_read_after_this_write )
							wait_for_http_message();

						// Start another write opertion
						// if there is something to send.
						init_write_if_necessary();
					}
					else
					{
						if( m_response_coordinator.empty() )
						{
							// Here upgrade req is the only request
							// to by handled by this connection.
							// So it is safe to call a handler for it.
							handle_upgrade_request();
						}
						else
						{
							// Do not start reading in any case,
							// but if there is at least one request preceding
							// upgrade-req, logic must continue http interaction.
							init_write_if_necessary();
						}
					}
				}
				else
				{
					// No keep-alive, close connection.
					close();
				}
			}
			else
			{
				if( ec != asio::error::operation_aborted )
				{
					trigger_error_and_close( [&]{
						return fmt::format(
							"[connection:{}] unable to write: {}",
							connection_id(),
							ec.message() );
					} );
				}
				// else: Operation aborted only in case of close was called.
			}
		}

		//! Close connection functions.
		//! \{

		//! Standard close routine.
		void
		close()
		{
			m_timer_guard->cancel();

			m_logger.trace( [&]{
				return fmt::format(
						"[connection:{}] close",
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
		trigger_error_and_close( MSG_BUILDER && msg_builder )
		{
			m_logger.error( std::move( msg_builder ) );

			close();
		}
		//! \}

		//! Connection
		stream_socket_t m_socket;

		//! Sync object for connection events.
		strand_t m_strand;

		//! Common paramaters of a connection.
		connection_settings_shared_ptr_t< TRAITS > m_settings;

		//! Input routine.
		connection_input_t m_input;

		//! Write to socket operation context.
		raw_resp_output_ctx_t m_resp_out_ctx;

		//! Response coordinator.
		response_coordinator_t m_response_coordinator;

		//! Timer to controll operations.
		//! \{

		//! Helper function to work with timer guard.
		template < typename FUNC >
		void
		schedule_operation_timeout_callback(
			std::chrono::steady_clock::duration timeout,
			FUNC && f )
		{
			std::weak_ptr< connection_base_t > weak_ctx = shared_from_this();

			m_timer_guard
				->schedule_operation_timeout_callback(
					get_executor(),
					timeout,
					[ weak_ctx = std::move( weak_ctx ), cb = std::move( f ) ](){
						if( auto ctx = weak_ctx.lock() )
						{
							cb();
						}
					} );
		}

		//! Statr guard read operation if necessary.
		void
		guard_read_operation()
		{
			// Guard read timeout for read operation only when
			// there is no request in process.
			if( m_response_coordinator.empty() )
			{
				schedule_operation_timeout_callback(
					m_settings->m_read_next_http_message_timelimit,
					[ this ](){
						m_logger.trace( [&]{
							return fmt::format(
									"[connection:{}] wait for request timed out",
									this->connection_id() );
						} );
						close();
					} );
			}
		}

		//! Start guard request handling operation if necessary.
		void
		guard_request_handling_operation()
		{
			// Guard handling request only when
			// there is no responses that are written.
			if( !m_resp_out_ctx.transmitting() )
			{
				schedule_operation_timeout_callback(
					m_settings->m_handle_request_timeout,
					[ this ](){
						m_logger.warn( [&]{
							return fmt::format(
									"[connection:{}] handle request timed out",
									this->connection_id() );
						} );

						close();
						// TODO: it is not always possible to write such resp.
						// write_response_parts_impl(
						// 	request_id,
						// 	response_output_flags_t{ true, false },
						// 	{ create_timeout_resp() } );
					} );
			}
		}

		//! Start guard write operation if necessary.
		void
		guard_write_operation()
		{
			schedule_operation_timeout_callback(
				m_settings->m_write_http_response_timelimit,
				[ this ](){
					m_logger.trace( [&]{
						return fmt::format(
								"[connection:{}] writing response timed out",
								this->connection_id() );
					} );
					close();
				} );
		}

		//! Operation timeout guard.
		timer_guard_instance_t m_timer_guard;
		//! \}

		//! Request handler.
		request_handler_t & m_request_handler;

		//! Logger for operation
		logger_t & m_logger;
};

//
// connection_factory_t
//

//! Factory for connections.
template < typename TRAITS >
class connection_factory_t
{
	public:
		using timer_factory_t = typename TRAITS::timer_factory_t;
		using logger_t = typename TRAITS::logger_t;
		using stream_socket_t = typename TRAITS::stream_socket_t;

		connection_factory_t(
			connection_settings_shared_ptr_t< TRAITS > connection_settings,
			asio::io_service & io_service,
			std::unique_ptr< timer_factory_t > timer_factory )
			:	m_connection_settings{ std::move( connection_settings ) }
			,	m_io_service{ io_service }
			,	m_timer_factory{ std::move( timer_factory ) }
			,	m_logger{ *(m_connection_settings->m_logger ) }
		{
			if( !m_timer_factory )
				throw exception_t{ "timer_factory not set" };
		}

		auto
		create_new_connection(
			stream_socket_t && socket )
		{
			using connection_type_t = connection_t< TRAITS >;
			return std::make_shared< connection_type_t >(
				m_connection_id_counter++,
				std::move( socket ),
				m_connection_settings,
				m_timer_factory->create_timer_guard( m_io_service ) );
		}

	private:
		std::uint64_t m_connection_id_counter{ 1 };

		connection_settings_shared_ptr_t< TRAITS > m_connection_settings;

		asio::io_service & m_io_service;

		std::unique_ptr< timer_factory_t > m_timer_factory;

		logger_t & m_logger;
};

} /* namespace impl */

} /* namespace restinio */
