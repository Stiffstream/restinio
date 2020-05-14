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
// break_signal_handling_t
//
/*!
 * @brief Indication of usage of break signal handlers for some forms
 * of run functions.
 *
 * @since v.0.5.1
 */
enum class break_signal_handling_t
{
	//! Signal handler should be used by run() function.
	used,
	//! Signal handler should not be used by run() function.
	skipped
};

/*!
 * @brief Make the indicator for usage of break signal handler.
 *
 * Usage example:
 * @code
 * restinio::run( restinio::on_thread_pool(
 * 		std::thread::hardware_concurrency(),
 * 		restinio::use_break_signal_handling(),
 * 		my_server) );
 * @endcode
 *
 * @since v.0.5.1
 */
inline constexpr break_signal_handling_t
use_break_signal_handling() noexcept
{
	return break_signal_handling_t::used;
}

/*!
 * @brief Make the indicator for absence of break signal handler.
 *
 * Usage example:
 * @code
 * restinio::run( restinio::on_thread_pool(
 * 		std::thread::hardware_concurrency(),
 * 		restinio::skip_break_signal_handling(),
 * 		my_server) );
 * @endcode
 *
 * @since v.0.5.1
 */
inline constexpr break_signal_handling_t
skip_break_signal_handling() noexcept
{
	return break_signal_handling_t::skipped;
}

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

	std::exception_ptr exception_caught;

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
					[&exception_caught]( std::exception_ptr ex ){
						// We can't throw an exception here!
						// Store it to rethrow later.
						exception_caught = ex;
					} );
			}
		} );

	server.open_async(
		[]{ /* Ok. */},
		[&ioctx, &exception_caught]( std::exception_ptr ex ){
			// Stop running io_service.
			// We can't throw an exception here!
			// Store it to rethrow later.
			ioctx.stop();
			exception_caught = ex;
		} );

	ioctx.run();

	// If an error was detected it should be propagated.
	if( exception_caught )
		std::rethrow_exception( exception_caught );
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

	std::exception_ptr exception_caught;

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
					[&exception_caught]( std::exception_ptr ex ){
						// We can't throw an exception here!
						// Store it to rethrow later.
						exception_caught = ex;
					} );
			}
		} );

	server.open_async(
		[]{ /* Ok. */},
		[&pool, &exception_caught]( std::exception_ptr ex ){
			// Stop running io_service.
			// We can't throw an exception here!
			// Store it to rethrow later.
			pool.stop();
			exception_caught = ex;
		} );

	pool.start();
	pool.wait();

	// If an error was detected it should be propagated.
	if( exception_caught )
		std::rethrow_exception( exception_caught );
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

//
// run_existing_server_on_thread_pool_t
//
/*!
 * @brief Helper type for holding parameters necessary for running
 * HTTP-server on a thread pool.
 *
 * @note This class is not intended for direct use. It is used by
 * RESTinio itself.
 *
 * @since v.0.5.1
 */
template<typename Traits>
class run_existing_server_on_thread_pool_t
{
	//! Size of thread pool.
	std::size_t m_pool_size;
	//! Should break signal handler be used?
	break_signal_handling_t m_break_handling;
	//! HTTP-server to be used on a thread pool.
	/*!
	 * We assume that this pointer will be valid pointer.
	 */
	http_server_t<Traits> * m_server;

public:
	//! Initializing constructor.
	run_existing_server_on_thread_pool_t(
		//! Size of the pool.
		std::size_t pool_size,
		//! Should break signal handler be used?
		break_signal_handling_t break_handling,
		//! A reference to HTTP-server to be run on a thread pool.
		//! This reference should outlive an instance of
		//! run_existing_server_on_thread_pool_t.
		http_server_t<Traits> & server )
		:	m_pool_size{ pool_size }
		,	m_break_handling{ break_handling }
		,	m_server{ &server }
	{}

	std::size_t
	pool_size() const noexcept { return m_pool_size; }

	break_signal_handling_t
	break_handling() const noexcept { return m_break_handling; }

	http_server_t<Traits> &
	server() const noexcept { return *m_server; }
};

/*!
 * @brief Helper function for running an existing HTTP-server on
 * a thread pool.
 *
 * Usage example:
 * @code
 * using my_server_t = restinio::http_server_t< my_server_traits_t >;
 * my_server_t server{
 * 	restinio::own_io_context(),
 * 	[](auto & settings) {
 * 		settings.port(...);
 * 		settings.address(...);
 * 		settings.request_handler(...);
 * 		...
 * 	}
 * };
 * ...
 * restinio::run( restinio::on_thread_pool(
 * 	std::thread::hardware_concurrency(),
 * 	restinio::use_break_signal_handling(),
 * 	server) );
 * @endcode
 *
 * @since v.0.5.1
 */
