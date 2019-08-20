/*
 * RESTinio
 */

/*!
 * @file
 * @brief Forward declarations for TLS-related things.
 *
 * @since v.0.5.2
 */

#pragma once

namespace restinio
{

namespace impl
{

class tls_socket_t;

// Just a forward declaration.
tls_socket_t *
make_tls_socket_pointer_for_state_listener(
	tls_socket_t & socket ) noexcept;

} /* namespace impl */

//! A public alias for the actual implementation of TLS-socket.
using tls_socket_t = impl::tls_socket_t;

} /* namespace restinio */

