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
		typename TIMER_FACTORY,
		typename LOGGER,
		typename REQUEST_HANDLER,
		typename STRAND >
using tls_traits_t = traits_t< TIMER_FACTORY, LOGGER, REQUEST_HANDLER, STRAND, tls_socket_t >;

//
// single_thread_traits_t
//

template <
		typename TIMER_FACTORY,
		typename LOGGER,
		typename REQUEST_HANDLER = default_request_handler_t >
using single_thread_tls_traits_t =
	tls_traits_t< TIMER_FACTORY, LOGGER, REQUEST_HANDLER, noop_strand_t >;

//
// prepare_connection_and_start_read()
//

//! Customizes connection init routine with an additional step:
//! perform handshake and only then start reading.
template < typename CONNECTION, typename START_READ_CB, typename FAILED_CB >
void
prepare_connection_and_start_read(
	tls_socket_t & socket,
	CONNECTION & con,
	START_READ_CB start_read_cb,
	FAILED_CB failed_cb )
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
template < typename SETTINGS >
class extra_settings_t< SETTINGS, tls_socket_t >
{
	public:
		virtual ~extra_settings_t() = default;

		extra_settings_t() = default;
		extra_settings_t( extra_settings_t && ) = default;

		SETTINGS &
		tls_context(
			asio::ssl::context context ) &
		{
			m_tls_context = std::move( context );
			return upcast_reference();
		}

		SETTINGS &&
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
		SETTINGS &
		upcast_reference()
		{
			return static_cast< SETTINGS & >( *this );
		}

		asio::ssl::context m_tls_context{ asio::ssl::context::sslv23 };
};

namespace impl
{

//
// socket_holder_t
//

//! A custom socket storage for tls_socket_t.
template <>
class socket_holder_t< tls_socket_t >
{
	protected:
		template < typename SETTINGS >
		socket_holder_t(
			SETTINGS & settings,
			asio::io_service & io_service )
			:	m_tls_context{ settings.tls_context() }
			,	m_io_service{ io_service }
			,	m_socket{ m_io_service, m_tls_context }
		{}

		virtual ~socket_holder_t() = default;

		tls_socket_t &
		socket()
		{
			return m_socket;
		}

		auto
		move_socket()
		{
			tls_socket_t res{ m_io_service, m_tls_context };
			std::swap( res, m_socket );
			return res;
		}

	private:
		asio::ssl::context m_tls_context;
		asio::io_service & m_io_service;
		tls_socket_t m_socket;
};

} /* namespace impl */

} /* namespace restinio */
