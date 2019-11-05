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
//FIXME: document this!
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
//FIXME: document this!
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
//FIXME: document this!
/*!
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
//FIXME: document this!
RESTINIO_NODISCARD
auto
mime_charsetc_symbol_producer()
{
	return ep_impl::symbol_producer_template_t< mime_charsetc_predicate_t >{};
}

//
// language_predicate_t
//
//FIXME: document this!
/*!
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
//FIXME: document this!
RESTINIO_NODISCARD
auto
language_symbol_producer()
{
	return ep_impl::symbol_producer_template_t< language_predicate_t >{};
}

//
// attr_char_predicate_t
//
//FIXME: document this!
/*!
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
//FIXME: document this!
RESTINIO_NODISCARD
auto
attr_char_symbol_producer()
{
	return ep_impl::symbol_producer_template_t< attr_char_predicate_t >{};
}

//
// hexdigit_predicate_t
//
//FIXME: document this!
/*!
 * @since v.0.6.1
 */
struct hexdigit_predicate_t
{
	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		const char normalized_actual = static_cast<char>(
						restinio::impl::to_lower_lut<unsigned char>()[
							static_cast<std::size_t>(
								static_cast<unsigned char>(actual))
						]
				);
		return hfp_impl::is_digit(normalized_actual)
				|| 'a' == normalized_actual
				|| 'b' == normalized_actual
				|| 'c' == normalized_actual
				|| 'd' == normalized_actual
				|| 'e' == normalized_actual
				|| 'f' == normalized_actual
				;
	}
};

//
// hexdigit_symbol_producer
//
//FIXME: document this!
RESTINIO_NODISCARD
auto
hexdigit_symbol_producer()
{
	return ep_impl::symbol_producer_template_t< hexdigit_predicate_t >{};
}

//
// pct_encoded_result_type_t
//
using pct_encoded_result_type_t = std::array< char, 3 >;

//
// pct_encoded_one_symbol_consumer_t
//
template< std::size_t I >
struct pct_encoded_one_symbol_consumer_t : public ep_impl::consumer_tag
{
	void
	consume( pct_encoded_result_type_t & to, char && symbol ) const noexcept
	{
		to[ I ] = symbol;
	}
};

//
// pct_encoded_symbols_producer
//
//FIXME: document this!
RESTINIO_NODISCARD
auto
pct_encoded_symbols_producer()
{
	return produce< pct_encoded_result_type_t >(
			symbol_producer( '%' ) >> pct_encoded_one_symbol_consumer_t<0>{},
			hexdigit_symbol_producer() >> pct_encoded_one_symbol_consumer_t<1>{},
			hexdigit_symbol_producer() >> pct_encoded_one_symbol_consumer_t<2>{}
		);
}

//
// pct_encoded_symbols_consumer_t
//
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
RESTINIO_NODISCARD
auto
ext_parameter_value_producer()
{
	return produce< std::string >(
			repeat( 1, N, mime_charsetc_symbol_producer() >> to_container() ),
			symbol_producer( '\'' ) >> to_container(),
			repeat( 0, N, language_symbol_producer() >> to_container() ),
			symbol_producer( '\'' ) >> to_container(),
			repeat( 1, N,
				alternatives(
					attr_char_symbol_producer() >> to_container(),
					pct_encoded_symbols_producer() >>
							pct_encoded_symbols_consumer_t{} )
			)
		);
}

} /* namespace content_disposition_details */

//
// content_disposition_value_t
//
struct content_disposition_value_t
{
	using parameter_t = parameter_with_mandatory_value_t;

	using parameter_container_t = parameter_with_mandatory_value_container_t;

	std::string value;
	parameter_container_t parameters;

	static auto
	make_parser()
	{
		using namespace content_disposition_details;

		return produce< content_disposition_value_t >(
			token_producer() >> to_lower()
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
									token_producer() >> &parameter_t::second,
									quoted_string_producer() >> &parameter_t::second
								)
							),
							sequence(
								ext_token_producer_t{}
										>> to_lower() >> &parameter_t::first,
								symbol('='),
								ext_parameter_value_producer() >> &parameter_t::second
							)
						)
					) >> to_container()
				)
			) >> &content_disposition_value_t::parameters
		);
	}

	static auto
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

