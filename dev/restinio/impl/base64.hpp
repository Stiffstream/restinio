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

using bitset8_t = std::bitset<8>;
using bitset6_t = std::bitset<6>;
using bitset24_t = std::bitset<24>;

const std::string base64_alphabet =
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
	for( const auto & ch : str )
	{
		if( !is_base64_char( ch ) && ch != '=' )
			throw std::runtime_error("Invalid base64 string '" + str + "'.");
	}
}

// TODO: refactoring.
inline std::string
base64_encode( const std::string & str )
{
	std::string result;

	for( auto i = 0 ; i < str.size() - str.size()%3 ; i += 3)
	{
		bitset24_t bs;

		bs |= str[i];
		bs <<= 8;
		bs |= str[i+1];
		bs <<= 8;
		bs |= str[i+2];

		result.push_back( base64_alphabet[ (bs >> 18).to_ulong() & 0x3F ] );
		result.push_back( base64_alphabet[ (bs >> 12).to_ulong() & 0x3F ] );
		result.push_back( base64_alphabet[ (bs >> 6).to_ulong() & 0x3F ] );
		result.push_back( base64_alphabet[ (bs).to_ulong() & 0x3F ] );
	}


	if( str.size()%3 )
	{
		bitset24_t bs;

		for( unsigned int i = str.size()%3,  shifts_needed = 2;
			shifts_needed;
			--shifts_needed )
		{
			if(i)
			{
				bs |= str[ str.size() - i];
				--i;
			}

			bs <<= 8;
		}

		result.push_back( base64_alphabet[ (bs >> 18).to_ulong() & 0x3F ] );
		result.push_back( base64_alphabet[ (bs >> 12).to_ulong() & 0x3F ] );

		if( (bs >> 8).to_ulong() & 0xFF )
			result.push_back( base64_alphabet[ (bs >> 6).to_ulong() & 0x3F ] );
		else
			result.push_back('=');

		if( (bs).to_ulong() & 0xFF )
			result.push_back( base64_alphabet[ (bs).to_ulong() & 0x3F ] );
		else
			result.push_back('=');
	}

	return result;
}

inline std::string
base64_decode( const std::string & str )
{
	std::string result;

	check_string_is_base64( str );

	for( auto i = 0 ; i < str.size()  ; i += 4)
	{
		bitset24_t bs;

		bs |= base64_alphabet.find_first_of(str[i]);
		bs <<= 6;
		bs |= base64_alphabet.find_first_of(str[i+1]);
		bs <<= 6;
		bs |= str[i+2] != '=' ? base64_alphabet.find_first_of(str[i+2]) : 0;
		bs <<= 6;
		bs |= str[i+3] != '=' ? base64_alphabet.find_first_of(str[i+3]) : 0;

		result.push_back( (bs >> 16).to_ulong() & 0xFF );
		if( (bs >> 8).to_ulong() & 0xFF )
			result.push_back( (bs >> 8).to_ulong() & 0xFF );
		if( (bs).to_ulong() & 0xFF )
		result.push_back( (bs).to_ulong() & 0xFF );
	}

	return result;
}

} /* namespace restinio */