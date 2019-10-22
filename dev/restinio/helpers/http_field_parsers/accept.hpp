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
struct accept_value_t
{
	struct item_t
	{
		using accept_ext_t = std::pair<
				std::string,
				restinio::optional_t< std::string > >;

		using accept_ext_container_t = std::map<
				std::string,
				restinio::optional_t< std::string > >;

		media_type_value_t m_media_type;
		restinio::optional_t< qvalue_t > m_weight;
		accept_ext_container_t m_accept_params;
	};

	using item_container_t = std::vector< item_t >;

	item_container_t m_items;

	static auto
	make_parser()
	{
		return produce< accept_value_t >(
			maybe_empty_comma_separated_list_producer< item_container_t >(
				produce< item_t >(
					media_type_value_t::make_parser() >> &item_t::m_media_type,
					maybe(
						weight_producer() >> &item_t::m_weight,
						produce< item_t::accept_ext_container_t >(
							repeat( 0, N,
								produce< item_t::accept_ext_t >(
									ows(),
									symbol(';'),
									ows(),
									token_producer() >> to_lower()
											>> &item_t::accept_ext_t::first,
									maybe(
										symbol('='),
										alternatives(
											token_producer()
													>> &item_t::accept_ext_t::second,
											quoted_string_producer()
													>> &item_t::accept_ext_t::second
										)
									)
								) >> to_container()
							)
						) >> &item_t::m_accept_params
					)
				)
			) >> &accept_value_t::m_items
		);
	}

	static std::pair< bool, accept_value_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

