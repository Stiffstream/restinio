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

#include <restinio/helpers/http_field_parser.hpp>

namespace restinio
{

namespace http_field_parsers
{

//
// cache_control_value_t
//
struct cache_control_value_t
{
	using directive_t = std::pair<
			std::string,
			restinio::optional_t<std::string> >;

	using directive_container_t = std::map<
			std::string, restinio::optional_t<std::string> >;

	directive_container_t m_directives;

	static auto
	make_parser()
	{
		using namespace restinio::http_field_parser;

		return produce< cache_control_value_t >(
			one_or_more_of< directive_container_t >(
				produce< directive_t >(
					rfc::token() >> to_lower() >> &directive_t::first,
					optional< std::string >(
						symbol('=') >> skip(),
						alternatives< std::string >(
							rfc::token() >> to_lower(),
							rfc::quoted_string() ) >> as_result()
					) >> &directive_t::second
				)
			) >> &cache_control_value_t::m_directives
		) >> as_result();
	}

	static std::pair< bool, cache_control_value_t >
	try_parse( string_view_t what )
	{
		using namespace restinio::http_field_parser;

		return try_parse_field_value< cache_control_value_t >(
				what,
				make_parser() );
	}
};

} /* namespace http_fields */

} /* namespace restinio */

