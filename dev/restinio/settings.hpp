/*
	restinio
*/

/*!
	HTTP-Server configuration.
*/

#pragma once

#include <chrono>
#include <tuple>
#include <utility>

#include <asio.hpp>

#include <restinio/exception.hpp>
#include <restinio/request_handler.hpp>
#include <restinio/io_context_wrapper.hpp>

namespace restinio
{

namespace details
{

//! Default instantiation for a specific type.
template < typename OBJECT_TYPE >
inline auto
create_default_object_instance( std::false_type )
{
	return std::unique_ptr< OBJECT_TYPE >{};
}

template < typename OBJECT_TYPE >
inline auto
create_default_object_instance( std::true_type )
{
	return std::make_unique< OBJECT_TYPE >();
}

} /* namespace details */

//
// create_default_object_instance
//

//! Default instantiation for a specific type.
template < typename OBJECT_TYPE>
inline auto
create_default_object_instance()
{
	typename std::is_default_constructible< OBJECT_TYPE >::type tag;
	return details::create_default_object_instance< OBJECT_TYPE >( tag );
}

//! Default instantiation for default_request_handler_t.
template <>
inline auto
create_default_object_instance< default_request_handler_t >()
{
	return details::create_default_object_instance< default_request_handler_t >(
			std::false_type{} );
}

//
// unsure_created()
//

//! Ensure that object was created.
/*!
*/
template < typename OBJECT_TYPE >
auto
ensure_created(
	std::unique_ptr< OBJECT_TYPE > mb_created_one,
	const std::string & fail_description )
{
	auto result{ std::move( mb_created_one ) };

	if( !result )
		result = create_default_object_instance< OBJECT_TYPE >();

	if( !result )
		throw exception_t{ fail_description };

	return result;
}

//
// extra_settings_t
//

//! Extra settings needed for working with socket.
template < typename SETTINGS, typename SOCKET_TYPE >
struct extra_settings_t
{
	virtual ~extra_settings_t() = default;

	// No extra settings by default.
};

//
// acceptor_options_t
//

//! An adapter for setting acceptor options before running server.
class acceptor_options_t
{
	public:
		acceptor_options_t( asio::ip::tcp::acceptor & acceptor )
			:	m_acceptor{ acceptor }
		{}

		template< typename OPTION >
		void
		set_option( const OPTION & option )
		{
			m_acceptor.set_option( option );
		}

		template< typename OPTION >
		void
		set_option( const OPTION & option, asio::error_code & ec )
		{
			m_acceptor.set_option( option, ec );
		}

		template< typename OPTION >
		void
		get_option( OPTION & option )
		{
			m_acceptor.get_option( option );
		}

		template< typename OPTION >
		void
		get_option( OPTION & option, asio::error_code & ec )
		{
			m_acceptor.get_option( option, ec );
		}

