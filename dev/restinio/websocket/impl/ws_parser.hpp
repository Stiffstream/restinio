/*
	restinio
*/

/*!
	Websocket.
*/

#pragma once

#include <restinio/exception.hpp>
#include <restinio/websocket/message.hpp>

#include <restinio/utils/impl/bitops.hpp>

#include <cstdint>
#include <vector>
#include <list>
#include <stdexcept>

namespace restinio
{

namespace websocket
{

namespace basic
{

//! Alias for byte.
using byte_t = unsigned char;

//! Bytes buffer.
using raw_data_t = std::string;

namespace impl
{

//! Websocket parser constants.
//! \{
constexpr size_t websocket_first_two_bytes_size = 2;
constexpr size_t websocket_max_payload_size_without_ext = 125;
constexpr size_t websocket_short_ext_payload_length = 2;
constexpr size_t websocket_long_ext_payload_length = 8;
constexpr size_t websocket_short_ext_len_code = 126;
constexpr size_t websocket_long_ext_len_code = 127;
constexpr size_t websocket_masking_key_size = 4;

constexpr byte_t bit_flag_7 = 0x80;
constexpr byte_t bit_flag_6 = 0x40;
constexpr byte_t bit_flag_5 = 0x20;
constexpr byte_t bit_flag_4 = 0x10;
constexpr byte_t opcode_mask = 0x0F;
constexpr byte_t payload_len_mask = 0x7F;
//! \}

//
// message_details_t
//

//! Websocket message class with more detailed protocol information.
class message_details_t
{
	public:
		message_details_t() = default;

		message_details_t(
			final_frame_flag_t final_flag,
			opcode_t opcode,
			size_t payload_len ) noexcept
			:	m_final_flag{ final_frame == final_flag }
			,	m_opcode{ opcode }
		{
			init_payload_len( payload_len );
		}

		message_details_t(
			final_frame_flag_t final_flag,
			opcode_t opcode,
			size_t payload_len,
			std::uint32_t masking_key )
			:	m_final_flag{ final_frame == final_flag }
			,	m_opcode{ opcode }
			,	m_mask_flag( true )
			,	m_masking_key( masking_key )
		{
			init_payload_len( payload_len );
		}

		//! Get payload len.
		std::uint64_t
		payload_len() const
		{
			// 126 and 127 are codes of ext payload. 125 and lower are real payload len.
			return m_payload_len > websocket_max_payload_size_without_ext? m_ext_payload_len: m_payload_len;
		}

		//! Set masking key.
		void
		set_masking_key( std::uint32_t value )
		{
			m_masking_key = value;
			m_mask_flag = true;
		}

		//! Final flag.
		bool m_final_flag = true;

		//! Reserved flags.
		//! \{
		bool m_rsv1_flag = false;
		bool m_rsv2_flag = false;
		bool m_rsv3_flag = false;
		//! \}

		//! Opcode.
		opcode_t m_opcode = opcode_t::continuation_frame;

		//! Mask flag.
		bool m_mask_flag = false;

		//! Payload len.
		/*!
			It contains payload len or ext payload len code.
		*/
		std::uint8_t m_payload_len = 0;

		//! Ext payload len.
		std::uint64_t m_ext_payload_len = 0;

		//! Masking key.
		std::uint32_t m_masking_key = 0;

	private:

		//! Initialize payload len.
		/*!
			Set only payload length if value is lower than 126 or payload length
			and ext payload length otherwise.
		*/
		void
		init_payload_len( size_t payload_len )
		{
			if( payload_len > websocket_max_payload_size_without_ext )
			{
				// if payload greater than 2bytes-number.
				m_payload_len = payload_len > 0xFFFF ?
					websocket_long_ext_len_code:
					websocket_short_ext_len_code;

				m_ext_payload_len = payload_len;
			}
			else
			{
				m_payload_len = static_cast< std::uint8_t >( payload_len );
			}
		}
};

//
// expected_data_t
//

//! Data with expected size.
struct expected_data_t
{
	expected_data_t() = default;

	expected_data_t( size_t expected_size )
		:	m_expected_size{ expected_size }
	{
		m_loaded_data.reserve( m_expected_size );
	}

	//! Expected data size in bytes.
	size_t m_expected_size{0};

	//! Buffer for accumulating data.
	raw_data_t m_loaded_data;

	//! Check all bytes are loaded.
	bool
	all_bytes_loaded() const
	{
		return m_loaded_data.size() == m_expected_size;
	}

	//! Try to add one more byte to loaded data and check loaded data size.
	/*!
		\return true if loaded data size equals expected size.
		\return false otherwise.
	*/
	bool
	add_byte_and_check_size( byte_t byte )
	{
		if( m_loaded_data.size() == m_expected_size )
			throw exception_t("Cannot add one more bytes to expected data.");

		m_loaded_data.push_back(static_cast<raw_data_t::value_type>(byte));

		return all_bytes_loaded();
	}

