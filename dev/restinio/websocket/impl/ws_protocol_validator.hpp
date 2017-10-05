/*!
	UTF-8 .
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
		void
		process_new_frame( const message_details_t & frame )
		{
			if( m_state != state_t::empty_state )
				throw std::runtime_error( "frame is processing now" );

			validate_frame_header( frame );

			if( m_current_continued_data_frame_type ==
					current_continued_data_frame_type_t::none &&
				frame.m_opcode == opcode_t::continuation_frame )
				throw std::runtime_error( "continuation frame can't be"
					" received without previus data frame" );

			if( m_current_continued_data_frame_type !=
					current_continued_data_frame_type_t::none &&
				is_data_frame( frame.m_opcode ) )
				throw std::runtime_error( "new data frame cant be started without"
					" because current chunked frame didnt marked as final" );

			if( frame.m_opcode == opcode_t::text_frame &&
				frame.m_final_flag == false )
				m_current_continued_data_frame_type =
					current_continued_data_frame_type_t::text;

			if( frame.m_opcode == opcode_t::binary_frame &&
				frame.m_final_flag == false )
				m_current_continued_data_frame_type =
					current_continued_data_frame_type_t::binary;

			m_current_frame = frame;
			m_state = state_t::processing_frame;
		}

		//! Validate next part of current frame.
		void
		process_next_payload_part( const char * data, size_t size )
		{
			if( m_state == state_t::empty_state )
				throw std::runtime_error( "current state is empty" );

			// if( )
		}

		//! Make final checks of payload if it is necessary and reset state.
		void
		finish_frame()
		{
			if( !m_utf8_checker.final() )
				throw std::runtime_error( "frame payload is not valid utf-8 data" );

			m_utf8_checker.reset();

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
		}

	private:

		void
		validate_frame_header( const message_details_t & frame ) const
		{
			if( !is_valid_opcode( frame.m_opcode ) )
				throw std::runtime_error( "invalid opcode" );

			if( is_control_frame(frame.m_opcode) && frame.m_final_flag == false )
				throw std::runtime_error( "control frame must be final" );

			if( frame.m_mask_flag == false )
				throw std::runtime_error( "mask must be present" );

			if( frame.m_rsv1_flag != 0 ||
				frame.m_rsv2_flag != 0 ||
				frame.m_rsv3_flag != 0)
				throw std::runtime_error( "reserved flags must be 0" );

			if( is_control_frame(frame.m_opcode) && frame.payload_len() > 125 )
				throw std::runtime_error(
					"payload len can't be greater than 125 bytes in control frames" );
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