/*
	restinio
*/

/*!
	Base64 implementation.
*/

#pragma once

#include <string>
#include <bitset>
#include <array>
#include <exception>
#include <iostream> // std::cout, debug

namespace restinio
{

namespace impl
{

namespace base64
{

using uint_type_t = std::uint_fast32_t;

using bitset24_t = std::bitset<24>;

const std::string BASE64_ALPHABET =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

inline bool
is_base64_char( unsigned char c )
{
	return std::isalnum(c) || c == '+' || c == '/';
}

inline void
check_string_is_base64( const std::string & str )
{
	if( str.size() < 4 )
		throw std::runtime_error("Invalid base64 string '" + str + "'.");

	for( const auto & ch : str )
	{
		if( !is_base64_char( ch ) && ch != '=' )
			throw std::runtime_error("Invalid base64 string '" + str + "'.");
	}
}

constexpr uint_type_t last_six_bits_mask = 0x3f;

inline uint_type_t
uch( char ch )
{
	return static_cast<uint_type_t>(static_cast<unsigned char>(ch));
}

template<unsigned int SHIFT>
char
rshift_then_extract( uint_type_t bs )
{
	return static_cast<char>((bs >> SHIFT) & last_six_bits_mask);
}

inline std::string
encode( const std::string & str )
{
	std::string result;

	const auto at = [&str](auto index) { return uch(str[index]); };

	const std::size_t group_size = 3u;
	const auto remaining = str.size() % group_size;

	result.reserve( (str.size()/group_size + (remaining ? 1:0)) * 4 );

	std::size_t i = 0;
	for(; i < str.size() - remaining; i += group_size )
	{
		uint_type_t bs = (at(i) << 16) | (at(i+1) << 8) | at(i+2);

		result.push_back( BASE64_ALPHABET[ rshift_then_extract<18>(bs) ] );
		result.push_back( BASE64_ALPHABET[ rshift_then_extract<12>(bs) ] );
		result.push_back( BASE64_ALPHABET[ rshift_then_extract<6>(bs) ] );
		result.push_back( BASE64_ALPHABET[ rshift_then_extract<0>(bs) ] );
	}

	if( remaining )
	{
		uint_type_t bs = 
				1u == remaining ?
				 	// only one char left.
				 	(at(i) << 16) :
					// two chars left.
					((at(i) << 16) | (at(i+1) << 8));

		result.push_back( BASE64_ALPHABET[ rshift_then_extract<18>(bs) ] );
		result.push_back( BASE64_ALPHABET[ rshift_then_extract<12>(bs) ] );

		if( (bs >> 8) & 0xFFu )
			result.push_back( BASE64_ALPHABET[ rshift_then_extract<6>(bs) ] );
		else
			result.push_back('=');

		if( bs & 0xFFu )
			result.push_back( BASE64_ALPHABET[ rshift_then_extract<0>(bs) ] );
		else
			result.push_back('=');
	}

	return result;
}

inline std::string
decode( const std::string & str )
{
	std::string result;

	check_string_is_base64( str );

	for( size_t i = 0 ; i < str.size()  ; i += 4)
	{
		bitset24_t bs;

		bs |= BASE64_ALPHABET.find_first_of(str[i]);
		bs <<= 6;
		bs |= BASE64_ALPHABET.find_first_of(str[i+1]);
		bs <<= 6;
		bs |= str[i+2] != '=' ? BASE64_ALPHABET.find_first_of(str[i+2]) : 0;
		bs <<= 6;
		bs |= str[i+3] != '=' ? BASE64_ALPHABET.find_first_of(str[i+3]) : 0;

		result.push_back( (bs >> 16).to_ulong() & 0xFF );
		if( (bs >> 8).to_ulong() & 0xFF )
			result.push_back( (bs >> 8).to_ulong() & 0xFF );
		if( (bs).to_ulong() & 0xFF )
		result.push_back( (bs).to_ulong() & 0xFF );
	}

	return result;
}

} /* namespace base64 */

} /* namespace impl */

} /* namespace restinio */
