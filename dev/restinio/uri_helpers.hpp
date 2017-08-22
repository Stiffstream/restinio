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
parse_get_params( const std::string & request_target )
{
	TABLE param_table;

	std::istringstream pstream{ request_target };

	std::string parameter_str;
	std::getline( pstream, parameter_str, '?' );

	while( std::getline( pstream, parameter_str, '&' ) )
	{
		auto eq_symbol_pos =parameter_str.find( '=' );

		if( eq_symbol_pos == std::string::npos )
			throw exception_t{ fmt::format( "bad params in uri: {}", request_target ) };

		const auto parameter_name =
			unescape_percent_encoding( parameter_str.substr( 0, eq_symbol_pos ) );

		const auto parameter_value =
			unescape_percent_encoding( parameter_str.substr( eq_symbol_pos + 1 ) );

		param_table[ parameter_name ] = parameter_value;
	}

	return param_table;
}

} /* namespace restinio */