template<typename Traits>
run_existing_server_on_thread_pool_t<Traits>
on_thread_pool(
	std::size_t pool_size,
	break_signal_handling_t break_handling,
	http_server_t<Traits> & server )
{
	return { pool_size, break_handling, server };
}

namespace impl {

/*!
 * \brief An implementation of run-function for thread pool case
 * with existing http_server instance.
 *
 * This function receives an already created thread pool object and
 * already created http-server and run it on this thread pool.
 *
 * \attention
 * This function installs break signal handler and stops server when
 * break signal is raised.
 *
 * \since
 * v.0.5.1
 */
template<typename Io_Context_Holder, typename Traits>
void
run_with_break_signal_handling(
	ioctx_on_thread_pool_t<Io_Context_Holder> & pool,
	http_server_t<Traits> & server )
{
	std::exception_ptr exception_caught;

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
					[&exception_caught]( std::exception_ptr ex ){
						// We can't throw an exception here!
						// Store it to rethrow later.
						exception_caught = ex;
					} );
			}
		} );

	server.open_async(
		[]{ /* Ok. */},
		[&pool, &exception_caught]( std::exception_ptr ex ){
			// Stop running io_service.
			// We can't throw an exception here!
			// Store it to rethrow later.
			pool.stop();
			exception_caught = ex;
		} );

	pool.start();
	pool.wait();

	// If an error was detected it should be propagated.
	if( exception_caught )
		std::rethrow_exception( exception_caught );
}

/*!
 * \brief An implementation of run-function for thread pool case
 * with existing http_server instance.
 *
 * This function receives an already created thread pool object and
 * already created http-server and run it on this thread pool.
 *
 * \note
 * This function doesn't install break signal handlers.
 *
 * \since
 * v.0.5.1
 */
template<typename Io_Context_Holder, typename Traits>
void
run_without_break_signal_handling(
	ioctx_on_thread_pool_t<Io_Context_Holder> & pool,
	http_server_t<Traits> & server )
{
	std::exception_ptr exception_caught;

	server.open_async(
		[]{ /* Ok. */},
		[&pool, &exception_caught]( std::exception_ptr ex ){
			// Stop running io_service.
			// We can't throw an exception here!
			// Store it to rethrow later.
			pool.stop();
			exception_caught = ex;
		} );

	pool.start();
	pool.wait();

	// If an error was detected it should be propagated.
	if( exception_caught )
		std::rethrow_exception( exception_caught );
}

} /* namespace impl */

/*!
 * @brief Helper function for running an existing HTTP-server on
 * a thread pool.
 *
 * Usage example:
 * @code
 * using my_server_t = restinio::http_server_t< my_server_traits_t >;
 * my_server_t server{
 * 	restinio::own_io_context(),
 * 	[](auto & settings) {
 * 		settings.port(...);
 * 		settings.address(...);
 * 		settings.request_handler(...);
 * 		...
 * 	}
 * };
 * ...
 * // run() returns if Ctrl+C is pressed or if HTTP-server will
 * // be shut down from elsewhere.
 * restinio::run( restinio::on_thread_pool(
 * 	std::thread::hardware_concurrency(),
 * 	restinio::use_break_signal_handling(),
 * 	server) );
 * @endcode
 *
 * @since v.0.5.1
 */
template<typename Traits>
inline void
run( run_existing_server_on_thread_pool_t<Traits> && params )
{
	using thread_pool_t = impl::ioctx_on_thread_pool_t<
			impl::external_io_context_for_thread_pool_t >;

	thread_pool_t pool{ params.pool_size(), params.server().io_context() };

	if( break_signal_handling_t::used == params.break_handling() )
		impl::run_with_break_signal_handling( pool, params.server() );
	else
		impl::run_without_break_signal_handling( pool, params.server() );
}

