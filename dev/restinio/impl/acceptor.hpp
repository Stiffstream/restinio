/*
	restinio
*/

/*!
	HTTP-Acceptor handler routine.
*/

#pragma once

#include <memory>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <restinio/impl/connection.hpp>

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
			asio::io_context & io_context )
			:	m_io_context{ io_context }
		{
			m_sockets.reserve( settings.concurrent_accepts_count() );

			while( m_sockets.size() < settings.concurrent_accepts_count() )
			{
				m_sockets.emplace_back( m_io_context );
			}
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
			auto res = std::move( m_sockets.at( idx ) );
			return res;
		}

		//! The number of sockets that can be used for
		//! cuncurrent accept operations.
		auto
		cuncurrent_accept_sockets_count() const
		{
			return m_sockets.size();
		}

	private:
		//! io_context for sockets to run on.
		asio::io_context & m_io_context;

		//! A temporary socket for receiving new connections.
		//! \note Must never be empty.
		std::vector< Socket > m_sockets;
};

//
// acceptor_t
//

//! Context for accepting http connections.
template < typename Traits >
class acceptor_t final
	:	public std::enable_shared_from_this< acceptor_t< Traits > >
	,	protected socket_supplier_t< typename Traits::stream_socket_t >
{
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
			asio::io_context & io_context,
			//! Connection factory.
			connection_factory_shared_ptr_t connection_factory,
			//! Logger.
			logger_t & logger )
			:	socket_holder_base_t{ settings, io_context }
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

				{
					// Set acceptor options.
					acceptor_options_t options{ m_acceptor };

					(*m_acceptor_options_setter)( options );
				}

				m_acceptor.bind( ep );
				m_acceptor.listen( asio::socket_base::max_connections );

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

		auto &
		get_open_close_operations_executor()
		{
			return m_open_close_operations_executor;
		}

	private:
		auto &
		get_executor()
		{
			return m_executor;
		}

		// Set a callback for a new connection.
		void
		accept_next( std::size_t i )
		{
			m_acceptor.async_accept(
				this->socket( i ).lowest_layer(),
				asio::bind_executor(
					get_executor(),
					[ i, ctx = this->shared_from_this() ]( const auto & ec ){
						if( !ec )
						{
							ctx->accept_current_connection( i, ec );
						}
					} ) );
		}

		//! Accept current connection.
		void
		accept_current_connection(
			//! socket index in the pool of sockets.
			std::size_t i,
			const std::error_code & ec )
		{
			if( !ec )
			{
				m_logger.trace( [&]{
					return fmt::format(
							"accept connection from {} on socket #{}",
							this->socket( i ).lowest_layer().remote_endpoint(), i );
				} );

				auto create_and_init_connection =
					[ sock = this->move_socket( i ), factory = m_connection_factory ]() mutable {
						// Create new connection handler.
						auto conn = factory->create_new_connection( std::move( sock ) );

						//! If connection handler was created,
						// then start waiting for request message.
						if( conn )
							conn->init();
					};

				if( m_separate_accept_and_create_connect )
				{
					asio::post(
						get_executor(),
						std::move( create_and_init_connection ) );
				}
				else
				{
					create_and_init_connection();
				}
			}
			else
			{
				// Something goes wrong with connection.
				m_logger.error( [&]{
					return fmt::format(
						"failed to accept connection on socket #{}: {}",
						i,
						ec.message() );
				} );
			}

			// Continue accepting.
			accept_next( i );
		}

		//! Close opened acceptor.
		void
		close_impl()
		{
			const auto ep = m_acceptor.local_endpoint();

			m_logger.trace( [&]{
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
		const asio::ip::tcp m_protocol;
		const std::string m_address;
		//! \}

		//! Server port listener and connection receiver routine.
		//! \{
		std::unique_ptr< acceptor_options_setter_t > m_acceptor_options_setter;
		asio::ip::tcp::acceptor m_acceptor;
		//! \}

		//! Asio executor.
		asio::executor m_executor;
		strand_t m_open_close_operations_executor;

		//! Do separate an accept operation and connection instantiation.
		const bool m_separate_accept_and_create_connect;

		//! Factory for creating connections.
		connection_factory_shared_ptr_t m_connection_factory;

		logger_t & m_logger;
};

} /* namespace impl */

} /* namespace restinio */
