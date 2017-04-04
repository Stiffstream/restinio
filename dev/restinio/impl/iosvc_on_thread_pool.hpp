#pragma once

#include <thread>

#include <asio.hpp>

namespace restinio
{

namespace impl
{

/*
 * Helper class for creating asio::io_service and running it
 * (via `io_service::run()`) on a thread pool.
 *
 * \note class is not thread-safe (except `io_service()` methon).
 * Expected usage scenario is to start and stop it on the same thread.
 */
class iosvc_on_thread_pool_t
{
	public:
		iosvc_on_thread_pool_t( const iosvc_on_thread_pool_t & ) = delete;
		iosvc_on_thread_pool_t( iosvc_on_thread_pool_t && ) = delete;

		iosvc_on_thread_pool_t(
			// Pool size.
			//FIXME: better to use not_null from gsl.
			std::size_t pool_size )
			:	m_pool( pool_size )
			,	m_status( status_t::stopped )
		{}

		// Makes sure the pool is stopped.
		~iosvc_on_thread_pool_t()
		{
			if( started() )
			{
				stop();
				wait();
			}
		}

		void
		start()
		{
			if( started() )
			{
				throw std::runtime_error{
					"io_service_with_thread_pool is already started" };
			}

			try
			{
				for( auto & t : m_pool )
					t = std::thread( [this] {
						asio::io_service::work work( m_io_service );
						m_io_service.run();
					} );

				// When all thread started successfully
				// status can be changed.
				m_status = status_t::started;
			}
			catch( const std::exception & )
			{
				m_io_service.stop();
				for( auto & t : m_pool )
					if( t.joinable() )
						t.join();
			}
		}

		void
		stop()
		{
			if( started() )
			{
				m_io_service.stop();
			}
		}

		void
		wait()
		{
			if( started() )
			{
				for( auto & t : m_pool )
					t.join();

				// When all threads are stopped status can be changed.
				m_status = status_t::stopped;
			}
		}

		bool
		started() const { return status_t::started == m_status; }

		asio::io_service &
		io_service() { return m_io_service; }

	private:
		enum class status_t { stopped, started };

		asio::io_service m_io_service;
		std::vector< std::thread > m_pool;
		status_t m_status;
};

} /* namespace impl */

} /* namespace restinio */

