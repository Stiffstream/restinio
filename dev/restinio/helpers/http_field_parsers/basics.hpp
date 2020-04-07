/*
 * RESTinio
 */

/*!
 * @file
 * @brief Utilities for parsing values of http-fields
 *
 * @since v.0.6.1
 */

#pragma once

#include <restinio/impl/string_caseless_compare.hpp>

#include <restinio/helpers/easy_parser.hpp>

#include <restinio/expected.hpp>

#include <algorithm>

namespace restinio
{

namespace http_field_parsers
{

using namespace restinio::easy_parser;

namespace meta = restinio::utils::metaprogramming;

namespace qvalue_details
{

//! A type to hold a qvalue.
using underlying_uint_t = std::uint_least16_t;

//! The maximal allowed value for a qvalue.
constexpr underlying_uint_t maximum = 1000u;
//! The minimal allowed value for a qvalue.
constexpr underlying_uint_t zero = 0u;

// A part of workaround for problem with static constexpr members
// detected after release of v.0.6.1.
enum class extremum_min_t { v };
enum class extremum_max_t { v };

//! A helper wrapper to indicate that value is already checked and
//! shouldn't be checked again.
class trusted
{
	const underlying_uint_t m_value;

public :
	explicit constexpr
	trusted( underlying_uint_t value ) noexcept : m_value{ value } {}

	constexpr auto get() const noexcept { return m_value; }
};

//! A helper wrapper to indicate that value hasn't been checked yet
//! and should be checked in the constructor of qvalue.
class untrusted
{
	underlying_uint_t m_value;

public :
	explicit
	untrusted( underlying_uint_t value ) : m_value{ value }
	{
		if( m_value > maximum )
			throw exception_t( "invalid value for "
					"http_field_parser::rfc::qvalue_t" );
	}

	auto get() const noexcept { return m_value; }
};

} /* namespace qvalue_details */

//
// qvalue_t
//
/*!
 * @brief A class for holding the parsed value of qvalue from RFC7231. 
 *
 * An important note: qvalue in RFC7231 is defined as a number with
 * fractional point. This number can have no more that three digits
 * after a point. And the non-factional part can only be 0 or 1. If
 * non-frational part is 1 then fractional part can only be 0. It means
 * that qvalue can be in the range [0.000, 1.000].
 *
 * To simplify work with qvalue RESTinio holds qvalue as a small integer
 * in the range [0, 1000] where 0 corresponds to 0.000 and 1000 corresponds
 * to 1.000. In means, for example, that value 0.05 will be represented and 50,
 * and the value 0.001 will be represented as 1, and the value 0.901 will
 * be represented as 901.
 *
 * The integer representation of qvalue can be obtained via as_uint()
 * method.
 *
 * Method as_string() returns std::string with number with fractional
 * part inside. It means:
 * @code
 * assert("1.000" == qvalue_t{qvalue_t::maximum}.as_string());
 * assert("0.901" == qvalue_t{qvalue_t::untrusted{901}}.as_string());
 * @endcode
 * Such representation is also used in `operator<<` for std::ostream.
 *
 * Instances of qvalue_t can be compared, operations `==`, `!=`, `<` and `<=`
 * are supported.
 *
 * There are two ways to construct a new qvalue object.
 *
 * The first and recommended way is intended for cases when the source
 * value for a new qvalue object isn't known at the compile-time and
 * produced somehow at the run-time. It's the usage of qvalue_t::untrusted
 * wrapper:
 * @code
 * qvalue_t weight{qvalue_t::untrusted{config.default_weigh() * 10}};
 * @endcode
 * In that case the value of `untrusted` will be checked in
 * qvalue_t::untrusted's constructor and an exception will be thrown if that
 * value isn't in [0, 1000] range.
 *
 * The second way is intended to be used with compile-time constants:
 * @code
 * qvalue_t weight{qvalue_t::trusted{250}};
 * @endcode
 * In the case of `trusted` the value is not checked in qvalue_t's
 * constructor. Therefore that way should be used with additional care.
 *
 * @since v.0.6.1
 */
class qvalue_t
{
public :
	//! The type of underlying small integer.
	using underlying_uint_t = qvalue_details::underlying_uint_t;
	//! The type that doesn't check a value to be used for qvalue.
	using trusted = qvalue_details::trusted;
	//! The type that checks a value to be used for qvalue. 
	using untrusted = qvalue_details::untrusted;

	//! The indicator that tells that new qvalue object should have
	//! the maximal allowed value.
	static constexpr qvalue_details::extremum_max_t maximum =
			qvalue_details::extremum_max_t::v;
	//! The indicator that tells that new qvalue object should have
	//! the minimal allowed value.
	static constexpr qvalue_details::extremum_min_t zero =
			qvalue_details::extremum_min_t::v;

private :
	// Note: with the terminal 0-symbol.
	using underlying_char_array_t = std::array<char, 6>;

	underlying_uint_t m_value{};

