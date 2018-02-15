#pragma once

#include <thread>

#include <restinio/asio_include.hpp>

#include <restinio/exception.hpp>

namespace restinio
{

namespace impl
{

/*
 * Helper class for creating io_context and running it
 * (via `io_context::run()`) on a thread pool.
 *
 * \note class is not thread-safe (except `io_context()` method).
 * Expected usage scenario is to start and stop it on the same thread.
 */
class ioctx_on_thread_pool_t
{
	public:
		ioctx_on_thread_pool_t( const ioctx_on_thread_pool_t & ) = delete;
		ioctx_on_thread_pool_t( ioctx_on_thread_pool_t && ) = delete;

		ioctx_on_thread_pool_t(
			// Pool size.
			//FIXME: better to use not_null from gsl.
			std::size_t pool_size )
			:	m_pool( pool_size )
			,	m_status( status_t::stopped )
		{}

		// Makes sure the pool is stopped.
		~ioctx_on_thread_pool_t()
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
				throw exception_t{
					"io_context_with_thread_pool is already started" };
			}

			try
			{
				for( auto & t : m_pool )
					t = std::thread( [this] {
						asio::executor_work_guard< asio::io_context::executor_type >
							work{ asio::make_work_guard( m_io_context ) };

						m_io_context.run();
					} );

				// When all thread started successfully
				// status can be changed.
				m_status = status_t::started;
			}
			catch( const std::exception & )
			{
				m_io_context.stop();
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
				m_io_context.stop();
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

		asio_ns::io_context &
		io_context() { return m_io_context; }

	private:
		enum class status_t : std::uint8_t { stopped, started };

		asio_ns::io_context m_io_context;
		std::vector< std::thread > m_pool;
		status_t m_status;
};

} /* namespace impl */

} /* namespace restinio */

