/*
	restinio
*/

/*!
	WebSocket messgage handler definition.
*/

#pragma once

#include <functional>

#include <restinio/connection_handle.hpp>
#include <restinio/ws_message.hpp>
#include <restinio/impl/ws_connection.hpp>

namespace restinio
{

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
			m_ws_connection_handle->init_read();
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
			bool final,
			opcode_t opcode,
			buffer_storage_t payload )
		{
			if( m_ws_connection_handle )
			{
				buffers_container_t bufs;
				bufs.reserve( 2 ); // ?

				// Create header serialize it and append to bufs .
				impl::ws_message_details_t details{
					final, opcode, asio::buffer_size( payload.buf() ) };

				bufs.emplace_back(
					impl::write_message_details( details ) );

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

template <
		typename TRAITS,
		typename WS_MESSAGE_HANDLER,
		typename WS_CLOSE_HANDLER >
websocket_unique_ptr_t
upgrade_to_websocket(
	request_t & req,
	http_header_fields_t upgrade_response_header_fields,
	WS_MESSAGE_HANDLER ws_message_handler,
	WS_CLOSE_HANDLER ws_close_handler )
{
	req.check_connection();

	// TODO: check if upgrade request.

	//! Check if mandatory field is available.
	if( !upgrade_response_header_fields.has_field( http_field::sec_websocket_accept ) )
	{
		throw exception_t{
			fmt::format( "{} field is mandatory for upgrade response",
				field_to_string( http_field::sec_websocket_accept ) ) };
	}

	if( !upgrade_response_header_fields.has_field( http_field::upgrade ) )
	{
		upgrade_response_header_fields.set_field( http_field::upgrade, "websocket" );
	}


	using connection_t = impl::connection_t< TRAITS >;
	auto conn_ptr = std::move( req.m_connection );
	auto & con = dynamic_cast< connection_t & >( *conn_ptr );

	using ws_connection_t = impl::ws_connection_t< TRAITS, WS_MESSAGE_HANDLER, WS_CLOSE_HANDLER >;

	auto upgrade_internals = con.move_upgrade_internals();
	auto ws_connection =
		std::make_shared< ws_connection_t >(
			con.connection_id(),
			std::move( upgrade_internals.m_socket ),
			std::move( upgrade_internals.m_strand ),
			std::move( upgrade_internals.m_timer_guard ),
			con.get_settings(),
			std::move( ws_message_handler ),
			std::move( ws_close_handler ) );

	buffers_container_t upgrade_response_bufs;
	{
		http_response_header_t upgrade_response_header{ 101, "Switching Protocols" };
		upgrade_response_header.swap_fields( upgrade_response_header_fields );
		upgrade_response_header.connection( http_connection_header_t::upgrade );

		upgrade_response_bufs.emplace_back(
			impl::create_header_string(
				upgrade_response_header,
				impl::content_length_field_presence_t::skip_content_length ) );
	}

	ws_connection->write_data( std::move( upgrade_response_bufs ) );

	return std::make_unique< websocket_t >( std::move( ws_connection ) );
}

//
// upgrade_to_websocket
//

template <
		typename TRAITS,
		typename WS_MESSAGE_HANDLER,
		typename WS_CLOSE_HANDLER >
websocket_unique_ptr_t
upgrade_to_websocket(
	request_t & req,
	std::string sec_websocket_accept_field_value,
	WS_MESSAGE_HANDLER ws_message_handler,
	WS_CLOSE_HANDLER ws_close_handler )
{
	http_header_fields_t upgrade_response_header_fields;
	upgrade_response_header_fields.set_field(
		http_field::sec_websocket_accept,
		std::move( sec_websocket_accept_field_value ) );

	return
		upgrade_to_websocket< TRAITS, WS_MESSAGE_HANDLER, WS_CLOSE_HANDLER >(
			req,
			std::move( upgrade_response_header_fields ),
			std::move( ws_message_handler ),
			std::move( ws_close_handler ) );
}

//
// upgrade_to_websocket
//

template <
		typename TRAITS,
		typename WS_MESSAGE_HANDLER,
		typename WS_CLOSE_HANDLER >
websocket_unique_ptr_t
upgrade_to_websocket(
	request_t & req,
	std::string sec_websocket_accept_field_value,
	std::string sec_websocket_protocol_field_value,
	WS_MESSAGE_HANDLER ws_message_handler,
	WS_CLOSE_HANDLER ws_close_handler )
{
	http_header_fields_t upgrade_response_header_fields;
	upgrade_response_header_fields.set_field(
		http_field::sec_websocket_accept,
		std::move( sec_websocket_accept_field_value ) );

	upgrade_response_header_fields.set_field(
		http_field::sec_websocket_protocol,
		std::move( sec_websocket_protocol_field_value ) );

	return
		upgrade_to_websocket< TRAITS, WS_MESSAGE_HANDLER, WS_CLOSE_HANDLER >(
			req,
			std::move( upgrade_response_header_fields ),
			std::move( ws_message_handler ),
			std::move( ws_close_handler ) );
}

} /* namespace restinio */