	underlying_char_array_t
	make_char_array() const noexcept
	{
		underlying_char_array_t result;
		if( qvalue_details::maximum == m_value )
		{
			std::strcpy( &result[0], "1.000" );
		}
		else
		{
			result[0] = '0';
			result[1] = '.';

			result[2] = '0' + static_cast<char>(m_value / 100u);
			const auto d2 = m_value % 100u;
			result[3] = '0' + static_cast<char>(d2 / 10u);
			const auto d3 = d2 % 10u;
			result[4] = '0' + static_cast<char>(d3);
			result[5] = 0;
		}

		return result;
	}

public :

	constexpr qvalue_t() = default;

	qvalue_t( untrusted val ) noexcept
		:	m_value{ val.get() }
	{}

	constexpr qvalue_t( trusted val ) noexcept
		:	m_value{ val.get() }
	{}

	constexpr qvalue_t( qvalue_details::extremum_min_t ) noexcept
		:	m_value{ qvalue_details::zero }
	{}

	constexpr qvalue_t( qvalue_details::extremum_max_t ) noexcept
		:	m_value{ qvalue_details::maximum }
	{}

	constexpr auto as_uint() const noexcept { return m_value; }

	auto as_string() const
	{
		return std::string{ &make_char_array().front() };
	}

	friend std::ostream &
	operator<<( std::ostream & to, const qvalue_t & what )
	{
		return (to << &what.make_char_array().front());
	}
};

RESTINIO_NODISCARD
inline bool
operator==( const qvalue_t & a, const qvalue_t & b ) noexcept
{
	return a.as_uint() == b.as_uint();
}

RESTINIO_NODISCARD
inline bool
operator!=( const qvalue_t & a, const qvalue_t & b ) noexcept
{
	return a.as_uint() != b.as_uint();
}

RESTINIO_NODISCARD
inline bool
operator<( const qvalue_t & a, const qvalue_t & b ) noexcept
{
	return a.as_uint() < b.as_uint();
}

RESTINIO_NODISCARD
inline bool
operator<=( const qvalue_t & a, const qvalue_t & b ) noexcept
{
	return a.as_uint() <= b.as_uint();
}

namespace impl
{

using namespace restinio::easy_parser::impl;

//
// is_alpha
//
/*!
 * @brief Is a character an ALPHA?
 *
 * See: https://tools.ietf.org/html/rfc5234#appendix-B.1
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline constexpr bool
is_alpha( const char ch ) noexcept
{
	return (ch >= '\x41' && ch <= '\x5A') ||
			(ch >= '\x61' && ch <= '\x7A');
}

//
// is_alpha_predicate_t
//
/*!
 * @brief A preducate for symbol_producer_template that checks that
 * a symbol is an alpha.
 *
 * @since v.0.6.2
 */
struct is_alpha_predicate_t
{
	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		return is_alpha(actual);
	}
};

//
// is_alphanum_predicate_t
//
/*!
 * @brief A preducate for symbol_producer_template that checks that
 * a symbol is an alpha or numeric.
 *
 * @since v.0.6.2
 */
struct is_alphanum_predicate_t
{
	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		return is_alpha(actual) || is_digit(actual);
	}
};

//
// is_vchar
//
/*!
 * @brief Is a character a VCHAR?
 *
 * See: https://tools.ietf.org/html/rfc5234#appendix-B.1
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline constexpr bool
is_vchar( const char ch ) noexcept
{
	return (ch >= '\x21' && ch <= '\x7E');
}

//
// is_vchar_predicate_t
//
/*!
 * @brief A preducate for symbol_producer_template that checks that
 * a symbol is a VCHAR.
 *
 * @since v.0.6.2
 */
struct is_vchar_predicate_t
{
	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		return is_vchar(actual);
	}
};

//
// is_obs_text
//
/*!
 * @brief Is a character an obs_text?
 *
 * See: https://tools.ietf.org/html/rfc7230 
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline constexpr bool
is_obs_text( const char ch ) noexcept
{
	constexpr unsigned short left = 0x80u;
	constexpr unsigned short right = 0xFFu;

	const unsigned short t = static_cast<unsigned short>(
			static_cast<unsigned char>(ch));

	return (t >= left && t <= right);
}

//
// is_qdtext
//
/*!
 * @brief Is a character a qdtext?
 *
 * See: https://tools.ietf.org/html/rfc7230 
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline constexpr bool
is_qdtext( const char ch ) noexcept
{
	return ch == SP ||
			ch == HTAB ||
			ch == '!' ||
			(ch >= '\x23' && ch <= '\x5B') ||
			(ch >= '\x5D' && ch <= '\x7E') ||
			is_obs_text( ch );
}

//
// is_ctext
//
/*!
 * @brief Is a character a ctext?
 *
 * See: https://tools.ietf.org/html/rfc7230 
 *
 * @since v.0.6.4
 */
RESTINIO_NODISCARD
inline constexpr bool
is_ctext( const char ch ) noexcept
{
	return ch == SP ||
			ch == HTAB ||
			(ch >= '\x21' && ch <= '\x27') ||
			(ch >= '\x2A' && ch <= '\x5B') ||
			(ch >= '\x5D' && ch <= '\x7E') ||
			is_obs_text( ch );
}

