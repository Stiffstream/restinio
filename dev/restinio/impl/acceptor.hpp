/*
	restinio
*/

/*!
	HTTP-Acceptor handler routine.
*/

#pragma once

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <restinio/impl/connection.hpp>

namespace restinio
{

namespace impl
{

//
// acceptor_t
//

//! Context for accepting http connections.
template < typename TRAITS >
class acceptor_t final
	:	public std::enable_shared_from_this< acceptor_t< TRAITS > >
{
	public:
		using connection_factory_t = impl::connection_factory_t< TRAITS >;
		using connection_factory_shared_ptr_t =
			std::shared_ptr< connection_factory_t >;
		using logger_t = typename TRAITS::logger_t;
		using strand_t = typename TRAITS::strand_t;

		acceptor_t(
			//! Server port.
			std::uint16_t port,
			//! Server protocol.
			asio::ip::tcp protocol,
			//! Is only local connections allowed.
			std::string address,
			//! ASIO io_service to run on.
			asio::io_service & io_service,
			//! Connection factory.
			connection_factory_shared_ptr_t connection_factory,
			logger_t & logger )
			:	m_port{ port }
			,	m_protocol{ protocol }
			,	m_address{ std::move( address ) }
			,	m_acceptor{ io_service }
			,	m_socket{ io_service }
			,	m_strand{ m_socket.get_executor() }
			,	m_connection_factory{ std::move( connection_factory ) }
			,	m_logger{ logger }
		{}

		//! Start listen on port specified in ctor.
		void
		open()
		{
			asio::ip::tcp::endpoint ep{ m_protocol, m_port };

			if( !m_address.empty() )
			{
				ep.address( asio::ip::address::from_string( m_address ) );
			}

			try
			{
				m_logger.trace( [&]{
					return fmt::format( "starting server on {}", ep );
				} );

				m_acceptor.open( ep.protocol() );

				m_acceptor.set_option(
					asio::ip::tcp::acceptor::reuse_address( true ) );

				m_acceptor.bind( ep );
				m_acceptor.listen( asio::socket_base::max_connections );

				// Call accept connections routine.
				accept_next();

				m_logger.info( [&]{
					return fmt::format( "server started  on {}", ep );
				} );
			}
			catch( const std::exception & ex )
			{
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
			const auto ep = m_acceptor.local_endpoint();

			m_logger.trace( [&]{
				return fmt::format( "closing server on {}", ep );
			} );

			if( m_acceptor.is_open() )
			{
				m_acceptor.close();
			}

			m_logger.info( [&]{
				return fmt::format( "server closed on {}", ep );
			} );
		}

		strand_t &
		get_executor()
		{
			return m_strand;
		}

	private:
		// Set a callback for a new connection.
		void
		accept_next()
		{
			m_acceptor.async_accept(
				m_socket,
				asio::wrap(
					get_executor(),
					[ ctx = this->shared_from_this() ]( auto ec ){
						// Check if acceptor is running.
						// Also it cover
						// `asio::error:operation_aborted == ec`
						// case.
						if( ctx->m_acceptor.is_open() )
						{
							ctx->accept_current_connection( ec );
						}
					} ) );
		}

		//! Accept current connection.
		void
		accept_current_connection( const std::error_code & ec )
		{
			if( !ec )
			{
				m_logger.trace( [&]{
					return fmt::format(
							"accept connection from: {}",
							m_socket.remote_endpoint() );
				} );

				// Create new connection handler.
				auto conn =
					m_connection_factory
						->create_new_connection( std::move( m_socket ) );

				//! If connection handler was created,
				// then start waiting for request message.
				if( conn )
					conn->wait_for_http_message();
			}
			else
			{
				// Something goes wrong with connection.
				m_logger.error( [&]{
					return fmt::format(
						"failed to accept connection: {}",
						ec );
				} );
			}

			// Continue accepting.
			accept_next();
		}

		//! Server endpoint.
		//! \{
		const std::uint16_t m_port;
		const asio::ip::tcp m_protocol;
		const std::string m_address;
		//! \}

		//! Server port listener and connection receiver routine.
		//! \{
		asio::ip::tcp::acceptor m_acceptor;
		asio::ip::tcp::socket m_socket;
		//! \}

		//! Sync object for acceptor events.
		strand_t m_strand;

		//! Factory for creating connections.
		connection_factory_shared_ptr_t m_connection_factory;

		logger_t & m_logger;
};

} /* namespace impl */

} /* namespace restinio */
