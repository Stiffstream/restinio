/*
	restinio
*/

/*!
	HTTP-Server.
*/

#pragma once

#include <future>

#include <restinio/settings.hpp>
#include <restinio/io_service_wrapper.hpp>
#include <restinio/request_handler.hpp>
#include <restinio/asio_timer_factory.hpp>
#include <restinio/loggers.hpp>
#include <restinio/impl/acceptor.hpp>

namespace restinio
{

//
// traits_t
//

template <
		typename TIMER_FACTORY,
		typename LOGGER,
		typename REQUEST_HANDLER = default_request_handler_t,
		typename STRAND = asio::strand< asio::executor > >
struct traits_t
{
	using timer_factory_t = TIMER_FACTORY;
	using logger_t = LOGGER;
	using request_handler_t = REQUEST_HANDLER;
	using strand_t = STRAND;
};

using noop_strand_t = asio::executor;

//
// single_thread_traits_t
//

template <
		typename TIMER_FACTORY,
		typename LOGGER,
		typename REQUEST_HANDLER = default_request_handler_t >
using single_thread_traits_t =
	traits_t< TIMER_FACTORY, LOGGER, REQUEST_HANDLER, noop_strand_t >;

//
// default_traits_t
//

using default_traits_t =
		traits_t<
			asio_timer_factory_t,
			null_logger_t >;

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
			restinio::create_child_io_service( N ),
			[&]( auto & settings ){
				//
				settings
					.port( args.port() )
					// .set_more_params( ... )
					.request_handler(
						[](
							restinio::http_request_data_shared_ptr_t req,
							restinio::connection_handle_shared_ptr_t conn ){
								// Handle request.
							} );
			} };

	server.open();

	// Running server.

	server.close();

	\endcode
*/
template < typename TRAITS = default_traits_t >
class http_server_t
{
		using connection_settings_t = impl::connection_settings_t< TRAITS >;
		using connection_factory_t = impl::connection_factory_t< TRAITS >;
		using acceptor_t = impl::acceptor_t< TRAITS >;

	public:
		http_server_t(
			io_service_wrapper_unique_ptr_t io_service_wrapper,
			server_settings_t< TRAITS > settings )
			:	m_io_service_wrapper{ std::move( io_service_wrapper ) }
		{
			auto conn_settings =
				std::make_shared< connection_settings_t >( settings );

			m_acceptor =
				std::make_shared< acceptor_t >(
					settings.port(),
					settings.protocol(),
					settings.address(),
					m_io_service_wrapper->io_service(),
					std::make_shared< connection_factory_t >(
						conn_settings,
						m_io_service_wrapper->io_service(),
						settings.timer_factory() ),
					*( conn_settings->m_logger ) );
		}

		template < typename CONFIGURATOR >
		http_server_t(
			io_service_wrapper_unique_ptr_t io_service_wrapper,
			CONFIGURATOR && configurator )
			:	http_server_t{
					std::move( io_service_wrapper ),
					exec_configurator< TRAITS, CONFIGURATOR >(
						std::forward< CONFIGURATOR >( configurator ) ) }
		{}

		//! Start/stop io_service.
		/*!
			Is usefull when using async_open() or async_close(),
			because in case of async operation it is
			up to user to guarantee that io_service runs.
		*/
		//! \{
		void
		start_io_service()
		{
			m_io_service_wrapper->start();
		}

		void
		stop_io_service()
		{
			m_io_service_wrapper->stop();
		}
		//! \}

		//! Starts server in async way.
		/*!
			\note It is necessary to be sure that ioservice is running
			(\see start_io_service()).
		*/
		template <
				typename SRV_OPEN_OK_CALLBACK,
				typename SRV_OPEN_ERR_CALLBACK >
		void
		open_async(
			SRV_OPEN_OK_CALLBACK && open_ok_cb,
			SRV_OPEN_ERR_CALLBACK && open_err_cb )
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
					catch( const std::exception & ex )
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
			start_io_service();

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
			\note It doesn't call io_service to stop
			(\see stop_io_service()).
		*/
		template <
				typename SRV_CLOSE_OK_CALLBACK,
				typename SRV_CLOSE_ERR_CALLBACK >
		void
		close_async(
			SRV_CLOSE_OK_CALLBACK && close_ok_cb,
			SRV_CLOSE_ERR_CALLBACK && close_err_cb )
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
					catch( const std::exception & ex )
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
			stop_io_service();
		}

		//! Shortcut for close_sync().
		void
		close()
		{
			close_sync();
		}

	private:
		//! A wrapper for asio io_service where server is running.
		io_service_wrapper_unique_ptr_t m_io_service_wrapper;

		//! Acceptor for new connections.
		std::shared_ptr< acceptor_t > m_acceptor;
};

} /* namespace restinio */
