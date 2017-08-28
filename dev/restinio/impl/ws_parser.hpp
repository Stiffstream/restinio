/*
	restinio
*/

/*!
	Websocket.
*/

#pragma once

#include <cstdint>
#include <vector>
#include <list>
#include <stdexcept>

#include <restinio/exception.hpp>
#include <restinio/ws_message.hpp>

namespace restinio
{

using byte_t = char;
using raw_data_t = std::string;

namespace impl
{

constexpr size_t WEBSOCKET_FIRST_TWO_BYTES_SIZE = 2;
constexpr size_t WEBSOCKET_MAX_PAYLOAD_SIZE_WITHOUT_EXT = 125;
constexpr size_t WEBSOCKET_SHORT_EXT_PAYLOAD_LENGTH = 2;
constexpr size_t WEBSOCKET_LONG_EXT_PAYLOAD_LENGTH = 8;
constexpr size_t WEBSOCKET_SHORT_EXT_LEN_CODE = 126;
constexpr size_t WEBSOCKET_LONG_EXT_LEN_CODE = 127;
constexpr size_t WEBSOCKET_MASKING_KEY_SIZE = 4;

constexpr byte_t BIT_FLAG_7 = 0x80;
constexpr byte_t BIT_FLAG_6 = 0x40;
constexpr byte_t BIT_FLAG_5 = 0x20;
constexpr byte_t BIT_FLAG_4 = 0x10;
constexpr byte_t OPCODE_MASK = 0x0F;
constexpr byte_t PAYLOAD_LEN_MASK = 0x7F;

//
// ws_message_details_t
//

class ws_message_details_t
{

	public:

		ws_message_details_t()
		{
		}

		ws_message_details_t( bool final, opcode_t opcode, size_t payload_len )
		:	m_final_flag{ final }
		,	m_opcode{ opcode }
		{
			init_payload_len( payload_len );
		}

		ws_message_details_t(
			bool final, opcode_t opcode, size_t payload_len, size_t masking_key )
		:	m_final_flag{ final }
		,	m_opcode{ opcode }
		,	m_mask_flag( true )
		,	m_masking_key( masking_key )
		{
			init_payload_len( payload_len );
		}

		std::uint64_t
		payload_len() const
		{
			// 126 and 127 are codes of ext payload. 125 and lower are real payload len.
			return m_payload_len > 125? m_ext_payload_len: m_payload_len;
		}

		void
		set_masking_key( std::uint32_t value )
		{
			m_masking_key = value;
			m_mask_flag = true;
		}

		ws_message_header_t
		transform_to_header() const
		{
			return ws_message_header_t{
				m_final_flag, m_opcode, payload_len(), m_masking_key };
		}

		bool m_final_flag = true;

		bool m_rsv1_flag = false;
		bool m_rsv2_flag = false;
		bool m_rsv3_flag = false;

		opcode_t m_opcode = opcode_t::continuation_frame;

		bool m_mask_flag = false;

		std::uint8_t m_payload_len = 0;

		std::uint64_t m_ext_payload_len = 0;

		std::uint32_t m_masking_key = 0;

	private:

		void
		init_payload_len( size_t payload_len )
		{
			if( payload_len > WEBSOCKET_MAX_PAYLOAD_SIZE_WITHOUT_EXT )
			{
				// if payload greater than 2bytes-number
				m_payload_len = payload_len > 0xFFFF ?
					WEBSOCKET_LONG_EXT_LEN_CODE:
					WEBSOCKET_SHORT_EXT_LEN_CODE;

				m_ext_payload_len = payload_len;
			}
			else
			{
				m_payload_len = payload_len;
			}
		}

};

//! Data with expected size.
struct expected_data_t
{
	expected_data_t( size_t expected_size )
		: m_expected_size{ expected_size }
	{
		m_loaded_data.reserve( m_expected_size );
	}

	size_t m_expected_size{0};
	raw_data_t m_loaded_data;

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

		m_loaded_data.push_back(byte);

