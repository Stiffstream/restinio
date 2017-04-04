/*
	restinio
*/

/*!
	HTTP-Connection handler routine.
*/

#pragma once

#include <asio.hpp>

#include <nodejs/http_parser/http_parser.h>

#include <fmt/format.h>

#include <restinio/http_headers.hpp>
#include <restinio/connection_handle.hpp>
#include <restinio/request_handler.hpp>

namespace restinio
{

namespace impl
{

//
// parser_ctx_t
//

//! Parsing result context for using in parser callbacks.
/*!
	All data is used as temps, and is usable only
	after parsing completes new requests then it is moved out.
*/
struct parser_ctx_t
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
	bool m_message_complete{ false };
	//! \}

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

//
// input_buffer_t
//

//! Helper class for reading bytes and feeding them to parser.
class input_buffer_t
{
	public:
		input_buffer_t( std::size_t size )
		{
			m_buf.resize( size );
		}

		//! Make asio buffer for reading bytes from socket.
		auto
		make_asio_buffer()
		{
			return asio::buffer( m_buf.data(), m_buf.size() );
		}

		//! Mark how many bytes were obtained.
		void
		obtained_bytes( std::size_t length )
		{
			m_ready_length = length; // Current bytes in buffer.
			m_ready_pos = 0; // Reset current pos.
		}

		//! Mark how many bytes were obtained.
		void
		consumed_bytes( std::size_t length )
		{
			m_ready_length -= length; // decrement buffer length.
			m_ready_pos += length; // Shift current pos.
		}

		//! How many unconsumed bytes are there in buffer.
		std::size_t
		length() const
		{
			return m_ready_length;
		}

		//! Get pointer to unconsumed bytes.
		/*!
			\note To check that buffer has unconsumed bytes use length().
		*/
		const char *
		bytes() const
		{
			return m_buf.data() + m_ready_pos;
		}

	private:
		//! Buffer for io operation.
		std::vector< char > m_buf;

		//! unconsumed data left in buffer:
		//! \{
		//! Start of data in buffer.
		std::size_t m_ready_pos{0};

		//! Data size.
		std::size_t m_ready_length{0};
		//! \}
};

//
// ct_cstr_len
//

//! compile time c-string length.
constexpr std::size_t
ct_cstr_len( const char * s )
{
	return *s ? 1 + ct_cstr_len( s + 1) : 0;
}

//! Creates a string for http response header.
inline std::string
create_header_string(
	const http_response_header_t & h,
	std::size_t buffer_size )
{
	std::string result;
	result.reserve( buffer_size );

	constexpr const char * header_part1 = "HTTP/";
	result.append( header_part1, ct_cstr_len( header_part1 ) );

	result += '0' + h.http_major();
	result += '.';
	result += '0' + h.http_minor();
	result += ' ';

	auto status_code = h.status_code();

//FIXME: я думаю, что для status_code нужно делать какую-то проверку того,
//что status_code находится в диапазоне 100..999.
//Возможно, это нужно делать на уровне http_response_header_t (например,
//в соответствующем setter-е). Или за счет введения типа
//bounded_value_t<100, 999>.
	result += '0' + ( status_code / 100 ) % 10;
	result += '0' + ( status_code / 10 ) % 10;
	result += '0' + ( status_code ) % 10;

	result += ' ';
	result += h.reason_phrase();

	constexpr const char * header_rn = "\r\n";
	result.append( header_rn, ct_cstr_len( header_rn ) );

	if( h.should_keep_alive() )
	{
		constexpr const char * header_part2_1 = "Connection: keep-alive\r\n";
		result.append( header_part2_1, ct_cstr_len( header_part2_1 ) );
	}
	else
	{
		constexpr const char * header_part2_2 = "Connection: close\r\n";
		result.append( header_part2_2, ct_cstr_len( header_part2_2 ) );
	}

	{
		std::array< char, 64 > buf;
		const auto n =
			std::snprintf(
				buf.data(),
				buf.size(),
				"Content-Length: %llu\r\n",
				static_cast< unsigned long long >( h.content_length() ) );

		result.append( buf.data(), n );
	}


	constexpr const char * header_field_sep = ": ";
	for( const auto & f : h )
	{
		result += f.m_name;
		result.append( header_field_sep, ct_cstr_len( header_field_sep ) );
		result += f.m_value;
		result.append( header_rn, ct_cstr_len( header_rn ) );
	}

	result.append( header_rn, ct_cstr_len( header_rn ) );

	return result;
}

inline auto
create_error_resp( std::uint16_t status, std::string phrase )
{
	http_response_header_t h{ status, std::move( phrase ) };
	h.should_keep_alive( false );
	h.http_major( 1 );
	h.http_minor( 1 );
	return h;
}

inline auto
create_not_implemented_resp()
{
	return create_error_resp( 501, "Not Implemented" );
}

inline auto
create_timeout_resp()
{
	return create_error_resp( 504, "Gateway Time-out" );
}

//
// connection_settings_t
//

//! Parameters shared between connections.
/*!
	Each connection has access to common params and
	server-agent throught this object.
*/
template < typename TRAITS >
struct connection_settings_t final
	:	public std::enable_shared_from_this< connection_settings_t< TRAITS > >
{
	using request_handler_t = typename TRAITS::request_handler_t;
	using logger_t = typename TRAITS::logger_t;

	template < typename SETTINGS >
	connection_settings_t(
		SETTINGS & settings )
		:	m_request_handler{ settings.request_handler() }
		,	m_buffer_size{ settings.buffer_size() }
		,	m_read_next_http_message_timelimit{
				settings.read_next_http_message_timelimit() }
		,	m_write_http_response_timelimit{
				settings.write_http_response_timelimit() }
		,	m_handle_request_timeout{
				settings.handle_request_timeout() }
		,	m_logger{ settings.logger() }
	{}

	//! Request handler factory.
	std::unique_ptr< request_handler_t > m_request_handler;

	//! Parser settings.
	/*!
		Parsing settings are common for each connection.
	*/
	const http_parser_settings m_parser_settings{ create_parser_settings() };

	//! Params from server_settings_t.
	//! \{
	std::size_t m_buffer_size{ 4 * 1024 };

	std::chrono::steady_clock::duration
		m_read_next_http_message_timelimit{ std::chrono::seconds( 60 ) };

	std::chrono::steady_clock::duration
		m_write_http_response_timelimit{ std::chrono::seconds( 5 ) };

	std::chrono::steady_clock::duration
		m_handle_request_timeout{ std::chrono::seconds( 10 ) };

	const std::unique_ptr< logger_t > m_logger;
};

template < typename TRAITS >
using connection_settings_shared_ptr_t =
	std::shared_ptr< connection_settings_t< TRAITS > >;

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