	//! Reset internal state on next expected data size.
	void
	reset( size_t expected_size )
	{
		m_expected_size = expected_size;
		m_loaded_data.clear();
		m_loaded_data.reserve( expected_size );
	}
};

//
// read_number_from_big_endian_bytes
//

//! Read number from buffer with network bytes order.
template <typename T>
inline void
read_number_from_big_endian_bytes( T & number, const raw_data_t & data )
{
	number = T{};
	for( const auto byte: data )
	{
		number <<= 8;
		number |= static_cast<std::uint8_t>( byte );
	}
}

//
// write_number_to_big_endian_bytes
//

//! Save number to buffer with network bytes order.
template <int Bytes>
inline void
write_number_to_big_endian_bytes( std::uint64_t& number, raw_data_t & data )
{
	for( auto i = 0 ; i < Bytes ; ++i )
	{
		auto shift_value = (Bytes - i - 1) * 8;
		data.push_back( static_cast<raw_data_t::value_type>(
				(number >> shift_value) & 0xFF) );
	}
}

//
// ws_parser_t
//

//! Websocket parser.
/*!
	This class can parse message from binary buffer.

	It is not necessary to have all buffer before parsing. Parser can process
	pieces of data and save intermediate state.
*/
class ws_parser_t
{
	public:

		//! Parse piece of data from buffer.
		size_t
		parser_execute( const char * data, size_t size )
		{
			size_t parsed_bytes = 0;

			while( parsed_bytes < size &&
				m_current_state != state_t::header_parsed )
			{
				byte_t byte = static_cast< byte_t >( data[parsed_bytes] );

				process_byte( byte );

				parsed_bytes++;
			}

			return parsed_bytes;
		}

		//! Check header of current websocket message is parsed.
		bool
		header_parsed() const
		{
			return m_current_state == state_t::header_parsed;
		}

		//! Reset internal state.
		/*!
			Need to call this function before processing next message.
		*/
		void
		reset()
		{
			m_current_state = state_t::waiting_for_first_2_bytes;
			m_current_msg = message_details_t();
			m_expected_data.reset( websocket_first_two_bytes_size );
		}

		//! Get current mesasge details.
		const message_details_t &
		current_message() const
		{
			return m_current_msg;
		}

	private:

		//! Buffer for parts of websocket message with known size.
		/*!
			Default value is first 2 bytes for flags and opcode.
		*/
		expected_data_t m_expected_data{ websocket_first_two_bytes_size };

		//! Current websocket message details.
		message_details_t m_current_msg;

		//! Internal state.
		enum class state_t
		{
			waiting_for_first_2_bytes,
			waiting_for_ext_len,
			waiting_for_mask_key,
			header_parsed
		};

		//! Current state.
		state_t m_current_state = state_t::waiting_for_first_2_bytes;

		//! Process one byte of incoming buffer.
		void
		process_byte( byte_t byte )
		{
			if( m_expected_data.add_byte_and_check_size(byte) )
			{
				switch( m_current_state )
				{

				case state_t::waiting_for_first_2_bytes:

					process_first_2_bytes();
					break;

				case state_t::waiting_for_ext_len:

					process_extended_length();
					break;

				case state_t::waiting_for_mask_key:

					process_masking_key();
					break;

				case state_t::header_parsed:

					break;
				}
			}
		}

		//! Process first two bytes of message.
		/*!
			Parse flags, opcode, payload length and set new state.
		*/
		void
		process_first_2_bytes()
		{
			parse_first_2_bytes(
				m_expected_data.m_loaded_data );

			size_t payload_len = m_current_msg.m_payload_len;

			if( payload_len > websocket_max_payload_size_without_ext )
			{
				size_t expected_data_size = payload_len == websocket_short_ext_len_code?
					websocket_short_ext_payload_length:
					websocket_long_ext_payload_length;

				m_expected_data.reset( expected_data_size );

				m_current_state = state_t::waiting_for_ext_len;
			}
			else if( m_current_msg.m_mask_flag )
			{
				size_t expected_data_size = websocket_masking_key_size;
				m_expected_data.reset( expected_data_size );

				m_current_state = state_t::waiting_for_mask_key;
			}
			else
			{
				size_t expected_data_size = payload_len;
				m_expected_data.reset( expected_data_size );

				m_current_state = state_t::header_parsed;
			}
		}

		//! Process extended length.
		/*!
			Parse extended length and set new state.
		*/
		void
		process_extended_length()
		{
			parse_ext_payload_len(
				m_current_msg.m_payload_len,
				m_expected_data.m_loaded_data );

			if( m_current_msg.m_mask_flag )
			{
				size_t expected_data_size = websocket_masking_key_size;
				m_expected_data.reset( expected_data_size );

				m_current_state = state_t::waiting_for_mask_key;
			}
			else
			{
				m_current_state = state_t::header_parsed;
			}
		}

		void
		//! Process extended length.
		/*!
			Parse masking key and set new state.
		*/
		process_masking_key()
		{
			parse_masking_key(
				m_current_msg.m_mask_flag,
				m_expected_data.m_loaded_data );

			m_current_state = state_t::header_parsed;
		}

