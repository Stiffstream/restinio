/*
	restinio
*/

/*!
	Socket adapter for asio::ssl::stream< asio::ip::tcp::socket >.
*/

#pragma once

#include <restinio/asio_include.hpp>

#if !defined(RESTINIO_USE_BOOST_ASIO)
  #include <asio/ssl.hpp>
#else
  #include <boost/asio/ssl.hpp>
#endif

namespace restinio
{

namespace impl
{

//
// tls_socket_t
//

//! Socket adapter for asio::ssl::stream< asio::ip::tcp::socket >.
/*!
	As asio::ssl::stream< asio::ip::tcp::socket > class is not movable
	and lack some some functionality compared to asio::ip::tcp::socket
	it is necesasary to have an adapter for it to use it the same way as
	asio::ip::tcp::socket in template classes and functions.
*/
class tls_socket_t
{
	public:
		using socket_t = asio_ns::ssl::stream< asio_ns::ip::tcp::socket >;
		using context_handle_t = std::shared_ptr< asio_ns::ssl::context >;
		// Needed for asio >= 1.16.0 (starting with boost-1.72.0)
#if RESTINIO_ASIO_VERSION >= 101600
		using executor_type = asio_ns::executor;
#endif
		tls_socket_t( const tls_socket_t & ) = delete;
		tls_socket_t & operator = ( const tls_socket_t & ) = delete;

		tls_socket_t(
			asio_ns::io_context & io_context,
			context_handle_t tls_context )
			:	m_context{ std::move( tls_context ) }
			,	m_socket{ std::make_unique< socket_t >( io_context, *m_context ) }
		{}

		tls_socket_t( tls_socket_t && ) = default;
		tls_socket_t & operator = ( tls_socket_t && ) = default;

		void
		swap( tls_socket_t & sock )
		{
			std::swap( m_context, sock.m_context );
			std::swap( m_socket, sock.m_socket );
		}

		auto &
		lowest_layer()
		{
			return m_socket->lowest_layer();
		}

		const auto &
		lowest_layer() const
		{
			return m_socket->lowest_layer();
		}

		/*!
		 * \brief Get an access to underlying Asio's socket.
		 *
		 * This feature can be useful if there is a need to call some
		 * Asio's socket specific methods like `native_handle`.
		 *
		 * \since
		 * v.0.5.2
		 */
		socket_t &
		asio_ssl_stream()
		{
			return *m_socket;
		}

		/*!
		 * \brief Get an access to underlying Asio's socket.
		 *
		 * This feature can be useful if there is a need to call some
		 * Asio's socket specific methods like `native_handle`.
		 *
		 * \since
		 * v.0.5.2
		 */
		const socket_t &
		asio_ssl_stream() const
		{
			return *m_socket;
		}

		auto
		get_executor()
		{
			return this->lowest_layer().get_executor();
		}

		auto
		remote_endpoint() const
		{
			return this->lowest_layer().remote_endpoint();
		}

		auto
		is_open() const
		{
			return this->lowest_layer().is_open();
		}

		template< typename... Args >
		void
		cancel( Args &&... args )
		{
			this->lowest_layer().cancel( std::forward< Args >( args )... );
		}

		template< typename... Args >
		auto
		async_read_some( Args &&... args )
		{
			return m_socket->async_read_some( std::forward< Args >( args )... );
		}

		template< typename... Args >
		auto
		async_write_some( Args &&... args )
		{
			return m_socket->async_write_some( std::forward< Args >( args )... );
		}

		template< typename... Args >
		void
		shutdown( Args &&... args )
		{
			this->lowest_layer().shutdown( std::forward< Args >( args )... );
		}

		template< typename... Args >
		void
		close( Args &&... args )
		{
			this->lowest_layer().close( std::forward< Args >( args )... );
		}

		template< typename... Args >
		auto
		async_handshake( Args &&... args )
		{
			return m_socket->async_handshake( std::forward< Args >( args )... );
		}

	private:
		context_handle_t m_context;
		std::unique_ptr< socket_t > m_socket;
};

} /* namespace impl */

} /* namespace restinio */