//
// is_ctext_predicate_t
//
/*!
 * @brief A preducate for symbol_producer_template that checks that
 * a symbol is a ctext.
 *
 * @since v.0.6.4
 */
struct is_ctext_predicate_t
{
	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		return is_ctext(actual);
	}
};

//
// ows_producer_t
//
/*!
 * @brief A producer for OWS.
 *
 * If an OWS found in the input stream it produces non-empty
 * optional_t<char> with SP as the value.
 *
 * See: https://tools.ietf.org/html/rfc7230
 *
 * @since v.0.6.1
 */
class ows_producer_t : public producer_tag< restinio::optional_t<char> >
{
public :
	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse(
		source_t & from ) const noexcept
	{
		std::size_t extracted_spaces{};
		character_t ch;
		for( ch = from.getch();
			!ch.m_eof && is_space(ch.m_ch);
			ch = from.getch() )
		{
			++extracted_spaces;
		}

		if( !ch.m_eof )
			// The first non-space char should be returned back.
			from.putback();

		if( extracted_spaces > 0u )
			return result_type{ ' ' };

		return result_type{ nullopt };
	}
};

//
// token_producer_t
//
/*!
 * @brief A producer for token.
 *
 * If a token is found in the input stream it produces std::string
 * with the value of that token.
 *
 * See: https://tools.ietf.org/html/rfc7230
 *
 * @since v.0.6.1
 */
class token_producer_t : public producer_tag< std::string >
{
	RESTINIO_NODISCARD
	static optional_t< parse_error_t >
	try_parse_value( source_t & from, std::string & accumulator )
	{
		error_reason_t reason = error_reason_t::pattern_not_found;

		do
		{
			const auto ch = from.getch();
			if( ch.m_eof )
			{
				reason = error_reason_t::unexpected_eof;
				break;
			}

			if( !is_token_char(ch.m_ch) )
			{
				from.putback();
				reason = error_reason_t::unexpected_character;
				break;
			}

			accumulator += ch.m_ch;
		}
		while( true );

		if( accumulator.empty() )
		{
			return parse_error_t{ from.current_position(), reason };
		}

		return nullopt;
	}

	RESTINIO_NODISCARD
	static constexpr bool
	is_token_char( const char ch ) noexcept
	{
		return is_alpha(ch) || is_digit(ch) ||
				ch == '!' ||
				ch == '#' ||
				ch == '$' ||
				ch == '%' ||
				ch == '&' ||
				ch == '\'' ||
				ch == '*' ||
				ch == '+' ||
				ch == '-' ||
				ch == '.' ||
				ch == '^' ||
				ch == '_' ||
				ch == '`' ||
				ch == '|' ||
				ch == '~';
	}

public :
	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse( source_t & from ) const
	{
		std::string value;
		const auto try_result = try_parse_value( from, value );
		if( !try_result )
			return std::move(value);
		else
			return make_unexpected( *try_result );
	}
};

//
// quoted_string_producer_t
//
/*!
 * @brief A producer for quoted_string.
 *
 * If a quoted_string is found in the input stream it produces std::string
 * with the value of that token.
 *
 * See: https://tools.ietf.org/html/rfc7230
 *
 * @since v.0.6.1
 */
class quoted_string_producer_t : public producer_tag< std::string >
{
	RESTINIO_NODISCARD
	static optional_t< parse_error_t >
	try_parse_value( source_t & from, std::string & accumulator )
	{
		error_reason_t reason = error_reason_t::pattern_not_found;

		bool second_quote_extracted{ false };
		do
		{
			const auto ch = from.getch();
			if( ch.m_eof )
			{
				reason = error_reason_t::unexpected_eof;
				break;
			}

			if( '"' == ch.m_ch )
				second_quote_extracted = true;
			else if( '\\' == ch.m_ch )
			{
				const auto next = from.getch();
				if( next.m_eof )
				{
					reason = error_reason_t::unexpected_eof;
					break;
				}
				else if( SP == next.m_ch || HTAB == next.m_ch ||
						is_vchar( next.m_ch ) ||
						is_obs_text( next.m_ch ) )
				{
					accumulator += next.m_ch;
				}
				else
				{
					reason = error_reason_t::unexpected_character;
					from.putback();
					break;
				}
			}
			else if( is_qdtext( ch.m_ch ) )
				accumulator += ch.m_ch;
			else
			{
				reason = error_reason_t::unexpected_character;
				from.putback();
				break;
			}
		}
		while( !second_quote_extracted );

		if( !second_quote_extracted )
			return parse_error_t{ from.current_position(), reason };
		else
			return nullopt;
	}

public :
	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse( source_t & from ) const
	{
		source_t::content_consumer_t consumer{ from };

		const auto ch = from.getch();
		if( !ch.m_eof )
		{
			if( '"' == ch.m_ch )
			{
				std::string value;
				const auto try_result = try_parse_value( from, value );
				if( !try_result )
				{
					consumer.commit();
					return std::move(value);
				}
				else
					return make_unexpected( *try_result );
			}
			else
			{
				return make_unexpected( parse_error_t{
						consumer.started_at(),
						error_reason_t::unexpected_character
				} );
			}
		}
		else
			return make_unexpected( parse_error_t{
					consumer.started_at(),
					error_reason_t::unexpected_eof
			} );
	}
};

