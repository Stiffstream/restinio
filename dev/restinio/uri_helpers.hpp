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

template < typename TABLE = std::unordered_map< std::string, std::string > >
TABLE
parse_get_params( const std::string & query_string )
{
	using it_t = std::string::const_iterator;

	TABLE result;

	static const auto find_char =
		[]( std::string::value_type ch, it_t from, it_t to ) -> it_t {
			for(; from != to; ++from)
				if( ch == *from ) return from;
			return to;
		};

	const it_t very_first_pos = query_string.begin();
	it_t e = query_string.end();
	it_t b = find_char( '?', very_first_pos, e );
	if( b != e )
	{
		// Skip '?'.
		++b;

		// If query_string contains #something at the end then
		// search must be stopped at '#' char.
		e = find_char( '#', b, e );

		while( b != e )
		{
			const it_t separator = find_char( '&', b, e );

			// Handle next pair of parameters found.
			const it_t eq = find_char( '=', b, separator );
			if( eq == separator )
				throw exception_t{ fmt::format(
						"invalid format of key-value pairs in query_string: {}, "
						"positions: [{}, {}]",
						query_string,
						std::distance(very_first_pos, b),
						std::distance(very_first_pos, eq) ) };

			std::string key = unescape_percent_encoding(
					query_string.substr(
							std::distance(very_first_pos, b),
							std::distance(b, eq) ) );
			std::string value = unescape_percent_encoding(
					query_string.substr(
							std::distance(very_first_pos, eq + 1),
							std::distance(eq, separator) - 1 ) );

			result.emplace( std::move(key), std::move(value) );

			b = separator;
			if( b != e )
				++b;
		}
	}

	return result;
}

} /* namespace restinio */
