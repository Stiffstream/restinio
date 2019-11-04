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

#include <restinio/helpers/http_field_parsers/basics.hpp>

namespace restinio
{

namespace http_field_parsers
{

//
// media_type_value_t
//
/*!
 * @brief Tools for working with media-type in HTTP-fields.
 *
 * This struct represents parsed value of media-type.
 * Media-type is present in different HTTP-fields and has the following
 * format:
@verbatim
@endverbatim
 *
 * @since v.0.6.1
 */
struct media_type_value_t
{
	using parameter_t = parameter_with_mandatory_value_t;

	using parameter_container_t = parameter_with_mandatory_value_container_t;

	std::string type;
	std::string subtype;
	parameter_container_t parameters;

	static auto
	make_default_parser()
	{
		return produce< media_type_value_t >(
			token_producer() >> to_lower() >> &media_type_value_t::type,
			symbol('/'),
			token_producer() >> to_lower() >> &media_type_value_t::subtype,
			params_with_value_producer() >> &media_type_value_t::parameters
		);
	}

	static auto
	make_weight_aware_parser()
	{
		return produce< media_type_value_t >(
			token_producer() >> to_lower() >> &media_type_value_t::type,
			symbol('/'),
			token_producer() >> to_lower() >> &media_type_value_t::subtype,
			produce< parameter_container_t >(
				repeat( 0, N,
					produce< parameter_t >(
						not_clause( weight_producer() >> skip() ),
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
			) >> &media_type_value_t::parameters
		);
	}

	static auto
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_default_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

