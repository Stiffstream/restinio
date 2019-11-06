/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of Content-Type HTTP-field.
 *
 * @since v.0.6.1
 */

#pragma once

#include <restinio/helpers/http_field_parsers/media-type.hpp>

namespace restinio
{

namespace http_field_parsers
{

//
// content_type_value_t
//
/*!
 * @brief Tools for working with the value of Content-Type HTTP-field.
 *
 * This struct represents parsed value of HTTP-field Content-Type
 * (see https://tools.ietf.org/html/rfc7231#section-3.1.1.5):
@verbatim
     Content-Type = media-type
@endverbatim
 *
 * Where `media-type` is repesented by media_type_value_t.
 *
 * @since v.0.6.1
 */
struct content_type_value_t
{
	media_type_value_t media_type;

	/*!
	 * @brief A factory function for a parser of Content-Type value.
	 *
	 * @since v.0.6.1
	 */
	RESTINIO_NODISCARD
	static auto
	make_parser()
	{
		return produce< content_type_value_t >(
			media_type_value_t::make_default_parser()
					>> &content_type_value_t::media_type
		);
	}

	/*!
	 * @brief An attempt to parse Content-Encoding HTTP-field.
	 *
	 * @since v.0.6.1
	 */
	RESTINIO_NODISCARD
	static expected_t< content_type_value_t, restinio::easy_parser::parse_error_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

