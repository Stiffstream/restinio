/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of Cache-Control HTTP-field.
 *
 * @since v.0.6.1
 */

#pragma once

#include <restinio/helpers/http_field_parsers/basics.hpp>

namespace restinio
{

namespace http_field_parsers
{

//
// cache_control_value_t
//
/*!
 * @brief Tools for working with the value of Cache-Control HTTP-field.
 *
 * This struct represents parsed value of HTTP-field Cache-Control
 * (see https://tools.ietf.org/html/rfc7234#section-5.2):
@verbatim
     Cache-Control   = 1#cache-directive

     cache-directive = token [ "=" ( token / quoted-string ) ]
@endverbatim
 *
 * @note
 * Parameter names are converted to lower case during the parsing.
 * Parameter values are left as they are.
 *
 * @since v.0.6.1
 */
struct cache_control_value_t
{
	using directive_t = parameter_with_optional_value_t;

	using directive_container_t = parameter_with_optional_value_container_t;

	directive_container_t directives;

	/*!
	 * @brief A factory function for a parser of Cache-Control value.
	 *
	 * @since v.0.6.1
	 */
	RESTINIO_NODISCARD
	static auto
	make_parser()
	{
		return produce< cache_control_value_t >(
			non_empty_comma_separated_list_p< directive_container_t >(
				produce< directive_t >(
					token_p() >> to_lower() >> &directive_t::first,
					maybe(
						symbol('='),
						alternatives(
							token_p() >> &directive_t::second,
							quoted_string_p() >> &directive_t::second
						)
					)
				)
			) >> &cache_control_value_t::directives
		);
	}

	/*!
	 * @brief An attempt to parse Cache-Control HTTP-field.
	 *
	 * @since v.0.6.1
	 */
	RESTINIO_NODISCARD
	static expected_t< cache_control_value_t, restinio::easy_parser::parse_error_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

