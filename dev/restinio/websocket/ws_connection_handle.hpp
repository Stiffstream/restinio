/*
	restinio
*/

/*!
	A base class for connection handle.
*/

#pragma once

#include <memory>

#include <restinio/common_types.hpp>
#include <restinio/buffers.hpp>

namespace restinio
{

namespace websocket
{

class ws_t;
using ws_handle_t = std::shared_ptr< ws_t >;

//
// ws_connection_base_t
//

//! WebSocket connection base.
class ws_connection_base_t
	:	public std::enable_shared_from_this< ws_connection_base_t >
{
	public:
		ws_connection_base_t(std::uint64_t id )
			:	m_connection_id{ id }
		{}
		virtual ~ws_connection_base_t() = default;

		//! Get connection id.
		std::uint64_t
		connection_id() const
		{
			return m_connection_id;
		}

		//! Shutdown websocket.
		virtual void
		shutdown() = 0;

		//! Kill websocket.
		virtual void
		kill() = 0;

		//! Start reading ws-messages.
		virtual void
		init_read( ws_handle_t wsh ) = 0;

		//! Write pieces of outgoing data.
		virtual void
		write_data(
			buffers_container_t bufs,
			bool is_close_frame ) = 0;

	private:
		//! Id of a connection.
		const std::uint64_t m_connection_id;
};

//! Alias for WebSocket connection handle.
using ws_connection_handle_t =
	std::shared_ptr< ws_connection_base_t >;

} /* namespace websocket */

} /* namespace restinio */
