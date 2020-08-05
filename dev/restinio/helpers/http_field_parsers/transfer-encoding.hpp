/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of Transfer-Encoding HTTP-field.
 *
 * @since v.0.6.9
 */

#pragma once

#include <restinio/helpers/http_field_parsers/basics.hpp>

#include <restinio/variant.hpp>

#include <tuple>

namespace restinio
{

namespace http_field_parsers
{

//
// transfer_encoding_value_t
//
/*!
 * @brief Tools for working with the value of Transfer-Encoding HTTP-field.
 *
 * This struct represents parsed value of HTTP-field Transfer-Encoding
 * (see https://tools.ietf.org/html/rfc7230#section-3.3.1 and
 * https://tools.ietf.org/html/rfc7230#section-4):
@verbatim
Transfer-Encoding  = 1#transfer-coding
transfer-coding    = "chunked"
                   / "compress"
                   / "deflate"
                   / ("gzip" | "x-gzip")
                   / transfer-extension
transfer-extension = token *( OWS ";" OWS transfer-parameter )
transfer-parameter = token BWS "=" BWS ( token / quoted-string )
@endverbatim
 *
 * @since v.0.6.9
 */
struct transfer_encoding_value_t
{
	//! Enumeration for transfer-coding values from RFC7230.
	enum class known_transfer_coding_t
	{
		chunked,
		compress,
		deflate,
		gzip,
	};

	RESTINIO_NODISCARD
	static constexpr known_transfer_coding_t chunked() noexcept
	{ return known_transfer_coding_t::chunked; }

	RESTINIO_NODISCARD
	static constexpr known_transfer_coding_t compress() noexcept
	{ return known_transfer_coding_t::compress; }

	RESTINIO_NODISCARD
	static constexpr known_transfer_coding_t deflate() noexcept
	{ return known_transfer_coding_t::deflate; }

	RESTINIO_NODISCARD
	static constexpr known_transfer_coding_t gzip() noexcept
	{ return known_transfer_coding_t::gzip; }

	//! Description of transfer-extension.
	struct transfer_extension_t
	{
		std::string token;
		parameter_with_mandatory_value_container_t transfer_parameters;

		RESTINIO_NODISCARD
		bool
		operator==( const transfer_extension_t & o ) const noexcept
		{
			return std::tie(this->token, this->transfer_parameters) ==
					std::tie(o.token, o.transfer_parameters);
		}
	};

	//! Type for one value from Transfer-Encoding HTTP-field.
	using value_t = variant_t<
			known_transfer_coding_t,
			transfer_extension_t
		>;

	using value_container_t = std::vector< value_t >;

	value_container_t values;

	/*!
	 * @brief A factory function for a parser of Transfer-Encoding value.
	 *
	 * @since v.0.6.9
	 */
	RESTINIO_NODISCARD
	static auto
	make_parser()
	{
		return produce< transfer_encoding_value_t >(
			non_empty_comma_separated_list_p< value_container_t >(
				produce< value_t >(
					alternatives(
						expected_caseless_token_p("chunked")
							>> just_result( chunked() ),
						expected_caseless_token_p("compress")
							>> just_result( compress() ),
						expected_caseless_token_p("deflate")
							>> just_result( deflate() ),
						expected_caseless_token_p("gzip")
							>> just_result( gzip() ),
						expected_caseless_token_p("x-gzip")
							>> just_result( gzip() ),
						produce< transfer_extension_t >(
							token_p() >> to_lower() >> &transfer_extension_t::token,
							params_with_value_p()
									>> &transfer_extension_t::transfer_parameters
						) >> as_result()
					)
				)
			) >> &transfer_encoding_value_t::values
		);
	}

	/*!
	 * @brief An attempt to parse Transfer-Encoding HTTP-field.
	 *
	 * @since v.0.6.9
	 */
	RESTINIO_NODISCARD
	static expected_t<
		transfer_encoding_value_t,
		restinio::easy_parser::parse_error_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

