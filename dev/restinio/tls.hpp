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

template < typename CONNECTION, typename START_READ_CB, typename FAILED_CB >
void
prepare_connection_and_start_read(
	asio::ssl::stream< asio::ip::tcp::socket > & socket,
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

} /* namespace restinio */
