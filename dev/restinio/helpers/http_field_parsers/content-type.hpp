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
struct content_type_value_t
{
	media_type_value_t m_media_type;

	static auto
	make_parser()
	{
		return produce< content_type_value_t >(
			media_type_value_t::make_default_parser()
					>> &content_type_value_t::m_media_type
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

