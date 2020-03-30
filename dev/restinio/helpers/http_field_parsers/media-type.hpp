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
 * format (see https://tools.ietf.org/html/rfc7231 section
 * `3.1.1.1. Media Type`):
@verbatim
     media-type = type "/" subtype *( OWS ";" OWS parameter )
     type       = token
     subtype    = token
     parameter  = token "=" ( token / quoted-string )
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

	/*!
	 * @brief Make a default parser that doesn't handles weight parameter
	 * a special way.
	 *
	 * This parser handles the following rules:
@verbatim
     media-type = type "/" subtype *( OWS ";" OWS parameter )
     type       = token
     subtype    = token
     parameter  = token "=" ( token / quoted-string )
@endverbatim
	 * @since v.0.6.1
	 */
	RESTINIO_NODISCARD
	static auto
	make_default_parser()
	{
		return produce< media_type_value_t >(
			token_p() >> to_lower() >> &media_type_value_t::type,
			symbol('/'),
			token_p() >> to_lower() >> &media_type_value_t::subtype,
			params_with_value_p() >> &media_type_value_t::parameters
		);
	}

	/*!
	 * @brief Make a special parser that stops when weight parameter is found.
	 *
	 * This parser handles the following rules:
@verbatim
     media-type = type "/" subtype *( ![weight] OWS ";" OWS parameter )
     type       = token
     subtype    = token
     parameter  = token "=" ( token / quoted-string )
     weight = OWS ";" OWS "q=" qvalue
     qvalue = ( "0" [ "." 0*3DIGIT ] )
            / ( "1" [ "." 0*3("0") ] )
@endverbatim
	 * @since v.0.6.1
	 */
	RESTINIO_NODISCARD
	static auto
	make_weight_aware_parser()
	{
		return produce< media_type_value_t >(
			token_p() >> to_lower() >> &media_type_value_t::type,
			symbol('/'),
			token_p() >> to_lower() >> &media_type_value_t::subtype,
			produce< parameter_container_t >(
				repeat( 0, N,
					produce< parameter_t >(
						not_clause( weight_p() >> skip() ),
						ows(),
						symbol(';'),
						ows(),
						token_p() >> to_lower() >> &parameter_t::first,
						symbol('='),
						alternatives(
							token_p() >> &parameter_t::second,
							quoted_string_p() >> &parameter_t::second
						)
					) >> to_container()
				)
			) >> &media_type_value_t::parameters
		);
	}

	/*!
	 * @brief An attempt to parse media-type value.
	 *
	 * @note
	 * This method uses make_default_parser() for actual parser.
	 *
	 * @since v.0.6.1
	 */
	RESTINIO_NODISCARD
	static expected_t< media_type_value_t, restinio::easy_parser::parse_error_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_default_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

