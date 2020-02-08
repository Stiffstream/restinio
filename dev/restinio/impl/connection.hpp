/*
	restinio
*/

/*!
	HTTP-connection routine.
*/

#pragma once

#include <restinio/asio_include.hpp>

#include <http_parser.h>

#include <restinio/impl/include_fmtlib.hpp>

#include <restinio/exception.hpp>
#include <restinio/http_headers.hpp>
#include <restinio/request_handler.hpp>
#include <restinio/impl/connection_base.hpp>
#include <restinio/impl/header_helpers.hpp>
#include <restinio/impl/response_coordinator.hpp>
#include <restinio/impl/connection_settings.hpp>
#include <restinio/impl/fixed_buffer.hpp>
#include <restinio/impl/write_group_output_ctx.hpp>
#include <restinio/impl/executor_wrapper.hpp>
#include <restinio/impl/sendfile_operation.hpp>

#include <restinio/utils/impl/safe_uint_truncate.hpp>
#include <restinio/utils/at_scope_exit.hpp>

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
#include "parser_callbacks.ipp"

//
// create_parser_settings()
//

//! Helper for setting parser settings.
/*!
	Is used to initialize const value in connection_settings_t ctor.
*/
template< typename Http_Methods >
inline http_parser_settings
create_parser_settings() noexcept
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
			return restinio_message_complete_cb< Http_Methods >( parser );
		};

	return parser_settings;
}

//
// connection_upgrade_stage_t
//

//! Enum for a flag specifying that connection is going to upgrade or not.
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

//
// connection_input_t
//

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

	//! Flag to track whether read operation is performed now.
	bool m_read_operation_is_running{ false };

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

template < typename Connection, typename Start_Read_CB, typename Failed_CB >
void
prepare_connection_and_start_read(
	asio_ns::ip::tcp::socket & ,
	Connection & ,
	Start_Read_CB start_read_cb,
	Failed_CB )
{
	// No preparation is needed, start
	start_read_cb();
}