//
// quoted_pair_producer_t
//
/*!
 * @brief A producer for quoted_pair.
 *
 * If a quoted_pair is found in the input stream it produces char
 * with the value of that token.
 *
 * See: https://tools.ietf.org/html/rfc7230
 *
 * @since v.0.6.4
 */
class quoted_pair_producer_t : public producer_tag< char >
{
public :
	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse( source_t & from ) const
	{
		source_t::content_consumer_t consumer{ from };

		error_reason_t reason = error_reason_t::unexpected_eof;

		const auto ch = from.getch();
		if( !ch.m_eof )
		{
			if( '\\' == ch.m_ch )
			{
				const auto next = from.getch();
				if( !next.m_eof )
				{
					if( SP == next.m_ch || HTAB == next.m_ch ||
							is_vchar( next.m_ch ) ||
							is_obs_text( next.m_ch ) )
					{
						consumer.commit();
						return next.m_ch;
					}

					reason = error_reason_t::unexpected_character;
				}
			}
			else
				reason = error_reason_t::unexpected_character;
		}

		return make_unexpected( parse_error_t{
				from.current_position(),
				reason
		} );
	}
};

//
// comment_producer_t
//
/*!
 * @brief A producer for comment.
 *
 * If a comment is found in the input stream it produces std::string
 * with the value of that token.
 *
 * Comment is defined that way:
@verbatim
comment        = "(" *( ctext / quoted-pair / comment ) ")"
ctext          = HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text
quoted-pair    = "\" ( HTAB / SP / VCHAR / obs-text )
@endverbatim
 *
 * See: https://tools.ietf.org/html/rfc7230
 *
 * @since v.0.6.4
 */
class comment_producer_t : public producer_tag< std::string >
{
public :
	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse( source_t & from ) const; // NOTE: implementation below.
};

} /* namespace impl */

//
// alpha_symbol_producer
//
/*!
 * @brief A factory for producer of ALPHA symbols.
 *
 * Usage example:
 * @code
	produce<std::string>(
		repeat(1, 20, alpha_symbol_p() >> to_container());
 * @endcode
 *
 * @since v.0.6.2
 */
RESTINIO_NODISCARD
inline auto
alpha_symbol_p()
{
	return restinio::easy_parser::impl::symbol_producer_template_t<
			impl::is_alpha_predicate_t >{};
}

//
// alphanum_symbol_producer
//
/*!
 * @brief A factory for producer of symbol that an ALPHA or DIGIT.
 *
 * Usage example:
 * @code
	produce<std::string>(
		repeat(1, 20, alphanum_symbol_p() >> to_container());
 * @endcode
 *
 * @since v.0.6.2
 */
RESTINIO_NODISCARD
inline auto
alphanum_symbol_p()
{
	return restinio::easy_parser::impl::symbol_producer_template_t<
			impl::is_alphanum_predicate_t >{};
}

//
// vchar_symbol_producer
//
/*!
 * @brief A factory for producer of VCHAR symbols.
 *
 * Usage example:
 * @code
	produce<std::string>(
		repeat(1, 20, vchar_symbol_p() >> to_container());
 * @endcode
 *
 * @since v.0.6.2
 */
RESTINIO_NODISCARD
inline auto
vchar_symbol_p()
{
	return restinio::easy_parser::impl::symbol_producer_template_t<
			impl::is_vchar_predicate_t >{};
}

//
// ctext_symbol_producer
//
/*!
 * @brief A factory for producer of ctext symbols.
 *
 * Usage example:
 * @code
	produce<std::string>(
		repeat(1, 20, ctext_symbol_p() >> to_container());
 * @endcode
 *
 * @since v.0.6.4
 */
RESTINIO_NODISCARD
inline auto
ctext_symbol_p()
{
	return restinio::easy_parser::impl::symbol_producer_template_t<
			impl::is_ctext_predicate_t >{};
}

//
// comment_producer
//
/*!
 * @brief A factory for producer of comment token.
 *
 * Usage example:
 * @code
	produce<std::string>(
		alternatives(
			token_p() >> as_result(),
			comment_p() >> as_result()
		)
	);
 * @endcode
 *
 * @since v.0.6.4
 */
RESTINIO_NODISCARD
inline auto
comment_p()
{
	return impl::comment_producer_t{};
}

