/*!
	UTF-8 .
*/

#pragma once

#include <string>
#include <array>
#include <exception>
#include <iostream> // std::cout, debug
#include <algorithm>

namespace restinio
{

namespace impl
{

inline bool
check_utf8_is_correct( const std::string & str )
{
	size_t current_char_bytes = 1;

	for( const auto & ch : str )
	{
		if( --current_char_bytes && (ch & 0xC0) != 0x80 )
		{
			// check bytes are 10xxxxxx.
			return false;
		}
		else if( (ch & 0x80) == 0x00)
		{
			// mask 0xxxxxxx
			current_char_bytes = 1;
		}
		else if( (ch & 0xE0) == 0xC0)
		{
			// mask 110xxxxx
			current_char_bytes = 2;
		}
		else if( (ch & 0xF0) == 0xE0)
		{
			// mask 1110xxxx
			current_char_bytes = 3;
		}
		else if( (ch & 0xF8) == 0xF0)
		{
			// mask 11110xxx
			current_char_bytes = 4;
		}
		else if( (ch & 0xFC) == 0xF8)
		{
			// mask 111110xx
			current_char_bytes = 5;
		}
		else if( (ch & 0xFE) == 0xFC)
		{
			// mask 1111110x
			current_char_bytes = 6;
		}
		else
			return false;
	}

	return current_char_bytes <2 ;
}

} /* namespace impl */

} /* namespace restinio */