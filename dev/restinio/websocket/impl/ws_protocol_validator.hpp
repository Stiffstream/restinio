/*!
	Protocol header validator.
*/

#pragma once

#include <restinio/exception.hpp>
#include <restinio/websocket/impl/utf8.hpp>
#include <restinio/websocket/impl/ws_parser.hpp>

namespace restinio
{

namespace websocket
{

namespace basic
{

namespace impl
{

//
// validation_state_t
//

//! States of validated frame.
enum class validation_state_t
{
	initial_state,
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
	incorrect_utf8_data
};

//
// validation_state_str
//

//! Helper function for logging validation states.
inline const char *
validation_state_str( validation_state_t state )
{
	static constexpr const char* table[] =
	{
		"initial_state",
		"frame_header_is_valid",
		"payload_part_is_valid",
		"frame_is_valid",
		"invalid_opcode",
		"empty_mask_from_client_side",
		"non_final_control_frame",
		"non_zero_rsv_flags",
		"payload_len_is_too_big",
		"continuation_frame_without_data_frame",
		"new_data_frame_without_finishing_previous",
		"invalid_close_code",
		"incorrect_utf8_data"
	};

	return table[static_cast<unsigned int>(state)];
}

//
// is_control_frame
//

//! Check frame is control frame.
/*!
	\return true if frame is control frame.
	\return false otherwise.
*/
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

//! Check frame is data frame.
/*!
	\return true if frame is data frame.
	\return false otherwise.
*/
inline bool
is_data_frame( opcode_t opcode )
{
	return opcode == opcode_t::text_frame ||
		opcode == opcode_t::binary_frame;
}

//
// unmasker_t
//

/*!
	This class is need to unmask byte sequences.

	Mask is 32 bit key.
*/
struct unmasker_t
{
	unmasker_t() = default;

	unmasker_t( uint32_t masking_key )
	:	m_mask{
			{::restinio::utils::impl::bitops::n_bits_from< std::uint8_t, 24 >(
				masking_key),
			::restinio::utils::impl::bitops::n_bits_from< std::uint8_t, 16 >(
				masking_key),
			::restinio::utils::impl::bitops::n_bits_from< std::uint8_t, 8 >(
				masking_key),
			::restinio::utils::impl::bitops::n_bits_from< std::uint8_t, 0 >(
				masking_key)} }
	{
	}

	//! Do unmask operation.
	/*!
		\return unmasked value.
	*/
	uint8_t
	unmask_byte( uint8_t masked_byte )
	{
		return masked_byte ^ m_mask[ (m_processed_bytes_count++) % 4 ];
	}

	//! Reset to initial state.
	void
	reset( uint32_t masking_key )
	{
		m_processed_bytes_count = 0;

		m_mask = mask_array_t{
			{::restinio::utils::impl::bitops::n_bits_from< std::uint8_t, 24 >(
				masking_key),
			::restinio::utils::impl::bitops::n_bits_from< std::uint8_t, 16 >(
				masking_key),
			::restinio::utils::impl::bitops::n_bits_from< std::uint8_t, 8 >(
				masking_key),
			::restinio::utils::impl::bitops::n_bits_from< std::uint8_t, 0 >(
				masking_key)} };

	}

	using mask_array_t = std::array< uint8_t, websocket_masking_key_size>;

	//! Bytes array with masking key.
	mask_array_t m_mask;

	//! Processed bytes counter.
	/*!
		It needs for taking remainder after division on 4. Result of this operation
		is index of value in mask array for next unmask operation.
	*/
	size_t m_processed_bytes_count{ 0 };
};

//
// ws_protocol_validator_t
//

//! Class for websocket protocol validations.
/*!
	This class checks:

	text frame and close frame are with valid uff-8 payload;
	close frame has a valid close code;
	continuation chunks of text frame have valid utf-8 text;
	there is invalid situation when continuation frame came without any data frame.
	continuation frame or control frame is wating after data frame with fin flag set in 0.
	opcode has a valid code
	control frame can't be fragmented.

*/
class ws_protocol_validator_t
{
	public:

		ws_protocol_validator_t() = default;

		ws_protocol_validator_t( bool do_unmask )
		:	m_unmask_flag{ do_unmask }
		{
		}

