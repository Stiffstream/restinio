/*
	restinio
*/

/*!
	ioservice wrappers for asio.
*/

#pragma once

#include <asio.hpp>

#include <restinio/impl/ioctx_on_thread_pool.hpp>

namespace restinio
{

//
// io_context_wrapper_t
//

//! Wrapper for handling asio io_context loop start/stop routine.
class io_context_wrapper_t
{
	public:
		virtual ~io_context_wrapper_t() {}

		virtual void
		start() = 0;

		virtual void
		stop() = 0;

		virtual asio::io_context &
		io_context() = 0;
};

using io_context_wrapper_unique_ptr_t =
	std::unique_ptr< io_context_wrapper_t >;

//
// proxy_io_context_wrapper_t
//

//! Implementation of io_context_wrapper for external io_context object.
class proxy_io_context_wrapper_t
	:	public io_context_wrapper_t
{
	public:
		proxy_io_context_wrapper_t( asio::io_context & io_context )
			:	m_io_context{ io_context }
		{}

		virtual void
		start() override
		{}

		virtual void
		stop() override
		{}

		virtual asio::io_context &
		io_context() override
		{
			return m_io_context;
		}

	private:
		asio::io_context & m_io_context;
};

//
// use_existing_io_context()
//

//! Create io_context wrapper for external io_context object.
inline io_context_wrapper_unique_ptr_t
use_existing_io_context( asio::io_context & io_context )
{
	return std::make_unique< proxy_io_context_wrapper_t >( io_context );
}

//
// child_io_context_wrapper_t
//

//! Implementation of io_context_wrapper for child io_context object.
class child_io_context_wrapper_t
	:	public io_context_wrapper_t
{
	public:
		child_io_context_wrapper_t(
			std::size_t thread_pool_size )
			:	m_ioctx{ thread_pool_size }
		{}

		virtual void
		start() override
		{
			m_ioctx.start();
		}

		virtual void
		stop() override
		{
			m_ioctx.stop();
			m_ioctx.wait();
		}

		virtual asio::io_context &
		io_context() override
		{
			return m_ioctx.io_context();
		}

	private:
		impl::ioctx_on_thread_pool_t m_ioctx;
};


//
// create_child_io_context()
//

//! Create io_context wrapper for external io_context object.
/*!
	A separate ioservice object is created and runs on a thread pool.
*/
inline io_context_wrapper_unique_ptr_t
create_child_io_context( std::size_t thread_pool_size )
{
	return std::make_unique< child_io_context_wrapper_t >( thread_pool_size );
}

} /* namespace restinio */
