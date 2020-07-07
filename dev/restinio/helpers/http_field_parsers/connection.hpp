/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of Connection HTTP-field.
 *
 * @since v.0.6.9
 */

#pragma once

#include <restinio/helpers/http_field_parsers/basics.hpp>

namespace restinio
{

namespace http_field_parsers
{

//
// connection_value_t
//
/*!
 * @brief Tools for working with the value of Connection HTTP-field.
 *
 * This struct represents parsed value of HTTP-field Connection
 * (see https://tools.ietf.org/html/rfc7230#section-6):
@verbatim
Connection        = 1#connection-option
connection-option = token
@endverbatim
 *
 * @note
 * Connection options are converted to lower case during the parsing.
 *
 * @since v.0.6.9
 */
struct connection_value_t
{
	using value_container_t = std::vector< std::string >;

	value_container_t values;

	/*!
	 * @brief A factory function for a parser of Connection value.
	 *
	 * @since v.0.6.9
	 */
	RESTINIO_NODISCARD
	static auto
	make_parser()
	{
		return produce< connection_value_t >(
			non_empty_comma_separated_list_p< value_container_t >(
				token_p() >> to_lower()
			) >> &connection_value_t::values
		);
	}

	/*!
	 * @brief An attempt to parse Connection HTTP-field.
	 *
	 * @since v.0.6.9
	 */
	RESTINIO_NODISCARD
	static expected_t< connection_value_t, restinio::easy_parser::parse_error_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

