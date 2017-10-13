/*
 * restinio
 */

/*!
 * \file
 * \brief Helper function for simple run of HTTP server.
 */

#pragma once

#include <restinio/impl/ioctx_on_thread_pool.hpp>

#include <restinio/http_server.hpp>

namespace restinio
{

//
// run_on_this_thread_settings_t
//
/*!
 * \brief Settings for the case when http_server must be run
 * on the context of the current thread.
 *
 * \note
 * Shouldn't be used directly. Only as result of on_this_thread()
 * function as parameter for run().
 */
template<typename Traits>
class run_on_this_thread_settings_t final
	:	public basic_server_settings_t<
				run_on_this_thread_settings_t<Traits>,
				Traits>
{
	using base_type_t = basic_server_settings_t<
				run_on_this_thread_settings_t<Traits>, Traits>;
public:
	// Inherit constructors from base class.
	using base_type_t::base_type_t;
};

//
// on_this_thread
//
/*!
 * \brief A special marker for the case when http_server must be
 * run on the context of the current thread.
 *
 * Usage example:
 * \code
 * // Run with the default traits.
 * run( restinio::on_this_thread()
 * 	.port(8080)
 * 	.address("localhost")
 * 	.request_handler(...) );
 * \endcode
 * For a case when some custom traits must be used:
 * \code
 * run( restinio::on_this_thread<my_server_traits_t>()
 * 	.port(8080)
 * 	.address("localhost")
 * 	.request_handler(...) );
 * \endcode
 */
template<typename Traits = default_traits_t>
run_on_this_thread_settings_t<Traits>
on_this_thread() { return run_on_this_thread_settings_t<Traits>{}; }

//
// run_on_thread_pool_settings_t
//
/*!
 * \brief Settings for the case when http_server must be run
 * on the context of the current thread.
 *
 * \note
 * Shouldn't be used directly. Only as result of on_thread_pool()
 * function as parameter for run().
 */
template<typename Traits>
class run_on_thread_pool_settings_t final
	:	public basic_server_settings_t<
				run_on_thread_pool_settings_t<Traits>,
				Traits>
{
	//! Size of the pool.
	std::size_t m_pool_size;

public:
	//! Constructor.
	run_on_thread_pool_settings_t(
		//! Size of the pool.
		std::size_t pool_size )
		:	m_pool_size(pool_size)
	{}

	//! Get the pool size.
	std::size_t
	pool_size() const { return m_pool_size; }
};

//
// on_thread_pool
//
/*!
 * \brief A special marker for the case when http_server must be
 * run on an thread pool.
 *
 * Usage example:
 * \code
 * // Run with the default traits.
 * run( restinio::on_thread_pool(16) // 16 -- is the pool size.
 * 	.port(8080)
 * 	.address("localhost")
 * 	.request_handler(...) );
 * \endcode
 * For a case when some custom traits must be used:
 * \code
 * run( restinio::on_thread_pool<my_server_traits_t>(16)
 * 	.port(8080)
 * 	.address("localhost")
 * 	.request_handler(...) );
 * \endcode
 */
template<typename Traits = default_traits_t>
run_on_thread_pool_settings_t<Traits>
on_thread_pool(
	//! Size of the pool.
	std::size_t pool_size )
{
	return run_on_thread_pool_settings_t<Traits>( pool_size );
}

//
// run()
//

//! Helper function for running http server until ctrl+c is hit.
template<typename Traits>
inline void
run(
	run_on_this_thread_settings_t<Traits> && settings )
{
	using settings_t = run_on_this_thread_settings_t<Traits>;
	using server_t = http_server_t<Traits>;

	// Use current thread to run.
	asio::io_context io_context;

	server_t server{
		restinio::external_io_context( io_context ),
		std::forward<settings_t>(settings) };

	asio::signal_set break_signals{ server.io_context(), SIGINT };
	break_signals.async_wait(
		[&]( const asio::error_code & ec, int ){
			if( !ec )
			{
				server.close_async(
					[&]{
						// Stop running io_service.
						io_context.stop();
					},
					[]( std::exception_ptr ex ){
						std::rethrow_exception( ex );
					} );
			}
		} );

	server.open_async(
		[]{ /* Ok. */},
		[]( std::exception_ptr ex ){
			std::rethrow_exception( ex );
		} );

	io_context.run();
}

//! Helper function for running http server until ctrl+c is hit.
template<typename Traits>
inline void
run( run_on_thread_pool_settings_t<Traits> && settings )
{
	using settings_t = run_on_thread_pool_settings_t<Traits>;
	using server_t = http_server_t<Traits>;

	impl::ioctx_on_thread_pool_t pool( settings.pool_size() );

	server_t server{
		restinio::external_io_context( pool.io_context() ),
		std::forward<settings_t>(settings) };

	asio::signal_set break_signals{ server.io_context(), SIGINT };
	break_signals.async_wait(
		[&]( const asio::error_code & ec, int ){
			if( !ec )
			{
				server.close_async(
					[&]{
						// Stop running io_service.
						pool.stop();
					},
					[]( std::exception_ptr ex ){
						std::rethrow_exception( ex );
					} );
			}
		} );

	server.open_async(
		[]{ /* Ok. */},
		[]( std::exception_ptr ex ){
			std::rethrow_exception( ex );
		} );

	pool.start();
	pool.wait();
}

} /* namespace restinio */

