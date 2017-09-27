/*
	restinio
*/

/*!
	Support for https.
*/

#pragma once

#include <asio/ssl.hpp>

#include <restinio/traits.hpp>
#include <restinio/impl/tls_socket.hpp>

namespace restinio
{

using tls_socket_t = impl::tls_socket_t;

//
// tls_traits_t
//

template <
		typename Timer_Factory,
		typename Logger,
		typename Request_Handler = default_request_handler_t,
		typename Strand = asio::strand< asio::executor > >
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
		asio::ssl::stream_base::server,
		[ start_read_cb = std::move( start_read_cb ),
			failed_cb = std::move( failed_cb ),
			con = con.shared_from_this() ]( const asio::error_code & ec ){
			if( !ec )
				start_read_cb();
			else
				failed_cb( ec );
		} );
}

//
// extra_settings_t
//

//! Customizes extra settings needed for working with socket.
/*!
	Adds tls context setting.
*/
template < typename Settings >
class extra_settings_t< Settings, tls_socket_t >
{
	public:
		virtual ~extra_settings_t() = default;

		extra_settings_t() = default;
		extra_settings_t( extra_settings_t && ) = default;

		Settings &
		tls_context(
			asio::ssl::context context ) &
		{
			m_tls_context = std::move( context );
			return upcast_reference();
		}

		Settings &&
		tls_context(
			asio::ssl::context context ) &&
		{
			return std::move( this->tls_context( std::move( context ) ) );
		}

		asio::ssl::context
		tls_context()
		{
			return asio::ssl::context{ std::move( m_tls_context ) };
		}

	private:
		Settings &
		upcast_reference()
		{
			return static_cast< Settings & >( *this );
		}

		asio::ssl::context m_tls_context{ asio::ssl::context::sslv23 };
};

namespace impl
{

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
			asio::io_context & io_context )
			:	m_tls_context{ settings.tls_context() }
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
		asio::ssl::context m_tls_context;
		asio::io_context & m_io_context;
		std::vector< tls_socket_t > m_sockets;
};

} /* namespace impl */

} /* namespace restinio */
