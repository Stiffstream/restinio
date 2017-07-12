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

template < typename STREAM_SOCKET >
class socket_holder_t
{
	protected:
		template < typename SETTINGS >
		socket_holder_t(
			SETTINGS & ,
			asio::io_service & io_service )
			:	m_io_service{}
			,	m_socket{ io_service }
		{}

		STREAM_SOCKET &
		socket()
		{
			return *m_socket;
		}

		std::unique_ptr< STREAM_SOCKET >
		move_socket()
		{
			auto res = make_unqique< STREAM_SOCKET >{ m_io_service };
			std::swap( res, m_socket );
			return res;
		}

	private:
		asio::io_service & m_io_service;
		std::unique_ptr< STREAM_SOCKET > m_socket{ make_unqique< STREAM_SOCKET >{ m_io_service} };
};

//
// acceptor_t
//

//! Context for accepting http connections.
template < typename TRAITS >
class acceptor_t final
	:	public std::enable_shared_from_this< acceptor_t< TRAITS > >
	,	public socket_holder_t< typename TRAITS::stream_socket_t >
{
	public:
		using connection_factory_t = impl::connection_factory_t< TRAITS >;
		using connection_factory_shared_ptr_t =
			std::shared_ptr< connection_factory_t >;
		using logger_t = typename TRAITS::logger_t;
		using strand_t = typename TRAITS::strand_t;
		using stream_socket_t = typename TRAITS::stream_socket_t;
		using socket_holder_base_t = socket_holder_t< stream_socket_t >;

		template < typename SETTINGS >
		acceptor_t(
			SETTINGS & settings,
			// //! Server port.
			// std::uint16_t port,
			// //! Server protocol.
			// asio::ip::tcp protocol,
			// //! Is only local connections allowed.
			// std::string address,
			//! ASIO io_service to run on.
			asio::io_service & io_service,
			//! Connection factory.
			connection_factory_shared_ptr_t connection_factory,
			logger_t & logger )
			:	socket_holder_base_t{ settings, io_service }
			,	m_port{ settings.port() }
			,	m_protocol{ settings.protocol() }
			,	m_address{ settings.address() }
			,	m_acceptor{ io_service }
			,	m_strand{ this->socket().lowest_layer().get_executor() }
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
				auto addr = m_address;
				if( addr == "localhost" )
					addr = "127.0.0.1";
				else if( addr == "ip6-localhost" )
					addr = "::1";

				ep.address( asio::ip::address::from_string( addr ) );
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
				this->socket().lowest_layer(),
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
							this->socket().lowest_layer().remote_endpoint() );
				} );

				// Create new connection handler.
				auto conn =
					m_connection_factory
						->create_new_connection( this->move_socket() );

				//! If connection handler was created,
				// then start waiting for request message.
				if( conn )
					conn->init();
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
		// stream_socket_t m_socket;
		//! \}

		//! Sync object for acceptor events.
		strand_t m_strand;

		//! Factory for creating connections.
		connection_factory_shared_ptr_t m_connection_factory;

		logger_t & m_logger;
};

} /* namespace impl */

} /* namespace restinio */
