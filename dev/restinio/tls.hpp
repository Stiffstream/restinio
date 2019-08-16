/*
	restinio
*/

/*!
	Support for https.
*/

#pragma once

#include <restinio/traits.hpp>
#include <restinio/impl/tls_socket.hpp>

namespace restinio
{

namespace connection_state
{

//FIXME: document this!
class tls_accessor_t
{
	tls_socket_t & m_tls_socket;

public:
	tls_accessor_t( tls_socket_t & tls_socket ) : m_tls_socket{tls_socket} {}

	//FIXME: document this!
	auto native_handle() const noexcept
	{
		return m_tls_socket.asio_ssl_stream().native_handle();
	}
};

//
// The implementation of TLS-related part of notice_t.
//

template< typename Lambda >
void
notice_t::try_inspect_tls( Lambda && lambda ) const
{
	if( m_tls_socket )
		lambda( tls_accessor_t{*m_tls_socket} );
}

//FIXME: document this!
template< typename Lambda >
decltype(auto)
notice_t::inspect_tls_or_throw( Lambda && lambda ) const
{
	if( !m_tls_socket )
		throw exception_t{ "an attempt to call inspect_tls for "
				"non-TLS-connection" };

	return lambda( tls_accessor_t{*m_tls_socket} );
}

//FIXME: document this!
template< typename Lambda, typename T >
T
notice_t::inspect_tls_or_default( Lambda && lambda, T && default_value ) const
{
	if( m_tls_socket )
		return lambda( tls_accessor_t{*m_tls_socket} );

	return default_value;
}

} /* namespace connection_state */

//
// tls_traits_t
//

template <
		typename Timer_Factory,
		typename Logger,
		typename Request_Handler = default_request_handler_t,
		typename Strand = asio_ns::strand< asio_ns::executor > >
using tls_traits_t = traits_t< Timer_Factory, Logger, Request_Handler, Strand, tls_socket_t >;

//
// single_thread_traits_t
//

template <
		typename Timer_Factory,
		typename Logger,
		typename Request_Handler = default_request_handler_t >
using single_thread_tls_traits_t =
	tls_traits_t< Timer_Factory, Logger, Request_Handler, noop_strand_t >;

using default_tls_traits_t = tls_traits_t< asio_timer_manager_t, null_logger_t >;

//
// prepare_connection_and_start_read()
//

//! Customizes connection init routine with an additional step:
//! perform handshake and only then start reading.
template < typename Connection, typename Start_Read_CB, typename Failed_CB >
void
prepare_connection_and_start_read(
	tls_socket_t & socket,
	Connection & con,
	Start_Read_CB start_read_cb,
	Failed_CB failed_cb )
{
	socket.async_handshake(
		asio_ns::ssl::stream_base::server,
		[ start_read_cb = std::move( start_read_cb ),
			failed_cb = std::move( failed_cb ),
			con = con.shared_from_this() ]( const asio_ns::error_code & ec ){
			if( !ec )
				start_read_cb();
			else
				failed_cb( ec );
		} );
}

//
// socket_type_dependent_settings_t
//

//! Customizes extra settings needed for working with socket.
/*!
	Adds tls context setting.
*/
template < typename Settings >
class socket_type_dependent_settings_t< Settings, tls_socket_t >
{
protected:
		~socket_type_dependent_settings_t() = default;

public:
		socket_type_dependent_settings_t() = default;
		socket_type_dependent_settings_t(
			socket_type_dependent_settings_t && ) = default;

		Settings &
		tls_context(
			asio_ns::ssl::context context ) &
		{
			m_tls_context = std::move( context );
			return upcast_reference();
		}

		Settings &&
		tls_context(
			asio_ns::ssl::context context ) &&
		{
			return std::move( this->tls_context( std::move( context ) ) );
		}

		asio_ns::ssl::context
		tls_context()
		{
			return asio_ns::ssl::context{ std::move( m_tls_context ) };
		}

	private:
		Settings &
		upcast_reference()
		{
			return static_cast< Settings & >( *this );
		}

		asio_ns::ssl::context m_tls_context{ asio_ns::ssl::context::sslv23 };
};

namespace impl
{

// An overload for the case of non-TLS-connection.
inline tls_socket_t *
make_tls_socket_pointer_for_state_listener(
	tls_socket_t & socket ) noexcept
{
	return &socket;
}

//
// socket_supplier_t
//

//! A custom socket storage for tls_socket_t.
template <>
class socket_supplier_t< tls_socket_t >
{
	protected:
		template < typename Settings >
		socket_supplier_t(
			Settings & settings,
			asio_ns::io_context & io_context )
			:	m_tls_context{ std::make_shared< asio_ns::ssl::context >( settings.tls_context() ) }
			,	m_io_context{ io_context }
		{
			m_sockets.reserve( settings.concurrent_accepts_count() );

			while( m_sockets.size() < settings.concurrent_accepts_count() )
			{
				m_sockets.emplace_back( m_io_context, m_tls_context );
			}
		}

		virtual ~socket_supplier_t() = default;

		tls_socket_t &
		socket(
			//! Index of a socket in the pool.
			std::size_t idx )
		{
			return m_sockets.at( idx );
		}

		auto
		move_socket(
			//! Index of a socket in the pool.
			std::size_t idx )
		{
			tls_socket_t res{ m_io_context, m_tls_context };
			std::swap( res, m_sockets.at( idx ) );
			return res;
		}

		//! The number of sockets that can be used for
		//! cuncurrent accept operations.
		auto
		cuncurrent_accept_sockets_count() const
		{
			return m_sockets.size();
		}

	private:
		std::shared_ptr< asio_ns::ssl::context > m_tls_context;
		asio_ns::io_context & m_io_context;
		std::vector< tls_socket_t > m_sockets;
};

} /* namespace impl */

} /* namespace restinio */