		//! Start work with new frame.
		/*!
			\attention methods finish_frame() or reset() should be called before
			processing a new frame.
		*/
		validation_state_t
		process_new_frame( const message_details_t & frame )
		{
			if( m_working_state != working_state_t::empty_state )
				throw exception_t( "another frame is processing now" );

			if( validate_frame_header( frame ) &&
				check_previous_frame_type( frame.m_opcode ) )
			{
				switch( frame.m_opcode )
				{
					case opcode_t::text_frame:
						if( !frame.m_final_flag )
							m_previous_data_frame = previous_data_frame_t::text;
					break;

					case opcode_t::binary_frame:
						if( !frame.m_final_flag )
							m_previous_data_frame = previous_data_frame_t::binary;
					break;

					case opcode_t::connection_close_frame:
						m_expected_close_code.reset(2);
					break;

					default:
					break;
				}

				m_current_frame = frame;
				m_working_state = working_state_t::processing_frame;

				if( m_unmask_flag )
				{
					m_unmasker.reset( frame.m_masking_key );
				}
			}

			return m_validation_state;
		}

		//! Validate next part of current frame.
		validation_state_t
		process_next_payload_part( const char * data, size_t size )
		{
			if( m_working_state == working_state_t::empty_state )
				throw exception_t( "current state is empty" );

			if( is_state_still_valid() )
				set_validation_state(
					validation_state_t::payload_part_is_valid );
			else
				return m_validation_state;

			for( size_t i = 0; i < size; ++i )
			{
				process_payload_byte(
					static_cast<std::uint8_t>(data[i]) );

				if( m_validation_state != validation_state_t::payload_part_is_valid )
					break;
			}

			return m_validation_state;
		}

		//! Validate next part of current frame and reset source part to unmasked data.
		validation_state_t
		process_and_unmask_next_payload_part( char * data, size_t size )
		{
			if( m_working_state == working_state_t::empty_state )
				throw exception_t( "current state is empty" );

			if( is_state_still_valid() )
				set_validation_state(
					validation_state_t::payload_part_is_valid );
			else
				return m_validation_state;

			for( size_t i = 0; i < size; ++i )
			{
				data[i] = static_cast<char>(process_payload_byte(
					static_cast<std::uint8_t>(data[i]) ));

				if( m_validation_state != validation_state_t::payload_part_is_valid )
					break;
			}

			return m_validation_state;
		}

		//! Make final checks of payload if it is necessary and reset state.
		validation_state_t
		finish_frame()
		{
			if( is_state_still_valid() )
				set_validation_state( validation_state_t::frame_is_valid );

			if( m_current_frame.m_final_flag )
			{
				if( !m_utf8_checker.finalized() )
					set_validation_state(
						validation_state_t::incorrect_utf8_data );

				m_utf8_checker.reset();
			}

			m_working_state = working_state_t::empty_state;

			// If continued data frame is present and current processed frame is
			// continuation frame with final bit set in 1 then reset current continued
			// data frame type.
			if( m_previous_data_frame != previous_data_frame_t::none &&
				!is_control_frame(m_current_frame.m_opcode) &&
				m_current_frame.m_final_flag )
			{
				m_previous_data_frame =
					previous_data_frame_t::none;
			}

			// Remember current frame vaidation state and return this value.
			auto this_frame_validation_state = m_validation_state;

			// Reset validation state for next frame.
			m_validation_state = validation_state_t::initial_state;

			return this_frame_validation_state;
		}

		//! Reset to initial state.
		void
		reset()
		{
			m_validation_state = validation_state_t::initial_state;
			m_working_state = working_state_t::empty_state;
			m_previous_data_frame =
				previous_data_frame_t::none;

			m_utf8_checker.reset();
		}

	private:

		//! Validate frame header.
		/*!
			\return true if current validation state is 'frame_header_is_valid' after
			all validation operations.

			\return false otherwise.
		*/
		bool
		validate_frame_header( const message_details_t & frame )
		{
			set_validation_state(
				validation_state_t::frame_header_is_valid );

			if( !is_valid_opcode( frame.m_opcode ) )
			{
				set_validation_state(
					validation_state_t::invalid_opcode );
			}
			else if( is_control_frame(frame.m_opcode) && !frame.m_final_flag )
			{
				set_validation_state(
					validation_state_t::non_final_control_frame );
			}
			else if( !frame.m_mask_flag )
			{
				set_validation_state(
					validation_state_t::empty_mask_from_client_side );
			}
			else if( frame.m_rsv1_flag != 0 ||
				frame.m_rsv2_flag != 0 ||
				frame.m_rsv3_flag != 0)
			{
				set_validation_state(
					validation_state_t::non_zero_rsv_flags );
			}
			else if( is_control_frame(frame.m_opcode) && frame.payload_len() >
				websocket_max_payload_size_without_ext )
			{
				set_validation_state(
					validation_state_t::payload_len_is_too_big );
			}

			return validation_state_t::frame_header_is_valid == m_validation_state;
		}