//
// initiate_shutdown
//
/*!
 * @brief Helper function for initiation of server shutdown.
 *
 * Can be useful if an existing HTTP-server is run via run() function.
 * For example:
 * @code
 * restinio::http_server_t< my_traits > server{ ... };
 * // Launch another thread that will perform some application logic.
 * std::thread app_logic_thread{ [&server] {
 * 	while(some_condition) {
 * 		...
 * 		if(exit_case) {
 * 			// HTTP-server should be shut down.
 * 			restinio::initiate_shutdown( server );
 * 			// Our work should be finished.
 * 			return;
 * 		}
 * 	}
 * } };
 * // Start HTTP-server. The current thread will be blocked until
 * // run() returns.
 * restinio::run( restinio::on_thread_pool(
 * 	4,
 * 	restinio::skip_break_signal_handling(),
 * 	server) );
 * // Now app_logic_thread can be joined.
 * app_logic_thread.join();
 * @endcode
 *
 * @since v.0.5.1
 */
template<typename Traits>
inline void
initiate_shutdown( http_server_t<Traits> & server )
{
	server.io_context().post( [&server] {
			server.close_sync();
			server.io_context().stop();
		} );
}

//
// on_pool_runner_t
//
/*!
 * @brief Helper class for running an existing HTTP-server on a thread pool
 * without blocking the current thread.
 *
 * Usage of run() functions has some drawbacks. For example, the current thread
 * on that run() is called, will be blocked until run() returns.
 *
 * Sometimes it is not appropriate and leads to tricks like that:
 * @code
 * // HTTP-server to be run on a thread pool.
 * restinio::http_server_t< my_traits > server{...};
 *
 * // Separate worker thread for calling restinio::run().
 * std::thread run_thread{ [&server] {
 * 	restinio::run( restinio::on_thread_pool(
 * 			16,
 * 			restinio::skip_break_signal_handling(),
 * 			server) );
 * 	// Now this thread is blocked until HTTP-server will be finished.
 * } };
 *
 * ... // Some application specific code here.
 *
 * // Now the server can be stopped.
 * restinio::initiate_shutdown( server );
 * run_thread.join();
 * @endcode
 *
 * Writing such code is a boring and error-prone task. The class
 * on_pool_runner_t can be used instead:
 * @code
 * // HTTP-server to be run on a thread pool.
 * restinio::http_server_t< my_traits > server{...};
 *
 * // Launch HTTP-server on a thread pool.
 * restinio::on_pool_runner_t< restinio::http_server_t<my_traits> > runner{
 * 		16,
 * 		server
 * };
 *
 * ... // Some application specific code here.
 *
 * // Now the server can be stopped.
 * runner.stop(); // (1)
 * runner.wait();
 * @endcode
 *
 * Moreover the code at point (1) in the example above it not necessary
 * because on_pool_runner_t automatically stops the server in the destructor.
 *
 * @since v.0.5.1
 */
template<typename Http_Server>
class on_pool_runner_t
{
	//! HTTP-server to be run.
	Http_Server & m_server;

	//! Thread pool for running the server.
	impl::ioctx_on_thread_pool_t< impl::external_io_context_for_thread_pool_t >
			m_pool;

public :
	on_pool_runner_t( const on_pool_runner_t & ) = delete;
	on_pool_runner_t( on_pool_runner_t && ) = delete;

	//! Initializing constructor.
	on_pool_runner_t(
		//! Size of thread pool.
		std::size_t pool_size,
		//! Server instance to be run.
		//! NOTE. This reference must be valid for all life-time
		//! of on_pool_runner instance.
		Http_Server & server )
		:	m_server{ server }
		,	m_pool{ pool_size, server.io_context() }
	{}

