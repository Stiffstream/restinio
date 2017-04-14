/*
	restinio
*/

/*!
	A base class for connection handle.
*/

#pragma once

#include <memory>

#include <restinio/common_types.hpp>

namespace restinio
{

//
// connection_base_t
//

class connection_base_t
	:	public std::enable_shared_from_this< connection_base_t >
{
	public:
		connection_base_t(std::uint64_t id )
			:	m_connection_id{ id }
		{}
		virtual ~connection_base_t() = default;

		//! Get connection id.
		std::uint64_t
		connection_id() const
		{
			return m_connection_id;
		}

		//! Write parts for specified request.
		virtual void
		write_response_parts(
			//! Request id.
			request_id_t request_id,
			//! Is these parts are final parts of response?
			response_output_flags_t is_final,
			//! parts of a response.
			std::vector< std::string > bufs ) = 0;

	private:
		//! Id of a connection.
		const std::uint64_t m_connection_id;
};

using connection_handle_t =
	std::shared_ptr< connection_base_t >;

} /* namespace restinio */
