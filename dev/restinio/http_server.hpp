/*
	restinio
*/

/*!
	HTTP-Server.
*/

#pragma once

#include <future>

#include <restinio/exception.hpp>
#include <restinio/settings.hpp>
#include <restinio/io_context_wrapper.hpp>
#include <restinio/request_handler.hpp>
#include <restinio/asio_timer_factory.hpp>
#include <restinio/null_logger.hpp>
#include <restinio/impl/acceptor.hpp>
#include <restinio/traits.hpp>

namespace restinio
{

//
// http_server_t
//

//! Class for http-server.
/*!
	With the help of this class one can run a serevr.
	Server can be started and stopped in sync or async way.

	Typycal use case is:
	\code
	// Create and initialize object.
	restinio::http_server_t< YOUR_TRAITS >
		server{
			restinio::create_child_io_context( N ),
			[&]( auto & settings ){
				//
				settings
					.port( args.port() )
					// .set_more_params( ... )
					.request_handler(
						[]( restinio::request_handle_t req ){
								// Handle request.
						} );
			} };

	server.open();

	// Running server.

	server.close();

	\endcode
*/
template < typename Traits = default_traits_t >
class http_server_t
{
		using connection_settings_t = impl::connection_settings_t< Traits >;
		using connection_factory_t = impl::connection_factory_t< Traits >;
		using acceptor_t = impl::acceptor_t< Traits >;

	public:
		http_server_t(
			io_context_wrapper_unique_ptr_t io_context_wrapper,
			server_settings_t< Traits > settings )
			:	m_io_context_wrapper{ std::move( io_context_wrapper ) }
		{
			auto conn_settings =
				std::make_shared< connection_settings_t >(
					settings,
					impl::create_parser_settings(),
					m_io_context_wrapper->io_context() );

			m_acceptor =
				std::make_shared< acceptor_t >(
					settings,
					m_io_context_wrapper->io_context(),
					std::make_shared< connection_factory_t >(
						conn_settings,
						settings.socket_options_setter(),
						settings.timer_factory() ),
					*( conn_settings->m_logger ) );
		}

		template < typename Configurator >
		http_server_t(
			io_context_wrapper_unique_ptr_t io_context_wrapper,
			Configurator && configurator )
			:	http_server_t{
					std::move( io_context_wrapper ),
					exec_configurator< Traits, Configurator >(
						std::forward< Configurator >( configurator ) ) }
		{}

		//! Start/stop io_context.
		/*!
			Is usefull when using async_open() or async_close(),
			because in case of async operation it is
			up to user to guarantee that io_context runs.
		*/
		//! \{
		void
		start_io_context()
		{
			m_io_context_wrapper->start();
		}

		void
		stop_io_context()
		{
			m_io_context_wrapper->stop();
		}
		//! \}

		//! Starts server in async way.
		/*!
			\note It is necessary to be sure that ioservice is running
			(\see start_io_context()).
		*/
		template <
				typename Server_Open_Ok_CB,
				typename Server_Open_Error_CB >
		void
		open_async(
			Server_Open_Ok_CB && open_ok_cb,
			Server_Open_Error_CB && open_err_cb )
		{
			asio::dispatch(
				m_acceptor->get_executor(),
				[ acceptor = m_acceptor,
					ok_cb = std::move( open_ok_cb ),
					err_cb = std::move( open_err_cb ) ](){
					try
					{
						acceptor->open();
						ok_cb();
					}
					catch( const std::exception & )
					{
						err_cb( std::current_exception() );
					}
				} );
		}

		//! Start server.
		/*!
			If server was started successfully then function returns,
			otherwise it throws.
		*/
		void
		open_sync()
		{
			// Make sure that we running ioservice.
			start_io_context();

			// Sync object.
			std::promise< void > open_result;

			open_async(
				[ &open_result ](){
					open_result.set_value();
				},
				[ &open_result ]( std::exception_ptr ex ){
					open_result.set_exception( std::move( ex ) );
				} );

			open_result.get_future().get();
		}

		//! Shortcut for open_sync().
		void
		open()
		{
			open_sync();
		}

		//! Closes server in async way.
		/*!
			\note It doesn't call io_context to stop
			(\see stop_io_context()).
		*/
		template <
				typename Server_Close_Ok_CB,
				typename Server_Close_Error_CB >
		void
		close_async(
			Server_Close_Ok_CB && close_ok_cb,
			Server_Close_Error_CB && close_err_cb )
		{
			asio::dispatch(
				m_acceptor->get_executor(),
				[ ctx = m_acceptor,
					ok_cb = std::move( close_ok_cb ),
					err_cb = std::move( close_err_cb ) ](){
					try
					{
						ctx->close();
						ok_cb();
					}
					catch( const std::exception & )
					{
						err_cb( std::current_exception() );
					}
				} );
		}

		//! Start server.
		/*!
			If server was stopped successfully then function returns,
			otherwise it throws.
		*/
		void
		close_sync()
		{
			// Sync object.
			std::promise< void > close_result;

			close_async(
				[ &close_result ](){
					close_result.set_value();
				},
				[ &close_result ]( std::exception_ptr ex ){
					close_result.set_exception( std::move( ex ) );
				} );

			close_result.get_future().wait();

			// Make sure that we stopped ioservice.
			stop_io_context();
		}

		//! Shortcut for close_sync().
		void
		close()
		{
			close_sync();
		}

	private:
		//! A wrapper for asio io_context where server is running.
		io_context_wrapper_unique_ptr_t m_io_context_wrapper;

		//! Acceptor for new connections.
		std::shared_ptr< acceptor_t > m_acceptor;
};

} /* namespace restinio */
