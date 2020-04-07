/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of Content-Disposition HTTP-field.
 *
 * @since v.0.6.1
 */

#pragma once

#include <restinio/helpers/http_field_parsers/basics.hpp>

namespace restinio
{

namespace http_field_parsers
{

namespace content_disposition_details
{

namespace ep_impl = restinio::easy_parser::impl;
namespace hfp_impl = restinio::http_field_parsers::impl;

//
// regular_token_producer_t
//
/*!
 * @brief A producer for token that is a "regular parameter name" in sense of
 * RCF6266 and RCF5987
 *
 * A regular parameter name can't have '*' symbol at the end.
 *
 * See: https://tools.ietf.org/html/rfc5987#section-3.2 and
 * https://tools.ietf.org/html/rfc6266#section-4.1
 *
 * @since v.0.6.1
 */
class regular_token_producer_t
	:	public hfp_impl::token_producer_t
{
public:
	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse( ep_impl::source_t & from ) const
	{
		ep_impl::source_t::content_consumer_t consumer{ from };
		const auto result = hfp_impl::token_producer_t::try_parse( from );
		if( result )
		{
			if( '*' == *(result->rbegin()) )
			{
				// Regular token can't have the trailing '*'.
				return make_unexpected( parse_error_t{
						consumer.started_at() + result->size() - 1,
						error_reason_t::unexpected_character
					} );
			}

			consumer.commit();
		}

		return result;
	}
};

//
// ext_token_producer_t
//
/*!
 * @brief A producer for token that is an "extended parameter name" in sense of
 * RCF6266 and RCF5987
 *
 * An extended parameter name has '*' symbol at the end.
 *
 * See: https://tools.ietf.org/html/rfc5987#section-3.2 and
 * https://tools.ietf.org/html/rfc6266#section-4.1
 *
 * @since v.0.6.1
 */
class ext_token_producer_t
	:	public hfp_impl::token_producer_t
{
public:
	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse( ep_impl::source_t & from ) const
	{
		ep_impl::source_t::content_consumer_t consumer{ from };
		const auto result = hfp_impl::token_producer_t::try_parse( from );
		if( result )
		{
			if( '*' != *(result->rbegin()) )
			{
				// Extended token should have the trailing '*'.
				return make_unexpected( parse_error_t{
						consumer.started_at(),
						error_reason_t::pattern_not_found
					} );
			}

			consumer.commit();
		}

		return result;
	}
};

//
// mime_charsetc_predicate_t
//
/*!
 * @brief A preducate for symbol_producer_template that checks that
 * a symbol is mime-charsetc symbol from RCF5987.
 *
 * See: https://tools.ietf.org/html/rfc5987#section-3.2
 *
 * @since v.0.6.1
 */
struct mime_charsetc_predicate_t
{
	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		return hfp_impl::is_alpha(actual)
				|| hfp_impl::is_digit(actual)
				|| '!' == actual
				|| '#' == actual
				|| '$' == actual
				|| '%' == actual
				|| '&' == actual
				|| '+' == actual
				|| '-' == actual
				|| '^' == actual
				|| '_' == actual
				|| '`' == actual
				|| '{' == actual
				|| '}' == actual
				|| '~' == actual
				;
	}
};

//
// mime_charsetc_symbol_producer
//
/*!
 * @brief A factory for producer that extracts mime-charsetc symbols.
 *
 * See: https://tools.ietf.org/html/rfc5987#section-3.2
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
auto
mime_charsetc_symbol_p()
{
	return ep_impl::symbol_producer_template_t< mime_charsetc_predicate_t >{};
}

//
// language_predicate_t
//
/*!
 * @brief A preducate for symbol_producer_template that checks that
 * a symbol is language symbol from RCF5646.
 *
 * @attention
 * In the current version of RESTinio only the presence of characters
 * defined in RFC5646 is checked. But those characters can form illegal
 * sequencies.
 *
 * See: https://tools.ietf.org/html/rfc5646#section-2.1 
 *
 * @since v.0.6.1
 */
struct language_predicate_t
{
	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		return hfp_impl::is_alpha(actual)
				|| hfp_impl::is_digit(actual)
				|| '-' == actual
				;
	}
};

//
// language_symbol_producer
//
/*!
 * @brief A factory for producer that extracts language symbols.
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
auto
language_symbol_p()
{
	return ep_impl::symbol_producer_template_t< language_predicate_t >{};
}

//
// attr_char_predicate_t
//
/*!
 * @brief A preducate for symbol_producer_template that checks that
 * a symbol is attr-char symbol from RCF5987.
 *
 * See: https://tools.ietf.org/html/rfc5987#section-3.2
 *
 * @since v.0.6.1
 */
struct attr_char_predicate_t
{
	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		return hfp_impl::is_alpha(actual)
				|| hfp_impl::is_digit(actual)
				|| '!' == actual
				|| '#' == actual
				|| '$' == actual
				|| '&' == actual
				|| '+' == actual
				|| '-' == actual
				|| '.' == actual
				|| '^' == actual
				|| '_' == actual
				|| '`' == actual
				|| '|' == actual
				|| '~' == actual
				;
	}
};

