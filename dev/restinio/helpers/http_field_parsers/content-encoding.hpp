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
struct content_encoding_value_t
{
	using value_container_t = std::vector< std::string >;

	value_container_t m_values;

	static auto
	make_parser()
	{
		return produce< content_encoding_value_t >(
			non_empty_comma_separated_list_producer< value_container_t >(
				token_producer() >> to_lower()
			) >> &content_encoding_value_t::m_values
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