		return m_loaded_data.size() == m_expected_size;
	}

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

template <typename T>
inline void
read_number_from_big_endian_bytes( T & number, const raw_data_t & data )
{
	if( data.empty() )
		return;

	for( size_t i = 0 ; i < data.size() ; ++i )
	{
		std::uint8_t byte = data[i];
		auto shift_value = (data.size() - i - 1) * 8;
		number |= ( static_cast<T>(byte) ) << shift_value;
	}
}

//
// write_number_to_big_endian_bytes
//

template <int BYTES>
inline void
write_number_to_big_endian_bytes( std::uint64_t& number, raw_data_t & data )
{
	for( auto i = 0 ; i < BYTES ; ++i )
	{
		std::uint8_t byte = data[i];
		auto shift_value = (BYTES - i - 1) * 8;
		data.push_back( (number >> shift_value) & 0xFF );
	}
}

//
// ws_parser_t
//

class ws_parser_t
{
	public:

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

		bool
		header_parsed() const
		{
			return m_current_state == state_t::header_parsed;
		}

		void
		reset()
		{
			m_current_state = state_t::waiting_for_first_2_bytes;
			m_current_msg = ws_message_details_t();
			m_expected_data.reset( WEBSOCKET_FIRST_TWO_BYTES_SIZE );
		}

		const ws_message_details_t &
		current_message() const
		{
			return m_current_msg;
		}

	private:

		impl::expected_data_t m_expected_data{ WEBSOCKET_FIRST_TWO_BYTES_SIZE };

		ws_message_details_t m_current_msg;

		enum class state_t
		{
			waiting_for_first_2_bytes,
			waiting_for_ext_len,
			waiting_for_mask_key,
			header_parsed
		};

