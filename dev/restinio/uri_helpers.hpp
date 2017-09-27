/*
	restinio
*/

/*!
	escape functions.
*/

#include <string>
#include <unordered_map>

#include <fmt/format.h>
#include <restinio/exception.hpp>

namespace restinio
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

inline const char *
modified_memchr( int chr , const char * from, const char * to )
{
	const char * result =
		static_cast< const char * >( std::memchr( from, chr, to - from ) );

	if( nullptr == result )
		result = to;

	return result;
}

} /* namespace impl */

//! Percent encoding.
//! \{
inline std::string
escape_percent_encoding( const std::string & data )
{
	std::string result;
	result.reserve( data.size() );

	for( auto c : data )
	{
		if( impl::ordinary_char( c ) )
			result += c;
		else
		{
			result += fmt::format( "%{:02X}", c );
		}
	}
	return result;
}

inline std::string
unescape_percent_encoding( const std::string & data )
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
					data.data() - d ) };
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

//! \}


template < typename Table = std::unordered_map< std::string, std::string > >
Table
parse_query_string( const std::string & query_string )
{
	const char * const very_first_pos = query_string.data();
	const char * query_remainder = very_first_pos;
	const char * query_end = query_remainder + query_string.size();

	Table result;

	query_remainder = impl::modified_memchr( '?', query_remainder, query_end );

	if( query_end > query_remainder )
	{
		// Skip '?'.
		++query_remainder;

		query_end = impl::modified_memchr( '#', query_remainder, query_end );

		while( query_end > query_remainder )
		{
			const char * const eq_symbol =
				impl::modified_memchr( '=', query_remainder, query_end );

			if( query_end == eq_symbol )
			{
				throw exception_t{
					fmt::format(
						"invalid format of key-value pairs in query_string: {}, "
						"no '=' symbol starting from position {}",
						query_string,
						std::distance(very_first_pos, query_remainder) ) };
			}

			const char * const amp_symbol_or_end =
				impl::modified_memchr( '&', eq_symbol + 1, query_end );

			// Handle next pair of parameters found.
			std::string key = unescape_percent_encoding(
					query_string.substr(
							std::distance(very_first_pos, query_remainder ),
							std::distance( query_remainder, eq_symbol ) ) );
			std::string value = unescape_percent_encoding(
					query_string.substr(
							std::distance(very_first_pos, eq_symbol + 1),
							std::distance(eq_symbol + 1, amp_symbol_or_end ) ) );

			result.emplace( std::move(key), std::move(value) );

			query_remainder = amp_symbol_or_end + 1;
		}
	}

	return result;
}

} /* namespace restinio */
