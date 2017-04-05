/*
	restinio
*/

/*!
	ioservice wrappers for asio.
*/

#pragma once

#include <asio.hpp>

#include <restinio/impl/iosvc_on_thread_pool.hpp>

namespace restinio
{

//
// io_service_wrapper_t
//

//! Wrapper for handling asio io_service loop start/stop routine.
class io_service_wrapper_t
{
	public:
		virtual ~io_service_wrapper_t() {}

		virtual void
		start() = 0;

		virtual void
		stop() = 0;

		virtual asio::io_service &
		io_service() = 0;
};

using io_service_wrapper_unique_ptr_t =
	std::unique_ptr< io_service_wrapper_t >;

//
// proxy_io_service_wrapper_t
//

//! Implementation of io_service_wrapper for external io_service object.
class proxy_io_service_wrapper_t
	:	public io_service_wrapper_t
{
	public:
		proxy_io_service_wrapper_t( asio::io_service & io_service )
			:	m_io_service{ io_service }
		{}

		virtual void
		start() override
		{}

		virtual void
		stop() override
		{}

		virtual asio::io_service &
		io_service() override
		{
			return m_io_service;
		}

	private:
		asio::io_service & m_io_service;
};

//
// create_proxy_io_service()
//

//! Create io_service wrapper for external io_service object.
inline io_service_wrapper_unique_ptr_t
create_proxy_io_service( asio::io_service & io_service )
{
	return std::make_unique< proxy_io_service_wrapper_t >( io_service );
}

//
// child_io_service_wrapper_t
//

//! Implementation of io_service_wrapper for child io_service object.
class child_io_service_wrapper_t
	:	public io_service_wrapper_t
{
	public:
		child_io_service_wrapper_t(
			std::size_t thread_pool_size )
			:	m_iosvc{ thread_pool_size }
		{}

		virtual void
		start() override
		{
			m_iosvc.start();
		}

		virtual void
		stop() override
		{
			m_iosvc.stop();
			m_iosvc.wait();
		}

		virtual asio::io_service &
		io_service() override
		{
			return m_iosvc.io_service();
		}

	private:
		impl::iosvc_on_thread_pool_t m_iosvc;
};


//
// create_proxy_io_service_wrapper()
//

//! Create io_service wrapper for external io_service object.
/*!
	A separate ioservice object is created and runs on a thread pool.
*/
inline io_service_wrapper_unique_ptr_t
create_child_io_service( std::size_t thread_pool_size )
{
	return std::make_unique< child_io_service_wrapper_t >( thread_pool_size );
}

} /* namespace restinio */
