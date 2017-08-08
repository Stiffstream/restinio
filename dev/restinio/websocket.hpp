/*
	restinio
*/

/*!
	WebSocket messgage handler definition.
*/

#pragma once

#include <functional>

#include <restinio/connection_handle.hpp>
#include <restinio/impl/ws_connection.hpp>

namespace restinio
{

//
// ws_message_t
//

//! WebSocket message.
class ws_message_t final
	:	public std::enable_shared_from_this< ws_message_t >
{
	public:
		ws_message_t(
			std::string payload )
			:	m_payload{ std::move( payload ) }
		{}

	private:
		std::string m_payload;
};

//! Request handler, that is the type for calling request handlers.
using ws_message_handle_t = std::shared_ptr< ws_message_t >;

//
// default_request_handler_t
//

using default_ws_message_handler_t =
		std::function< void ( ws_message_handle_t ) >;

//
// websocket_t
//

//! A WebSocket bind.
class websocket_t
{
	public:
		websocket_t( const websocket_t & ) = delete;
		websocket_t( websocket_t && ) = delete;
		void operator = ( const websocket_t & ) = delete;
		void operator = ( websocket_t && ) = delete;

		websocket_t(
			ws_connection_handle_t ws_connection_handle )
			:	m_ws_connection_handle{ std::move( ws_connection_handle ) }
		{
			// TODO:
			// On accepting upgrade request
			// we must send http-response to finish with handshake
			// So here must be initialized first write operation
			// that carries response.
		}

		~websocket_t()
		{
			try
			{
				close();
			}
			catch( ... )
			{}
		}

		void
		close()
		{
			if( m_ws_connection_handle )
			{
				auto con = std::move( m_ws_connection_handle );
				con->close();
			}
		}

		//! Send_websocket message
		void
		send_message(
			/*TODO: Заголовок_или_то_из_чего_делается_заголовок */
			buffer_storage_t payload )
		{
			if( m_ws_connection_handle )
			{
				buffers_container_t bufs;
				bufs.reserve( 2 );

				// TODO:
				// Create header serialize it and append to bufs .

				/*TODO:
					bufs.emplace_back(
						сериализовать_в_буфер( Заголовок_или_то_из_чего_делается_заголовок ) );
				*/

				bufs.emplace_back( std::move( payload ) );

				m_ws_connection_handle->write_data( std::move( bufs ) );
			}
			else
			{
				throw exception_t{ "websocket is closed" };
			}
		}

	private:
		ws_connection_handle_t m_ws_connection_handle;
};

//! Alias for websocket_t unique_ptr.
using websocket_unique_ptr_t = std::unique_ptr< websocket_t >;

//
// upgrade_to_websocket
//

template < typename TRAITS, typename WS_MESSAGE_HANDLER >
websocket_unique_ptr_t
upgrade_to_websocket(
	/*TODO: params???*/
	request_t & req,
	WS_MESSAGE_HANDLER ws_message_handler )
{
	req.check_connection();

	// TODO: check if upgrade request.

	using connection_t = impl::connection_t< TRAITS >;
	auto conn_ptr = std::move( req.m_connection );
	auto & con = dynamic_cast< connection_t & >( *conn_ptr );

	return std::make_unique< websocket_t >(
		std::make_shared< impl::ws_connection_t >(
			con.connection_id(),
			con.move_socket(),
			con.get_settings(),
			std::move( ws_message_handler ) ) );
}

} /* namespace restinio */