		state_t m_current_state = state_t::waiting_for_first_2_bytes;

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
				}
			}
		}

		void
		process_first_2_bytes()
		{
			parse_first_2_bytes(
				m_expected_data.m_loaded_data );

			size_t payload_len = m_current_msg.m_payload_len;

			if( payload_len > WEBSOCKET_MAX_PAYLOAD_SIZE_WITHOUT_EXT )
			{
				size_t expected_data_size = payload_len == WEBSOCKET_SHORT_EXT_LEN_CODE?
					WEBSOCKET_SHORT_EXT_PAYLOAD_LENGTH:
					WEBSOCKET_LONG_EXT_PAYLOAD_LENGTH;

				m_expected_data.reset( expected_data_size );

				m_current_state = state_t::waiting_for_ext_len;
			}
			else if( m_current_msg.m_mask_flag )
			{
				size_t expected_data_size = WEBSOCKET_MASKING_KEY_SIZE;
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

		void
		process_extended_length()
		{
			parse_ext_payload_len(
				m_current_msg.m_payload_len,
				m_expected_data.m_loaded_data );

			if( m_current_msg.m_mask_flag )
			{
				size_t expected_data_size = WEBSOCKET_MASKING_KEY_SIZE;
				m_expected_data.reset( expected_data_size );

				m_current_state = state_t::waiting_for_mask_key;
			}
			else
			{
				m_current_state = state_t::header_parsed;
			}
		}

		void
		process_masking_key()
		{
			parse_masking_key(
				m_current_msg.m_mask_flag,
				m_expected_data.m_loaded_data );

			m_current_state = state_t::header_parsed;
		}

		void
		parse_first_2_bytes(
			const raw_data_t & data )
		{
			if( data.size() != 2 )
				throw exception_t( "Incorrect size of raw data: 2 bytes expected." );

			m_current_msg.m_final_flag = data[0] & BIT_FLAG_7;
			m_current_msg.m_rsv1_flag = data[0] & BIT_FLAG_6;
			m_current_msg.m_rsv2_flag = data[0] & BIT_FLAG_5;
			m_current_msg.m_rsv3_flag = data[0] & BIT_FLAG_4;

			m_current_msg.m_opcode = static_cast< opcode_t >( data[0] & OPCODE_MASK );

			m_current_msg.m_mask_flag = data[1] & BIT_FLAG_7;
			m_current_msg.m_payload_len = data[1] & PAYLOAD_LEN_MASK;
		}

		void
		parse_ext_payload_len(
			std::uint8_t payload_len,
			const raw_data_t & data )
		{
			if( payload_len == WEBSOCKET_SHORT_EXT_LEN_CODE )
			{
				if( data.size() != 2 )
					throw exception_t(
						"Incorrect size of raw data: 2 bytes expected." );

				read_number_from_big_endian_bytes(
					m_current_msg.m_ext_payload_len, data );
			}
			else if( payload_len == WEBSOCKET_LONG_EXT_LEN_CODE )
			{
				if( data.size() != 8 )
					throw exception_t(
						"Incorrect size of raw data: 8 bytes expected." );

				auto left_shift_bytes = []( byte_t byte, size_t shift_count )
				{
					return static_cast<std::uint64_t>(byte) << shift_count;
				};

				read_number_from_big_endian_bytes(
					m_current_msg.m_ext_payload_len, data );
			}
		}

		void
		parse_masking_key(
			bool mask_flag,
			const raw_data_t & data )
		{
			if( mask_flag )
			{
				if( data.size() != 4 )
					throw exception_t(
						"Incorrect size of raw data: 4 bytes expected." );

				auto left_shift_bytes = []( byte_t byte, size_t shift_count )
				{
					return static_cast<std::uint32_t>(byte) << shift_count;
				};

				read_number_from_big_endian_bytes(
					m_current_msg.m_masking_key, data );
			}
		}

};

inline void
mask_unmask_payload( std::uint32_t masking_key, raw_data_t & payload )
{
	const auto MASK_SIZE = 4;
	uint8_t mask[ MASK_SIZE ] = { };

	for( auto i = 0; i < MASK_SIZE; ++i )
	{
		auto shift_value = ( MASK_SIZE - i - 1 )* 8;
		mask[i] = ( masking_key >>  shift_value ) & 0xFF;
	}

	for ( size_t index = 0; index < payload.size( ); index++ )
	{
		payload[ index ] ^= mask[ index % MASK_SIZE ];
	}
}

inline raw_data_t
write_message_details(
	const ws_message_details_t & message )
{
	raw_data_t result;

	byte_t byte = 0x00;

	if( message.m_final_flag ) byte |= BIT_FLAG_7;
	if( message.m_rsv1_flag ) byte |= BIT_FLAG_6;
	if( message.m_rsv2_flag ) byte |= BIT_FLAG_5;
	if( message.m_rsv3_flag ) byte |= BIT_FLAG_4;

	byte |= static_cast< std::uint8_t> (message.m_opcode) & OPCODE_MASK;

	result.push_back( byte );

	byte = 0x00;

	if( message.m_mask_flag )
		byte |= BIT_FLAG_7;

	auto length = message.m_payload_len;

	if( length < WEBSOCKET_SHORT_EXT_LEN_CODE )
	{
		byte |= length;
		result.push_back( byte );
	}
	else if ( length == WEBSOCKET_SHORT_EXT_LEN_CODE )
	{
		byte |= WEBSOCKET_SHORT_EXT_LEN_CODE;

		result.push_back( byte );

		auto ext_len = message.m_ext_payload_len;

		write_number_to_big_endian_bytes< WEBSOCKET_SHORT_EXT_PAYLOAD_LENGTH>(
			ext_len, result );
	}
	else if ( length == WEBSOCKET_LONG_EXT_LEN_CODE )
	{
		byte |= WEBSOCKET_LONG_EXT_LEN_CODE;

		result.push_back( byte );

		auto ext_len = message.m_ext_payload_len;

		write_number_to_big_endian_bytes< WEBSOCKET_LONG_EXT_PAYLOAD_LENGTH >(
			ext_len, result );
	}

	if( message.m_mask_flag )
	{
		auto masking_key = message.m_masking_key;
		const auto MASK_SIZE = 4;
		uint8_t mask[ MASK_SIZE ] = { };

		for( auto i = 0; i < MASK_SIZE; ++i )
		{
			auto shift_value = i * 8;
			mask[i] = ( masking_key >>  shift_value ) & 0xFF;
		}

		result.push_back( mask[ 3 ] );
		result.push_back( mask[ 2 ] );
		result.push_back( mask[ 1 ] );
		result.push_back( mask[ 0 ] );
	}

	return result;
}

} /* namespace impl */

} /* namespace restinio */
