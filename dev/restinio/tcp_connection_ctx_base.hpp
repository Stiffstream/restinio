/*
	restinio
*/

/*!
	A base class for all classes that deal with connection context.
*/

#pragma once

#include <memory>

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
		tcp_connection_ctx_base_t(std::uint64_t id )
			:	m_connection_id{ id }
		{}

		virtual ~tcp_connection_ctx_base_t() = default;

		//! Get connection id.
		std::uint64_t
		connection_id() const
		{
			return m_connection_id;
		}

		//! Check timeouts for all activities.
		virtual void
		check_timeout() = 0;

	protected:
		template < typename Derived >
		std::shared_ptr< Derived >
		shared_from_concrete()
		{
			return std::static_pointer_cast< Derived >( shared_from_this() );
		}

	private:
		//! Id of a connection.
		const std::uint64_t m_connection_id;
};

//! Alias for http connection handle.
using tcp_connection_ctx_handle_t = std::shared_ptr< tcp_connection_ctx_base_t >;
using tcp_connection_ctx_weak_handle_t = std::weak_ptr< tcp_connection_ctx_base_t >;

} /* namespace restinio */

