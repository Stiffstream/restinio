/*
	restinio
*/

/*!
	WebSocket messgage handler definition.
*/

#pragma once

#include <functional>

#include <restinio/connection_handle.hpp>

namespace restinio
{

enum class opcode_t : std::uint8_t
{
	continuation_frame = 0x00,
	text_frame = 0x01,
	binary_frame = 0x02,
	connection_close_frame = 0x08,
	ping_frame = 0x09,
	pong_frame = 0x0A
};

//
// ws_message_header_t
//

//! Websocket message payload.
struct ws_message_header_t
{
	//! Final flag.
	bool m_is_final = true;

	//! Opcode.
	opcode_t m_opcode = opcode_t::continuation_frame;

	//! Message payload length.
	std::uint64_t m_payload_len = 0;

	//! Masking key.
	std::uint32_t m_masking_key = 0;
};

//
// ws_message_t
//

//! WebSocket message.
class ws_message_t final
	:	public std::enable_shared_from_this< ws_message_t >
{
	public:

		ws_message_t()
		{}

		ws_message_t(
			ws_message_header_t header,
			std::string payload )
			:	m_header{ std::move( header ) }
			,	m_payload{ std::move( payload ) }
		{}

	private:

		//! Websocket message header.
		ws_message_header_t m_header;

		//! Websocket message payload.
		std::string m_payload;
};

//! Request handler, that is the type for calling request handlers.
using ws_message_handle_t = std::shared_ptr< ws_message_t >;

//
// default_request_handler_t
//

using default_ws_message_handler_t =
		std::function< void ( ws_message_handle_t ) >;


} /* namespace restinio */
