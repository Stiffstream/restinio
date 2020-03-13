/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of User-Agent HTTP-field.
 *
 * @since v.0.6.4
 */

#pragma once

#include <restinio/helpers/http_field_parsers/basics.hpp>

#include <restinio/variant.hpp>

namespace restinio
{

namespace http_field_parsers
{

//
// user_agent_value_t
//
/*!
 * @brief Tools for working with the value of User-Agent HTTP-field.
 *
 * This struct represents parsed value of HTTP-field User-Agent
 * (see https://tools.ietf.org/html/rfc7231#section-5.5.3):
@verbatim
     User-Agent = product *( RWS ( product / comment ) )

     product         = token ["/" product-version]
     product-version = token
@endverbatim
 *
 * @since v.0.6.4
 */
struct user_agent_value_t
{
	/*!
	 * @brief A type for holding an info about a product.
	 *
	 * @since v.0.6.4
	 */
	struct product_t
	{
		std::string product;
		restinio::optional_t<std::string> product_version;
	};

	/*!
	 * @brief A type for holding an info about a product or a comment.
	 *
	 * @since v.0.6.4
	 */
	using tail_item_t = restinio::variant_t< product_t, std::string >;

	product_t product;
	std::vector< tail_item_t > tail;

	/*!
	 * @brief A factory function for a parser of User-Agent value.
	 *
	 * @since v.0.6.4
	 */
	RESTINIO_NODISCARD
	static auto
	make_parser()
	{
		auto product_producer = produce< product_t >(
				token_p() >> &product_t::product,
				maybe(
					symbol('/'),
					token_p() >> &product_t::product_version
				)
			);

		return produce< user_agent_value_t >(
				product_producer >> &user_agent_value_t::product,
				produce< std::vector< tail_item_t > >(
					repeat( 0, N,
						space(),
						ows(),
						alternatives(
							product_producer >> to_container(),
							comment_p() >> to_container()
						)
					)
				) >> &user_agent_value_t::tail
		);
	}

	/*!
	 * @brief An attempt to parse User-Agent HTTP-field.
	 *
	 * @since v.0.6.4
	 */
	RESTINIO_NODISCARD
	static expected_t< user_agent_value_t, restinio::easy_parser::parse_error_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

