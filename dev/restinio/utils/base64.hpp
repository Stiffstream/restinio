/*
	restinio
*/

/*!
	Base64 implementation.
*/

#pragma once

#include <restinio/exception.hpp>
#include <restinio/expected.hpp>

#include <restinio/utils/impl/bitops.hpp>

#include <restinio/impl/include_fmtlib.hpp>

#include <string>
#include <array>
#include <exception>
#include <iostream> // std::cout, debug

namespace restinio
{

namespace utils
{

namespace base64
{

#include "base64_lut.ipp"

using uint_type_t = std::uint_fast32_t;

inline bool
is_base64_char( char c ) noexcept
{
	return 1 == is_base64_char_lut< unsigned char >()[
			static_cast<unsigned char>(c) ];
}

inline bool
is_valid_base64_string( string_view_t str ) noexcept
{
	enum class expected_type { b64ch, b64ch_or_padding, padding };

	if( str.size() < 4u )
		return false;

	expected_type expects = expected_type::b64ch;
	std::uint_fast8_t b64chars_found = 0u; // Can't be greater than 2.
	std::uint_fast8_t paddings_found = 0u; // Can't be greater than 3.
	for( const auto ch : str )
	{
		switch( expects )
		{
			case expected_type::b64ch:
			{
				// Because '=' is a part of base64_chars, it should be checked
				// individually.
				if( '=' == ch )
					return false;
				else if( is_base64_char( ch ) )
				{
					++b64chars_found;
					if( b64chars_found >= 2u )
						expects = expected_type::b64ch_or_padding;
				}
				else
					return false;
				break;
			}
			case expected_type::b64ch_or_padding:
			{
				if( '=' == ch )
				{
					expects = expected_type::padding;
					++paddings_found;
				}
				else if( is_base64_char( ch ) )
				{
					/* Nothing to do */
				}
				else
					return false;

				break;
			}
			case expected_type::padding:
			{
				if( '=' == ch )
				{
					++paddings_found;
					if( paddings_found > 2u )
						return false;
				}
				else
					return false;

				break;
			}
		}
	}

	return true;
}

inline uint_type_t
uch( char ch )
{
	return static_cast<uint_type_t>(static_cast<unsigned char>(ch));
}

template<unsigned int Shift>
char
sixbits_char( uint_type_t bs )
{
	return ::restinio::utils::impl::bitops::n_bits_from< char, Shift, 6 >(bs);
}

inline std::string
encode( string_view_t str )
{
	std::string result;

	const auto at = [&str](auto index) { return uch(str[index]); };

	const auto alphabet_char = [](auto ch) {
		return static_cast<char>(
				base64_alphabet< unsigned char >()[
						static_cast<unsigned char>(ch) ]);
	};

	constexpr std::size_t group_size = 3u;
	const auto remaining = str.size() % group_size;

	result.reserve( (str.size()/group_size + (remaining ? 1:0)) * 4 );

	std::size_t i = 0;
	for(; i < str.size() - remaining; i += group_size )
	{
		uint_type_t bs = (at(i) << 16) | (at(i+1) << 8) | at(i+2);

		result.push_back( alphabet_char( sixbits_char<18>(bs) ) );
		result.push_back( alphabet_char( sixbits_char<12>(bs) ) );
		result.push_back( alphabet_char( sixbits_char<6>(bs) ) );
		result.push_back( alphabet_char( sixbits_char<0>(bs) ) );
	}

	if( remaining )
	{
		// Some code duplication to avoid additional IFs.
		if( 1u == remaining )
		{
			uint_type_t bs = (at(i) << 16);
			result.push_back( alphabet_char( sixbits_char<18>(bs) ) );
			result.push_back( alphabet_char( sixbits_char<12>(bs) ) );

			result.push_back('=');
		}
		else
		{
			uint_type_t bs = (at(i) << 16) | (at(i+1) << 8);

			result.push_back( alphabet_char( sixbits_char<18>(bs) ) );
			result.push_back( alphabet_char( sixbits_char<12>(bs) ) );
			result.push_back( alphabet_char( sixbits_char<6>(bs) ) );
		}

		result.push_back('=');
	}

	return result;
}

//! Description of base64 decode error.
enum class decoding_error_t
{
	invalid_base64_sequence
};

inline expected_t< std::string, decoding_error_t >
try_decode( string_view_t str )
{
	if( !is_valid_base64_string( str ) )
		return make_unexpected( decoding_error_t::invalid_base64_sequence );

	constexpr std::size_t group_size = 4;

	std::string result;
	result.reserve( (str.size() / group_size) * 3 );

	const unsigned char * const decode_table = base64_decode_lut< unsigned char >();

	const auto at = [&str](auto index) {
		return static_cast<unsigned char>(str[index]);
	};

	for( size_t i = 0 ; i < str.size(); i += group_size)
	{

		uint_type_t bs{};
		int paddings_found = 0u;

		bs |= decode_table[ at(i) ];

		bs <<= 6;
		bs |= decode_table[ at(i+1) ];

		bs <<= 6;
		if( '=' == str[i+2] )
		{
			++paddings_found;
		}
		else
		{
			bs |= decode_table[ at(i+2) ];
		}

		bs <<= 6;
		if( '=' == str[i+3] )
		{
			++paddings_found;
		}
		else
		{
			bs |= decode_table[ at(i+3) ];
		}

		using ::restinio::utils::impl::bitops::n_bits_from;

		result.push_back( n_bits_from< char, 16 >(bs) );
		if( paddings_found < 2 )
		{
			result.push_back( n_bits_from< char, 8 >(bs) );
		}
		if( paddings_found < 1 )
		{
			result.push_back( n_bits_from< char, 0 >(bs) );
		}
	}

	return result;
}

namespace impl
{

inline void
throw_exception_on_invalid_base64_string( string_view_t str )
{
	constexpr size_t max_allowed_len = 32u;
	// If str is too long only a part of it will be included
	// in the error message.
	if( str.size() > max_allowed_len )
		throw exception_t{
				fmt::format( "invalid base64 string that starts with '{}'",
						str.substr( 0u, max_allowed_len ) )
		};
	else
		throw exception_t{
			fmt::format( "invalid base64 string '{}'", str ) };
}

} /* namespace impl */

inline std::string
decode( string_view_t str )
{
	auto result = try_decode( str );
	if( !result )
		impl::throw_exception_on_invalid_base64_string( str );

	return std::move( *result );
}

} /* namespace base64 */

} /* namespace utils */

} /* namespace restinio */

