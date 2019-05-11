/*
	restinio
*/

/*!
	Percent encoding routine.
*/

#pragma once

#include <string>

#include <fmt/format.h>

#include <restinio/string_view.hpp>
#include <restinio/exception.hpp>

namespace restinio
{

namespace utils
{

namespace impl
{

inline bool
ordinary_char( char c )
{
	return
		( '0' <= c && c <= '9' ) ||
		( 'a' <= c && c <= 'z' ) ||
		( 'A' <= c && c <= 'Z' ) ||
		'-' == c ||
		'.' == c ||
		'~' == c ||
		'_' == c;
};

inline bool
is_hexdigit( char c )
{
	return
		( '0' <= c && c <= '9' ) ||
		( 'a' <= c && c <= 'f' ) ||
		( 'A' <= c && c <= 'F' );
};

inline char
extract_escaped_char( char c1,  char c2 )
{
	char result;

	if( '0' <= c1 && c1 <= '9' )
		result = c1 - '0';
	else
	{
		c1 |= 0x20;
		result = 10 + c1 - 'a';
	}

	result <<= 4;

	if( '0' <= c2 && c2 <= '9' )
		result += c2 - '0';
	else
	{
		c2 |= 0x20;
		result += 10 + c2 - 'a';
	}

	return result;
};

} /* namespace impl */

//! Percent encoding.
//! \{
inline std::string
escape_percent_encoding( const string_view_t data )
{
	std::string result;
	const auto escaped_chars_count = static_cast<std::size_t>(
			std::count_if(
					data.begin(),
					data.end(),
					[]( auto c ){ return !impl::ordinary_char(c); } ));

	if( 0 == escaped_chars_count )
	{
		// No escaped chars.
		result.assign( data.data(), data.size() );
	}
	else
	{
		// Having escaped chars.
		result.reserve( data.size() + 2*escaped_chars_count );
		for( auto c : data )
		{
			if( impl::ordinary_char( c ) )
				result += c;
			else
			{
				result += fmt::format( "%{:02X}", c );
			}
		}
	}

	return result;
}

inline std::string
unescape_percent_encoding( const string_view_t data )
{
	std::string result;
	result.reserve( data.size() );

	std::size_t chars_to_handle = data.size();
	const char * d = data.data();

	while( 0 < chars_to_handle )
	{
		char c = *d;
		if( impl::ordinary_char( c ) )
		{
			result += c;
			--chars_to_handle;
			++d;
		}
		else if( '+' == c )
		{
			result += ' ';
			--chars_to_handle;
			++d;
		}
		else if( '%' != c )
		{
			throw exception_t{
				fmt::format(
					"invalid non-escaped char with code {:#02X} at pos: {}",
					c,
					d - data.data() ) };
		}
		else if( chars_to_handle >= 3 &&
			impl::is_hexdigit( d[ 1 ] ) &&
			impl::is_hexdigit( d[ 2 ] ) )
		{
			result += impl::extract_escaped_char( d[ 1 ], d[ 2 ] );
			chars_to_handle -= 3;
			d += 3;
		}
		else
		{
			throw exception_t{
				fmt::format(
					"invalid escape sequence at pos {}", d - data.data() ) };
		}
	}
	return result;
}

inline std::size_t
inplace_unescape_percent_encoding( char * data, std::size_t size )
{
	std::size_t result_size = size;
	std::size_t chars_to_handle = size;
	const char * d = data;
	char * dest = data;

	while( 0 < chars_to_handle )
	{
		char c = *d;
		if( impl::ordinary_char( c ) )
		{
			// Skip.
			*dest++ = c;
			--chars_to_handle;
			++d;
		}
		else if( '+' == c )
		{
			// Replace with space.
			*dest++ = ' ';
			--chars_to_handle;
			++d;
		}
		else if( '%' != c )
		{
			throw exception_t{
				fmt::format(
					"invalid non-escaped char with code {:#02X} at pos: {}",
					c,
					d - data ) };
		}
		else if( chars_to_handle >= 3 &&
			impl::is_hexdigit( d[ 1 ] ) &&
			impl::is_hexdigit( d[ 2 ] ) )
		{
			*dest++ = impl::extract_escaped_char( d[ 1 ], d[ 2 ] );
			chars_to_handle -= 3;
			d += 3;
			result_size -= 2; // 3 chars => 1 char.
		}
		else
		{
			throw exception_t{
				fmt::format(
					"invalid escape sequence at pos {}", d - data ) };
		}
	}

	return result_size;
}

//! \}

} /* namespace utils */

} /* namespace restinio */

