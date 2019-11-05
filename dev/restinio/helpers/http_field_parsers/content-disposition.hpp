/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of Content-Disposition HTTP-field.
 *
 * @since v.0.6.1
 */

#pragma once

#include <restinio/helpers/http_field_parsers/basics.hpp>

namespace restinio
{

namespace http_field_parsers
{

namespace content_disposition_details
{

namespace ep_impl = restinio::easy_parser::impl;
namespace hfp_impl = restinio::http_field_parsers::impl;

//
// regular_token_producer_t
//
//FIXME: document this!
class regular_token_producer_t
	:	public hfp_impl::token_producer_t
{
public:
	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse( ep_impl::source_t & from ) const
	{
		ep_impl::source_t::content_consumer_t consumer{ from };
		const auto result = hfp_impl::token_producer_t::try_parse( from );
		if( result )
		{
			if( '*' == *(result->rbegin()) )
			{
				// Regular token can't have the trailing '*'.
				return make_unexpected( parse_error_t{
						consumer.started_at() + result->size() - 1,
						error_reason_t::unexpected_character
					} );
			}

			consumer.commit();
		}

		return result;
	}
};

//
// ext_token_producer_t
//
//FIXME: document this!
class ext_token_producer_t
	:	public hfp_impl::token_producer_t
{
public:
	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse( ep_impl::source_t & from ) const
	{
		ep_impl::source_t::content_consumer_t consumer{ from };
		const auto result = hfp_impl::token_producer_t::try_parse( from );
		if( result )
		{
			if( '*' != *(result->rbegin()) )
			{
				// Extended token should have the trailing '*'.
				return make_unexpected( parse_error_t{
						consumer.started_at(),
						error_reason_t::pattern_not_found
					} );
			}

			consumer.commit();
		}

		return result;
	}
};
} /* namespace content_disposition_details */

//
// content_disposition_value_t
//
struct content_disposition_value_t
{
	using parameter_t = parameter_with_mandatory_value_t;

	using parameter_container_t = parameter_with_mandatory_value_container_t;

	std::string value;
	parameter_container_t parameters;

	static auto
	make_parser()
	{
		using namespace content_disposition_details;

		return produce< content_disposition_value_t >(
			token_producer() >> to_lower()
					>> &content_disposition_value_t::value,
			produce< parameter_container_t >(
				repeat( 0, N,
					produce< parameter_t >(
						ows(),
						symbol(';'),
						ows(),
						alternatives(
							sequence(
								regular_token_producer_t{}
										>> to_lower() >> &parameter_t::first,
								symbol('='),
								alternatives(
									token_producer() >> &parameter_t::second,
									quoted_string_producer() >> &parameter_t::second
								)
							),
							sequence(
								ext_token_producer_t{}
										>> to_lower() >> &parameter_t::first,
								symbol('='),
								quoted_string_producer() >> &parameter_t::second
							)
						)
					) >> to_container()
				)
			) >> &content_disposition_value_t::parameters
		);
	}

	static auto
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

