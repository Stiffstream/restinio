/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of Accept HTTP-field.
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
// accept_value_t
//
/*!
 * @brief Tools for working with the value of Accept HTTP-field.
 *
 * This struct represents parsed value of HTTP-field Accept
 * (see https://tools.ietf.org/html/rfc7231#section-5.3.2):
@verbatim
     Accept = #( media-range [ accept-params ] )

     media-range    = ( "*" "/" "*"
                      / ( type "/" "*" )
                      / ( type "/" subtype )
                      ) *( OWS ";" OWS parameter )
     accept-params  = weight *( accept-ext )
     accept-ext = OWS ";" OWS token [ "=" ( token / quoted-string ) ]

     weight = OWS ";" OWS "q=" qvalue
     qvalue = ( "0" [ "." 0*3DIGIT ] )
            / ( "1" [ "." 0*3("0") ] )
@endverbatim
 *
 * @note
 * Parameter names are converted to lower case during the parsing.
 * Parameter values are left as they are.
 *
 * @since v.0.6.1
 */
struct accept_value_t
{
	struct item_t
	{
		using accept_ext_t = parameter_with_optional_value_t;

		using accept_ext_container_t = parameter_with_optional_value_container_t;

		media_type_value_t media_type;
		restinio::optional_t< qvalue_t > weight;
		accept_ext_container_t accept_params;
	};

	using item_container_t = std::vector< item_t >;

	item_container_t items;

	/*!
	 * @brief A factory function for a parser of Accept value.
	 *
	 * @since v.0.6.1
	 */
	RESTINIO_NODISCARD
	static auto
	make_parser()
	{
		const auto media_type = media_type_value_t::make_weight_aware_parser();

		return produce< accept_value_t >(
			maybe_empty_comma_separated_list_p< item_container_t >(
				produce< item_t >(
					media_type >> &item_t::media_type,
					maybe(
						weight_p() >> &item_t::weight,
						params_with_opt_value_p() >> &item_t::accept_params
					)
				)
			) >> &accept_value_t::items
		);
	}

	/*!
	 * @brief An attempt to parse Accept HTTP-field.
	 *
	 * @since v.0.6.1
	 */
	RESTINIO_NODISCARD
	static expected_t< accept_value_t, restinio::easy_parser::parse_error_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