		//! Process payload byte.
		/*!
			Do all necessary validations with payload byte.

			\return unmasked byte if unmask flag is set.
			\return copy of original byte if unmask flag isn't set.
		*/
		std::uint8_t
		process_payload_byte( std::uint8_t byte )
		{
			byte = m_unmask_flag?
				m_unmasker.unmask_byte( byte ): byte;

			if( m_current_frame.m_opcode == opcode_t::text_frame ||
				(m_current_frame.m_opcode == opcode_t::continuation_frame &&
					m_previous_data_frame == previous_data_frame_t::text) )
			{
				if( !m_utf8_checker.process_byte( byte ) )
				{
					set_validation_state(
						validation_state_t::incorrect_utf8_data );
				}
			}
			else if( m_current_frame.m_opcode == opcode_t::connection_close_frame )
			{
				if( !m_expected_close_code.all_bytes_loaded() )
				{
					if( m_expected_close_code.add_byte_and_check_size(byte) )
					{
						uint16_t status_code{0};

						read_number_from_big_endian_bytes(
							status_code,m_expected_close_code.m_loaded_data );

						validate_close_code( status_code );
					}
				}
				else
				{
					if( !m_utf8_checker.process_byte( byte ) )
						set_validation_state(
							validation_state_t::incorrect_utf8_data );
				}
			}

			return byte;
		}

		//! Check previous frame type.
		/*!
			Need for following cases:

			1) check current frame is not continuation frame withot
			any data frame before.
			2) check current frame is not new data frame with unfinished
			 other data frame before.

			\return true if previous and current frames are without any conflicts.

			\return false otherwise.
		*/
		bool
		check_previous_frame_type( opcode_t opcode )
		{
			if( m_previous_data_frame == previous_data_frame_t::none &&
				opcode == opcode_t::continuation_frame )
			{
				set_validation_state(
					validation_state_t::continuation_frame_without_data_frame );

				return false;
			}
			else if( m_previous_data_frame !=
					previous_data_frame_t::none &&
				is_data_frame( opcode ) )
			{
				set_validation_state(
					validation_state_t::new_data_frame_without_finishing_previous );

				return false;
			}

			return true;
		}

		//! Validate close code.
		void
		validate_close_code( uint16_t close_code )
		{
			if( close_code < 1000 ||
				(close_code > 1011 && close_code < 3000) ||
				close_code > 4999 )
			{
				set_validation_state(
					validation_state_t::invalid_close_code );

				return;
			}

			if( close_code == 1004 ||
				close_code == 1005 ||
				close_code == 1006 )
			{
				set_validation_state(
					validation_state_t::invalid_close_code );

				return;
			}
		}

		//! Check validation state is still valid.
		bool
		is_state_still_valid() const
		{
			return m_validation_state == validation_state_t::initial_state ||
				m_validation_state == validation_state_t::frame_header_is_valid ||
				m_validation_state == validation_state_t::payload_part_is_valid ||
				m_validation_state == validation_state_t::frame_is_valid;
		}

		//! Try to set validation state.
		/*!
			Set validation state with new value.
		*/
		void
		set_validation_state( validation_state_t state )
		{
				m_validation_state = state;
		}

		//! Current validation state.
		/*!
			Normal case is sequence of states:
			frame_header_is_valid -> payload_part_is_valid ->  frame_is_valid.

			After set in invalid state validator will save this state until frame
			will be finished or validator will be reset.
		*/
		validation_state_t m_validation_state{
			validation_state_t::initial_state };

		//! Validator's orking states.
		enum class working_state_t
		{
			//! Waiting for new frame.
			empty_state,
			//! Frame is processing now.
			processing_frame
		};

		//! Previous unfinished data frame type.
		/*!
			It needs for understanding what kind of data is present in
			current continued frames.
		*/
		enum class previous_data_frame_t
		{
			none,
			text,
			binary
		};

		//! Working state.
		working_state_t m_working_state{ working_state_t::empty_state };

		//!  Previous unfinished data frame.
		previous_data_frame_t m_previous_data_frame{
			previous_data_frame_t::none };

		//! Current frame details.
		message_details_t m_current_frame;

		//! Buffer for accumulating 2 bytes of close code.
		expected_data_t m_expected_close_code;

		//! UTF-8 checker for text frames and close frames.
		restinio::utils::utf8_checker_t m_utf8_checker;

		//! This flag set if it's need to unmask payload parts.
		bool m_unmask_flag{ false };

		//! Unmask payload coming from client side.
		unmasker_t m_unmasker;
};

} /* namespace impl */

} /* namespace basic */

} /* namespace websocket */

} /* namespace restinio */
