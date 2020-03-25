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
#include <restinio/expected.hpp>

#include <restinio/utils/utf8_checker.hpp>

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
 * @brief Traits for escaping and unescaping symbols in
 * a query string in very relaxed mode.
 *
 * In that mode all characters described in that rule from
 * [RCF3986](https://tools.ietf.org/html/rfc3986) can be used as unescaped:
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
 * Additionaly this traits allows to use unescaped space character.
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

/*!
 * @brief Type that indicates that unescaping of percent-encoded symbols
 * completed successfully.
 *
 * @since v.0.6.5
 */
struct unescape_percent_encoding_success_t {};

/*!
 * @brief Type that indicates a failure of unescaping of percent-encoded
 * symbols.
 *
 * @since v.0.6.5
 */
class unescape_percent_encoding_failure_t
{
	//! Description of a failure.
	std::string m_description;

public:
	unescape_percent_encoding_failure_t(
		std::string description )
		:	m_description{ std::move(description) }
	{}

	//! Get a reference to the description of the failure.
	RESTINIO_NODISCARD
	const std::string &
	description() const noexcept { return m_description; }

	//! Get out the value of the description of the failure.
	/*!
	 * This method is intended for cases when this description should be move
	 * elsewhere (to another object like unescape_percent_encoding_failure_t or
	 * to some exception-like object).
	 */
	RESTINIO_NODISCARD
	std::string
	giveout_description() noexcept { return std::move(m_description); }
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

//
// do_unescape_percent_encoding
//
/*!
 * @brief The actual implementation of unescape-percent-encoding procedure.
 *
 * @since v.0.6.5
 */
template<
	typename Traits,
	typename Chars_Collector >
RESTINIO_NODISCARD
expected_t<
	unescape_percent_encoding_success_t,
	unescape_percent_encoding_failure_t >
do_unescape_percent_encoding(
	const string_view_t data,
	Chars_Collector && collector )
{
	std::size_t chars_to_handle = data.size();
	const char * d = data.data();

	utf8_checker_t utf8_checker;
	bool expect_next_utf8_byte = false;

	const auto current_pos = [&d, &data]() noexcept { return d - data.data(); };

	while( 0 < chars_to_handle )
	{
		char c = *d;
		if( expect_next_utf8_byte && '%' != c )
			return make_unexpected( unescape_percent_encoding_failure_t{
					fmt::format(
							"next byte from UTF-8 sequence expected at {}",
							current_pos() )
				} );

		if( '%' == c )
		{
			if( chars_to_handle >= 3 &&
				is_hexdigit( d[ 1 ] ) &&
				is_hexdigit( d[ 2 ] ) )
			{
				const auto ch = extract_escaped_char( d[ 1 ], d[ 2 ] );
				if( !utf8_checker.process_byte( static_cast<std::uint8_t>(ch) ) )
					return make_unexpected( unescape_percent_encoding_failure_t{
							fmt::format( "invalid UTF-8 sequence detected at {}",
									current_pos() )
						} );

				collector( ch );
				chars_to_handle -= 3;
				d += 3;

				expect_next_utf8_byte = !utf8_checker.finalized();
				if( !expect_next_utf8_byte )
					utf8_checker.reset();
			}
			else
			{
				return make_unexpected( unescape_percent_encoding_failure_t{
						fmt::format(
							"invalid escape sequence at pos {}", current_pos() )
					} );
			}
		}
		else if( '+' == c )
		{
			collector( ' ' );
			--chars_to_handle;
			++d;
		}
		else if( Traits::ordinary_char( c ) )
		{
			collector( c );
			--chars_to_handle;
			++d;
		}
		else
		{
			return make_unexpected( unescape_percent_encoding_failure_t{
					fmt::format(
						"invalid non-escaped char with code {:#02X} at pos: {}",
						c,
						current_pos() )
				} );
		}
	}

	if( expect_next_utf8_byte )
		return make_unexpected( unescape_percent_encoding_failure_t{
				fmt::format( "unfinished UTF-8 sequence" )
			} );

	return unescape_percent_encoding_success_t{};
}

} /* namespace impl */

//! Percent encoding.
//! \{
template< typename Traits = restinio_default_unescape_traits >
RESTINIO_NODISCARD
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
RESTINIO_NODISCARD
std::string
unescape_percent_encoding( const string_view_t data )
{
	std::string result;
	result.reserve( data.size() );

	auto r = impl::do_unescape_percent_encoding<Traits>(
			data,
			[&result]( char ch ) { result += ch; } );
	if( !r )
		throw exception_t{ r.error().giveout_description() };

	return result;
}

/*!
 * @brief Helper function for unescaping percent-encoded string.
 *
 * This function doesn't throw if some character can't be unescaped or
 * some ill-formed sequence is found.
 *
 * @note
 * This function is not noexcept and can throw on other types of
 * failures (like unability to allocate a memory).
 *
 * @since v.0.6.5
 */
template< typename Traits = restinio_default_unescape_traits >
RESTINIO_NODISCARD
expected_t< std::string, unescape_percent_encoding_failure_t >
try_unescape_percent_encoding( const string_view_t data )
{
	std::string result;
	result.reserve( data.size() );

	auto r = impl::do_unescape_percent_encoding<Traits>(
			data,
			[&result]( char ch ) { result += ch; } );
	if( !r )
		return make_unexpected( std::move(r.error()) );

	return std::move(result);
}

template< typename Traits = restinio_default_unescape_traits >
RESTINIO_NODISCARD
std::size_t
inplace_unescape_percent_encoding( char * data, std::size_t size )
{
	std::size_t result_size = 0u;
	char * dest = data;

	auto r = impl::do_unescape_percent_encoding<Traits>(
			string_view_t{ data, size },
			[&result_size, &dest]( char ch ) {
				*dest++ = ch;
				++result_size;
			} );
	if( !r )
		throw exception_t{ r.error().giveout_description() };

	return result_size;
}

/*!
 * @brief Helper function for unescaping percent-encoded string inplace.
 *
 * This function doesn't throw if some character can't be unescaped or
 * some ill-formed sequence is found.
 *
 * @note
 * This function is not noexcept and can throw on other types of
 * failures.
 *
 * @since v.0.6.5
 */
template< typename Traits = restinio_default_unescape_traits >
RESTINIO_NODISCARD
expected_t< std::size_t, unescape_percent_encoding_failure_t >
try_inplace_unescape_percent_encoding( char * data, std::size_t size )
{
	std::size_t result_size = 0u;
	char * dest = data;

	auto r = impl::do_unescape_percent_encoding<Traits>(
			string_view_t{ data, size },
			[&result_size, &dest]( char ch ) {
				*dest++ = ch;
				++result_size;
			} );
	if( !r )
		return make_unexpected( std::move(r.error()) );

	return result_size;
}

//! \}

namespace uri_normalization
{

namespace unreserved_chars
{

namespace impl
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
 * @brief Internal helper to perform the main logic of enumeration
 * of symbols in URI.
 *
 * Inspect the content of \a what and calls \a one_byte_handler if
 * single characted should be used as output, otherwise calls
 * \a three_bytes_handler (if percent-encoding sequence from three chars
 * should be passed to the output as is).
 *
 * @attention
 * Throws if invalid UTF-8 sequence is found.
 *
 * @brief v.0.6.5
 */
template<
	typename One_Byte_Handler,
	typename Three_Byte_Handler >
void
run_normalization_algo(
	string_view_t what,
	One_Byte_Handler && one_byte_handler,
	Three_Byte_Handler && three_byte_handler )
{
	using namespace restinio::utils::impl;

	std::size_t chars_to_handle = what.size();
	const char * d = what.data();

	utf8_checker_t utf8_checker;
	bool expect_next_utf8_byte = false;

	const auto current_pos = [&d, &what]() noexcept { return d - what.data(); };

	while( 0 < chars_to_handle )
	{
		if( expect_next_utf8_byte && '%' != *d )
			throw exception_t{
				fmt::format( "next byte from UTF-8 sequence expected at {}",
						current_pos() )
			};

		if( '%' != *d )
		{
			// Just one symbol to the output.
			one_byte_handler( *d );
			++d;
			--chars_to_handle;
		}
		else if( chars_to_handle >= 3 &&
			is_hexdigit( d[ 1 ] ) && is_hexdigit( d[ 2 ] ) )
		{
			const char ch = extract_escaped_char( d[ 1 ], d[ 2 ] );
			if( !utf8_checker.process_byte( static_cast<std::uint8_t>(ch) ) )
				throw exception_t{
						fmt::format( "invalid UTF-8 sequence detected at {}",
								current_pos() )
				};

			bool keep_three_bytes = true;

			if( utf8_checker.finalized() )
			{
				expect_next_utf8_byte = false;

				const auto symbol = utf8_checker.current_symbol();
				utf8_checker.reset();

				if( symbol < 0x80u )
				{
					const char ascii_char = static_cast<char>(symbol);
					if( is_unreserved_char( ascii_char ) )
					{
						// percent encoded char will be replaced by one char.
						one_byte_handler( ascii_char );
						keep_three_bytes = false;
					}
				}
			}
			else
			{
				expect_next_utf8_byte = true;
			}

			if( keep_three_bytes )
			{
				// this part of multi-byte char will go to the output as is.
				three_byte_handler( d[ 0 ], d[ 1 ], d[ 2 ] );
			}

			chars_to_handle -= 3;
			d += 3u;
		}
		else
		{
			throw exception_t{
				fmt::format( "invalid escape sequence at pos {}", current_pos() )
			};
		}
	}

	if( expect_next_utf8_byte )
		throw exception_t{ fmt::format( "unfinished UTF-8 sequence" ) };
}

} /* namespace impl */

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

	impl::run_normalization_algo( what,
			[&calculated_capacity]( char ) noexcept {
				++calculated_capacity;
			},
			[&calculated_capacity]( char, char, char ) noexcept {
				calculated_capacity += 3u;
			} );

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
	impl::run_normalization_algo( what,
			[&dest]( char ch ) noexcept {
				*dest++ = ch;
			},
			[&dest]( char ch1, char ch2, char ch3 ) noexcept {
				dest[ 0 ] = ch1;
				dest[ 1 ] = ch2;
				dest[ 2 ] = ch3;
				dest += 3;
			} );
}

} /* namespace unreserved_chars */

} /* namespace uri_normalization */

} /* namespace utils */

} /* namespace restinio */

