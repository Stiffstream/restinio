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

namespace websocket
{

//
// opcode_t
//

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
//  status_code_t
//

enum class status_code_t : std::uint16_t
{
	normal_closure = 1000,
	going_away = 1001,
	protocol_error = 1002,
	cant_accept_data = 1003,
	invalid_message_data = 1007,
	policy_violation = 1008,
	too_big_message = 1009,
	more_extensions_expected = 1010,
	unexpected_condition = 1011
};

//
// message_t
//

//! WebSocket message.
class message_t final
	:	public std::enable_shared_from_this< message_t >
{
	public:

		message_t() = default;

		message_t(
			bool is_final,
			opcode_t opcode )
			:	m_is_final{ is_final }
			,	m_opcode{ opcode }
		{
		}

		message_t(
			bool is_final,
			opcode_t opcode,
			std::string payload )
			:	m_is_final{ is_final }
			,	m_opcode{ opcode }
			,	m_payload{ std::move( payload ) }
		{
		}

		bool
		is_final() const
		{
			return m_is_final;
		}

		opcode_t
		opcode() const
		{
			return m_opcode;
		}

		void
		set_opcode( opcode_t opcode )
		{
			m_opcode = opcode;
		}

		const std::string&
		payload() const
		{
			return m_payload;
		}

		std::string&
		payload()
		{
			return m_payload;
		}

		void
		set_payload( std::string str )
		{
			m_payload = std::move( str );
		}

	private:

		//! Final flag.
		bool m_is_final = true;

		//! Opcode.
		opcode_t m_opcode = opcode_t::continuation_frame;

		//! Websocket message payload.
		std::string m_payload;
};

//! Request handler, that is the type for calling request handlers.
using message_handle_t = std::shared_ptr< message_t >;

//
// default_request_handler_t
//

using default_message_handler_t =
		std::function< void ( message_handle_t ) >;


} /* namespace websocket */

} /* namespace restinio */
