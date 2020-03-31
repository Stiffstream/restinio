/*
	restinio
*/

/*!
	HTTP-Acceptor handler routine.
*/

#pragma once

#include <memory>

#include <restinio/impl/include_fmtlib.hpp>

#include <restinio/impl/connection.hpp>

#include <restinio/utils/suppress_exceptions.hpp>

namespace restinio
{

namespace impl
{

//
// socket_supplier_t
//

/*
	A helper base class that hides a pool of socket instances.

	It prepares a socket for new connections.
	And as it is template class over a socket type
	it givies an oportunity to customize details for
	other types of sockets (like `asio::ssl::stream< asio::ip::tcp::socket >`)
	that can be used.
*/
template < typename Socket >
class socket_supplier_t
{
	protected:
		template < typename Settings >
		socket_supplier_t(
			//! Server settings.
			Settings & settings,
			//! A context the server runs on.
			asio_ns::io_context & io_context )
			:	m_io_context{ io_context }
		{
			m_sockets.reserve( settings.concurrent_accepts_count() );

			std::generate_n(
				std::back_inserter( m_sockets ),
				settings.concurrent_accepts_count(),
				[this]{
					return Socket{m_io_context};
				} );

			assert( m_sockets.size() == settings.concurrent_accepts_count() );
		}

		//! Get the reference to socket.
		Socket &
		socket(
			//! Index of a socket in the pool.
			std::size_t idx )
		{
			return m_sockets.at( idx );
		}

		//! Extract the socket via move.
		Socket
		move_socket(
			//! Index of a socket in the pool.
			std::size_t idx )
		{
			return std::move( socket(idx ) );
		}

		//! The number of sockets that can be used for
		//! cuncurrent accept operations.
		auto
		cuncurrent_accept_sockets_count() const noexcept
		{
			return m_sockets.size();
		}

	private:
		//! io_context for sockets to run on.
		asio_ns::io_context & m_io_context;

		//! A temporary socket for receiving new connections.
		//! \note Must never be empty.
		std::vector< Socket > m_sockets;
};

namespace acceptor_details
{

/*!
 * @brief A class for holding actual IP-blocker.
 *
 * This class holds shared pointer to actual IP-blocker object and
 * provides actual inspect_incoming() implementation.
 *
 * @since v.0.5.1
 */
template< typename Ip_Blocker >
struct ip_blocker_holder_t
{
	std::shared_ptr< Ip_Blocker > m_ip_blocker;

	template< typename Settings >
	ip_blocker_holder_t(
		const Settings & settings )
		:	m_ip_blocker{ settings.ip_blocker() }
	{}

	template< typename Socket >
	restinio::ip_blocker::inspection_result_t
	inspect_incoming( Socket & socket ) const noexcept
	{
		return m_ip_blocker->inspect(
				restinio::ip_blocker::incoming_info_t{
					socket.lowest_layer().remote_endpoint()
				} );
	}
};

/*!
 * @brief A specialization of ip_blocker_holder for case of
 * noop_ip_blocker.
 *
 * This class doesn't hold anything and doesn't do anything.
 *
 * @since v.0.5.1
 */
template<>
struct ip_blocker_holder_t< restinio::ip_blocker::noop_ip_blocker_t >
{
	template< typename Settings >
	ip_blocker_holder_t( const Settings & ) { /* nothing to do */ }

