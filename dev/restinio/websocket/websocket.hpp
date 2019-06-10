/*
	restinio
*/

/*!
	WebSocket messgage handler definition.
*/

#pragma once

#include <functional>

#include <restinio/websocket/message.hpp>
#include <restinio/websocket/impl/ws_connection_base.hpp>
#include <restinio/websocket/impl/ws_connection.hpp>
#include <restinio/utils/base64.hpp>
#include <restinio/utils/sha1.hpp>

namespace restinio
{

namespace websocket
{

namespace basic
{

//
// ws_t
//

//! A WebSocket bind.
/*!
	An abstraction for websocket. User have to keep this handle during all the period
	that websocket is used. It must be stored in a `shared_ptr<ws_t>` (ws_handle_t)
	and when the last reference on this handle is lost underlying connection will be closed.
*/
class ws_t
	:	public std::enable_shared_from_this< ws_t >
{
	public:
		//
		// activate()
		//

		//! Activate websocket: start receiving messages.
		friend void activate( ws_t & ws )
		{
			ws.m_ws_connection_handle->init_read( ws.shared_from_this() );
		}

		ws_t( const ws_t & ) = delete;
		ws_t( ws_t && ) = delete;
		ws_t & operator = ( const ws_t & ) = delete;
		ws_t & operator = ( ws_t && ) = delete;

		ws_t(
			impl::ws_connection_handle_t ws_connection_handle,
			endpoint_t remote_endpoint )
			:	m_ws_connection_handle{ std::move( ws_connection_handle ) }
			,	m_remote_endpoint{ std::move( remote_endpoint ) }
		{}

		~ws_t()
		{
			try
			{
				shutdown();
			}
			catch( ... )
			{}
		}

		//! Get connection id.
		/*!
			If connection exists then its id is returned,
			otherwise retursn zero.
		*/
		connection_id_t
		connection_id() const
		{
			return m_ws_connection_handle ? m_ws_connection_handle->connection_id() : 0;
		}

		//! Shutdown websocket: wait for all outgoing data to be sent,
		//! and close connection.
		void
		shutdown()
		{
			if( m_ws_connection_handle )
			{
				auto con = std::move( m_ws_connection_handle );
				con->shutdown();
			}
		}

		//! Kill websocket: close underlying tcp socket.
		//! Do not tolerate unsent outgoing data.
		void
		kill()
		{
			if( m_ws_connection_handle )
			{
				auto con = std::move( m_ws_connection_handle );
				con->kill();
			}
		}

		//! Send_websocket message.
		void
		send_message(
			final_frame_flag_t final_flag,
			opcode_t opcode,
			writable_item_t payload,
			write_status_cb_t wscb = write_status_cb_t{} )
		{
			if( m_ws_connection_handle )
			{
				if( restinio::writable_item_type_t::trivial_write_operation ==
					payload.write_type() )
				{
					writable_items_container_t bufs;
					bufs.reserve( 2 );

					// Create header serialize it and append to bufs .
					impl::message_details_t details{
						final_flag, opcode, asio_ns::buffer_size( payload.buf() ) };

					bufs.emplace_back(
						impl::write_message_details( details ) );

					bufs.emplace_back( std::move( payload ) );

					write_group_t wg{ std::move( bufs ) };

					if( wscb )
					{
						wg.after_write_notificator( std::move( wscb ) );
					}

					// TODO: set flag.
					const bool is_close_frame =
						opcode_t::connection_close_frame == opcode;

					if( is_close_frame )
					{
						auto con = std::move( m_ws_connection_handle );
						con->write_data(
							std::move( wg ),
							is_close_frame );
					}
					else
					{
						m_ws_connection_handle->write_data(
							std::move( wg ),
							is_close_frame );
					}
				}
				else
				{
					throw exception_t{ "ws doesn't support sendfile" };
				}
			}
			else
			{
				throw exception_t{ "websocket is not available" };
			}
		}

		void
		send_message( message_t msg, write_status_cb_t wscb = write_status_cb_t{} )
		{
			send_message(
				msg.final_flag(),
				msg.opcode(),
				writable_item_t{ std::move( msg.payload() ) },
				std::move( wscb ) );
		}

		//! Get the remote endpoint of the underlying connection.
		const endpoint_t & remote_endpoint() const noexcept { return m_remote_endpoint; }

	private:
		impl::ws_connection_handle_t m_ws_connection_handle;

		//! Remote endpoint for this ws-connection.
		const endpoint_t m_remote_endpoint;
};

//! Alias for ws_t handle.
using ws_handle_t = std::shared_ptr< ws_t >;

//
// activation_t
//

//! Flags for websocket activation policies.
enum class activation_t
{
	//! Activate immediately after upgrade operation.
	immediate,
	//! User will initiate activation later.
	delayed
};

//
// upgrade()
//

//! Upgrade http-connection of a current request to a websocket connection.
template <
		typename Traits,
		typename WS_Message_Handler >
ws_handle_t
upgrade(
	//! Upgrade request.
	request_t & req,
	//! Activation policy.
	activation_t activation_flag,
	//! Response header fields.
	http_header_fields_t upgrade_response_header_fields,
	//! Message handler.
	WS_Message_Handler ws_message_handler )
{
	// TODO: check if upgrade request?

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

	using connection_t = restinio::impl::connection_t< Traits >;
	auto conn_ptr = std::move( restinio::impl::access_req_connection( req ) );
	if( !conn_ptr )
	{
		throw exception_t{ "no connection for upgrade: already moved" };
	}
	auto & con = dynamic_cast< connection_t & >( *conn_ptr );

	using ws_connection_t = impl::ws_connection_t< Traits, WS_Message_Handler >;

	auto upgrade_internals = con.move_upgrade_internals();
	auto ws_connection =
		std::make_shared< ws_connection_t >(
			con.connection_id(),
			std::move( upgrade_internals.m_settings ),
			std::move( upgrade_internals.m_socket ),
			std::move( ws_message_handler ) );

	writable_items_container_t upgrade_response_bufs;
	{
		http_response_header_t upgrade_response_header{ status_switching_protocols() };
		upgrade_response_header.swap_fields( upgrade_response_header_fields );
		upgrade_response_header.connection( http_connection_header_t::upgrade );

		const auto content_length_flag =
			restinio::impl::content_length_field_presence_t::skip_content_length;

		upgrade_response_bufs.emplace_back(
			restinio::impl::create_header_string(
				upgrade_response_header,
				content_length_flag ) );
	}

	ws_connection->write_data(
		write_group_t{ std::move( upgrade_response_bufs ) },
		false );

	auto result =
		std::make_shared< ws_t >( std::move( ws_connection ), req.remote_endpoint() );

	if( activation_t::immediate == activation_flag )
	{
		activate( *result );
	}

	// Returns strong handle on websocket, thus giving an ownership.
	return result;
}

template <
		typename Traits,
		typename WS_Message_Handler >
auto
upgrade(
	request_t & req,
	activation_t activation_flag,
	std::string sec_websocket_accept_field_value,
	WS_Message_Handler ws_message_handler )
{
	http_header_fields_t upgrade_response_header_fields;
	upgrade_response_header_fields.set_field(
		http_field::sec_websocket_accept,
		std::move( sec_websocket_accept_field_value ) );

	return
		upgrade< Traits, WS_Message_Handler >(
			req,
			activation_flag,
			std::move( upgrade_response_header_fields ),
			std::move( ws_message_handler ) );
}

template <
		typename Traits,
		typename WS_Message_Handler >
auto
upgrade(
	request_t & req,
	activation_t activation_flag,
	std::string sec_websocket_accept_field_value,
	std::string sec_websocket_protocol_field_value,
	WS_Message_Handler ws_message_handler )
{
	http_header_fields_t upgrade_response_header_fields;
	upgrade_response_header_fields.set_field(
		http_field::sec_websocket_accept,
		std::move( sec_websocket_accept_field_value ) );

	upgrade_response_header_fields.set_field(
		http_field::sec_websocket_protocol,
		std::move( sec_websocket_protocol_field_value ) );

	return
		upgrade< Traits, WS_Message_Handler >(
			req,
			activation_flag,
			std::move( upgrade_response_header_fields ),
			std::move( ws_message_handler ) );
}

template <
		typename Traits,
		typename WS_Message_Handler >
auto
upgrade(
	request_t & req,
	activation_t activation_flag,
	WS_Message_Handler ws_message_handler )
{
	const char * websocket_accept_field_suffix = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	const auto ws_key =
		req.header().get_field( restinio::http_field::sec_websocket_key ) +
		websocket_accept_field_suffix;

	auto digest = restinio::utils::sha1::make_digest( ws_key );

	std::string sec_websocket_accept_field_value = utils::base64::encode(
		utils::sha1::to_string( digest ) );

	http_header_fields_t upgrade_response_header_fields;
	upgrade_response_header_fields.set_field(
		http_field::sec_websocket_accept,
		std::move( sec_websocket_accept_field_value ) );

	return
		upgrade< Traits, WS_Message_Handler >(
			req,
			activation_flag,
			std::move( upgrade_response_header_fields ),
			std::move( ws_message_handler ) );
}

} /* namespace basic */

} /* namespace websocket */

} /* namespace restinio */
