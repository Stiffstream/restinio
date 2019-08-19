/*
	restinio
*/

/*!
	Base64 implementation.
*/

#pragma once

#include <restinio/exception.hpp>

#include <restinio/utils/impl/bitops.hpp>

#include <restinio/impl/include_fmtlib.hpp>

#include <string>
#include <bitset>
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

using bitset24_t = std::bitset<24>;

inline bool
is_base64_char( char c )
{
	return 1 == is_base64_char_lut< unsigned char >()[
			static_cast<unsigned char>(c) ];
}

inline void
check_string_is_base64( string_view_t str )
{
	auto throw_invalid_string = [&]{
			throw exception_t{
				fmt::format( "invalid base64 string '{}'", str ) };
		};

	// TODO: Handle long strings.

	if( str.size() < 4 )
		throw_invalid_string();

	for( const auto & ch : str )
	{
		if( !is_base64_char( ch ) && ch != '=' )
			throw_invalid_string();
	}
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
		uint_type_t bs =
				1u == remaining ?
				 	// only one char left.
				 	(at(i) << 16) :
					// two chars left.
					((at(i) << 16) | (at(i+1) << 8));

		result.push_back( alphabet_char( sixbits_char<18>(bs) ) );
		result.push_back( alphabet_char( sixbits_char<12>(bs) ) );

		if( (bs >> 8) & 0xFFu )
			result.push_back( alphabet_char( sixbits_char<6>(bs) ) );
		else
			result.push_back('=');

		if( bs & 0xFFu )
			result.push_back( alphabet_char( sixbits_char<0>(bs) ) );
		else
			result.push_back('=');
	}

	return result;
}

inline std::string
decode( string_view_t str )
{
	constexpr std::size_t group_size = 4;

	std::string result;

	check_string_is_base64( str );
	result.reserve( (str.size() / group_size) * 3 );

	const unsigned char * const decode_table = base64_decode_lut< unsigned char >();

	const auto at = [&str](auto index) {
		return static_cast<unsigned char>(str[index]);
	};

	for( size_t i = 0 ; i < str.size(); i += group_size)
	{

		uint_type_t bs{};

		bs |= decode_table[ at(i) ];
		bs <<= 6;
		bs |= decode_table[ at(i+1) ];
		bs <<= 6;
		bs |= str[i+2] != '=' ? decode_table[ at(i+2) ] : 0;
		bs <<= 6;
		bs |= str[i+3] != '=' ? decode_table[ at(i+3) ] : 0;


		using ::restinio::utils::impl::bitops::n_bits_from;

		result.push_back( n_bits_from< char, 16 >(bs) );
		const auto c2 = n_bits_from< char, 8 >(bs);
		if( c2 )
			result.push_back( c2 );
		const auto c3 = n_bits_from< char, 0 >(bs);
		if( c3 )
			result.push_back( c3 );
	}

	return result;
}

} /* namespace base64 */

} /* namespace utils */

} /* namespace restinio */