	/*!
	 * @brief Start the server with callbacks that will be called on
	 * success or failure.
	 *
	 * The @a on_ok should be a function/functor with the format:
	 * @code
	 * void () noexcept;
	 * @endcode
	 *
	 * The @a on_error should be a function/functor with the format:
	 * @code
	 * void (std::exception_ptr) noexcept;
	 * @endcode
	 *
	 * @note
	 * Both callbacks will be passed to http_server_t::open_async method.
	 * It means that @a on_error callback will be called for errors detected
	 * by open_async() methods.
	 *
	 * @attention
	 * Both callbacks should be noexcept functions/functors.
	 *
	 * Usage example:
	 * @code
	 * using my_http_server = restinio::http_server_t<some_traits>;
	 *
	 * my_http_server server{...};
	 * restinio::on_pool_runner_t<my_http_server> runner{16, server};
	 *
	 * std::promise<void> run_promise;
	 * auto run_future = run_promise.get_future();
	 * runner.start(
	 * 	// Ok callback.
	 * 	[&run_promise]() noexcept {
	 * 		run_promise.set_value();
	 * 	},
	 * 	// Error callback.
	 * 	[&run_promise](std::exception_ptr ex) noexcept {
	 * 		run_promise.set_exception(std::move(ex));
	 * 	});
	 * // Wait while HTTP-server started (or start failed).
	 * run_future.get();
	 * @endcode
	 *
	 * @since v.0.6.7
	 */
	template<
		typename On_Ok_Callback,
		typename On_Error_Callback >
	void
	start(
		//! A callback to be called if HTTP-server started successfully.
		On_Ok_Callback && on_ok,
		//! A callback to be called if HTTP-server is not started by
		//! some reasons. Please note that this callback is passed
		//! to http_server_t::open_async() and will be called only
		//! for errors detected by open_async() methods.
		//! If some error is detected outside of open_async() (for
		//! example a failure to start a thread pool) then on_error
		//! callback won't be called.
		On_Error_Callback && on_error )
	{
		static_assert( noexcept(on_ok()), "On_Ok_Callback should be noexcept" );
		static_assert( noexcept(on_error(std::declval<std::exception_ptr>())),
				"On_Error_Callback should be noexcept" );

		m_server.open_async(
			[callback = std::move(on_ok)]{ callback(); },
			[this, callback = std::move(on_error)]( std::exception_ptr ex ){
				// There is no sense to run pool.
				m_pool.stop();

				callback( std::move(ex) );
			} );

		m_pool.start();
	}

	//! Start the server.
	/*!
	 * It just a shorthand for a version of `start` method with callbacks
	 * where all callbacks to nothing.
	 */
	void
	start()
	{
		this->start(
				[]() noexcept { /* nothing to do */ },
				[]( std::exception_ptr ) noexcept { /* nothing to do */ } );
	}

	//! Is server started.
	bool
	started() const noexcept { return m_pool.started(); }

	//FIXME: there should be a version of stop() with callbacks like
	//for start() method above.
	//! Stop the server.
	/*!
	 * @note
	 * This method is noexcept since v.0.6.7
	 */
	void
	stop() noexcept
	{
		m_server.close_async(
			[this]{
				// Stop running io_service.
				m_pool.stop();
			},
			[]( std::exception_ptr /*ex*/ ){
				//FIXME: the exception should be stored to be handled
				//later in wait() method.
				//NOTE: this fix is planned for v.0.7.0.
				//std::rethrow_exception( ex );
			} );
	}

	//FIXME this method should be replaced by two new method in v.0.7.0:
	//
	// enum class action_on_exception_t { drop, rethrow };
	// wait(action_on_exception_t action);
	//
	// template<typename Exception_Handler>
	// wait(Exception_Handler && on_exception);
	//
	//! Wait for full stop of the server.
	/*!
	 * @note
	 * This method is noexcept since v.0.6.7
	 */
	void
	wait() noexcept { m_pool.wait(); }
};

// Forward declaration.
// It's necessary for running_server_handle_t.
template< typename Http_Server >
class running_server_instance_t;

//
// running_server_handle_t
//
/*!
 * @brief The type to be used as a handle for running server instance.
 *
 * The handle should be seen as a Moveable and not Copyable type.
 *
 * @since v.0.6.7
 */
template< typename Traits >
using running_server_handle_t =
		std::unique_ptr< running_server_instance_t< http_server_t<Traits> > >;

//
// running_server_instance_t
//
/*!
 * @brief A helper class used in an implementation of #run_async function.
 *
 * An instance of that class holds an HTTP-server and thread pool on that
 * this HTTP-server is launched.
 *
 * The HTTP-server will automatically be stopped in the destructor.
 * However, a user can stop the HTTP-server manually by using
 * stop() and wait() methods.
 *
 * @since v.0.6.7
 */
template< typename Http_Server >
class running_server_instance_t
{
	template< typename Traits >
	friend running_server_handle_t<Traits>
	run_async(
			io_context_holder_t,
			server_settings_t<Traits> &&,
			std::size_t thread_pool_size );

	//! Actual server instance.
	Http_Server m_server;

	//! The runner of the server.
	on_pool_runner_t< Http_Server > m_runner;

	//! Initializing constructor.
	running_server_instance_t(
		io_context_holder_t io_context,
		server_settings_t< typename Http_Server::traits_t > && settings,
		std::size_t thread_pool_size )
		:	m_server{ std::move(io_context), std::move(settings) }
		,	m_runner{ thread_pool_size, m_server }
	{}


