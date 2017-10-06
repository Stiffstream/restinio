/*!
	Protocol header validator .
*/

#pragma once

#include <restinio/exception.hpp>
#include <restinio/websocket/impl/utf8.hpp>
#include <restinio/websocket/impl/ws_parser.hpp>

namespace restinio
{

namespace websocket
{

namespace impl
{

enum class validation_state_t
{
	//correct codes
	frame_header_is_valid,
	payload_part_is_valid,
	frame_is_valid,
	// header validation error codes
	invalid_opcode,
	empty_mask_from_client_side,
	non_final_control_frame,
	non_zero_rsv_flags,
	payload_len_is_too_big,
	// frame order error codes
	continuation_frame_without_data_frame,
	new_data_frame_without_finishing_previous,
	// payload validation error codes
	invalid_close_code,
	incorrect_utf8_data,
	unknown_state
};

//
// is_control_frame
//

inline bool
is_control_frame( opcode_t opcode )
{
	return opcode == opcode_t::connection_close_frame ||
		opcode == opcode_t::ping_frame ||
		opcode == opcode_t::pong_frame;
}

//
// is_data_frame
//

inline bool
is_data_frame( opcode_t opcode )
{
	return opcode == opcode_t::text_frame ||
		opcode == opcode_t::binary_frame;
}

//
// ws_protocol_validator_t
//

//! Class for websocket protocol validations.
/*!
	This class checks:

	text frame and close frame are with valid uff-8 payload;
	close frame has а valid close code;
	continuation chunks of text frame have valid utf-8 text;
	there is invalid situation when continuation frame came without any data frame.
	continuation frame or control frame is wating after data frame with fin flag set in 0.
	opcode has а valid code
	control frame can't be fragmented.

*/
class ws_protocol_validator_t
{
	public:

		ws_protocol_validator_t() = default;

		//! Start work with new frame.
		/*!
			\attention method finish_frame() should be called before processing a new frame.
		*/
		validation_state_t
		process_new_frame( const message_details_t & frame )
		{
			if( m_state != state_t::empty_state )
				throw exception_t( "another frame is processing now" );

			validation_state_t result = validation_state_t::unknown_state;

			if( m_current_continued_data_frame_type ==
					current_continued_data_frame_type_t::none &&
				frame.m_opcode == opcode_t::continuation_frame )
			{
				result = validation_state_t::continuation_frame_without_data_frame;
			}
			else if( m_current_continued_data_frame_type !=
					current_continued_data_frame_type_t::none &&
				is_data_frame( frame.m_opcode ) )
			{
				result = validation_state_t::new_data_frame_without_finishing_previous;
			}
			else
			{
				result = validate_frame_header( frame );

				if( result == validation_state_t::frame_header_is_valid )
				{
					if( frame.m_opcode == opcode_t::text_frame &&
					frame.m_final_flag == false )
					{
						m_current_continued_data_frame_type =
							current_continued_data_frame_type_t::text;
					}
					else if( frame.m_opcode == opcode_t::binary_frame &&
						frame.m_final_flag == false )
					{
						m_current_continued_data_frame_type =
							current_continued_data_frame_type_t::binary;
					}

					m_current_frame = frame;
					m_state = state_t::processing_frame;

					result = validation_state_t::frame_header_is_valid;
				}
			}

			return result;
		}

		//! Validate next part of current frame.
		validation_state_t
		process_next_payload_part( const char * data, size_t size )
		{
			if( m_state == state_t::empty_state )
				throw exception_t( "current state is empty" );

			if( m_current_frame.m_opcode == opcode_t::text_frame ||
				m_current_frame.m_opcode == opcode_t::continuation_frame &&
				m_current_continued_data_frame_type ==
					current_continued_data_frame_type_t::text )
			{
				for( size_t i = 0; i < size; ++i )
				{
					if( !m_utf8_checker.process_byte( data[i] ) )
						return validation_state_t::incorrect_utf8_data;
				}
			}

			return validation_state_t::payload_part_is_valid;
		}

		//! Make final checks of payload if it is necessary and reset state.
		validation_state_t
		finish_frame()
		{
			if( m_current_frame.m_final_flag )
			{
				if( !m_utf8_checker.final() )
					return validation_state_t::incorrect_utf8_data;

				m_utf8_checker.reset();
			}

			m_state = state_t::empty_state;

			// if continued data frame is present and current processed frame
			// continuation frame with final bit set in 1 then reset current continued
			// data frame type
			if( m_current_continued_data_frame_type !=
					current_continued_data_frame_type_t::none &&
				!is_control_frame(m_current_frame.m_opcode) &&
				m_current_frame.m_final_flag )
				m_current_continued_data_frame_type =
					current_continued_data_frame_type_t::none;

			return validation_state_t::frame_is_valid;
		}

		void
		reset()
		{
			m_state = state_t::empty_state;
			m_current_continued_data_frame_type =
				current_continued_data_frame_type_t::none;

			m_utf8_checker.reset();
		}

	private:

		validation_state_t
		validate_frame_header( const message_details_t & frame ) const
		{
			if( !is_valid_opcode( frame.m_opcode ) )
				return validation_state_t::invalid_opcode;

			if( is_control_frame(frame.m_opcode) && frame.m_final_flag == false )
				return validation_state_t::non_final_control_frame;

			if( frame.m_mask_flag == false )
				return validation_state_t::empty_mask_from_client_side;

			if( frame.m_rsv1_flag != 0 ||
				frame.m_rsv2_flag != 0 ||
				frame.m_rsv3_flag != 0)
				return validation_state_t::non_zero_rsv_flags;

			if( is_control_frame(frame.m_opcode) && frame.payload_len() > 125 )
				return validation_state_t::payload_len_is_too_big;

			return validation_state_t::frame_header_is_valid;
		}

		enum class state_t
		{
			empty_state,
			processing_frame
		};

		enum class current_continued_data_frame_type_t
		{
			none,
			text,
			binary
		};

		state_t m_state{ state_t::empty_state };

		current_continued_data_frame_type_t m_current_continued_data_frame_type{
			current_continued_data_frame_type_t::none };

		message_details_t m_current_frame;

		expected_data_t m_expected_close_code{2};

		utf8_checker_t m_utf8_checker;

		size_t m_current_frame_payload_processed_bytes{ 0 };
};

} /* namespace impl */

} /* namespace websocket */

} /* namespace restinio */