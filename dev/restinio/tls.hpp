/*
	restinio
*/

/*!
	Support for https.
*/
#pragma once

#include <asio/ssl.hpp>

namespace restinio
{

using tls_socket_t = asio::ssl::stream< asio::ip::tcp::socket >;

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
		[ after_init_cb = std::move( after_init_cb ),
			failed_cb = std::move( failed_cb ),
			con = con.shared_from_this() ]( const asio::error_code & ec ){
			if( !ec )
				after_init_cb();
			else
				failed_cb( ec );
		} );
}

//
// extra_settings_t
//

//! Extra settings needed for working with socket.
template < typename SETTINGS >
class extra_settings_t< SETTINGS, tls_socket_t >
{
	public:
		virtual ~extra_settings_t() = default;

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

	private:
		SETTINGS &
		upcast_reference()
		{
			return static_cast< SETTINGS & >( *this );
		}

		asio::ssl::context m_tls_context;
};


} /* namespace restinio */
