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

using byte_t = unsigned char;
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
// message_details_t
//

//! TODO!
class message_details_t
{
	public:
		message_details_t()
		{}

		message_details_t( bool final, opcode_t opcode, size_t payload_len )
			:	m_final_flag{ final }
			,	m_opcode{ opcode }
		{
			init_payload_len( payload_len );
		}

		message_details_t(
			bool final,
			opcode_t opcode,
			size_t payload_len,
			std::uint32_t masking_key )
			:	m_final_flag{ final }
			,	m_opcode{ opcode }
			,	m_mask_flag( true )
			,	m_masking_key( masking_key )
		{
			init_payload_len( payload_len );
		}

		message_details_t( const message_details_t & ) = default;
		// const message_details_t & operator= ( const message_details_t & ) = default;

		// message_details_t( message_details_t && ) = delete;
		// message_details_t & operator= ( message_details_t && ) = delete;

		std::uint64_t
		payload_len() const
		{
			// 126 and 127 are codes of ext payload. 125 and lower are real payload len.
			return m_payload_len > WEBSOCKET_MAX_PAYLOAD_SIZE_WITHOUT_EXT? m_ext_payload_len: m_payload_len;
		}

		void
		set_masking_key( std::uint32_t value )
		{
			m_masking_key = value;
			m_mask_flag = true;
		}

		//

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
				// if payload greater than 2bytes-number.
				m_payload_len = payload_len > 0xFFFF ?
					WEBSOCKET_LONG_EXT_LEN_CODE:
					WEBSOCKET_SHORT_EXT_LEN_CODE;

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
	expected_data_t( size_t expected_size )
		:	m_expected_size{ expected_size }
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

template <int BYTES>
inline void
write_number_to_big_endian_bytes( std::uint64_t& number, raw_data_t & data )
{
	for( auto i = 0 ; i < BYTES ; ++i )
	{
		auto shift_value = (BYTES - i - 1) * 8;
		data.push_back( (number >> shift_value) & 0xFF );
	}
}

//
// ws_parser_t
//

//! TODO!
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
			m_current_msg = message_details_t();
			m_expected_data.reset( WEBSOCKET_FIRST_TWO_BYTES_SIZE );
		}

		const message_details_t &
		current_message() const
		{
			return m_current_msg;
		}

	private:
		expected_data_t m_expected_data{ WEBSOCKET_FIRST_TWO_BYTES_SIZE };

		message_details_t m_current_msg;

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

				case state_t::header_parsed:

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
			m_current_msg.m_final_flag = (data[0] & BIT_FLAG_7) != 0;
			m_current_msg.m_rsv1_flag = (data[0] & BIT_FLAG_6) != 0;
			m_current_msg.m_rsv2_flag = (data[0] & BIT_FLAG_5) != 0;
			m_current_msg.m_rsv3_flag = (data[0] & BIT_FLAG_4) != 0;

			m_current_msg.m_opcode = static_cast< opcode_t >( data[0] & OPCODE_MASK );

			m_current_msg.m_mask_flag = (data[1] & BIT_FLAG_7) != 0;
			m_current_msg.m_payload_len = data[1] & PAYLOAD_LEN_MASK;
		}

		void
		parse_ext_payload_len(
			std::uint8_t payload_len,
			const raw_data_t & data )
		{
			if( payload_len == WEBSOCKET_SHORT_EXT_LEN_CODE )
			{
				read_number_from_big_endian_bytes(
					m_current_msg.m_ext_payload_len, data );
			}
			else if( payload_len == WEBSOCKET_LONG_EXT_LEN_CODE )
			{
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
				read_number_from_big_endian_bytes(
					m_current_msg.m_masking_key, data );
			}
		}

};

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

inline raw_data_t
write_message_details(
	const message_details_t & message )
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

} /* namespace impl */

} /* namespace websocket */

} /* namespace restinio */