// An overload for the case of non-TLS-connection.
inline tls_socket_t *
make_tls_socket_pointer_for_state_listener(
	asio_ns::ip::tcp::socket & ) noexcept
{
	return nullptr;
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
template < typename Traits >
class connection_t final
	:	public connection_base_t
	,	public executor_wrapper_t< typename Traits::strand_t >
{
		using executor_wrapper_base_t = executor_wrapper_t< typename Traits::strand_t >;

	public:
		using timer_manager_t = typename Traits::timer_manager_t;
		using timer_guard_t = typename timer_manager_t::timer_guard_t;
		using request_handler_t = typename Traits::request_handler_t;
		using logger_t = typename Traits::logger_t;
		using strand_t = typename Traits::strand_t;
		using stream_socket_t = typename Traits::stream_socket_t;

		connection_t(
			//! Connection id.
			connection_id_t conn_id,
			//! Connection socket.
			stream_socket_t && socket,
			//! Settings that are common for connections.
			connection_settings_handle_t< Traits > settings,
			//! Remote endpoint for that connection.
			endpoint_t remote_endpoint )
			:	connection_base_t{ conn_id }
			,	executor_wrapper_base_t{ socket.get_executor() }
			,	m_socket{ std::move( socket ) }
			,	m_settings{ std::move( settings ) }
			,	m_remote_endpoint{ std::move( remote_endpoint ) }
			,	m_input{ m_settings->m_buffer_size }
			,	m_response_coordinator{ m_settings->m_max_pipelined_requests }
			,	m_timer_guard{ m_settings->create_timer_guard() }
			,	m_request_handler{ *( m_settings->m_request_handler ) }
			,	m_logger{ *( m_settings->m_logger ) }
		{
			// Notify of a new connection instance.
			m_logger.trace( [&]{
					return fmt::format(
						"[connection:{}] start connection with {}",
						connection_id(),
						m_remote_endpoint );
			} );
		}

		// Disable copy/move.
		connection_t( const connection_t & ) = delete;
		connection_t( connection_t && ) = delete;
		connection_t & operator = ( const connection_t & ) = delete;
		connection_t & operator = ( connection_t && ) = delete;

		~connection_t() override
		{
			restinio::utils::log_trace_noexcept( m_logger,
				[&]{
					return fmt::format(
						"[connection:{}] destructor called",
						connection_id() );
				} );
		}

		void
		init()
		{
			prepare_connection_and_start_read(
				m_socket,
				*this,
				[ & ]{
					// Inform state listener if it used.
					m_settings->call_state_listener( [this]() noexcept {
							return connection_state::notice_t{
									this->connection_id(),
									this->m_remote_endpoint,
									connection_state::accepted_t{
											make_tls_socket_pointer_for_state_listener(
													m_socket )
									}
								};
						} );

					// Start timeout checking.
					m_prepared_weak_ctx = shared_from_this();
					init_next_timeout_checking();

					// Start reading request.
					wait_for_http_message();
				},
				[ & ]( const asio_ns::error_code & ec ){
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

		//! Internals that are necessary for upgrade.
		struct upgrade_internals_t
		{
			upgrade_internals_t(
				upgrade_internals_t && ) = default;

			upgrade_internals_t(
				connection_settings_handle_t< Traits > settings,
				stream_socket_t socket )
				:	m_settings{ std::move( settings ) }
				,	m_socket{ std::move( socket ) }
			{}

			connection_settings_handle_t< Traits > m_settings;
			stream_socket_t m_socket;
		};

		//! Move socket out of connection.
		upgrade_internals_t
		move_upgrade_internals()
		{
			return upgrade_internals_t{
				m_settings,
				std::move( m_socket ) };
		}

	private:
		//! Start (continue) a chain of read-parse-read-... operations.
		inline void
		consume_message()
		{
			if( !m_input.m_read_operation_is_running )
			{
				m_logger.trace( [&]{
					return fmt::format(
							"[connection:{}] continue reading request",
							connection_id() );
				} );


				m_input.m_read_operation_is_running = true;
				m_socket.async_read_some(
					m_input.m_buf.make_asio_buffer(),
					asio_ns::bind_executor(
						this->get_executor(),
						[this, ctx = shared_from_this()]
						// NOTE: this lambda is noexcept since v.0.6.0.
						( const asio_ns::error_code & ec,
							std::size_t length ) noexcept {
							m_input.m_read_operation_is_running = false;
							RESTINIO_ENSURE_NOEXCEPT_CALL( after_read( ec, length ) );
						} ) );
			}
			else
			{
				m_logger.trace( [&]{
					return fmt::format(
							"[connection:{}] skip read operation: already running",
							connection_id() );
				} );
			}
		}

		//! Handle read operation result.
		inline void
		after_read( const asio_ns::error_code & ec, std::size_t length ) noexcept
		{
			if( !ec )
			{
				// Exceptions shouldn't go out of `after_read`.
				// So intercept them and close the connection in the case
				// of an exception.
				try
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
				catch( const std::exception & x )
				{
					trigger_error_and_close( [&] {
							return fmt::format(
									"[connection:{}] unexpected exception during the "
									"handling of incoming data: {}",
									connection_id(),
									x.what() );
						} );
				}
			}
			else
			{
				// Well, if it is actually an error
				// then close connection.
				if( !error_is_operation_aborted( ec ) )
				{
					if ( !error_is_eof( ec ) || 0 != m_input.m_parser.nread )
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
						restinio::utils::log_trace_noexcept( m_logger,
							[&]{
								return fmt::format(
										"[connection:{}] EOF and no request, "
										"close connection",
										connection_id() );
							} );

						RESTINIO_ENSURE_NOEXCEPT_CALL( close() );
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
								shared_from_concrete< connection_base_t >(),
								m_remote_endpoint ) ) )
					{
						// If handler refused request, say not implemented.
						write_response_parts_impl(
							request_id,
							response_output_flags_t{
								response_parts_attr_t::final_parts,
								response_connection_attr_t::connection_close },
							write_group_t{ create_not_implemented_resp() } );
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
								parser_ctx.m_header.get_field_or(
									http_field::upgrade, default_value ) );
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

			// Do not guard upgrade request.
			cancel_timeout_checking();

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
						shared_from_concrete< connection_base_t >(),
						m_remote_endpoint) ) )
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
						write_group_t{ create_not_implemented_resp() } );
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
			// We can't even do read-only access because
			// upgrade handling might take place
			// in distinct execution context.
			// So no even a log messages here.
		}

		//! Write parts for specified request.
		virtual void
		write_response_parts(
			//! Request id.
			request_id_t request_id,
			//! Resp output flag.
			response_output_flags_t response_output_flags,
			//! Part of the response data.
			write_group_t wg ) override
		{
			//! Run write message on io_context loop if possible.
			asio_ns::dispatch(
				this->get_executor(),
				[ this,
					request_id,
					response_output_flags,
					actual_wg = std::move( wg ),
					ctx = shared_from_this() ]
				// NOTE that this lambda is noexcept since v.0.6.0.
				() mutable noexcept
					{
						try
						{
							write_response_parts_impl(
								request_id,
								response_output_flags,
								std::move( actual_wg ) );
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
			//! Part of the response data.
			write_group_t wg )
		{
			auto invoke_after_write_cb_with_error = [&]{
				try
				{
					wg.invoke_after_write_notificator_if_exists(
						make_asio_compaible_error(
							asio_convertible_error_t::write_was_not_executed ) );
				}
				catch( const std::exception & ex )
				{
					m_logger.error( [&]{
						return fmt::format(
							"[connection:{}] notificator error: {}",
							connection_id(),
							ex.what() );
					} );
				}
			};

			if( m_socket.is_open() )
			{
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
							"flags: {}, write group size: {}",
							connection_id(),
							request_id,
							response_output_flags,
							wg.items_count() );
					} );

					m_response_coordinator.append_response(
						request_id,
						response_output_flags,
						std::move( wg ) );

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
					invoke_after_write_cb_with_error();
				}
			}
			else
			{
				m_logger.warn( [&]{
					return fmt::format(
							"[connection:{}] try to write response, "
							"while socket is closed",
							connection_id() );
				} );
				invoke_after_write_cb_with_error();
			}
		}

		// Check if there is something to write,
		// and if so starts write operation.
		void
		init_write_if_necessary()
		{
			assert( !m_response_coordinator.closed() );

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

			// Remember if all response cells were busy.
			const bool response_coordinator_full_before =
				m_response_coordinator.is_full();

			auto next_write_group = m_response_coordinator.pop_ready_buffers();

			if( next_write_group )
			{
				m_logger.trace( [&]{
					return fmt::format(
						"[connection:{}] start next write group for response (#{}), "
						"size: {}",
						this->connection_id(),
						next_write_group->second,
						next_write_group->first.items_count() );
				} );

				// Check if all response cells busy:
				const bool response_coordinator_full_after =
					m_response_coordinator.is_full();

				// Whether we need to resume read after this group is written?
				m_init_read_after_this_write =
					response_coordinator_full_before &&
					!response_coordinator_full_after;

				if( 0 < next_write_group->first.status_line_size() )
				{
					// We need to extract status line out of the first buffer
					assert(
						writable_item_type_t::trivial_write_operation ==
						next_write_group->first.items().front().write_type() );

					m_logger.trace( [&]{
						// Get status line:
						const string_view_t
							status_line{
								asio_ns::buffer_cast< const char * >(
									next_write_group->first.items().front().buf() ),
								next_write_group->first.status_line_size() };

						return
							fmt::format(
								"[connection:{}] start response (#{}): {}",
								this->connection_id(),
								next_write_group->second,
								status_line );
					} );
				}

				// Initialize write context with a new write group.
				m_write_output_ctx.start_next_write_group(
					std::move( next_write_group->first ) );

				// Start the loop of sending data from current write group.
				handle_current_write_ctx();
			}
			else
			{
				handle_nothing_to_write();
			}
		}

		// Use aliases for shorter names.
		using none_write_operation_t = write_group_output_ctx_t::none_write_operation_t;
		using trivial_write_operation_t = write_group_output_ctx_t::trivial_write_operation_t;
		using file_write_operation_t = write_group_output_ctx_t::file_write_operation_t;

		//! Start/continue/continue handling output data of current write group.
		/*!
			This function is a starting point of a loop process of sending data
			from a given write group.
			It extracts the next bunch of trivial buffers or a
			sendfile-runner and starts an appropriate write operation.
			In data of a given write group finishes,
			finish_handling_current_write_ctx() is invoked thus breaking the loop.

			@note
			Since v.0.6.0 this method is noexcept.
		*/
		void
		handle_current_write_ctx() noexcept
		{
			try
			{
				auto wo = m_write_output_ctx.extract_next_write_operation();

				if( holds_alternative< trivial_write_operation_t >( wo ) )
				{
					handle_trivial_write_operation( get< trivial_write_operation_t >( wo ) );
				}
				else if( holds_alternative< file_write_operation_t >( wo ) )
				{
					handle_file_write_operation( get< file_write_operation_t >( wo ) );
				}
				else
				{
					assert( holds_alternative< none_write_operation_t >( wo ) );
					finish_handling_current_write_ctx();
				}
			}
			catch( const std::exception & ex )
			{
				trigger_error_and_close( [&]{
					return fmt::format(
						"[connection:{}] handle_current_write_ctx failed: {}",
						connection_id(),
						ex.what() );
				} );
			}
		}

		//! Run trivial buffers write operation.
		void
		handle_trivial_write_operation( const trivial_write_operation_t & op )
		{
			// Asio buffers (param for async write):
			auto & bufs = op.get_trivial_bufs();

			if( m_response_coordinator.closed() )
			{
				m_logger.trace( [&]{
					return fmt::format(
							"[connection:{}] sending resp data with "
							"connection-close attribute "
							"buf count: {}, "
							"total size: {}",
							connection_id(),
							bufs.size(),
							op.size() );
				} );

				// Reading new requests is useless.
				asio_ns::error_code ignored_ec;
				m_socket.cancel( ignored_ec );
			}
			else
			{
				m_logger.trace( [&]{
					return fmt::format(
						"[connection:{}] sending resp data, "
						"buf count: {}, "
						"total size: {}",
						connection_id(),
						bufs.size(),
						op.size() ); } );
			}

			// There is somethig to write.
			asio_ns::async_write(
				m_socket,
				bufs,
				asio_ns::bind_executor(
					this->get_executor(),
					[this, ctx = shared_from_this()]
					// NOTE: since v.0.6.0 this lambda is noexcept.
					( const asio_ns::error_code & ec, std::size_t written ) noexcept
					{
						if( !ec )
						{
							restinio::utils::log_trace_noexcept( m_logger,
								[&]{
									return fmt::format(
											"[connection:{}] outgoing data was sent: {} bytes",
											connection_id(),
											written );
								} );
						}

						RESTINIO_ENSURE_NOEXCEPT_CALL( after_write( ec ) );
					} ) );

			guard_write_operation();
		}

		//! Run sendfile write operation.
		void
		handle_file_write_operation( file_write_operation_t & op )
		{
			if( m_response_coordinator.closed() )
			{
				m_logger.trace( [&]{
					return fmt::format(
							"[connection:{}] sending resp file data with "
							"connection-close attribute, "
							"total size: {}",
							connection_id(),
							op.size() );
				} );

				// Reading new requests is useless.
				asio_ns::error_code ignored_ec;
				m_socket.cancel( ignored_ec );
			}
			else
			{
				m_logger.trace( [&]{
					return fmt::format(
						"[connection:{}] sending resp file data, total size: {}",
						connection_id(),
						op.size() );
				} );
			}

			guard_sendfile_operation( op.timelimit() );

			auto op_ctx = op;

			op_ctx.start_sendfile_operation(
				this->get_executor(),
				m_socket,
				asio_ns::bind_executor(
					this->get_executor(),
					[this, ctx = shared_from_this(),
						// Store operation context till the end
						op_ctx ]
					// NOTE: since v.0.6.0 this lambda is noexcept
					(const asio_ns::error_code & ec, file_size_t written ) mutable noexcept
					{
						// NOTE: op_ctx should be reset just before return from
						// that lambda. We can't call reset() until the end of
						// the lambda because lambda object itself will be
						// destroyed.
						auto op_ctx_reseter = restinio::utils::at_scope_exit(
								[&op_ctx] {
									// Reset sendfile operation context.
									RESTINIO_ENSURE_NOEXCEPT_CALL( op_ctx.reset() );
								} );

						if( !ec )
						{
							restinio::utils::log_trace_noexcept( m_logger,
								[&]{
									return fmt::format(
											"[connection:{}] file data was sent: {} bytes",
											connection_id(),
											written );
								} );
						}
						else
						{
							restinio::utils::log_error_noexcept( m_logger,
								[&]{
									return fmt::format(
											"[connection:{}] send file data error: {} ({}) bytes",
											connection_id(),
											ec.value(),
											ec.message() );
								} );
						}

						RESTINIO_ENSURE_NOEXCEPT_CALL( after_write( ec ) );
					} ) );
		}

		//! Do post write actions for current write group.
		void
		finish_handling_current_write_ctx()
		{
			// Finishing writing this group.
			m_logger.trace( [&]{
				return fmt::format(
						"[connection:{}] finishing current write group",
						this->connection_id() );
			} );

			// Group notificators are called from here (if exist):
			m_write_output_ctx.finish_write_group();

			if( !m_response_coordinator.closed() )
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
					if( m_init_read_after_this_write )
					{
						wait_for_http_message();
					}

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

		void
		handle_nothing_to_write()
		{
			if( m_response_coordinator.closed() )
			{
				// Bufs empty but there happened to
				// be a response context marked as complete
				// (final_parts) and having connection-close attr.
				// It is because `init_write_if_necessary()`
				// is called only under `!m_response_coordinator.closed()`
				// condition, so if no bufs were obtained
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

		//! Handle write response finished.
		/*!
		 * @note
		 * Since v.0.6.0 this method is noexcept.
		 */
		void
		after_write( const asio_ns::error_code & ec ) noexcept
		{
			if( !ec )
			{
				RESTINIO_ENSURE_NOEXCEPT_CALL( handle_current_write_ctx() );
			}
			else
			{
				if( !error_is_operation_aborted( ec ) )
				{
					trigger_error_and_close( [&]{
						return fmt::format(
							"[connection:{}] unable to write: {}",
							connection_id(),
							ec.message() );
					} );
				}
				// else: Operation aborted only in case of close was called.

				try
				{
					m_write_output_ctx.fail_write_group( ec );
				}
				catch( const std::exception & ex )
				{
					restinio::utils::log_error_noexcept( m_logger,
						[&]{
							return fmt::format(
								"[connection:{}] notificator error: {}",
								connection_id(),
								ex.what() );
						} );
				}
			}
		}

		//! Close connection functions.
		//! \{

		//! Standard close routine.
		void
		close() noexcept
		{
			restinio::utils::log_trace_noexcept( m_logger,
				[&]{
					return fmt::format(
						"[connection:{}] close",
						connection_id() );
				} );

			// shutdown() and close() should be called regardless of
			// possible exceptions.
			restinio::utils::suppress_exceptions(
				m_logger,
				"connection.socket.shutdown",
				[this] {
					asio_ns::error_code ignored_ec;
					m_socket.shutdown(
						asio_ns::ip::tcp::socket::shutdown_both,
						ignored_ec );
				} );
			restinio::utils::suppress_exceptions(
				m_logger,
				"connection.socket.close",
				[this] {
					m_socket.close();
				} );

			restinio::utils::log_trace_noexcept( m_logger,
				[&]{
					return fmt::format(
						"[connection:{}] close: close socket",
						connection_id() );
				} );

			// Clear stuff.
			RESTINIO_ENSURE_NOEXCEPT_CALL( cancel_timeout_checking() );

			restinio::utils::log_trace_noexcept( m_logger,
				[&]{
					return fmt::format(
						"[connection:{}] close: timer canceled",
						connection_id() );
				} );

			RESTINIO_ENSURE_NOEXCEPT_CALL( m_response_coordinator.reset() );

			restinio::utils::log_trace_noexcept( m_logger,
				[&]{
					return fmt::format(
						"[connection:{}] close: reset responses data",
						connection_id() );
				} );

			// Inform state listener if it used.
			m_settings->call_state_listener_suppressing_exceptions(
				[this]() noexcept {
					return connection_state::notice_t{
							this->connection_id(),
							this->m_remote_endpoint,
							connection_state::closed_t{}
						};
				} );
		}

		//! Trigger an error.
		/*!
			Closes the connection and write to log
			an error message.
		*/
		template< typename Message_Builder >
		void
		trigger_error_and_close( Message_Builder msg_builder ) noexcept
		{
			// An exception from logger/msg_builder shouldn't prevent
			// a call to close().
			restinio::utils::log_error_noexcept(
					m_logger, std::move(msg_builder) );

			RESTINIO_ENSURE_NOEXCEPT_CALL( close() );
		}
		//! \}

		//! Connection.
		stream_socket_t m_socket;

		//! Common paramaters of a connection.
		connection_settings_handle_t< Traits > m_settings;

		//! Remote endpoint for this connection.
		const endpoint_t m_remote_endpoint;

		//! Input routine.
		connection_input_t m_input;

		//! Write to socket operation context.
		write_group_output_ctx_t m_write_output_ctx;

		// Memo flag: whether we need to resume read after this group is written
		bool m_init_read_after_this_write{ false };

		//! Response coordinator.
		response_coordinator_t m_response_coordinator;

		//! Timer to controll operations.
		//! \{

		//! Check timeouts for all activities.
		static connection_t &
		cast_to_self( tcp_connection_ctx_base_t & base )
		{
			return static_cast< connection_t & >( base );
		}

		//! Schedules real timedout operations check on
		//! the executer of a connection.
		virtual void
		check_timeout( tcp_connection_ctx_handle_t & self ) override
		{
			asio_ns::dispatch(
				this->get_executor(),
				[ ctx = std::move( self ) ]
				// NOTE: this lambda is noexcept since v.0.6.0.
				() noexcept {
					auto & conn_object = cast_to_self( *ctx );
					// If an exception will be thrown we can only
					// close the connection.
					try
					{
						conn_object.check_timeout_impl();
					}
					catch( const std::exception & x )
					{
						conn_object.trigger_error_and_close( [&] {
								return fmt::format( "[connection: {}] unexpected "
										"error during timeout handling: {}",
										conn_object.connection_id(),
										x.what() );
							} );
					}
				} );
		}

		//! Callback type for timedout operations.
		using timout_cb_t = void (connection_t::* )( void );

		//! Callback to all if timeout happened.
		timout_cb_t m_current_timeout_cb{ nullptr };

		//! Timeout point of a current guarded operation.
		std::chrono::steady_clock::time_point m_current_timeout_after;
		//! Timer guard.
		timer_guard_t m_timer_guard;
		//! A prepared weak handle for passing it to timer guard.
		tcp_connection_ctx_weak_handle_t m_prepared_weak_ctx;

		//! Check timed out operation.
		void
		check_timeout_impl()
		{
			if( std::chrono::steady_clock::now() > m_current_timeout_after )
			{
				if( m_current_timeout_cb )
					(this->*m_current_timeout_cb)();
			}
			else
			{
				init_next_timeout_checking();
			}
		}

		//! Schedule next timeout checking.
		void
		init_next_timeout_checking()
		{
			m_timer_guard.schedule( m_prepared_weak_ctx );
		}

		//! Stop timout guarding.
		void
		cancel_timeout_checking() noexcept
		{
			m_current_timeout_cb = nullptr;
			RESTINIO_ENSURE_NOEXCEPT_CALL( m_timer_guard.cancel() );
		}

		//! Helper function to work with timer guard.
		void
		schedule_operation_timeout_callback(
			std::chrono::steady_clock::time_point timeout_after,
			timout_cb_t timout_cb )
		{
			m_current_timeout_after = timeout_after;
			m_current_timeout_cb = timout_cb;
		}

		void
		schedule_operation_timeout_callback(
			std::chrono::steady_clock::duration timeout,
			timout_cb_t timout_cb )
		{
			schedule_operation_timeout_callback(
				std::chrono::steady_clock::now() + timeout,
				timout_cb );
		}

		void
		handle_xxx_timeout( const char * operation_name )
		{
			m_logger.trace( [&]{
				return fmt::format(
						"[connection:{}] {} timed out",
						connection_id(),
						operation_name );
			} );

			close();
		}

		void
		handle_read_timeout()
		{
			handle_xxx_timeout( "wait for request" );
		}

		//! Statr guard read operation if necessary.
		void
		guard_read_operation()
		{
			if( m_response_coordinator.empty() )
			{
				schedule_operation_timeout_callback(
					m_settings->m_read_next_http_message_timelimit,
					&connection_t::handle_read_timeout );
			}
		}

		void
		handle_request_handling_timeout()
		{
			handle_xxx_timeout( "handle request" );
		}

		//! Start guard request handling operation if necessary.
		void
		guard_request_handling_operation()
		{
			if( !m_write_output_ctx.transmitting() )
			{
				schedule_operation_timeout_callback(
					m_settings->m_handle_request_timeout,
					&connection_t::handle_request_handling_timeout );
			}
		}

		void
		handle_write_response_timeout()
		{
			handle_xxx_timeout( "writing response" );
		}

		//! Start guard write operation if necessary.
		void
		guard_write_operation()
		{
			schedule_operation_timeout_callback(
				m_settings->m_write_http_response_timelimit,
				&connection_t::handle_write_response_timeout );
		}

		void
		handle_sendfile_timeout()
		{
			handle_xxx_timeout( "writing response (sendfile)" );
		}

		void
		guard_sendfile_operation( std::chrono::steady_clock::duration timelimit )
		{
			if( std::chrono::steady_clock::duration::zero() == timelimit )
				timelimit = m_settings->m_write_http_response_timelimit;

			schedule_operation_timeout_callback(
				timelimit,
				&connection_t::handle_sendfile_timeout );
		}
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
template < typename Traits >
class connection_factory_t
{
	public:
		using logger_t = typename Traits::logger_t;
		using stream_socket_t = typename Traits::stream_socket_t;

		connection_factory_t(
			connection_settings_handle_t< Traits > connection_settings,
			std::unique_ptr< socket_options_setter_t > socket_options_setter )
			:	m_connection_settings{ std::move( connection_settings ) }
			,	m_socket_options_setter{ std::move( socket_options_setter ) }
			,	m_logger{ *(m_connection_settings->m_logger ) }
		{}

		// NOTE: since v.0.6.3 it returns non-empty
		// shared_ptr<connection_t<Traits>> or anexception is thrown in
		// the case of an error.
		auto
		create_new_connection(
			stream_socket_t socket,
			endpoint_t remote_endpoint )
		{
			using connection_type_t = connection_t< Traits >;

			{
				socket_options_t options{ socket.lowest_layer() };
				(*m_socket_options_setter)( options );
			}

			return std::make_shared< connection_type_t >(
				m_connection_id_counter++,
				std::move( socket ),
				m_connection_settings,
				std::move( remote_endpoint ) );
		}

	private:
		connection_id_t m_connection_id_counter{ 1 };

		connection_settings_handle_t< Traits > m_connection_settings;

		std::unique_ptr< socket_options_setter_t > m_socket_options_setter;

		logger_t & m_logger;
};

} /* namespace impl */

} /* namespace restinio */