	//! Start the HTTP-server.
	/*!
	 * Returns when HTTP-server started or some startup failure detected.
	 * It means that the caller thread will be blocked until HTTP-server
	 * calls on_ok or on_error callback.
	 *
	 * Throws an exception on an error.
	 */
	void
	start()
	{
		std::promise<void> p;
		auto f = p.get_future();
		m_runner.start(
				[&p]() noexcept { p.set_value(); },
				[&p]( std::exception_ptr ex ) noexcept {
					p.set_exception( std::move(ex) );
				} );
		f.get();
	}

public :
	/*!
	 * Stop the HTTP-server.
	 *
	 * This method initiates shutdown procedure that can take some
	 * time. But stop() returns without the waiting for the completeness
	 * of the shutdown. To wait for the completeness use wait() method:
	 *
	 * @code
	 * auto server = restinio::run_async(...);
	 * ...
	 * server->stop(); // Returns without the waiting.
	 * ... // Some other actions.
	 * server->wait(); // Returns only when HTTP-server stopped.
	 * @endcode
	 *
	 * @attention
	 * The current version doesn't guarantee that stop() can be called
	 * safely several times. Please take care of that and call stop()
	 * only once.
	 */
	void
	stop() noexcept
	{
		m_runner.stop();
	}

	/*!
	 * @brief Wait for the shutdown of HTTP-server.
	 *
	 * @note
	 * Method stop() should be called before the call to wait():
	 * @code
	 * auto server = restinio::run_async(...);
	 * ...
	 * server->stop(); // Initiates the shutdown and returns without the waiting.
	 * server->wait(); // Returns only when HTTP-server stopped.
	 * @endcode
	 *
	 * @attention
	 * The current version doesn't guarantee that wait() can be called
	 * safely several times. Please take care of that and call wait()
	 * only once.
	 */
	void
	wait() noexcept
	{
		m_runner.wait();
	}
};

//
// run_async
//
/*!
 * @brief Creates an instance of HTTP-server and launches it on a
 * separate thread or thread pool.
 *
 * Usage example:
 * @code
 * int main() {
 * 	auto server = restinio::run_async(
 * 		// Asio's io_context to be used.
 * 		// HTTP-server will use own Asio's io_context object.
 * 		restinio::own_io_context(),
 * 		// The settings for the HTTP-server.
 * 		restinio::server_settings_t{}
 * 			.address("127.0.0.1")
 * 			.port(8080)
 * 			.request_handler(...),
 * 		// The size of thread-pool for the HTTP-server.
 * 		16);
 * 	// If we are here and run_async doesn't throw then HTTP-server
 * 	// is started.
 *
 * 	... // Some other actions.
 *
 * 	// No need to stop HTTP-server manually. It will be automatically
 * 	// stopped in the destructor of `server` object.
 * }
 * @endcode
 * Or, if user-defined traits should be used:
 * @code
 * int main() {
 * 	struct my_traits : public restinio::default_traits_t {
 * 		...
 * 	};
 *
 * 	auto server = restinio::run_async<my_traits>(
 * 		restinio::own_io_context(),
 * 		restinio::server_settings_t<my_traits>{}
 * 			.address(...)
 * 			.port(...)
 * 			.request_handler(...),
 * 		// Use just one thread for the HTTP-server.
 * 		1u);
 *
 * 	... // Some other actions.
 * }
 * @endcode
 *
 * run_async() returns control when HTTP-server is started or some
 * startup failure is detected. But if a failure is detected then an
 * exception is thrown. So if run_async() returns successfuly then
 * HTTP-server is working.
 *
 * The started HTTP-server will be automatically stopped at the
 * destruction of the returned value. Because of that the returned
 * value should be stored for the time while HTTP-server is needed.
 *
 * The started HTTP-server can be stopped manually by calling
 * stop() and wait() methods:
 * @code
 * auto server = restinio::run_async(...);
 * ...
 * server->stop(); // Returns without the waiting.
 * ... // Some other actions.
 * server->wait(); // Returns only when HTTP-server stopped.
 * @endcode
 *
 * @since v.0.6.7
 */
template< typename Traits = default_traits_t >
RESTINIO_NODISCARD
running_server_handle_t< Traits >
run_async(
	io_context_holder_t io_context,
	server_settings_t< Traits > && settings,
	std::size_t thread_pool_size )
{
	running_server_handle_t< Traits > handle{
			new running_server_instance_t< http_server_t<Traits> >{
					std::move(io_context),
					std::move(settings),
					thread_pool_size }
	};

	handle->start();
	
	return handle;
}

} /* namespace restinio */

