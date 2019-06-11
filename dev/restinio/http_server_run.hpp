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
template<typename Traits = default_single_thread_traits_t>
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
/*!
 * Can be useful when RESTinio server should be run on the user's
 * own io_context instance.
 *
 * For example:
 * \code
 * asio::io_context iosvc;
 * ... // iosvc used by user.
 * restinio::run(iosvc,
 * 		restinio::on_this_thread<my_traits>()
 * 			.port(8080)
 * 			.address("localhost")
 * 			.request_handler([](auto req) {...}));
 * \endcode
 *
 * \since
 * v.0.4.2
 */
template<typename Traits>
inline void
run(
	//! Asio's io_context to be used.
	//! Note: this reference should remain valid until RESTinio server finished.
	asio_ns::io_context & ioctx,
	//! Settings for that server instance.
	run_on_this_thread_settings_t<Traits> && settings )
{
	using settings_t = run_on_this_thread_settings_t<Traits>;
	using server_t = http_server_t<Traits>;

	server_t server{
		restinio::external_io_context( ioctx ),
		std::forward<settings_t>(settings) };

	asio_ns::signal_set break_signals{ server.io_context(), SIGINT };
	break_signals.async_wait(
		[&]( const asio_ns::error_code & ec, int ){
			if( !ec )
			{
				server.close_async(
					[&]{
						// Stop running io_service.
						ioctx.stop();
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

	ioctx.run();
}

//! Helper function for running http server until ctrl+c is hit.
/*!
 * This function creates its own instance of Asio's io_context and
 * uses it exclusively.
 *
 * Usage example:
 * \code
 * restinio::run(
 * 		restinio::on_this_thread<my_traits>()
 * 			.port(8080)
 * 			.address("localhost")
 * 			.request_handler([](auto req) {...}));
 * \endcode
 */
template<typename Traits>
inline void
run(
	run_on_this_thread_settings_t<Traits> && settings )
{
	asio_ns::io_context io_context;
	run( io_context, std::move(settings) );
}

namespace impl {

/*!
 * \brief An implementation of run-function for thread pool case.
 *
 * This function receives an already created thread pool object and
 * creates and runs http-server on this thread pool.
 *
 * \since
 * v.0.4.2
 */
template<typename Io_Context_Holder, typename Traits>
void
run(
	ioctx_on_thread_pool_t<Io_Context_Holder> & pool,
	run_on_thread_pool_settings_t<Traits> && settings )
{
	using settings_t = run_on_thread_pool_settings_t<Traits>;
	using server_t = http_server_t<Traits>;

	server_t server{
		restinio::external_io_context( pool.io_context() ),
		std::forward<settings_t>(settings) };

	asio_ns::signal_set break_signals{ server.io_context(), SIGINT };
	break_signals.async_wait(
		[&]( const asio_ns::error_code & ec, int ){
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

} /* namespace impl */

//! Helper function for running http server until ctrl+c is hit.
/*!
 * This function creates its own instance of Asio's io_context and
 * uses it exclusively.
 *
 * Usage example:
 * \code
 * restinio::run(
 * 		restinio::on_thread_pool<my_traits>(4)
 * 			.port(8080)
 * 			.address("localhost")
 * 			.request_handler([](auto req) {...}));
 * \endcode
 */
template<typename Traits>
inline void
run( run_on_thread_pool_settings_t<Traits> && settings )
{
	using thread_pool_t = impl::ioctx_on_thread_pool_t<
			impl::own_io_context_for_thread_pool_t >;

	thread_pool_t pool( settings.pool_size() );

	impl::run( pool, std::move(settings) );
}

//! Helper function for running http server until ctrl+c is hit.
/*!
 * Can be useful when RESTinio server should be run on the user's
 * own io_context instance.
 *
 * For example:
 * \code
 * asio::io_context iosvc;
 * ... // iosvc used by user.
 * restinio::run(iosvc,
 * 		restinio::on_thread_pool<my_traits>(4)
 * 			.port(8080)
 * 			.address("localhost")
 * 			.request_handler([](auto req) {...}));
 * \endcode
 *
 * \since
 * v.0.4.2
 */
template<typename Traits>
inline void
run(
	//! Asio's io_context to be used.
	//! Note: this reference should remain valid until RESTinio server finished.
	asio_ns::io_context & ioctx,
	//! Settings for that server instance.
	run_on_thread_pool_settings_t<Traits> && settings )
{
	using thread_pool_t = impl::ioctx_on_thread_pool_t<
			impl::external_io_context_for_thread_pool_t >;

	thread_pool_t pool{ settings.pool_size(), ioctx };

	impl::run( pool, std::move(settings) );
}

//FIXME: document this!
//
// run_existing_server_on_thread_pool_t
//
template<typename Traits>
class run_existing_server_on_thread_pool_t
{
	std::size_t m_pool_size;
	http_server_t<Traits> * m_server;

public:
	run_existing_server_on_thread_pool_t(
		std::size_t pool_size,
		http_server_t<Traits> & server )
		:	m_pool_size{ pool_size }
		,	m_server{ &server }
	{}

	std::size_t
	pool_size() const noexcept { return m_pool_size; }

	http_server_t<Traits> &
	server() const noexcept { return *m_server; }
};

//FIXME: document this!
template<typename Traits>
run_existing_server_on_thread_pool_t<Traits>
on_thread_pool(
	std::size_t pool_size,
	http_server_t<Traits> & server )
{
	return { pool_size, server };
}

namespace impl {

/*!
 * \brief An implementation of run-function for thread pool case
 * with existing http_server instance.
 *
 * This function receives an already created thread pool object and
 * already created http-server and run it on this thread pool.
 *
 * \since
 * v.0.5.1
 */
template<typename Io_Context_Holder, typename Traits>
void
run(
	ioctx_on_thread_pool_t<Io_Context_Holder> & pool,
	http_server_t<Traits> & server )
{
	asio_ns::signal_set break_signals{ server.io_context(), SIGINT };
	break_signals.async_wait(
		[&]( const asio_ns::error_code & ec, int ){
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

} /* namespace impl */

//FIXME: actualize docs!
//! Helper function for running http server until ctrl+c is hit.
/*!
 * This function creates its own instance of Asio's io_context and
 * uses it exclusively.
 *
 * Usage example:
 * \code
 * restinio::run(
 * 		restinio::on_thread_pool<my_traits>(4)
 * 			.port(8080)
 * 			.address("localhost")
 * 			.request_handler([](auto req) {...}));
 * \endcode
 */
template<typename Traits>
inline void
run( run_existing_server_on_thread_pool_t<Traits> && params )
{
	using thread_pool_t = impl::ioctx_on_thread_pool_t<
			impl::external_io_context_for_thread_pool_t >;

	thread_pool_t pool{ params.pool_size(), params.server().io_context() };

	impl::run( pool, params.server() );
}

} /* namespace restinio */

