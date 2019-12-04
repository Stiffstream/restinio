/*
	restinio
*/

/*!
	Percent encoding routine.
*/

#pragma once

#include <string>

#include <restinio/impl/include_fmtlib.hpp>

#include <restinio/string_view.hpp>
#include <restinio/variant.hpp>
#include <restinio/exception.hpp>

namespace restinio
{

namespace utils
{

/*!
 * @brief The default traits for escaping and unexcaping symbols in
 * a query string.
 *
 * Unescaped asterisk is not allowed.
 *
 * @since v.0.4.9.1
 */
struct restinio_default_unescape_traits
{
	static constexpr bool
	ordinary_char( char c ) noexcept
	{
		return
			( '0' <= c && c <= '9' ) ||
			( 'a' <= c && c <= 'z' ) ||
			( 'A' <= c && c <= 'Z' ) ||
			'-' == c ||
			'.' == c ||
			'~' == c ||
			'_' == c;
	}
};

/*!
 * @brief The traits for escaping and unexcaping symbols in
 * JavaScript-compatible mode.
 *
 * Unescaped asterisk is allowed.
 *
 * @since v.0.4.9.1
 */
struct javascript_compatible_unescape_traits
{
	static constexpr bool
	ordinary_char( char c ) noexcept
	{
		return
			( '0' <= c && c <= '9' ) ||
			( 'a' <= c && c <= 'z' ) ||
			( 'A' <= c && c <= 'Z' ) ||
			'-' == c ||
			'.' == c ||
			'~' == c ||
			'_' == c ||
			'*' == c;
	}
};

namespace impl
{

inline bool
is_hexdigit( char c )
{
	return
		( '0' <= c && c <= '9' ) ||
		( 'a' <= c && c <= 'f' ) ||
		( 'A' <= c && c <= 'F' );
}

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
}

} /* namespace impl */

//! Percent encoding.
//! \{
template< typename Traits = restinio_default_unescape_traits >
std::string
escape_percent_encoding( const string_view_t data )
{
	std::string result;
	const auto escaped_chars_count = static_cast<std::size_t>(
			std::count_if(
					data.begin(),
					data.end(),
					[]( auto c ){ return !Traits::ordinary_char(c); } ));

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
			if( Traits::ordinary_char( c ) )
				result += c;
			else
			{
				result += fmt::format( "%{:02X}", c );
			}
		}
	}

	return result;
}

template< typename Traits = restinio_default_unescape_traits >
std::string
unescape_percent_encoding( const string_view_t data )
{
	std::string result;
	result.reserve( data.size() );

	std::size_t chars_to_handle = data.size();
	const char * d = data.data();

	while( 0 < chars_to_handle )
	{
		char c = *d;
		if( Traits::ordinary_char( c ) )
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

template< typename Traits = restinio_default_unescape_traits >
std::size_t
inplace_unescape_percent_encoding( char * data, std::size_t size )
{
	std::size_t result_size = size;
	std::size_t chars_to_handle = size;
	const char * d = data;
	char * dest = data;

	while( 0 < chars_to_handle )
	{
		char c = *d;
		if( Traits::ordinary_char( c ) )
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

namespace uri_normalization
{

//FIXME: document this!
struct required_capacity_t
{
	std::size_t m_value;
};

//FIXME: document this!
struct keep_unchanged_t {};

namespace unreserved_chars
{

//FIXME: document this!
RESTINIO_NODISCARD
constexpr inline bool
is_unreserved_char( const char ch ) noexcept
{
	return restinio_default_unescape_traits::ordinary_char( ch );
}

//FIXME: document this!
RESTINIO_NODISCARD
inline variant_t<required_capacity_t, keep_unchanged_t>
estimate_required_capacity(
	string_view_t what )
{
	std::size_t calculated_capacity = 0u;

	std::size_t chars_to_handle = what.size();
	const char * d = what.data();

	while( 0 < chars_to_handle )
	{
		if( '%' != *d )
		{
			// Just one symbol to the output.
			++calculated_capacity;
			--chars_to_handle;
			++d;
		}
		else if( chars_to_handle >= 3 &&
			impl::is_hexdigit( d[ 1 ] ) && impl::is_hexdigit( d[ 2 ] ) )
		{
			const char ch = impl::extract_escaped_char( d[ 1 ], d[ 2 ] );
			if( is_unreserved_char( ch ) )
				// percent encoded char will be replaced by one char.
				++calculated_capacity; 
			else
				// this percent encoding sequence will go to the output.
				calculated_capacity += 3;

			chars_to_handle -= 3;
			d += 3;
		}
		else
		{
			throw exception_t{
				fmt::format( "invalid escape sequence at pos {}", d - what.data() )
			};
		}
	}

	if( calculated_capacity != what.size() )
		return required_capacity_t{ calculated_capacity };
	else
		return keep_unchanged_t{};
}

//FIXME: document this!
inline void
normalize_to(
	string_view_t what,
	char * dest )
{
	std::size_t chars_to_handle = what.size();
	const char * d = what.data();

	while( 0 < chars_to_handle )
	{
		if( '%' != *d )
		{
			// Just one symbol to the output.
			*dest = *d;
			++dest;
			++d;
			--chars_to_handle;
		}
		else if( chars_to_handle >= 3 &&
			impl::is_hexdigit( d[ 1 ] ) && impl::is_hexdigit( d[ 2 ] ) )
		{
			const char ch = impl::extract_escaped_char( d[ 1 ], d[ 2 ] );
			if( is_unreserved_char( ch ) )
			{
				// percent encoded char will be replaced by one char.
				*dest = ch;
				++dest;
			}
			else
			{
				// this percent encoding sequence will go to the output.
				dest[ 0 ] = d[ 0 ];
				dest[ 1 ] = d[ 1 ];
				dest[ 3 ] = d[ 3 ];
				dest += 3;
			}

			chars_to_handle -= 3;
			d += 3;
		}
		else
		{
			throw exception_t{
				fmt::format( "invalid escape sequence at pos {}", d - what.data() )
			};
		}
	}
}

} /* namespace unreserved_chars */

} /* namespace uri_normalization */

} /* namespace utils */

} /* namespace restinio */