//
// ows_producer
//
/*!
 * @brief A factory function to create an ows_producer.
 *
 * Usage example:
 * @code
 * produce<std::string>(
 * 	ows_p() >> skip(),
 * 	symbol('v'),
 * 	symbol('='),
 * 	ows_p() >> skip(),
 * 	token_p() >> as_result()
 * );
 * @endcode
 *
 * @note
 * Factory function ows() can be used instead of expression `ows_p() >> skip()`.
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline auto
ows_p() noexcept { return impl::ows_producer_t{}; }

//
// ows
//
/*!
 * @brief A factory function to create an OWS clause.
 *
 * This clause handles an optional sequence of spaces in the input stream and 
 * skips the value of that sequence.
 *
 * Usage example:
 * @code
 * produce<std::string>(
 * 	ows(),
 * 	symbol('v'),
 * 	symbol('='),
 * 	ows(),
 * 	token_p() >> as_result()
 * );
 * @endcode
 * This expression corresponds the following rule:
   @verbatim
   T := OWS 'v' '=' OWS token
   @endverbatim
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline auto
ows() noexcept { return ows_p() >> skip(); }

//
// token_producer
//
/*!
 * @brief A factory function to create a token_producer.
 *
 * Usage example:
 * @code
 * using parameter = std::pair<std::string, std::string>;
 * produce<parameter>(
 * 	ows(),
 * 	token_p() >> &parameter::first,
 * 	symbol('='),
 * 	ows(),
 * 	token_p() >> &parameter::second
 * );
 * @endcode
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline auto
token_p() noexcept { return impl::token_producer_t{}; }

//
// quoted_string_producer
//
/*!
 * @brief A factory function to create a quoted_string_producer.
 *
 * Usage example:
 * @code
 * using parameter = std::pair<std::string, std::string>;
 * produce<parameter>(
 * 	ows(),
 * 	token_p() >> &parameter::first,
 * 	symbol('='),
 * 	ows(),
 * 	alternatives(
 * 		token_p() >> &parameter::second,
 * 		quoted_string_p() >> &parameter::second
 * 	)
 * );
 * @endcode
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline auto
quoted_string_p() noexcept
{
	return impl::quoted_string_producer_t{};
}

//
// quoted_pair_producer
//
/*!
 * @brief A factory function to create a quoted_pair_producer.
 *
 * Usage example:
 * @code
 * produce<std::string>(
 * 	repeat(1, N,
 * 		alternatives(
 * 			ctext_symbol_p() >> to_container(),
 * 			quoted_pair_p() >> to_container()
 * 	)
 * );
 * @endcode
 *
 * @since v.0.6.4
 */
RESTINIO_NODISCARD
inline auto
quoted_pair_p() noexcept
{
	return impl::quoted_pair_producer_t{};
}

namespace impl
{

//
// comment_producer_t implementation
//
RESTINIO_NODISCARD
inline expected_t< comment_producer_t::result_type, parse_error_t >
comment_producer_t::try_parse( source_t & from ) const
{
	return produce< std::string >(
			sequence(
				symbol('('),
				repeat(0, N,
					alternatives(
						ctext_symbol_p() >> to_container(),
						quoted_pair_p() >> to_container(),
						comment_p() >> custom_consumer(
							[]( std::string & dest, std::string && what ) {
								dest += what;
							} )
					) ),
				symbol(')') )
		).try_parse( from );
}

//
// qvalue_producer_t
//
/*!
 * @brief An implementation of producer of qvalue.
 *
 * Handles the following rule:
   @verbatim
   qvalue = ( "0" [ "." 0*3DIGIT ] )
          / ( "1" [ "." 0*3("0") ] )
   @endverbatim
 * and produces an instance of qvalue_t.
 *
 * See: https://tools.ietf.org/html/rfc7231
 *
 * @since v.0.6.1
 */
class qvalue_producer_t
	:	public producer_tag< qvalue_t >
{
	// This type has to be used as type parameter for produce().
	struct zero_initialized_unit_t
	{
		qvalue_t::underlying_uint_t m_value{0u};
	};

	//! A helper class to be used to accumulate actual integer
	//! while when the next digit is extracted from the input stream.
	class digit_consumer_t : public consumer_tag
	{
		const qvalue_t::underlying_uint_t m_multiplier;
	
	public :
		constexpr digit_consumer_t( qvalue_t::underlying_uint_t m )
			:	m_multiplier{ m }
		{}
	
		void
		consume( zero_initialized_unit_t & dest, char && digit )
		{
			dest.m_value += m_multiplier *
					static_cast< qvalue_t::underlying_uint_t >(digit - '0');
		}
	};

public :
	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse( source_t & from ) const noexcept
	{
		const auto parse_result = produce< zero_initialized_unit_t >(
				alternatives(
					sequence(
						symbol('0'),
						maybe(
							symbol('.'),
							maybe( digit_p() >> digit_consumer_t{100},
								maybe( digit_p() >> digit_consumer_t{10},
									maybe( digit_p() >> digit_consumer_t{1} )
								)
							)
						)
					),
					sequence(
						symbol_p('1') >> digit_consumer_t{1000},
						maybe(
							symbol('.'),
							maybe( symbol('0'),
								maybe( symbol('0'),
									maybe( symbol('0') )
								)
							)
						)
					)
				)
			).try_parse( from );

		if( parse_result )
			return qvalue_t{ qvalue_t::trusted{ parse_result->m_value } };
		else
			return make_unexpected( parse_result.error() );
	}
};

} /* namespace impl */

