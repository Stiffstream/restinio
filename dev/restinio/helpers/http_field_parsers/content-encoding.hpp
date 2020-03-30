/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of Content-Encoding HTTP-field.
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
// content_encoding_value_t
//
/*!
 * @brief Tools for working with the value of Content-Encoding HTTP-field.
 *
 * This struct represents parsed value of HTTP-field Content-Encoding
 * (see https://tools.ietf.org/html/rfc7231#section-3.1.2.2):
@verbatim
     Content-Encoding = 1#content-coding
     content-coding   = token
@endverbatim
 *
 * @note
 * Parameter names are converted to lower case during the parsing.
 * Parameter values are left as they are.
 *
 * @since v.0.6.1
 */
struct content_encoding_value_t
{
	using value_container_t = std::vector< std::string >;

	value_container_t values;

	/*!
	 * @brief A factory function for a parser of Content-Encoding value.
	 *
	 * @since v.0.6.1
	 */
	RESTINIO_NODISCARD
	static auto
	make_parser()
	{
		return produce< content_encoding_value_t >(
			non_empty_comma_separated_list_p< value_container_t >(
				token_p() >> to_lower()
			) >> &content_encoding_value_t::values
		);
	}

	/*!
	 * @brief An attempt to parse Content-Encoding HTTP-field.
	 *
	 * @since v.0.6.1
	 */
	RESTINIO_NODISCARD
	static expected_t< content_encoding_value_t, restinio::easy_parser::parse_error_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

