#pragma once

#include <thread>

#include <restinio/asio_include.hpp>

#include <restinio/exception.hpp>

namespace restinio
{

namespace impl
{

/*!
 * \brief A class for holding actual instance of Asio's io_context.
 *
 * \note
 * This class is intended to be used as template argument for
 * ioctx_on_thread_pool_t template.
 *
 * \since
 * v.0.4.2
 */
class own_io_context_for_thread_pool_t
{
	asio_ns::io_context m_ioctx;

public:
	own_io_context_for_thread_pool_t() = default;

	//! Get access to io_context object.
	auto & io_context() noexcept { return m_ioctx; }
};

/*!
 * \brief A class for holding a reference to external Asio's io_context.
 *
 * \note
 * This class is intended to be used as template argument for
 * ioctx_on_thread_pool_t template.
 *
 * \since
 * v.0.4.2
 */
class external_io_context_for_thread_pool_t
{
	asio_ns::io_context & m_ioctx;

public:
	//! Initializing constructor.
	external_io_context_for_thread_pool_t(
		//! External io_context to be used.
		asio_ns::io_context & ioctx )
		:	m_ioctx{ ioctx }
	{}

	//! Get access to io_context object.
	auto & io_context() noexcept { return m_ioctx; }
};

/*!
 * Helper class for creating io_context and running it
 * (via `io_context::run()`) on a thread pool.
 *
 * \note class is not thread-safe (except `io_context()` method).
 * Expected usage scenario is to start and stop it on the same thread.
 *
 * \tparam Io_Context_Holder A type which actually holds io_context object
 * or a reference to an external io_context object.
 */
template< typename Io_Context_Holder >
class ioctx_on_thread_pool_t
{
	public:
		ioctx_on_thread_pool_t( const ioctx_on_thread_pool_t & ) = delete;
		ioctx_on_thread_pool_t( ioctx_on_thread_pool_t && ) = delete;

		template< typename... Io_Context_Holder_Ctor_Args >
		ioctx_on_thread_pool_t(
			// Pool size.
			//FIXME: better to use not_null from gsl.
			std::size_t pool_size,
			// Optional arguments for Io_Context_Holder instance.
			Io_Context_Holder_Ctor_Args && ...ioctx_holder_args )
			:	m_ioctx_holder{
					std::forward<Io_Context_Holder_Ctor_Args>(ioctx_holder_args)... }
			,	m_pool( pool_size )
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
				std::generate(
					begin( m_pool ),
					end( m_pool ),
					[this]{
						return
							std::thread{ [this] {
								auto work{ asio_ns::make_work_guard(
											m_ioctx_holder.io_context() ) };

								m_ioctx_holder.io_context().run();
							} };
					} );

				// When all thread started successfully
				// status can be changed.
				m_status = status_t::started;
			}
			catch( const std::exception & )
			{
				io_context().stop();
				for( auto & t : m_pool )
					if( t.joinable() )
						t.join();

				throw;
			}
		}

		// NOTE: this method is marked as noexcept in v.0.6.7.
		// It's because this method can be called from destructors and
		// there is no way to recover from an exception thrown from
		// this method.
		void
		stop() noexcept
		{
			if( started() )
			{
				io_context().stop();
			}
		}

		// NOTE: this method is marked as noexcept in v.0.6.7.
		// It's because this method can be called from destructors and
		// there is no way to recover from an exception thrown from
		// this method.
		void
		wait() noexcept
		{
			if( started() )
			{
				for( auto & t : m_pool )
					t.join();

				// When all threads are stopped status can be changed.
				m_status = status_t::stopped;
			}
		}

		bool started() const noexcept { return status_t::started == m_status; }

		asio_ns::io_context &
		io_context() noexcept
		{
			return m_ioctx_holder.io_context();
		}

	private:
		enum class status_t : std::uint8_t { stopped, started };

		Io_Context_Holder m_ioctx_holder;
		std::vector< std::thread > m_pool;
		status_t m_status;
};

} /* namespace impl */

} /* namespace restinio */

