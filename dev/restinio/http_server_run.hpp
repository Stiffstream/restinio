/*
 * restinio
 */

/*!
 * \file
 * \brief Helper function for simple run of HTTP server.
 */

#pragma once

#include <restinio/http_server.hpp>

namespace restinio
{

//
// run_on_this_thread_settings_t
//
//FIXME: document this!
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
//FIXME: document this!
template<typename Traits = default_traits_t>
run_on_this_thread_settings_t<Traits>
on_this_thread() { return run_on_this_thread_settings_t<Traits>{}; }

//
// run_on_thread_pool_settings_t
//
//FIXME: document this!
template<typename Traits>
class run_on_thread_pool_settings_t final
	:	public basic_server_settings_t<
				run_on_thread_pool_settings_t<Traits>,
				Traits>
{
	using base_type_t = basic_server_settings_t<
				run_on_thread_pool_settings_t<Traits>, Traits>;

	//! Size of the pool.
	std::size_t m_pool_size;

public:
	//! Constructor.
	run_on_thread_pool_settings_t(
		//! Size of the pool.
		std::size_t pool_size )
		:	m_pool_size(pool_size)
	{}

	std::size_t
	pool_size() const { return m_pool_size; }
};

//
// on_thread_pool
//
//FIXME: document this!
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
run( run_on_this_thread_settings_t<Traits> && settings )
{
	using settings_t = run_on_this_thread_settings_t<Traits>;
	using server_t = http_server_t<Traits>;

	// Use current thread to run.
	asio::io_context io_context;

	server_t server{
		restinio::use_existing_io_context( io_context ),
		std::forward<settings_t>(settings) };

	server.open_async(
		[]{ /* Ok. */},
		[]( std::exception_ptr ex ){
			std::rethrow_exception( ex );
		} );

	asio::signal_set break_signals{ server.io_context(), SIGINT };
	break_signals.async_wait(
		[&]( const asio::error_code & ec, int ){
			if( !ec )
				server.close_async(
					[]{ /* Ok. */ },
					[]( std::exception_ptr ex ){
						std::rethrow_exception( ex );
					} );
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

	const auto pool_size = settings.pool_size();

	server_t server{
		restinio::create_child_io_context( pool_size ),
		std::forward<settings_t>(settings) };

	std::promise< void > promise;

	asio::signal_set break_signals{ server.io_context(), SIGINT };
	break_signals.async_wait(
		[&]( const asio::error_code & ec, int ){
			if( !ec )
				promise.set_value();
		} );

	server.open();
	promise.get_future().get();
	server.close();
}

} /* namespace restinio */

