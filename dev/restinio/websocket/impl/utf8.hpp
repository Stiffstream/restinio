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

namespace websocket
{

namespace impl
{

inline bool
check_utf8_is_correct( const std::string & str )
{
	size_t rest_bytes = 0;

	for( const auto & ch : str )
	{
		if( rest_bytes > 0 )
		{
			// check bytes are 10xxxxxx.
			if( (ch & 0xC0) == 0x80 )
			{
				rest_bytes--;
			}
			else
				return false;
		}
		else if( (ch & 0x80) == 0x00)
		{
			// mask 0xxxxxxx
			rest_bytes = 0;
		}
		else if( (ch & 0xE0) == 0xC0)
		{
			// mask 110xxxxx
			rest_bytes = 1;
		}
		else if( (ch & 0xF0) == 0xE0)
		{
			// mask 1110xxxx
			rest_bytes = 2;
		}
		else if( (ch & 0xF8) == 0xF0)
		{
			// mask 11110xxx
			rest_bytes = 3;
		}
		else if( (ch & 0xFC) == 0xF8)
		{
			// mask 111110xx
			rest_bytes = 4;
		}
		else if( (ch & 0xFE) == 0xFC)
		{
			// mask 1111110x
			rest_bytes = 5;
		}
		else
			return false;
	}

	return rest_bytes == 0 ;
}

} /* namespace impl */

} /* namespace websocket */

} /* namespace restinio */