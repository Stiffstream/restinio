/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of Accept-Encoding HTTP-field.
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
// accept_encoding_value_t
//
/*!
 * @brief Tools for working with the value of Accept-Encoding HTTP-field.
 *
 * This struct represents parsed value of HTTP-field Accept-Encoding
 * (see https://tools.ietf.org/html/rfc7231#section-5.3.4):
@verbatim
Accept-Encoding  = #( codings [ weight ] )
codings          = content-coding / "identity" / "*"
content-coding   = token
@endverbatim
 *
 * @note
 * Values of `condings` are converted to lower case during the parsing.
 *
 * @since v.0.6.2
 */
struct accept_encoding_value_t
{
	struct item_t
	{
		std::string content_coding;
		qvalue_t weight{ qvalue_t::maximum };
	};

	using item_container_t = std::vector< item_t >;

	item_container_t codings;

	/*!
	 * @brief A factory function for a parser of Accept-Encoding value.
	 *
	 * @since v.0.6.2
	 */
	RESTINIO_NODISCARD
	static auto
	make_parser()
	{
		return produce< accept_encoding_value_t >(
			maybe_empty_comma_separated_list_p< item_container_t >(
				produce< item_t >(
					token_p() >> to_lower() >> &item_t::content_coding,
					maybe( weight_p() >> &item_t::weight )
				)
			) >> &accept_encoding_value_t::codings
		);
	}

	/*!
	 * @brief An attempt to parse Accept-Encoding HTTP-field.
	 *
	 * @since v.0.6.2
	 */
	RESTINIO_NODISCARD
	static expected_t< accept_encoding_value_t, restinio::easy_parser::parse_error_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

