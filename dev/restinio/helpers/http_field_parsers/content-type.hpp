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

#include <restinio/helpers/http_field_parser.hpp>
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
		using namespace restinio::http_field_parser;

		return produce< content_type_value_t >(
			media_type_value_t::make_parser() >> &content_type_value_t::m_media_type
		);
	}

	static std::pair< bool, content_type_value_t >
	try_parse( string_view_t what )
	{
		using namespace restinio::http_field_parser;

		return try_parse_field_value( what, make_parser() );
	}
};

} /* namespace http_fields */

} /* namespace restinio */