//
// attr_char_symbol_producer
//
/*!
 * @brief A factory for producer that extracts attr-char symbols.
 *
 * See: https://tools.ietf.org/html/rfc5987#section-3.2
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
auto
attr_char_symbol_p()
{
	return ep_impl::symbol_producer_template_t< attr_char_predicate_t >{};
}

//
// pct_encoded_result_type_t
//
/*!
 * @brief A type for representing extraction of percent-encoded char
 * from the input stream.
 *
 * Exactly three symbols are extracted: `% HEXDIGIT HEXDIGIT`
 *
 * @since v.0.6.1
 */
using pct_encoded_result_type_t = std::array< char, 3 >;

//
// pct_encoded_symbols_producer
//
/*!
 * @brief A producer that extract a sequence of symbols represented
 * a percent-encoded character.
 *
 * This producer returns instances of pct_encoded_result_type_t.
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
auto
pct_encoded_symbols_p()
{
	return produce< pct_encoded_result_type_t >(
			symbol_p( '%' ) >> to_container(),
			hexdigit_p() >> to_container(),
			hexdigit_p() >> to_container()
		);
}

//
// pct_encoded_symbols_consumer_t
//
/*!
 * @brief A special consumer that inserts an extracted sequence
 * of symbols into the result string.
 *
 * @since v.0.6.1
 */
struct pct_encoded_symbols_consumer_t : public ep_impl::consumer_tag
{
	void
	consume( std::string & to, pct_encoded_result_type_t && from ) const
	{
		to.append( &from[0], from.size() );
	}
};

//
// ext_parameter_value_producer
//
/*!
 * @brief A producer for an "extended parameter value" in sense of
 * RCF6266 and RCF5987
 *
 * This producer return std::string object.
 *
 * It handles the following rules:
@verbatim
ext-value     = mime-charset  "'" [ language ] "'" value-chars

mime-charset  = 1*mime-charsetc
mime-charsetc = ALPHA / DIGIT
              / "!" / "#" / "$" / "%" / "&"
              / "+" / "-" / "^" / "_" / "`"
              / "{" / "}" / "~"

language      = 0*language-char
language-char = ALPHA / DIGIT / "-"

value-chars   = *( pct-encoded / attr-char )

pct-encoded   = "%" HEXDIG HEXDIG

attr-char     = ALPHA / DIGIT
              / "!" / "#" / "$" / "&" / "+" / "-" / "."
              / "^" / "_" / "`" / "|" / "~"
@endverbatim
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
auto
ext_parameter_value_p()
{
	return produce< std::string >(
			repeat( 1, N, mime_charsetc_symbol_p() >> to_container() ),
			symbol_p( '\'' ) >> to_container(),
			repeat( 0, N, language_symbol_p() >> to_container() ),
			symbol_p( '\'' ) >> to_container(),
			repeat( 0, N,
				alternatives(
					attr_char_symbol_p() >> to_container(),
					pct_encoded_symbols_p() >>
							pct_encoded_symbols_consumer_t{} )
			)
		);
}

} /* namespace content_disposition_details */

//
// content_disposition_value_t
//
/*!
 * @brief Tools for working with the value of Content-Disposition HTTP-field.
 *
 * This struct represents parsed value of HTTP-field Content-Disposition
 * (see https://tools.ietf.org/html/rfc6266).
 *
 * @note
 * - the main value of Content-Disposition field is converted to lower case;
 * - parameter names are converted to lower case during the parsing;
 * - parameter values are left as they are;
 * - values of extended parameters are left as they are (it means that if
 *   there is "filename*=utf-8''Some%20name" then the value of
 *   "filename*" parameter will be "utf-8''Some%20name").
 *
 * @since v.0.6.1
 */
struct content_disposition_value_t
{
	using parameter_t = parameter_with_mandatory_value_t;

	using parameter_container_t = parameter_with_mandatory_value_container_t;

	std::string value;
	parameter_container_t parameters;

	/*!
	 * @brief A factory function for a parser of Content-Disposition value.
	 *
	 * @since v.0.6.1
	 */
	RESTINIO_NODISCARD
	static auto
	make_parser()
	{
		using namespace content_disposition_details;

		return produce< content_disposition_value_t >(
			token_p() >> to_lower()
					>> &content_disposition_value_t::value,
			produce< parameter_container_t >(
				repeat( 0, N,
					produce< parameter_t >(
						ows(),
						symbol(';'),
						ows(),
						alternatives(
							sequence(
								regular_token_producer_t{}
										>> to_lower() >> &parameter_t::first,
								symbol('='),
								alternatives(
									token_p() >> &parameter_t::second,
									quoted_string_p() >> &parameter_t::second
								)
							),
							sequence(
								ext_token_producer_t{}
										>> to_lower() >> &parameter_t::first,
								symbol('='),
								ext_parameter_value_p() >> &parameter_t::second
							)
						)
					) >> to_container()
				)
			) >> &content_disposition_value_t::parameters
		);
	}

	/*!
	 * @brief An attempt to parse Content-Disposition HTTP-field.
	 *
	 * @since v.0.6.1
	 */
	RESTINIO_NODISCARD
	static expected_t< content_disposition_value_t, restinio::easy_parser::parse_error_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