//
// qvalue_producer
//
/*!
 * @brief A factory function to create a qvalue_producer.
 *
 * Usage example:
 * @code
 * produce<qvalue_t>(
 * 	alternatives(symbol('r'), symbol('R')),
 * 	symbol('='),
 * 	qvalue_p() >> as_result()
 * );
 * @endcode
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline auto
qvalue_p() noexcept
{
	return impl::qvalue_producer_t{};
}

//
// weight_producer
//
/*!
 * @brief A factory function to create a producer for weight parameter.
 *
 * Returns a producer that handles the following rules:
   @verbatim
   weight = OWS ';' OWS ('q' / 'Q') '=' qvalue

   qvalue = ( "0" [ "." 0*3DIGIT ] )
          / ( "1" [ "." 0*3("0") ] )
   @endverbatim
 *
 * See: https://tools.ietf.org/html/rfc7231
 *
 * That producer produces a value of type qvalue_t.
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline auto
weight_p() noexcept
{
	return produce< qvalue_t >(
			ows(),
			symbol(';'),
			ows(),
			caseless_symbol('q'),
			symbol('='),
			qvalue_p() >> as_result()
		);
}

namespace impl
{

//
// non_empty_comma_separated_list_producer_t
//
/*!
 * @brief A template for a producer that handles non-empty list of
 * comma-separated values.
 *
 * That producer handles the following rule:
@verbatim
1#element => *( "," OWS ) element *( OWS "," [ OWS element ] )
@endverbatim
 *
 * See: https://tools.ietf.org/html/rfc7230
 * (section "7. ABNF List Extension: #rule")
 *
 * @tparam Container the type of container to be produced.
 *
 * @tparam Element_Producer the type of the producer of a single item.
 * 
 * @since v.0.6.1
 */
template<
	typename Container,
	typename Element_Producer >
class non_empty_comma_separated_list_producer_t
	: public producer_tag< Container >
{
	static_assert( impl::is_producer_v<Element_Producer>,
			"Element_Producer should be a value producer type" );

	Element_Producer m_element;

public :
	using typename producer_tag< Container >::result_type;

	non_empty_comma_separated_list_producer_t(
		Element_Producer && element )
		:	m_element{ std::move(element) }
	{}

	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse( source_t & from )
	{
		Container tmp_value;

		const auto appender = to_container();

		const auto process_result = sequence(
				repeat( 0, N, symbol(','), ows() ),
				m_element >> appender,  
				repeat( 0, N,
					ows(), symbol(','),
					maybe( ows(), m_element >> appender )
				)
			).try_process( from, tmp_value );

		if( !process_result )
			return std::move(tmp_value);
		else
			return make_unexpected( *process_result );
	}
};

//
// maybe_empty_comma_separated_list_producer_t
//
/*!
 * @brief A template for a producer that handles possibly empty list of
 * comma-separated values.
 *
 * That producer handles the following rule:
@verbatim
#element => [ ( "," / element ) *( OWS "," [ OWS element ] ) ]
@endverbatim
 *
 * See: https://tools.ietf.org/html/rfc7230
 * (section "7. ABNF List Extension: #rule")
 *
 * @tparam Container the type of container to be produced.
 *
 * @tparam Element_Producer the type of the producer of a single item.
 * 
 * @since v.0.6.1
 */
template<
	typename Container,
	typename Element_Producer >
class maybe_empty_comma_separated_list_producer_t
	:	public producer_tag< Container >
{
	static_assert( impl::is_producer_v<Element_Producer>,
			"Element_Producer should be a value producer type" );

	Element_Producer m_element;

public :
	using typename producer_tag< Container >::result_type;

	maybe_empty_comma_separated_list_producer_t(
		Element_Producer && element )
		:	m_element{ std::move(element) }
	{}

	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse( source_t & from )
	{
		Container tmp_value;

		const auto appender = to_container();

		const auto process_result = maybe(
				alternatives( symbol(','), m_element >> appender ),
				repeat( 0, N,
					ows(), symbol(','),
					maybe( ows(), m_element >> appender )
				)
			).try_process( from, tmp_value );

		if( !process_result )
			return std::move(tmp_value);
		else
			return make_unexpected( *process_result );
	}
};

} /* namespace impl */

//
// non_empty_comma_separated_list_producer
//
/*!
 * @brief A factory for a producer that handles non-empty list of
 * comma-separated values.
 *
 * That producer handles the following rule:
@verbatim
1#element => *( "," OWS ) element *( OWS "," [ OWS element ] )
@endverbatim
 *
 * See: https://tools.ietf.org/html/rfc7230
 * (section "7. ABNF List Extension: #rule")
 *
 * Usage example:
 * @code
	auto parse = produce< byte_ranges_data >(
			make_bytes_prefix_parser(),
			non_empty_comma_separated_list_p< std::vector< byte_range > >(
					make_byte_range_parser()
			) >> &byte_ranges_data::ranges
	);
 * @endcode
 *
 * @tparam Container the type of container to be produced.
 *
 * @tparam Element_Producer the type of the producer of a single item.
 * 
 * @since v.0.6.1
 */
