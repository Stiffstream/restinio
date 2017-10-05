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
		template<typename D>
		http_server_t(
			io_context_wrapper_unique_ptr_t io_context_wrapper,
			basic_server_settings_t< D, Traits > && settings )
			:	m_io_context_wrapper{ std::move( io_context_wrapper ) }
		{
			using actual_settings_type = basic_server_settings_t<D, Traits>;

			auto conn_settings =
				std::make_shared< connection_settings_t >(
					std::forward<actual_settings_type>(settings),
					impl::create_parser_settings(),
					m_io_context_wrapper->io_context(),
					settings.timer_factory() );

			m_acceptor =
				std::make_shared< acceptor_t >(
					settings,
					m_io_context_wrapper->io_context(),
					std::make_shared< connection_factory_t >(
						conn_settings,
						settings.socket_options_setter() ),
					*( conn_settings->m_logger ) );
		}

		template<
			typename Configurator,
			// Use SFINAE.
			// This constructor must be called only if Configurator
			// allows to call operator() with server_settings_t& arg.
			typename = decltype(
					std::declval<Configurator>()(
							*(static_cast<server_settings_t<Traits>*>(nullptr)))) >
		http_server_t(
			io_context_wrapper_unique_ptr_t io_context_wrapper,
			Configurator && configurator )
			:	http_server_t{
					std::move( io_context_wrapper ),
					exec_configurator< Traits, Configurator >(
						std::forward< Configurator >( configurator ) ) }
		{}

		//! It is allowed to inherit from http_server_t
		virtual ~http_server_t()
		{
			auto acc = std::move( m_acceptor );
			asio::post(
				acc->get_open_close_operations_executor(),
				[ ctx = acc ]{ ctx->ensure_close(); } );
		}

		//! Get io_context on which server runs.
		asio::io_context &
		io_context()
		{
			return m_io_context_wrapper->io_context();
		}

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
			asio::post(
				m_acceptor->get_open_close_operations_executor(),
				[ acceptor = m_acceptor,
					ok_cb = std::move( open_ok_cb ),
					err_cb = std::move( open_err_cb ) ]{
					try
					{
						acceptor->open();
						call_nothrow_cb( ok_cb );
					}
					catch( const std::exception & )
					{
						call_nothrow_cb( err_cb, std::current_exception() );
					}
				} );
		}

		//! Start server.
		/*!
			If server was started successfully then function returns,
			otherwise it throws.
		*/
		void
		start()
		{
			if( sync_running_state_t::not_running == m_sync_running_state )
			{
				// Make sure that we running io_context.
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
				m_sync_running_state = sync_running_state_t::running;
			}
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
			asio::post(
				m_acceptor->get_open_close_operations_executor(),
				[ acceptor = m_acceptor,
					ok_cb = std::move( close_ok_cb ),
					err_cb = std::move( close_err_cb ) ]{
					try
					{
						acceptor->close();
						call_nothrow_cb( ok_cb );
					}
					catch( const std::exception & )
					{
						call_nothrow_cb( err_cb, std::current_exception() );
					}
				} );
		}

		//! Start server.
		/*!
			If server was stopped successfully then function returns,
			otherwise it throws.
		*/
		void
		stop()
		{
			if( sync_running_state_t::running == m_sync_running_state )
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

				// Make sure that we stopped io_context.
				stop_io_context();
				m_sync_running_state = sync_running_state_t::not_running;
			}
		}
	private:
		//! A wrapper for asio io_context where server is running.
		io_context_wrapper_unique_ptr_t m_io_context_wrapper;

		//! Acceptor for new connections.
		std::shared_ptr< acceptor_t > m_acceptor;

		//! State of server.
		enum class sync_running_state_t
		{
			not_running,
			running,
		};

		//! Server state.
		sync_running_state_t m_sync_running_state{ sync_running_state_t::not_running };

		//! Call callback and terminate the application if callback throws.
		template< typename Callback, typename... Args >
		static void call_nothrow_cb( Callback && cb, Args && ...args ) noexcept
		{
			cb( std::forward<Args>(args)... );
		}
};

} /* namespace restinio */
