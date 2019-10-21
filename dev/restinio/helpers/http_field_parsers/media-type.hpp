/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to Media-Type value in HTTP-fields.
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
// media_type_value_t
//
struct media_type_value_t
{
	using parameter_t = std::pair< std::string, std::string >;

	using parameter_container_t = std::vector< parameter_t >;

	std::string m_type;
	std::string m_subtype;
	parameter_container_t m_parameters;

	static auto
	make_parser()
	{
		using namespace restinio::http_field_parser;
		using namespace restinio::http_field_parser::rfc;

		return produce< media_type_value_t >(
			token_producer() >> to_lower() >> &media_type_value_t::m_type,
			symbol('/'),
			token_producer() >> to_lower() >> &media_type_value_t::m_subtype,
			produce< parameter_container_t >(
				repeat( 0, N,
					produce< parameter_t >(
						ows(),
						symbol(';'),
						ows(),
						token_producer() >> to_lower() >> &parameter_t::first,
						symbol('='),
						alternatives(
							token_producer() >> &parameter_t::second,
							quoted_string_producer() >> &parameter_t::second
						)
					) >> to_container()
				)
			) >> &media_type_value_t::m_parameters
		);
	}

	static std::pair< bool, media_type_value_t >
	try_parse( string_view_t what )
	{
		using namespace restinio::http_field_parser;

		return try_parse_field_value( what, make_parser() );
	}
};

} /* namespace http_fields */

} /* namespace restinio */

