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

//FIXME: document this!
using connection_id_t = std::uint64_t;

//! An alias for endpoint type from Asio.
using endpoint_t = asio_ns::ip::tcp::endpoint;

namespace connection_state
{

//FIXME: document this!
enum class cause_t
{
	accepted,
	closed,
	upgraded_to_websocket
};

//FIXME: document this!
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

//FIXME: document this!
struct noop_listener_t
{
	// empty type by design.
};

} /* namespace connection_state */

} /* namespace restinio */

