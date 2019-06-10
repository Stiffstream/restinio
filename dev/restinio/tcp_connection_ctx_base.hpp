/*
	restinio
*/

/*!
	A base class for all classes that deal with connection context.
*/

#pragma once

#include <memory>

#include <restinio/connection_state_listener.hpp>

namespace restinio
{

//
// tcp_connection_ctx_base_t
//

//! TCP connection base.
/*!
	Class serves as a root class for all connection context wrappers,
	that can be passed in asio callbacks.
*/
class tcp_connection_ctx_base_t
	:	public std::enable_shared_from_this< tcp_connection_ctx_base_t >
{
	public:
		tcp_connection_ctx_base_t(connection_id_t id )
			:	m_connection_id{ id }
		{}

		virtual ~tcp_connection_ctx_base_t() = default;

		//! Get connection id.
		connection_id_t connection_id() const noexcept { return m_connection_id; }

		//! Check timeouts for all activities.
		virtual void
		check_timeout(
			//! A handle to itself (eliminates one shared_ptr instantiation).
			std::shared_ptr< tcp_connection_ctx_base_t > & self ) = 0;

	protected:

		//! Cast self to derived class.
		template < typename Derived >
		std::shared_ptr< Derived >
		shared_from_concrete()
		{
			return std::static_pointer_cast< Derived >( shared_from_this() );
		}

	private:
		//! Id of a connection.
		const connection_id_t m_connection_id;
};

//! Alias for http connection handle.
using tcp_connection_ctx_handle_t = std::shared_ptr< tcp_connection_ctx_base_t >;

//! Alias for http connection weak handle.
using tcp_connection_ctx_weak_handle_t = std::weak_ptr< tcp_connection_ctx_base_t >;

} /* namespace restinio */

