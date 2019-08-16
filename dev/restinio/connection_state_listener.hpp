/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to connection state listeners.
 *
 * @since v.0.5.1
 */

#pragma once

#include <restinio/common_types.hpp>
#include <restinio/variant.hpp>
#include <restinio/tls_fwd.hpp>

namespace restinio
{

namespace connection_state
{

//FIXME: document this!
class common_notice_info_t
{
	connection_id_t m_conn_id;
	endpoint_t m_remote_endpoint;

public:
	common_notice_info_t(
		connection_id_t conn_id,
		endpoint_t remote_endpoint )
		:	m_conn_id{ conn_id }
		,	m_remote_endpoint{ remote_endpoint }
	{}

	//! Get the connection id.
	connection_id_t
	connection_id() const noexcept { return m_conn_id; }

	//! Get the remote endpoint for the connection.
	endpoint_t
	remote_endpoint() const noexcept { return m_remote_endpoint; }
};

//FIXME: document this!
class accepted_t final : public common_notice_info_t
{
	/*!
	 * \brief An optional pointer to TLS-related connection.
	 *
	 * Will be nullptr for non-TLS connections.
	 *
	 * \since
	 * v.0.6.0
	 */
	tls_socket_t * m_tls_socket;

public:
	accepted_t(
		connection_id_t conn_id,
		endpoint_t remote_endpoint,
		tls_socket_t * tls_socket )
		:	common_notice_info_t{ conn_id, remote_endpoint }
		,	m_tls_socket{ tls_socket }
	{}

//FIXME: document this!
	template< typename Lambda >
	void
	try_inspect_tls( Lambda && lambda ) const;

//FIXME: document this!
	template< typename Lambda >
	decltype(auto)
	inspect_tls_or_throw( Lambda && lambda ) const;

//FIXME: document this!
	template< typename Lambda, typename T >
	T
	inspect_tls_or_default( Lambda && lambda, T && default_value ) const;
};

//FIXME: document this!
class closed_t final : public common_notice_info_t
{
public:
	using common_notice_info_t::common_notice_info_t;
};

//FIXME: document this!
class upgraded_to_websocket_t final : public common_notice_info_t
{
public:
	using common_notice_info_t::common_notice_info_t;
};

//FIXME: document this!
using notice_t = variant_t<accepted_t, closed_t, upgraded_to_websocket_t>;

/*!
 * @brief The default no-op state listener.
 *
 * This type is used for connection_state_listener_t trait by default.
 * 
 * NOTE. When this type if used no calls to state listener will be generated.
 * It means that there won't be any performance penalties related to
 * invoking of state listener's state_changed() method.
 *
 * @since v.0.5.1
 */
struct noop_listener_t
{
	// empty type by design.
};

} /* namespace connection_state */

} /* namespace restinio */

