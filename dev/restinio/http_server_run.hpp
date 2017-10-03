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

#if 0
	else
	{
		server_t server{
			restinio::create_child_io_context( n ),
			std::move( s ) };

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
#endif

} /* namespace restinio */

