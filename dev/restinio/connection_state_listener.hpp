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

#include <cstdint>

#include <restinio/asio_include.hpp>

namespace restinio
{

/*!
 * @brief Type for ID of connection.
 */
using connection_id_t = std::uint64_t;

//! An alias for endpoint type from Asio.
using endpoint_t = asio_ns::ip::tcp::endpoint;

namespace connection_state
{

/*!
 * @brief Enumeration for available causes for invocation of state listener.
 *
 * @since v.0.5.1
 */
enum class cause_t
{
	//! Connection from a client has been accepted.
	accepted,
	//! Connection from a client has been closed.
	//! Connection can be closed as result of an error or as a normal
	//! finishing of request handling.
	closed,
	//! Connection has been upgraded to WebSocket.
	//! State listener won't be invoked for that connection anymore.
	upgraded_to_websocket
};

/*!
 * @brief An object with info about connection to be passed to state listener.
 *
 * That object contains available information of the connection for that
 * state listener is called.
 *
 * NOTE. Content of this type can be changed in future versions of RESTinio.
 *
 * @since v.0.5.1
 */
class notice_t
{
	connection_id_t m_conn_id;
	endpoint_t m_remote_endpoint;
	cause_t m_cause;

public :
	//! Initializing constructor.
	notice_t(
		connection_id_t conn_id,
		endpoint_t remote_endpoint,
		cause_t cause )
		:	m_conn_id{ conn_id }
		,	m_remote_endpoint{ remote_endpoint }
		,	m_cause{ cause }
	{}

	//! Get the connection id.
	connection_id_t
	connection_id() const noexcept { return m_conn_id; }

	//! Get the remote endpoint for the connection.
	endpoint_t
	remote_endpoint() const noexcept { return m_remote_endpoint; }

	//! Get the cause for the notification.
	cause_t
	cause() const noexcept { return m_cause; }
};

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

