/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of Accept-Charset HTTP-field.
 *
 * @since v.0.6.2
 */

#pragma once

#include <restinio/helpers/http_field_parsers/basics.hpp>

namespace restinio
{

namespace http_field_parsers
{

//
// accept_charset_value_t
//
/*!
 * @brief Tools for working with the value of Accept-Charset HTTP-field.
 *
 * This struct represents parsed value of HTTP-field Accept-Charset
 * (see https://tools.ietf.org/html/rfc7231#section-5.3.3):
@verbatim
Accept-Charset = 1#( ( charset / "*" ) [ weight ] )
charset = token
@endverbatim
 *
 * @note
 * Values of `charset` are converted to lower case during the parsing.
 *
 * @since v.0.6.2
 */
struct accept_charset_value_t
{
	struct item_t
	{
		std::string charset;
		qvalue_t weight{ qvalue_t::maximum };
	};

	using item_container_t = std::vector< item_t >;

	item_container_t charsets;

	/*!
	 * @brief A factory function for a parser of Accept-Charset value.
	 *
	 * @since v.0.6.2
	 */
	RESTINIO_NODISCARD
	static auto
	make_parser()
	{
		return produce< accept_charset_value_t >(
			non_empty_comma_separated_list_p< item_container_t >(
				produce< item_t >(
					token_p() >> to_lower() >> &item_t::charset,
					maybe( weight_p() >> &item_t::weight )
				)
			) >> &accept_charset_value_t::charsets
		);
	}

	/*!
	 * @brief An attempt to parse Accept-Charset HTTP-field.
	 *
	 * @since v.0.6.2
	 */
	RESTINIO_NODISCARD
	static expected_t< accept_charset_value_t, restinio::easy_parser::parse_error_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