	template< typename Socket >
	restinio::ip_blocker::inspection_result_t
	inspect_incoming( Socket & /*socket*/ ) const noexcept
	{
		return restinio::ip_blocker::inspection_result_t::allow;
	}
};

} /* namespace acceptor_details */

//
// acceptor_t
//

//! Context for accepting http connections.
template < typename Traits >
class acceptor_t final
	:	public std::enable_shared_from_this< acceptor_t< Traits > >
	,	protected socket_supplier_t< typename Traits::stream_socket_t >
	,	protected acceptor_details::ip_blocker_holder_t< typename Traits::ip_blocker_t >
{
		using ip_blocker_base_t = acceptor_details::ip_blocker_holder_t<
				typename Traits::ip_blocker_t >;

	public:
		using connection_factory_t = impl::connection_factory_t< Traits >;
		using connection_factory_shared_ptr_t =
			std::shared_ptr< connection_factory_t >;
		using logger_t = typename Traits::logger_t;
		using strand_t = typename Traits::strand_t;
		using stream_socket_t = typename Traits::stream_socket_t;
		using socket_holder_base_t = socket_supplier_t< stream_socket_t >;

		template < typename Settings >
		acceptor_t(
			Settings & settings,
			//! ASIO io_context to run on.
			asio_ns::io_context & io_context,
			//! Connection factory.
			connection_factory_shared_ptr_t connection_factory,
			//! Logger.
			logger_t & logger )
			:	socket_holder_base_t{ settings, io_context }
			,	ip_blocker_base_t{ settings }
			,	m_port{ settings.port() }
			,	m_protocol{ settings.protocol() }
			,	m_address{ settings.address() }
			,	m_acceptor_options_setter{ settings.acceptor_options_setter() }
			,	m_acceptor{ io_context }
			,	m_executor{ io_context.get_executor() }
			,	m_open_close_operations_executor{ io_context.get_executor() }
			,	m_separate_accept_and_create_connect{ settings.separate_accept_and_create_connect() }
			,	m_connection_factory{ std::move( connection_factory ) }
			,	m_logger{ logger }
		{}

		//! Start listen on port specified in ctor.
		void
		open()
		{
			if( m_acceptor.is_open() )
			{
				const auto ep = m_acceptor.local_endpoint();
				m_logger.warn( [&]{
					return fmt::format( "server already started on {}", ep );
				} );
				return;
			}

			asio_ns::ip::tcp::endpoint ep{ m_protocol, m_port };

			if( !m_address.empty() )
			{
				auto addr = m_address;
				if( addr == "localhost" )
					addr = "127.0.0.1";
				else if( addr == "ip6-localhost" )
					addr = "::1";

				ep.address( asio_ns::ip::address::from_string( addr ) );
			}

			try
			{
				m_logger.trace( [&]{
					return fmt::format( "starting server on {}", ep );
				} );

				m_acceptor.open( ep.protocol() );

				{
					// Set acceptor options.
					acceptor_options_t options{ m_acceptor };

					(*m_acceptor_options_setter)( options );
				}

				m_acceptor.bind( ep );
				m_acceptor.listen( asio_ns::socket_base::max_connections );

				// Call accept connections routine.
				for( std::size_t i = 0; i< this->cuncurrent_accept_sockets_count(); ++i )
				{
					m_logger.info( [&]{
						return fmt::format( "init accept #{}", i );
					} );

					accept_next( i );
				}

				m_logger.info( [&]{
					return fmt::format( "server started on {}", ep );
				} );
			}
			catch( const std::exception & ex )
			{
				// Acceptor should be closes in the case of an error.
				if( m_acceptor.is_open() )
					m_acceptor.close();

				m_logger.error( [&]() -> auto {
					return fmt::format( "failed to start server on {}: {}",
						ep,
						ex.what() );
				} );

				throw;
			}
		}

		//! Close listener if any.
		void
		close()
		{
			if( m_acceptor.is_open() )
			{
				close_impl();
			}
			else
			{
				m_logger.trace( [&]{
					return fmt::format( "server already closed" );
				} );
			}
		}

		//! Get an executor for close operation.
		auto &
		get_open_close_operations_executor() noexcept
		{
			return m_open_close_operations_executor;
		}

	private:
		//! Get executor for acceptor.
		auto & get_executor() noexcept { return m_executor; }

		//! Set a callback for a new connection.
		/*!
		 * @note
		 * This method is marked as noexcept in v.0.6.0.
		 * It seems that nothing prevents exceptions from a call to
		 * async_accept. But we just don't know what to do in that case.
		 * So at the moment the call to `std::terminate` because an
		 * exception is raised inside `noexcept` method seems to be an
		 * appropriate solution.
		 */
		void
		accept_next( std::size_t i ) noexcept
		{
			m_acceptor.async_accept(
				this->socket( i ).lowest_layer(),
				asio_ns::bind_executor(
					get_executor(),
					[i, ctx = this->shared_from_this()]( const auto & ec ) noexcept {
						if( !ec )
						{
							ctx->accept_current_connection( i, ec );
						}
					} ) );
		}

		//! Accept current connection.
		/*!
		 * @note
		 * This method is marked as noexcept in v.0.6.0.
		 */
		void
		accept_current_connection(
			//! socket index in the pool of sockets.
			std::size_t i,
			const std::error_code & ec ) noexcept
		{
			if( !ec )
			{
				restinio::utils::suppress_exceptions(
						m_logger,
						"accept_current_connection",
						[this, i] {
							accept_connection_for_socket_with_index( i );
						} );
			}
			else
			{
				// Something goes wrong with connection.
				restinio::utils::log_error_noexcept( m_logger,
					[&]{
						return fmt::format(
							"failed to accept connection on socket #{}: {}",
							i,
							ec.message() );
					} );
			}

			// Continue accepting.
			accept_next( i );
		}

		/*!
		 * @brief Performs actual actions for accepting a new connection.
		 *
		 * @note
		 * This method can throw. An we expect that it can throw sometimes.
		 *
		 * @since v.0.6.0
		 */
		void
		accept_connection_for_socket_with_index(
			//! socket index in the pool of sockets.
			std::size_t i )
		{
			auto incoming_socket = this->move_socket( i );

			auto remote_endpoint =
					incoming_socket.lowest_layer().remote_endpoint();

			m_logger.trace( [&]{
				return fmt::format(
						"accept connection from {} on socket #{}",
						remote_endpoint, i );
			} );

			// Since v.0.5.1 the incoming connection must be
			// inspected by IP-blocker.
			const auto inspection_result = this->inspect_incoming(
					incoming_socket );

			switch( inspection_result )
			{
			case restinio::ip_blocker::inspection_result_t::deny:
				// New connection can be used. It is disabled by IP-blocker.
				m_logger.warn( [&]{
					return fmt::format(
							"accepted connection from {} on socket #{} denied by"
							" IP-blocker",
							remote_endpoint, i );
				} );
				// incoming_socket will be closed automatically.
			break;

			case restinio::ip_blocker::inspection_result_t::allow:
				// Acception of the connection can be continued.
				do_accept_current_connection(
						std::move(incoming_socket),
						remote_endpoint );
			break;
			}
		}

		void
		do_accept_current_connection(
			stream_socket_t incoming_socket,
			endpoint_t remote_endpoint )
		{
			auto create_and_init_connection =
				[sock = std::move(incoming_socket),
				factory = m_connection_factory,
				ep = std::move(remote_endpoint),
				logger = &m_logger]() mutable noexcept {
					// NOTE: this code block shouldn't throw!
					restinio::utils::suppress_exceptions(
							*logger,
							"do_accept_current_connection.create_and_init_connection",
							[&] {
								// Create new connection handler.
								// NOTE: since v.0.6.3 this method throws in
								// the case of an error. Because of that there is
								// no need to check the value returned.
								auto conn = factory->create_new_connection(
										std::move(sock), std::move(ep) );

								// Start waiting for request message.
								conn->init();
							} );
				};

			if( m_separate_accept_and_create_connect )
			{
				asio_ns::post(
					get_executor(),
					std::move( create_and_init_connection ) );
			}
			else
			{
				create_and_init_connection();
			}
		}

		//! Close opened acceptor.
		void
		close_impl()
		{
			const auto ep = m_acceptor.local_endpoint();

			// An exception in logger should not prevent a call of close()
			// for m_acceptor.
			restinio::utils::log_trace_noexcept( m_logger,
				[&]{
					return fmt::format( "closing server on {}", ep );
				} );

			m_acceptor.close();

			m_logger.info( [&]{
				return fmt::format( "server closed on {}", ep );
			} );
		}

		//! Server endpoint.
		//! \{
		const std::uint16_t m_port;
		const asio_ns::ip::tcp m_protocol;
		const std::string m_address;
		//! \}

		//! Server port listener and connection receiver routine.
		//! \{
		std::unique_ptr< acceptor_options_setter_t > m_acceptor_options_setter;
		asio_ns::ip::tcp::acceptor m_acceptor;
		//! \}

		//! Asio executor.
		asio_ns::executor m_executor;
		strand_t m_open_close_operations_executor;

		//! Do separate an accept operation and connection instantiation.
		const bool m_separate_accept_and_create_connect;

		//! Factory for creating connections.
		connection_factory_shared_ptr_t m_connection_factory;

		logger_t & m_logger;
};

} /* namespace impl */

} /* namespace restinio */
