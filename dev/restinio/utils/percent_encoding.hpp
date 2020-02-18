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
 * @brief Traits for escaping and unexcaping symbols in
 * a query string in correspondence with application/x-www-form-urlencoded
 * rules.
 *
 * Reference for more details: https://url.spec.whatwg.org/#concept-urlencoded-byte-serializer
 *
 * @since v.0.6.5
 */
struct x_www_form_urlencoded_unescape_traits
{
	static constexpr bool
	ordinary_char( char c ) noexcept
	{
		return
			( '0' <= c && c <= '9' ) ||
			( 'a' <= c && c <= 'z' ) ||
			( 'A' <= c && c <= 'Z' ) ||
			'*' == c ||
			'-' == c ||
			'.' == c ||
			'_' == c;
	}
};

/*!
 * @brief Traits for escaping and unexcaping symbols in
 * a query string in very relaxed mode.
 *
 * In that mode all characters described in that rule from
 * [RCF3986](https://tools.ietf.org/html/rfc3986) can be used as unexceped:
@verbatim
query         = *( pchar / "/" / "?" )
pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
reserved      = gen-delims / sub-delims
gen-delims    = ":" / "/" / "?" / "#" / "[" / "]" / "@"
sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
                 / "*" / "+" / "," / ";" / "="
@endverbatim
 *
 * Additionaly this traits allows to use unexcaped space character.
 *
 * @since v.0.6.5
 */
struct relaxed_unescape_traits
{
	static bool
	ordinary_char( char c ) noexcept
	{
		return nullptr != std::strchr(
				" " // Space
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ" // ALPHA
				"abcdefghijklmnopqrstuvwxyz"
				"0123456789" // DIGIT
				"-._~" // unreserved
				":/?#[]@" // gen-delims
				"!$&'()*+,;=", c );
	}
};

/*!
 * @brief The traits for escaping and unexcaping symbols in
 * JavaScript-compatible mode.
 *
 * The following symbols are allowed to be unescaped:
 * `-`, `.`, `~`, `_`, `*`, `!`, `'`, `(`, `)`
 *
 * @note
 * The list of allowed symbols was extended in v.0.6.5.
 *
 * @since v.0.4.9.1, v.0.6.5
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
			'*' == c ||
			'!' == c ||
			'\'' == c ||
			'(' == c ||
			')' == c;
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
		if( '+' == c )
		{
			// Replace with space.
			*dest++ = ' ';
			--chars_to_handle;
			++d;
		}
		else if( '%' == c )
		{
			if( chars_to_handle >= 3 &&
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
		else if( Traits::ordinary_char( c ) )
		{
			// Skip.
			*dest++ = c;
			--chars_to_handle;
			++d;
		}
		else
		{
			throw exception_t{
				fmt::format(
					"invalid non-escaped char with code {:#02X} at pos: {}",
					c,
					d - data ) };
		}
	}

	return result_size;
}

//! \}

namespace uri_normalization
{

namespace unreserved_chars
{

/*!
 * @brief Is this symbol a part of unreserved set?
 *
 * See https://tools.ietf.org/html/rfc3986#section-2.3 for more details.
 *
 * @since v.0.6.2
 */
RESTINIO_NODISCARD
constexpr inline bool
is_unreserved_char( const char ch ) noexcept
{
	// In this version of RESTinio class restinio_default_unescape_traits
	// already implements necessary check.
	return restinio_default_unescape_traits::ordinary_char( ch );
}

/*!
 * @brief Calculate the size of a buffer to hold normalized value of a URI.
 *
 * If @a what has some chars from unreserved set in percent-encoded form
 * then this function returns the size of a buffer to hold normalized value
 * of @a what. Otherwise the original size of @a what is returned.
 *
 * @note
 * This functions throws if @a what has invalid value.
 *
 * @since v.0.6.2
 */
RESTINIO_NODISCARD
inline std::size_t
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

	return calculated_capacity;
}

/*!
 * @brief Perform normalization of URI value.
 *
 * Copies the content of @a what into @a dest and replaces the
 * percent-encoded representation of chars from unreserved set into
 * their normal values.
 *
 * @attention
 * The capacity of @a dest should be enough to hold the result value.
 * It's assumed that estimate_required_capacity() is called before that
 * function and the result of estimate_required_capacity() is used for
 * allocation of a buffer for @a dest.
 *
 * @note
 * This functions throws if @a what has invalid value.
 *
 * @since v.0.6.2
 */
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
				dest[ 2 ] = d[ 2 ];
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