		//! Parse first two bytes of message from buffer.
		void
		parse_first_2_bytes(
			const raw_data_t & data )
		{
			m_current_msg.m_final_flag = (data[0] & bit_flag_7) != 0;
			m_current_msg.m_rsv1_flag = (data[0] & bit_flag_6) != 0;
			m_current_msg.m_rsv2_flag = (data[0] & bit_flag_5) != 0;
			m_current_msg.m_rsv3_flag = (data[0] & bit_flag_4) != 0;

			m_current_msg.m_opcode = static_cast< opcode_t >( data[0] & opcode_mask );

			m_current_msg.m_mask_flag = (data[1] & bit_flag_7) != 0;
			m_current_msg.m_payload_len = data[1] & payload_len_mask;
		}

		//! Parse extended length from buffer.
		void
		parse_ext_payload_len(
			std::uint8_t payload_len,
			const raw_data_t & data )
		{
			if( payload_len == websocket_short_ext_len_code )
			{
				read_number_from_big_endian_bytes(
					m_current_msg.m_ext_payload_len, data );
			}
			else if( payload_len == websocket_long_ext_len_code )
			{
				read_number_from_big_endian_bytes(
					m_current_msg.m_ext_payload_len, data );
			}
		}

		//! Parse masking key from buffer.
		void
		parse_masking_key(
			bool mask_flag,
			const raw_data_t & data )
		{
			if( mask_flag )
			{
				read_number_from_big_endian_bytes(
					m_current_msg.m_masking_key, data );
			}
		}
};

//! Do msak/unmask operation with buffer.
inline void
mask_unmask_payload( std::uint32_t masking_key, raw_data_t & payload )
{
	using namespace ::restinio::utils::impl::bitops;

	const std::size_t MASK_SIZE = 4;
	const uint8_t mask[ MASK_SIZE ] = {
		n_bits_from< std::uint8_t, 24 >(masking_key),
		n_bits_from< std::uint8_t, 16 >(masking_key),
		n_bits_from< std::uint8_t, 8 >(masking_key),
		n_bits_from< std::uint8_t, 0 >(masking_key),
	};

	const auto payload_size = payload.size();
	for( std::size_t i = 0; i < payload_size; )
	{
		for( std::size_t j = 0; j < MASK_SIZE && i < payload_size; ++j, ++i )
		{
			payload[ i ] ^= mask[ j ];
		}
	}
}

//! Serialize websocket message details into bytes buffer.
/*!
	\return buffer with written websocket message.
*/
inline raw_data_t
write_message_details( const message_details_t & message )
{
	raw_data_t result;

	byte_t byte = 0x00;

	if( message.m_final_flag ) byte |= bit_flag_7;
	if( message.m_rsv1_flag ) byte |= bit_flag_6;
	if( message.m_rsv2_flag ) byte |= bit_flag_5;
	if( message.m_rsv3_flag ) byte |= bit_flag_4;

	byte |= static_cast< std::uint8_t> (message.m_opcode) & opcode_mask;

	result.push_back( static_cast<raw_data_t::value_type>(byte) );

	byte = 0x00;

	if( message.m_mask_flag )
		byte |= bit_flag_7;

	auto length = message.m_payload_len;

	if( length < websocket_short_ext_len_code )
	{
		byte |= length;
		result.push_back( static_cast<raw_data_t::value_type>(byte) );
	}
	else if ( length == websocket_short_ext_len_code )
	{
		byte |= websocket_short_ext_len_code;

		result.push_back( static_cast<raw_data_t::value_type>(byte) );

		auto ext_len = message.m_ext_payload_len;

		write_number_to_big_endian_bytes< websocket_short_ext_payload_length>(
			ext_len, result );
	}
	else if ( length == websocket_long_ext_len_code )
	{
		byte |= websocket_long_ext_len_code;

		result.push_back( static_cast<raw_data_t::value_type>(byte) );

		auto ext_len = message.m_ext_payload_len;

		write_number_to_big_endian_bytes< websocket_long_ext_payload_length >(
			ext_len, result );
	}

	if( message.m_mask_flag )
	{
		using namespace ::restinio::utils::impl::bitops;

		using ch_type = raw_data_t::value_type;

		const auto key = message.m_masking_key;
		result.push_back( n_bits_from< ch_type, 24 >(key) );
		result.push_back( n_bits_from< ch_type, 16 >(key) );
		result.push_back( n_bits_from< ch_type, 8 >(key) );
		result.push_back( n_bits_from< ch_type, 0 >(key) );
	}

	return result;
}

//! Serialize websocket message details into bytes buffer.
/*!
	\return buffer with written websocket message.
*/
inline raw_data_t
write_message_details(
	final_frame_flag_t final_flag,
	opcode_t opcode,
	size_t payload_len )
{
	return write_message_details(
			message_details_t{ final_flag, opcode, payload_len } );
}

//! Serialize websocket message details into bytes buffer.
/*!
	\return buffer with written websocket message.
*/
inline raw_data_t
write_message_details(
	final_frame_flag_t final_flag,
	opcode_t opcode,
	size_t payload_len,
	std::uint32_t masking_key )
{
	return write_message_details(
			message_details_t{ final_flag, opcode, payload_len, masking_key } );
}

} /* namespace impl */

} /* namespace basic */

} /* namespace websocket */

} /* namespace restinio */