template<
	typename Container,
	typename Element_Producer >
RESTINIO_NODISCARD
auto
non_empty_comma_separated_list_p( Element_Producer element )
{
	static_assert( impl::is_producer_v<Element_Producer>,
			"Element_Producer should be a value producer type" );

	return impl::non_empty_comma_separated_list_producer_t<
			Container,
			Element_Producer >{ std::move(element) };
}

//
// maybe_empty_comma_separated_list_producer
//
/*!
 * @brief A factory for a producer that handles possibly empty list of
 * comma-separated values.
 *
 * That producer handles the following rule:
@verbatim
#element => [ ( "," / element ) *( OWS "," [ OWS element ] ) ]
@endverbatim
 *
 * See: https://tools.ietf.org/html/rfc7230
 * (section "7. ABNF List Extension: #rule")
 *
 * Usage example:
 * @code
	auto parse = produce< byte_ranges_data >(
			make_bytes_prefix_parser(),
			maybe_empty_comma_separated_list_p< std::vector< byte_range > >(
					make_byte_range_parser()
			) >> &byte_ranges_data::ranges
	);
 * @endcode
 *
 * @tparam Container the type of container to be produced.
 *
 * @tparam Element_Producer the type of the producer of a single item.
 * 
 * @since v.0.6.1
 */
template<
	typename Container,
	typename Element_Producer >
RESTINIO_NODISCARD
auto
maybe_empty_comma_separated_list_p( Element_Producer element )
{
	static_assert( impl::is_producer_v<Element_Producer>,
			"Element_Producer should be a value producer type" );

	return impl::maybe_empty_comma_separated_list_producer_t<
			Container,
			Element_Producer >{ std::move(element) };
}

//
// parameter_with_mandatory_value_t
//
/*!
 * @brief A type that describes a parameter with mandatory value.
 *
 * @since v.0.6.1
 */
using parameter_with_mandatory_value_t = std::pair< std::string, std::string >;

//
// parameter_with_mandatory_value_container_t
//
/*!
 * @brief A type of container for parameters with mandatory values.
 *
 * @since v.0.6.1
 */
using parameter_with_mandatory_value_container_t =
		std::vector< parameter_with_mandatory_value_t >;

//
// not_found_t
//
/*!
 * @brief An empty type to be used as indicator of negative search result.
 *
 * @since v.0.6.1
 */
struct not_found_t {};

//
// find_first
//
/*!
 * @brief A helper function to find the first occurence of a parameter
 * with the specified value.
 *
 * @note
 * The caseless (case-insentive) search is used. It means that
 * search with value "charset" will returns items "CharSet", "charset",
 * "CHARSET" and so on.
 *
 * Usage example:
 * @code
 * const auto charset = find_first(content_type_params, "charset");
 * if(charset) {
 * 	... // Handle the value of charset parameter.
 * }
 * @endcode
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline expected_t< string_view_t, not_found_t >
find_first(
	const parameter_with_mandatory_value_container_t & where,
	string_view_t what )
{
	const auto it = std::find_if( where.begin(), where.end(),
			[&what]( const auto & pair ) {
				return restinio::impl::is_equal_caseless( pair.first, what );
			} );
	if( it != where.end() )
		return string_view_t{ it->second };
	else
		return make_unexpected( not_found_t{} );
}

namespace impl
{

namespace params_with_value_producer_details
{

//
// make_parser
//
/*!
 * @brief Helper function that creates an instance of producer
 * of parameter_with_mandatory_value_container.
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline auto
make_parser()
{
	return produce< parameter_with_mandatory_value_container_t >(
			repeat( 0, N,
				produce< parameter_with_mandatory_value_t >(
					ows(),
					symbol(';'),
					ows(),
					token_p() >> to_lower()
							>> &parameter_with_mandatory_value_t::first,
					symbol('='),
					alternatives(
						token_p()
								>> &parameter_with_mandatory_value_t::second,
						quoted_string_p()
								>> &parameter_with_mandatory_value_t::second
					)
				) >> to_container()
			)
		);
}

} /* namespace params_with_value_producer_details */

//
// params_with_value_producer_t
//
/*!
 * @brief A type of producer that produces instances of
 * parameter_with_mandatory_value_container.
 *
 * @since v.0.6.1
 */
class params_with_value_producer_t
	:	public producer_tag< parameter_with_mandatory_value_container_t >
{
	using actual_producer_t = std::decay_t<
			decltype(params_with_value_producer_details::make_parser()) >;

	actual_producer_t m_producer{
			params_with_value_producer_details::make_parser() };

public :
	params_with_value_producer_t() = default;

	RESTINIO_NODISCARD
	auto
	try_parse( source_t & from )
	{
		return m_producer.try_parse( from );
	}
};

} /* namespace impl */

