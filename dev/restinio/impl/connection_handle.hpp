/*
	restinio
*/

/*!
	A base class for connection handle.
*/

#pragma once

#include <memory>

#include <restinio/tcp_connection_ctx_base.hpp>
#include <restinio/buffers.hpp>

namespace restinio
{

namespace impl
{

//
// connection_base_t
//

//! HTTP connection base.
class connection_base_t
	:	public tcp_connection_ctx_base_t
{
	public:
		connection_base_t(std::uint64_t id )
			:	tcp_connection_ctx_base_t{ id }
		{}
		virtual ~connection_base_t() = default;

		//! Write parts for specified request.
		virtual void
		write_response_parts(
			//! Request id.
			request_id_t request_id,
			//! Resp output flag.
			response_output_flags_t response_output_flags,
			//! parts of a response.
			buffers_container_t bufs ) = 0;
};

//! Alias for http connection handle.
using connection_handle_t = std::shared_ptr< connection_base_t >;

} /* namespace impl */

} /* namespace restinio */
