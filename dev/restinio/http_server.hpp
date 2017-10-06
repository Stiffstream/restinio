/*
	restinio
*/

/*!
	HTTP-Server.
*/

#pragma once

#include <restinio/exception.hpp>
#include <restinio/settings.hpp>
#include <restinio/request_handler.hpp>
#include <restinio/asio_timer_factory.hpp>
#include <restinio/null_logger.hpp>
#include <restinio/impl/acceptor.hpp>
#include <restinio/traits.hpp>

#include <memory>

namespace restinio
{

//
// io_context_shared_ptr_t
//
using io_context_shared_ptr_t = std::shared_ptr< asio::io_context >;

//
// io_context_holder_t
//
/*!
 * \brief Helper class for holding shared pointer to io_context.
 *
 * It intended to be used as argument to http_server_t's constructor.
 */
class io_context_holder_t
{
	io_context_shared_ptr_t m_context;
public :
	io_context_holder_t( io_context_shared_ptr_t context )
		: m_context( std::move(context) )
	{}

	io_context_shared_ptr_t
	giveaway_context()
	{
		return std::move(m_context);
	}
};

//
// own_io_context
//
/*!
 * \brief Function which tells that http_server should create and use
 * its own instance of io_context.
 *
 * Usage example:
 * \code
 * restinio::http_server_t<> server(
 * 		restinio::own_io_context(),
 * 		restinio::server_settings_t<>()... );
 * \endcode
 */
inline io_context_holder_t
own_io_context()
{
	return { std::make_shared< asio::io_context >() };
}

//
// external_io_context
//
/*!
 * \brief Function which tells that http_server should use external
 * instance of io_context and should not controll its lifetime.
 *
 * Usage example:
 * \code
 * asio::io_context ctx;
 * ...
 * restinio::http_server_t<> server(
 * 		restinio::external_io_context(ctx),
 * 		restinio::server_settings_t<>()...);
 * \endcode
 */
inline io_context_holder_t
external_io_context( asio::io_context & ctx )
{
	return { std::shared_ptr< asio::io_context >(
			std::addressof(ctx),
			// Empty deleter.
			[]( asio::io_context * ){} )
	};
}

//
// http_server_t
//

//FIXME: example in code must be updated!
//! Class for http-server.
/*!
	With the help of this class one can run a serevr.
	Server can be started and stopped in sync or async way.

	Typycal use case is:
	\code
	// Create and initialize object.
	restinio::http_server_t< YOUR_TRAITS >
		server{
			restinio::own_io_context(),
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
			io_context_holder_t io_context,
			basic_server_settings_t< D, Traits > && settings )
			:	m_io_context{ io_context.giveaway_context() }
		{
			using actual_settings_type = basic_server_settings_t<D, Traits>;

			auto conn_settings =
				std::make_shared< connection_settings_t >(
					std::forward<actual_settings_type>(settings),
					impl::create_parser_settings(),
					this->io_context(),
					settings.timer_factory() );

			m_acceptor =
				std::make_shared< acceptor_t >(
					settings,
					this->io_context(),
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
			io_context_holder_t io_context,
			Configurator && configurator )
			:	http_server_t{
					io_context,
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
			return *m_io_context;
		}

		//! Starts server in async way.
		/*!
			\note It is necessary to be sure that ioservice is running.
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
				[ this,
					ok_cb = std::move( open_ok_cb ),
					err_cb = std::move( open_err_cb ) ]{
					try
					{
						open_sync();
						call_nothrow_cb( ok_cb );
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
			if( running_state_t::not_running == m_running_state )
			{
				m_acceptor->open();
				m_running_state = running_state_t::running;
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
				[ this,
					ok_cb = std::move( close_ok_cb ),
					err_cb = std::move( close_err_cb ) ]{
					try
					{
						close_sync();
						call_nothrow_cb( ok_cb );
					}
					catch( const std::exception & )
					{
						err_cb( std::current_exception() );
					}
				} );
		}

		//! Stop server.
		/*!
			If server was stopped successfully then function returns,
			otherwise it throws.
		*/
		void
		close_sync()
		{
			if( running_state_t::running == m_running_state )
			{
				m_acceptor->close();
				m_running_state = running_state_t::not_running;
			}
		}

	private:
		//! A wrapper for asio io_context where server is running.
		io_context_shared_ptr_t m_io_context;

		//! Acceptor for new connections.
		std::shared_ptr< acceptor_t > m_acceptor;

		//! State of server.
		enum class running_state_t
		{
			not_running,
			running,
		};

		//! Server state.
		running_state_t m_running_state{ running_state_t::not_running };

		//! Call callback and terminate the application if callback throws.
		template< typename Callback >
		static void call_nothrow_cb( Callback && cb ) noexcept
		{
			cb();
		}
};

} /* namespace restinio */

