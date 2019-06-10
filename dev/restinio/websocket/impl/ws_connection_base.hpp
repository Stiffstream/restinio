/*
	restinio
*/

/*!
	A base class for connection handle.
*/

#pragma once

#include <memory>

#include <restinio/tcp_connection_ctx_base.hpp>
#include <restinio/common_types.hpp>
#include <restinio/buffers.hpp>

namespace restinio
{

namespace websocket
{

namespace basic
{

class ws_t;
using ws_handle_t = std::shared_ptr< ws_t >;

namespace impl
{

//
// ws_connection_base_t
//

//! WebSocket connection base.
class ws_connection_base_t
	:	public tcp_connection_ctx_base_t
{
	public:
		ws_connection_base_t( connection_id_t id )
			:	tcp_connection_ctx_base_t{ id }
		{}

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
			write_group_t wg,
			bool is_close_frame ) = 0;
};

//! Alias for WebSocket connection handle.
using ws_connection_handle_t = std::shared_ptr< ws_connection_base_t >;

} /* namespace impl */

} /* namespace basic */

} /* namespace websocket */

} /* namespace restinio */