		connection_t(
			//! Connection id.
			std::uint64_t conn_id,
			//! Connection socket.
			asio::ip::tcp::socket && socket,
			//! Settings that are common for connections.
			connection_settings_shared_ptr_t< TRAITS > settings,
			//! Operation timeout guard.
			timer_guard_instance_t timer_guard )
			:	connection_base_t{ conn_id }
			,	m_socket{ std::move( socket ) }
			,	m_strand{ m_socket.get_executor() }
			,	m_settings{ std::move( settings ) }
			,	m_buf{ m_settings->m_buffer_size }
			,	m_timer_guard{ std::move( timer_guard ) }
			,	m_request_handler{ *( m_settings->m_request_handler ) }
			,	m_logger{ *( m_settings->m_logger ) }
		{
			// Notify of a new connection instance.
			m_logger.trace( [&](){
					return fmt::format(
						"[connection:{}] start connection with {}",
						connection_id(),
						m_socket.remote_endpoint() );
			} );
		}

		~connection_t()
		{
			try
			{
				// Notify of a new connection instance.
				m_logger.trace( [&](){
					return fmt::format(
						"[connection:{}] destroyed",
						connection_id() );
				} );
			}
			catch( ... )
			{}
		}

		strand_t &
		get_executor()
		{
			return m_strand;
		}

		//! Start reading next htttp-message.
		void
		wait_for_http_message()
		{
			m_logger.trace( [&](){
				 return fmt::format(
						"[connection:{}] start waiting for request",
						connection_id() );
			} );

			// Prepare parser for consuming new request message.
			reset_parser();

			// Guard total time for a request to be read.
			// guarding here makes the total read process
			// to run in read_next_http_message_timelimit.
			schedule_operation_timeout_callback(
				m_settings->m_read_next_http_message_timelimit,
				[ this ](){
					m_logger.trace( [&](){
						return fmt::format(
								"[connection:{}] wait for request timed out",
								this->connection_id() );
					} );
					close();
				} );

			if( 0 != m_buf.length() )
			{
				// If a pipeline requests were sent by client
				// then the biginning (or even entire request) of it
				// is in the buffer obtained from socket in previous
				// read operation.
				consume_data( m_buf.bytes(), m_buf.length() );
			}
			else
			{
				// Next request (if any) must be obtained from socket.
				consume_message();
			}
		}