	private:
		asio::ip::tcp::acceptor & m_acceptor;
};

using acceptor_options_setter_t = std::function< void ( acceptor_options_t & ) >;

template <>
inline auto
create_default_object_instance< acceptor_options_setter_t >()
{
	return std::make_unique< acceptor_options_setter_t >(
		[]( acceptor_options_t & acceptor_options ){
			acceptor_options.set_option( asio::ip::tcp::acceptor::reuse_address( true ) );
		} );
}


//
// server_settings_t
//

//! A fluent style interface for setting http server params.
template < typename TRAITS >
class server_settings_t final
	:	public extra_settings_t< server_settings_t< TRAITS >, typename TRAITS::stream_socket_t >
{
	public:
		server_settings_t(
			std::uint16_t port = 8080,
			asio::ip::tcp protocol = asio::ip::tcp::v4() )
			:	m_port{ port }
			,	m_protocol{ protocol }
		{}

		//! Server endpoint.
		//! \{
		server_settings_t &
		port( std::uint16_t p ) &
		{
			m_port = p;
			return *this;
		}

		server_settings_t &&
		port( std::uint16_t p ) &&
		{
			return std::move( this->port( p ) );
		}

		std::uint16_t
		port() const
		{
			return m_port;
		}

		server_settings_t &
		protocol( asio::ip::tcp p ) &
		{
			m_protocol = p;
			return *this;
		}

		server_settings_t &&
		protocol( asio::ip::tcp p ) &&
		{
			return std::move( this->protocol( p ) );
		}

		asio::ip::tcp
		protocol() const
		{
			return m_protocol;
		}

		server_settings_t &
		address( std::string addr ) &
		{
			m_address = std::move(addr);
			return *this;
		}

		server_settings_t &&
		address( std::string addr ) &&
		{
			return std::move( this->address( std::move( addr ) ) );
		}

		const std::string &
		address() const
		{
			return m_address;
		}
		//! \}

		//! Size of buffer for io operations.
		/*!
			It limits a size of chunk that can be read from socket in a single
			read operattion (async read).
		*/
		//! {
		server_settings_t &
		buffer_size( std::size_t s ) &
		{
			m_buffer_size = s;
			return *this;
		}

		server_settings_t &&
		buffer_size( std::size_t s ) &&
		{
			return std::move( this->buffer_size( s ) );
		}

		std::size_t
		buffer_size() const
		{
			return m_buffer_size;
		}
		//! }

		//! A period for holding connection before completely receiving
		//! new http-request. Starts counting since connection is establised
		//! or a previous request was responsed.
		/*!
			Generaly it defines timeout for keep-alive connections.
		*/
		//! \{
		server_settings_t &
		read_next_http_message_timelimit( std::chrono::steady_clock::duration d ) &
		{
			m_read_next_http_message_timelimit = std::move( d );
			return *this;
		}

		server_settings_t &&
		read_next_http_message_timelimit( std::chrono::steady_clock::duration d ) &&
		{
			return std::move( this->read_next_http_message_timelimit( std::move( d ) ) );
		}

		std::chrono::steady_clock::duration
		read_next_http_message_timelimit() const
		{
			return m_read_next_http_message_timelimit;
		}
		//! \}

		//! A period of time wait for response to be written to socket.
		//! \{
		server_settings_t &
		write_http_response_timelimit( std::chrono::steady_clock::duration d ) &
		{
			m_write_http_response_timelimit = std::move( d );
			return *this;
		}

		server_settings_t &&
		write_http_response_timelimit( std::chrono::steady_clock::duration d ) &&
		{
			return std::move( this->write_http_response_timelimit( std::move( d ) ) );
		}

		std::chrono::steady_clock::duration
		write_http_response_timelimit() const
		{
			return m_write_http_response_timelimit;
		}
		//! \}

		//! A period of time that is given for a handler to create response.
		//! \{
		server_settings_t &
		handle_request_timeout( std::chrono::steady_clock::duration d ) &
		{
			m_handle_request_timeout = std::move( d );
			return *this;
		}

		server_settings_t &&
		handle_request_timeout( std::chrono::steady_clock::duration d ) &&
		{
			return std::move( this->handle_request_timeout( std::move( d ) ) );
		}

		std::chrono::steady_clock::duration
		handle_request_timeout() const
		{
			return m_handle_request_timeout;
		}
		//! \}

		//! Max pipelined requests to receive on single connection.
		//! \{
		server_settings_t &
		max_pipelined_requests( std::size_t mpr ) &
		{
			m_max_pipelined_requests = mpr;
			return *this;
		}

		server_settings_t &&
		max_pipelined_requests( std::size_t mpr ) &&
		{
			return std::move( this->max_pipelined_requests( mpr ) );
		}

		std::size_t
		max_pipelined_requests() const
		{
			return m_max_pipelined_requests;
		}
		//! \}


		//! Request handler.
		//! \{
		using request_handler_t = typename TRAITS::request_handler_t;

		server_settings_t &
		request_handler( std::unique_ptr< request_handler_t > handler ) &
		{
			m_request_handler = std::move( handler );
			return *this;
		}

		template< typename... PARAMS >
		server_settings_t &
		request_handler( PARAMS &&... params ) &
		{
			return set_instance(
					m_request_handler,
					std::forward< PARAMS >( params )... );
		}


		template< typename... PARAMS >
		server_settings_t &&
		request_handler( PARAMS &&... params ) &&
		{
			return std::move( this->request_handler( std::forward< PARAMS >( params )... ) );
		}

		std::unique_ptr< request_handler_t >
		request_handler()
		{
			return ensure_created(
				std::move( m_request_handler ),
				"request handler must be set" );
		}
		//! \}


		//! Timers factory.
		//! \{
		using timer_factory_t = typename TRAITS::timer_factory_t;

		template< typename... PARAMS >
		server_settings_t &
		timer_factory( PARAMS &&... params ) &
		{
			return set_instance(
					m_timer_factory,
					std::forward< PARAMS >( params )... );
		}

		template< typename... PARAMS >
		server_settings_t &&
		timer_factory( PARAMS &&... params ) &&
		{
			return std::move( this->timer_factory( std::forward< PARAMS >( params )... ) );
		}

		std::unique_ptr< timer_factory_t >
		timer_factory()
		{
			return ensure_created(
				std::move( m_timer_factory ),
				"timer factory must be set" );
		}
		//! \}

		//! Logger.
		//! \{
		using logger_t = typename TRAITS::logger_t;

		template< typename... PARAMS >
		server_settings_t &
		logger( PARAMS &&... params ) &
		{
			return set_instance(
					m_logger,
					std::forward< PARAMS >( params )... );
		}

		template< typename... PARAMS >
		server_settings_t &&
		logger( PARAMS &&... params ) &&
		{
			return std::move( this->logger( std::forward< PARAMS >( params )... ) );
		}

		std::unique_ptr< logger_t >
		logger()
		{
			return ensure_created(
				std::move( m_logger ),
				"logger must be set" );
		}
		//! \}

		//! Acceptor options setter.
		//! \{
		server_settings_t &
		acceptor_options_setter( acceptor_options_setter_t aos ) &
		{
			if( m_acceptor_options_setter )
				throw exception_t{ "acceptor options setter cannot be empty" };

			return set_instance(
					m_acceptor_options_setter,
					std::move( aos ) );
		}

		server_settings_t &&
		acceptor_options_setter( acceptor_options_setter_t aos ) &&
		{
			return std::move( this->acceptor_options_setter( std::move( aos ) ) );
		}

		std::unique_ptr< acceptor_options_setter_t >
		acceptor_options_setter()
		{
			return ensure_created(
				std::move( m_acceptor_options_setter ),
				"acceptor_options_setter must be set" );
		}
		//! \}

		//! Max number of running concurrent accepts.
		//! \{
		server_settings_t &
		concurrent_accepts_count( std::size_t n ) &
		{
			if( 0 == n || 1024 < n )
				throw exception_t{
					fmt::format(
						"invalid value for number of cuncurrent connects: {}",
						n ) };

			m_concurrent_accepts_count = n;
			return *this;
		}

		server_settings_t &&
		concurrent_accepts_count( std::size_t n ) &&
		{
			return std::move( this->concurrent_accepts_count( n ) );
		}

		std::size_t
		concurrent_accepts_count() const
		{
			return m_concurrent_accepts_count;
		}
		//! \}

	private:
		template< typename TARGET, typename... PARAMS >
		server_settings_t &
		set_instance( std::unique_ptr< TARGET > & t, PARAMS &&... params )
		{
			t =
				std::make_unique< TARGET >(
					std::forward< PARAMS >( params )... );

			return *this;
		}

		//! Server endpoint.
		//! \{
		std::uint16_t m_port;
		asio::ip::tcp m_protocol;
		std::string m_address;
		//! \}

		//! Size of buffer for io operations.
		std::size_t m_buffer_size{ 4 * 1024 };

		//! Operations timeouts.
		//! \{
		std::chrono::steady_clock::duration
			m_read_next_http_message_timelimit{ std::chrono::seconds( 60 ) };

		std::chrono::steady_clock::duration
			m_write_http_response_timelimit{ std::chrono::seconds( 5 ) };

		std::chrono::steady_clock::duration
			m_handle_request_timeout{ std::chrono::seconds( 10 ) };
		//! \}

		//! Max pipelined requests to receive on single connection.
		std::size_t m_max_pipelined_requests{ 1 };

		//! Request handler.
		std::unique_ptr< request_handler_t > m_request_handler;

		//! Timers factory.
		std::unique_ptr< timer_factory_t > m_timer_factory;

		//! Logger.
		std::unique_ptr< logger_t > m_logger;

		//! Acceptor options setter.
		std::unique_ptr< acceptor_options_setter_t > m_acceptor_options_setter;

		std::size_t m_concurrent_accepts_count{ 1 };
};

template < typename TRAITS, typename CONFIGURATOR >
auto
exec_configurator( CONFIGURATOR && configurator )
{
	server_settings_t< TRAITS > result;

	configurator( result );

	return result;
}

} /* namespace restinio */
