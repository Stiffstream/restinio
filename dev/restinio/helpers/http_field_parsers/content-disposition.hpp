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

//
// content_disposition_value_t
//
struct content_disposition_value_t
{
	using parameter_t = parameter_with_mandatory_value_t;

	using parameter_container_t = parameter_with_mandatory_value_container_t;

	std::string m_value;
	parameter_container_t m_parameters;

	static auto
	make_parser()
	{
		return produce< content_disposition_value_t >(
			token_producer() >> to_lower()
					>> &content_disposition_value_t::m_value,
			params_with_value_producer()
					>> &content_disposition_value_t::m_parameters
		);
	}

	static std::pair< bool, content_disposition_value_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

