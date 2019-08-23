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
#include <restinio/impl/acceptor.hpp>
#include <restinio/traits.hpp>

#include <memory>

namespace restinio
{

//
// io_context_shared_ptr_t
//
using io_context_shared_ptr_t = std::shared_ptr< asio_ns::io_context >;

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
	return { std::make_shared< asio_ns::io_context >() };
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
external_io_context( asio_ns::io_context & ctx )
{
	return { std::shared_ptr< asio_ns::io_context >(
			std::addressof(ctx),
			// Empty deleter.
			[]( asio_ns::io_context * ){} )
	};
}

//
// http_server_t
//

//! Class for http-server.
/*!
	With the help of this class one can run a server.
	Server can be started and stopped in sync or async way.

	Please note that it is responsibility of user to provide a working
	context for http_server. It means that user must call
	asio::io_context::run() on some work thread (or on several working
	threads).

	Sync way for starting and stopping a http_server can be used only if
	http_server_t::open_sync() and http_server_t::close_sync() methods
	are called somewhere inside asio::io_context::run(). For example:
	\code
	// Create and initialize object.
	restinio::http_server_t< my_traits_t > server{
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

	// Post initial action to asio event loop.
	asio::post( server.io_context(),
		[&] {
			// Starting the server in a sync way.
			server.open_sync();
		} );

	// Running server.
	server.io_context().run();
	\endcode

	Async way for starting and stopping a http_server can be used if
	http_server_t::open_async() and http_server_t::open_async() can be
	called from any other thread. For example:
	\code
	asio::io_context io_ctx;
	restinio::http_server_t< my_traits_t > server{
			restinio::external_io_context(io_ctx),
			[&]( auto & settings ) { ... } };

	// Launch thread on which server will work.
	std::thread server_thread{ [&] {
			io_ctx.run();
		} };

	// Start server in async way. Actual start will be performed
	// on the context of server_thread.
	server.open_async(
			// Ok callback. Nothing to do.
			[]{},
			// Error callback. Rethrow an exception.
			[]( auto ex_ptr ) {
				std::rethrow_exception( ex_ptr );
			} );
	...
	// Wait while server_thread finishes its work.
	server_thread.join();
	\endcode
*/
template < typename Traits = default_traits_t >
class http_server_t
{
		using connection_settings_t = impl::connection_settings_t< Traits >;
		using connection_factory_t = impl::connection_factory_t< Traits >;
		using acceptor_t = impl::acceptor_t< Traits >;
		using timer_manager_t = typename Traits::timer_manager_t;
		using timer_manager_handle_t = std::shared_ptr< timer_manager_t >;

	public:
		/*!
		 * @brief An alias for Traits type.
		 *
		 * @since v.0.5.0
		 */
		using traits_t = Traits;

		// This is not Copyable nor Moveable type.
		http_server_t( const http_server_t & ) = delete;
		http_server_t( http_server_t && ) = delete;

		template<typename D>
		http_server_t(
			io_context_holder_t io_context,
			basic_server_settings_t< D, Traits > && settings )
			:	m_io_context{ io_context.giveaway_context() }
			,	m_cleanup_functor{ settings.giveaway_cleanup_func() }
		{
			// Since v.0.5.1 the presence of custom connection state
			// listener should be checked before the start of HTTP server.
			settings.ensure_valid_connection_state_listener();
			// The presence of IP-blocker should also be checked.
			settings.ensure_valid_ip_blocker();

			// Now we can continue preparation of HTTP server.

			using actual_settings_type = basic_server_settings_t<D, Traits>;

			auto timer_factory = settings.timer_factory();
			m_timer_manager = timer_factory->create( this->io_context() );

			auto conn_settings =
				std::make_shared< connection_settings_t >(
					std::forward< actual_settings_type >(settings),
					impl::create_parser_settings< typename Traits::http_methods_mapper_t >(),
					m_timer_manager );

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
			// Ensure server is closed after destruction of http_server instance.
			close_sync();
		}

		//! Get io_context on which server runs.
		asio_ns::io_context & io_context() noexcept { return *m_io_context; }

		//! Starts server in async way.
		/*!
			\note It is necessary to be sure that ioservice is running.

			\attention
			\a open_ok_cb and \a open_err_cb should be noexcept
			functions/lambdas. This requirement is not enforced by
			static_assert in RESTinio's code to avoid problems in
			cases when `std::function` is used for these callbacks.
		*/
		template <
				typename Server_Open_Ok_CB,
				typename Server_Open_Error_CB >
		void
		open_async(
			Server_Open_Ok_CB open_ok_cb,
			Server_Open_Error_CB open_err_cb )
		{
			asio_ns::post(
				m_acceptor->get_open_close_operations_executor(),
				[ this,
					ok_cb = std::move( open_ok_cb ),
					err_cb = std::move( open_err_cb ) ]{
					try
					{
						open_sync();
						call_nothrow_cb( ok_cb );
					}
					catch( ... )
					{
						call_nothrow_cb( [&err_cb] {
								err_cb( std::current_exception() );
							} );
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
				m_timer_manager->start();
				m_acceptor->open();
				m_running_state = running_state_t::running;
			}
		}

		//! Closes server in async way.
		/*!
			\note It doesn't call io_context to stop
			(\see stop_io_context()).

			\attention
			\a close_ok_cb and \a close_err_cb should be noexcept
			functions/lambdas. This requirement is not enforced by
			static_assert in RESTinio's code to avoid problems in
			cases when `std::function` is used for these callbacks.
		*/
		template <
				typename Server_Close_Ok_CB,
				typename Server_Close_Error_CB >
		void
		close_async(
			Server_Close_Ok_CB close_ok_cb,
			Server_Close_Error_CB close_err_cb )
		{
			asio_ns::post(
				m_acceptor->get_open_close_operations_executor(),
				[ this,
					ok_cb = std::move( close_ok_cb ),
					err_cb = std::move( close_err_cb ) ]{
					try
					{
						close_sync();
						call_nothrow_cb( ok_cb );
					}
					catch( ... )
					{
						call_nothrow_cb( [&err_cb] {
								err_cb( std::current_exception() );
							} );
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
				m_timer_manager->stop();
				m_acceptor->close();
				call_cleanup_functor();
				m_running_state = running_state_t::not_running;
			}
		}

	private:
		//! A wrapper for asio io_context where server is running.
		io_context_shared_ptr_t m_io_context;

		//! An optional user's cleanup functor.
		cleanup_functor_t m_cleanup_functor;

		//! Acceptor for new connections.
		std::shared_ptr< acceptor_t > m_acceptor;

		//! Timer manager object.
		timer_manager_handle_t m_timer_manager;

		//! State of server.
		enum class running_state_t
		{
			not_running,
			running,
		};

		//! Server state.
		running_state_t m_running_state{ running_state_t::not_running };

		//! Call a cleanup functor if it is defined.
		/*!
		 * \note
		 * Cleanup functor can be called only once. Next call to
		 * call_cleanup_functor() will do nothing.
		 *
		 * \attention
		 * Cleanup functor can't throw.
		 */
		void
		call_cleanup_functor() noexcept
		{
			if( m_cleanup_functor )
			{
				cleanup_functor_t fn{ std::move(m_cleanup_functor) };
				fn();
			}
		}

		//! Call callback and terminate the application if callback throws.
		template< typename Callback >
		static void call_nothrow_cb( Callback && cb ) noexcept
		{
			cb();
		}
};

} /* namespace restinio */