		//! Initiate write response data to socket.
		virtual void
		write_response_message(
			//! Response header.
			http_response_header_t http_header,
			//! Body.
			std::string http_body ) override
		{
			http_header.content_length( http_body.size() );

			//! Run write message on io_service loop if possible.
			asio::dispatch(
				get_executor(),
				[ this,
					header = std::move( http_header ),
					body = std::move( http_body ),
					ctx = shared_from_this() ](){
					write_response_message_impl(
						header,
						std::move( body ) );
				} );
		}

		//! Close this connection
		void
		close()
		{
			m_timer_guard->cancel();

			m_logger.trace( [&](){
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

	private:
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

		//! Write response data to socket.
		/*!
			\note Body param is value,
			because it is supposed to passed using move-semantics.
		*/
		void
		write_response_message_impl(
			//! Response header.
			const http_response_header_t & http_header,
			//! Body.
			std::string body = std::string{} )
		{
			if( !m_socket.is_open() ||
				m_resp_ctx.transmitting() )
			{
				m_logger.warn( [&](){
					return fmt::format(
							"[connection:{}] try to write response, "
							"while already sending response",
							connection_id() );
				} );

				return;
			}

			m_logger.trace( [&](){
				return fmt::format(
					"[connection:{}] writing response ",
					connection_id() );
			} );

			// Start writting.
			asio::async_write(
				m_socket,
				m_resp_ctx.create_bufs(
					create_header_string(
						http_header,
						m_settings->m_buffer_size ),
					std::move( body ) ),
				asio::wrap(
					get_executor(),
					[ this,
						ctx = shared_from_this(),
						should_keep_alive = http_header.should_keep_alive() ]
						( auto ec, std::size_t written ){
							this->after_write( ec, written, should_keep_alive );
					} ) );

			schedule_operation_timeout_callback(
				m_settings->m_write_http_response_timelimit,
				[ this ](){
					m_logger.trace( [&](){
						return fmt::format(
								"[connection:{}] writing response timed out",
								this->connection_id() );
					} );
					close();
				} );
		}

		//! Handle write response finished.
		void
		after_write(
			const std::error_code & ec,
			std::size_t written,
			bool should_keep_alive )
		{
			if( ec )
			{
				if( ec != asio::error::operation_aborted )
					trigger_error_and_close( [&](){
						return fmt::format(
							"[connection:{}] unable to write response: {}",
							connection_id(),
							ec.message() );
					} );
				else
				{
					// Simply close if operation was aborted.
					close();
				}
			}
			else
			{
				// Release allocated strings data.
				m_resp_ctx.done();

				m_logger.trace( [&](){
					return fmt::format(
							"[connection:{}] response was sent",
							connection_id() );
				} );

				if( should_keep_alive )
				{
					m_logger.trace( [&](){
						return fmt::format(
								"[connection:{}] should keep alive",
								this->connection_id() );
					} );

					// If keep-alive needed
					// then start waiting for request
					wait_for_http_message();
				}
				else
				{
					// No keep-alive, close connection.
					close();
				}
			}
		}

		//! Start (continue) a chain of read-parse-read-... operations.
		void
		consume_message()
		{
			m_logger.trace( [&](){
				return fmt::format(
						"[connection:{}] continue reading request",
						connection_id() );
			} );

			m_socket.async_read_some(
				m_buf.make_asio_buffer(),
				asio::wrap(
					get_executor(),
					[ this, ctx = shared_from_this() ]( auto ec, std::size_t length ){
						this->after_read( ec, length );
					} ) );
		}

		void
		after_read(
			const std::error_code & ec,
			std::size_t length )
		{
			if( !ec )
			{
				m_logger.trace( [&](){
					return fmt::format(
							"[connection:{}] received {} bytes",
							this->connection_id(),
							length );
				} );

				m_buf.obtained_bytes( length );

				consume_data( m_buf.bytes(), length );
			}
			else
			{
				// Well, if it is actually an error
				// then close connection.
				if( ec != asio::error::operation_aborted )
				{
					if ( ec != asio::error::eof || 0 != m_parser.nread )
						trigger_error_and_close( [&](){
							return fmt::format(
									"[connection:{}] read socket error: {}; "
									"parsed bytes: {}",
									connection_id(),
									ec.message(),
									m_parser.nread );
						} );
					else
					{
						// A case that is not such an  error:
						// on a connection (most probably keeped alive
						// after previous request, but a new also applied)
						// no bytes were consumed and remote peer closes connection.
						m_logger.trace( [&](){
							return fmt::format(
									"[connection:{}] EOF and no request, "
									"close connection",
									connection_id() );
						} );

						close();
					}
				}
				else
				{
					// Simply close.
					close();
				}
			}
		}

		//! Parse some data.
		void
		consume_data( const char * data, std::size_t length )
		{
			const auto nparsed =
				http_parser_execute(
					&m_parser,
					&( m_settings->m_parser_settings ),
					data,
					length );

			// If entire http-message was obtained,
			// parser is stopped and the might be a part of consecutive request
			// left in buffer, so we mark how many bytes were obtained.
			// and next message read (if any) will be started from already existing
			// data left in buffer.
			m_buf.consumed_bytes( nparsed );

			if( HPE_OK != m_parser.http_errno &&
				HPE_PAUSED != m_parser.http_errno )
			{
				// PARSE ERROR:
				auto err = HTTP_PARSER_ERRNO( &m_parser );

				trigger_error_and_close( [&](){
					return fmt::format(
							"[connection:{}] parser error {}: {}",
							connection_id(),
							http_errno_name( err ),
							http_errno_description( err ) );
				} );

				// nothing to do.
				return;
			}

			if( m_parser_ctx.m_message_complete )
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
			m_logger.trace( [&](){
				return fmt::format(
						"[connection:{}] request received: {} {}",
						connection_id(),
						http_method_str( (http_method) m_parser.method ),
						m_parser_ctx.m_header.request_target() );
			} );

			try
			{
				// TODO: mb there is a way to
				// track if response was emmited immediately in handler
				// or it was delegated
				// so it is possible to omit this timer scheduling.
				schedule_operation_timeout_callback(
					m_settings->m_handle_request_timeout,
					[ this ](){
						m_logger.warn( [&](){
							return fmt::format(
									"[connection:{}] handle request timed out",
									this->connection_id() );
						} );

						write_response_message_impl( create_timeout_resp() );
					} );

				if( request_rejected() ==
					m_request_handler(
						std::make_shared< http_request_t >(
							std::move( m_parser_ctx.m_header ),
							std::move( m_parser_ctx.m_body ) ),
						shared_from_this() ) )
				{
					// If handler refused request, say not implemented.
					write_response_message_impl( create_not_implemented_resp() );
				}
			}
			catch( const std::exception & ex )
			{
				trigger_error_and_close( [&](){
					return fmt::format(
							"[connection:{}] error while handling request: {}",
							this->connection_id(),
							ex.what() );
				} );
			}
		}

		//! Trigger an error.
		/*!
			Closes the connection and
			notifies server agent of an error with connection.
		*/
		template< typename MSG_BUILDER >
		void
		trigger_error_and_close( MSG_BUILDER && msg_builder )
		{
			m_logger.error( std::move( msg_builder ) );

			close();
		}


		//! Connection
		asio::ip::tcp::socket m_socket;

		//! Sync object for connection events.
		strand_t m_strand;

		//! Common paramaters for buffer.
		connection_settings_shared_ptr_t< TRAITS > m_settings;

		//! Input buffer.
		input_buffer_t m_buf;

		//! HTTP-parser.
		//! \{
		http_parser m_parser;
		parser_ctx_t m_parser_ctx;
		//! \}

		//
		// raw_resp_ctx_t
		//

		//! Helper class for writting response data.
		struct raw_resp_ctx_t
		{
			const std::array< asio::const_buffer, 2 > &
			create_bufs(
				std::string header,
				std::string body )
			{
				m_header.assign( std::move( header ) );
				m_body.assign( std::move( body ) );


				m_bufs[ 0 ] = asio::buffer( m_header.data(), m_header.size() );

				if( !m_body.empty() )
					m_bufs[ 1 ] = asio::buffer( m_body.data(), m_body.size() );

				m_transmitting = true;
				return m_bufs;
			}

			void
			done()
			{
				m_header.clear();
				m_body.clear();
				m_transmitting = false;
			}

			bool
			transmitting() const
			{
				return m_transmitting;
			}

			private:
				std::string m_header;
				std::string m_body;
				std::array< asio::const_buffer, 2 > m_bufs;
				bool m_transmitting{ false };
		};

		raw_resp_ctx_t m_resp_ctx;

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
				throw std::invalid_argument{ "timer_factory not set" };
		}

		auto
		create_new_connection(
			asio::ip::tcp::socket && socket )
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