//
// params_with_value_producer
//
/*!
 * @brief A factory of producer of parameter_with_mandatory_value_container.
 *
 * Creates a produces that handles the following rule:
@verbatim
T := *( OWS ';' OWS token '=' OWS (token / quoted_string))
@endverbatim
 *
 * Usage example:
 * @code
 * struct my_field {
 * 	std::string value;
 * 	parameter_with_mandatory_value_container_t params;
 * };
 * produce<my_field>(
 * 	token_p() >> to_lower() >> &my_field,
 * 	params_with_value_p() >> &my_field
 * );
 * @endcode
 *
 * @note
 * Parameters names are converted to lower case. Parameters' values
 * are not changed and stored as they are.
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline impl::params_with_value_producer_t
params_with_value_p() { return {}; }

//
// parameter_with_optional_value_t
//
/*!
 * @brief A type that describes a parameter with optional value.
 *
 * @since v.0.6.1
 */
using parameter_with_optional_value_t =
		std::pair< std::string, restinio::optional_t<std::string> >;

//
// parameter_with_optional_value_container_t
//
/*!
 * @brief A type of container for parameters with optional values.
 *
 * @since v.0.6.1
 */
using parameter_with_optional_value_container_t =
		std::vector< parameter_with_optional_value_t >;

//
// find_first
//
/*!
 * @brief A helper function to find the first occurence of a parameter
 * with the specified value.
 *
 * @note
 * The caseless (case-insentive) search is used. It means that
 * search with value "charset" will returns items "CharSet", "charset",
 * "CHARSET" and so on.
 *
 * Usage example:
 * @code
 * const auto max_age = find_first(cache_control_params, "max-age");
 * if(max_age) {
 * 	if(*max_age) {
 * 		... // Handle the value of max-age parameter.
 * 	}
 * 	else {
 * 		... // Handle the case where max-age specified but without a value.
 * 	}
 * }
 * @endcode
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline expected_t< restinio::optional_t<string_view_t>, not_found_t >
find_first(
	const parameter_with_optional_value_container_t & where,
	string_view_t what )
{
	const auto it = std::find_if( where.begin(), where.end(),
			[&what]( const auto & pair ) {
				return restinio::impl::is_equal_caseless( pair.first, what );
			} );
	if( it != where.end() )
	{
		const auto opt = it->second;
		if( opt )
			return string_view_t{ *opt };
		else
			return restinio::optional_t< string_view_t >{ nullopt };
	}
	else
		return make_unexpected( not_found_t{} );
}

namespace impl
{

namespace params_with_opt_value_producer_details
{

//
// make_parser
//
/*!
 * @brief Helper function that creates an instance of producer
 * of parameter_with_optional_value_container.
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline auto
make_parser()
{
	return produce< parameter_with_optional_value_container_t >(
			repeat( 0, N,
				produce< parameter_with_optional_value_t >(
					ows(),
					symbol(';'),
					ows(),
					token_p() >> to_lower()
							>> &parameter_with_optional_value_t::first,
					maybe(
						symbol('='),
						alternatives(
							token_p()
									>> &parameter_with_optional_value_t::second,
							quoted_string_p()
									>> &parameter_with_optional_value_t::second
						)
					)
				) >> to_container()
			)
		);
}

} /* namespace params_with_opt_value_producer_details */

//
// params_with_opt_value_producer_t
//
/*!
 * @brief A type of producer that produces instances of
 * parameter_with_optional_value_container.
 *
 * @since v.0.6.1
 */
class params_with_opt_value_producer_t
	:	public producer_tag< parameter_with_optional_value_container_t >
{
	using actual_producer_t = std::decay_t<
			decltype(params_with_opt_value_producer_details::make_parser()) >;

	actual_producer_t m_producer{
			params_with_opt_value_producer_details::make_parser() };

public :
	params_with_opt_value_producer_t() = default;

	RESTINIO_NODISCARD
	auto
	try_parse( source_t & from )
	{
		return m_producer.try_parse( from );
	}
};

} /* namespace impl */

//
// params_with_opt_value_producer
//
/*!
 * @brief A factory of producer of parameter_with_optional_value_container.
 *
 * Creates a produces that handles the following rule:
@verbatim
T := *( OWS ';' OWS token ['=' OWS (token / quoted_string)] )
@endverbatim
 *
 * Usage example:
 * @code
 * struct my_field {
 * 	std::string value;
 * 	parameter_with_optional_value_container_t params;
 * };
 * produce<my_field>(
 * 	token_p() >> to_lower() >> &my_field,
 * 	params_with_opt_value_p() >> &my_field
 * );
 * @endcode
 *
 * @note
 * Parameters names are converted to lower case. Parameters' values
 * are not changed and stored as they are.
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline impl::params_with_opt_value_producer_t
params_with_opt_value_p() { return {}; }

} /* namespace http_field_parser */

} /* namespace restinio */

