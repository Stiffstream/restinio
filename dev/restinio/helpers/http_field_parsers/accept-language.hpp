/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of Accept-Language HTTP-field.
 *
 * @since v.0.6.2
 */

#pragma once

#include <restinio/helpers/http_field_parsers/basics.hpp>

namespace restinio
{

namespace http_field_parsers
{

namespace accept_language_details
{

namespace ep_impl = restinio::easy_parser::impl;
namespace hfp_impl = restinio::http_field_parsers::impl;

RESTINIO_NODISCARD
inline auto
make_language_tag_p()
{
	return produce<std::string>(
			repeat(1u, 8u, alpha_symbol_p() >> to_container()),
			repeat(0u, N,
					symbol_p('-') >> to_container(),
					repeat(1u, 8u, alphanum_symbol_p() >> to_container())
			)
	);
}

RESTINIO_NODISCARD
inline auto
make_language_range_p()
{
	return produce<std::string>(
			alternatives(
					symbol_p('*') >> to_container(),
					make_language_tag_p() >> as_result()
			)
	);
}

} /* namespace accept_language_details */

//
// accept_language_value_t
//
/*!
 * @brief Tools for working with the value of Accept-Language HTTP-field.
 *
 * This struct represents parsed value of HTTP-field Accept-Charset
 * (see https://tools.ietf.org/html/rfc7231#section-5.3.5 and
 * https://tools.ietf.org/html/rfc4647#section-2.1):
@verbatim
Accept-Language = 1#( language-range [ weight ] )
language-range  = (1*8ALPHA *("-" 1*8alphanum)) / "*"
alphanum        = ALPHA / DIGIT
@endverbatim
 *
 * @note
 * Values of `language-range` keep their case during parsing
 * (it means that they are not converted to lower or upper case).
 *
 * @since v.0.6.2
 */
struct accept_language_value_t
{
	struct item_t
	{
		std::string language_range;
		qvalue_t weight{ qvalue_t::maximum };
	};

	using item_container_t = std::vector< item_t >;

	item_container_t languages;

	/*!
	 * @brief A factory function for a parser of Accept-Language value.
	 *
	 * @since v.0.6.2
	 */
	RESTINIO_NODISCARD
	static auto
	make_parser()
	{
		using namespace accept_language_details;

		return produce< accept_language_value_t >(
			non_empty_comma_separated_list_p< item_container_t >(
				produce< item_t >(
					make_language_range_p() >> &item_t::language_range,
					maybe( weight_p() >> &item_t::weight )
				)
			) >> &accept_language_value_t::languages
		);
	}

	/*!
	 * @brief An attempt to parse Accept-Language HTTP-field.
	 *
	 * @since v.0.6.2
	 */
	RESTINIO_NODISCARD
	static expected_t< accept_language_value_t, restinio::easy_parser::parse_error_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

