/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of Content-Coding HTTP-field.
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
// content_coding_value_t
//
struct content_coding_value_t
{
	std::string m_value;

	static auto
	make_parser()
	{
		return produce< content_coding_value_t >(
			token_producer() >> to_lower() >> &content_coding_value_t::m_value
		);
	}

	static std::pair< bool, content_coding_value_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

