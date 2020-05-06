/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of Authorization HTTP-field.
 *
 * @since v.0.6.7
 */

#pragma once

#include <restinio/helpers/http_field_parsers/basics.hpp>

#include <restinio/variant.hpp>

#include <iostream>

namespace restinio
{

namespace http_field_parsers
{

namespace authorization_details
{

namespace hfp_impl = restinio::http_field_parsers::impl;

//
// is_token68_char_predicate_t
//
/*!
 * @brief A preducate for symbol_producer_template that checks that
 * a symbol can be used inside token68 from RFC7235.
 *
 * @since v.0.6.7
 */
struct is_token68_char_predicate_t
	: protected hfp_impl::is_alphanum_predicate_t
{
	using base_type_t = hfp_impl::is_alphanum_predicate_t;

	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		return base_type_t::operator()(actual)
				|| '-' == actual
				|| '.' == actual
				|| '_' == actual
				|| '~' == actual
				|| '+' == actual
				|| '/' == actual
				;
	}
};

//
// token68_symbol_p
//
RESTINIO_NODISCARD
inline auto
token68_symbol_p()
{
	return restinio::easy_parser::impl::symbol_producer_template_t<
			is_token68_char_predicate_t >{};
}

//FIXME: document this!
struct token68_t
{
	std::string value;
};

inline std::ostream &
operator<<( std::ostream & to, const token68_t & v )
{
	return (to << v.value);
}

//
// token68_p
//
RESTINIO_NODISCARD
inline auto
token68_p()
{
	// NOTE: token68 should consume all input.
	// So there should not be any symbols after the value.
	return produce< token68_t >(
			produce< std::string >(
				repeat( 1, N, token68_symbol_p() >> to_container() ),
				repeat( 0, N, symbol_p('=') >> to_container() ),
				not_clause( any_symbol_p() >> skip() )
			) >> &token68_t::value
		);
}

} /* authorization_details */

//
// authorization_value_t
//
/*!
 * @brief Tools for working with the value of Authorization HTTP-field.
 *
 * This struct represents parsed value of HTTP-field Authorization
 * (see https://tools.ietf.org/html/rfc7235):
@verbatim
Authorization = credentials

credentials = auth-scheme [ 1*SP ( token68 / [ #auth-param ] ) ]

auth-scheme = token

auth-param = token BWS "=" BWS ( token / quoted-string )

token68 = 1*( ALPHA / DIGIT / "-" / "." / "_" / "~" / "+" / "/" ) *"="
@endverbatim
 *
 * @since v.0.6.7
 */
struct authorization_value_t
{
	//FIXME: document this!
	enum class value_form_t
	{
		token,
		quoted_string
	};

	//FIXME: document this!
	struct param_value_t
	{
		std::string value;
		value_form_t form;
	};

	//FIXME: document this!
	struct param_t
	{
		std::string name;
		param_value_t value;
	};

	using param_container_t = std::vector< param_t >;

	//FIXME: document this!
	using token68_t = authorization_details::token68_t;

	//FIXME: document this!
	using auth_param_t = variant_t< token68_t, param_container_t >;

	std::string auth_scheme;
	auth_param_t auth_param;

	/*!
	 * @brief A factory function for a parser of Authorization value.
	 *
	 * @since v.0.6.7
	 */
	RESTINIO_NODISCARD
	static auto
	make_parser()
	{
		using namespace authorization_details;

		auto token_to_v = []( std::string v ) -> param_value_t {
			return { std::move(v), value_form_t::token };
		};
		auto qstring_to_v = []( std::string v ) -> param_value_t {
			return { std::move(v), value_form_t::quoted_string };
		};

		return produce< authorization_value_t >(
				token_p() >> to_lower() >> &authorization_value_t::auth_scheme,
				maybe(
					repeat( 1, N, space() ),
					produce< auth_param_t >(
						alternatives(
							token68_p() >> as_result(),
							maybe_empty_comma_separated_list_p< param_container_t >(
								produce< param_t >(
									token_p() >> to_lower() >> &param_t::name,
									ows(),
									symbol('='),
									ows(),
									produce< param_value_t >(
										alternatives(
											token_p() >> convert( token_to_v ) >> as_result(),
											quoted_string_p() >> convert( qstring_to_v )
													>> as_result()
										)
									) >> &param_t::value
								)
							) >> as_result()
						)
					) >> &authorization_value_t::auth_param
				)
		);
	}

	/*!
	 * @brief An attempt to parse Authorization HTTP-field.
	 *
	 * @since v.0.6.7
	 */
	RESTINIO_NODISCARD
	static expected_t<
			authorization_value_t,
			restinio::easy_parser::parse_error_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

inline std::ostream &
operator<<(
	std::ostream & to,
	const authorization_value_t::param_value_t & v )
{
	if(authorization_value_t::value_form_t::token == v.form)
		to << v.value;
	else
		to << '"' << v.value << '"';
	return to;
}

inline std::ostream &
operator<<(
	std::ostream & to,
	const authorization_value_t::param_t & v )
{
	return (to << v.name << '=' << v.value);
}

inline std::ostream &
operator<<(
	std::ostream & to,
	const authorization_value_t::auth_param_t & p )
{
	struct printer_t
	{
		std::ostream & to;

		void
		operator()( const authorization_value_t::token68_t & t ) const
		{
			to << t;
		}

		void
		operator()( const authorization_value_t::param_container_t & c ) const
		{
			bool first = true;
			to << '{';
			for( const auto & param : c )
			{
				if( !first )
					to << ", ";
				else
					first = false;

				to << param;
			}
			to << '}';
		}
	};

	restinio::visit( printer_t{ to }, p );

	return to;
}

inline std::ostream &
operator<<(
	std::ostream & to,
	const authorization_value_t & v )
{
	return (to << v.auth_scheme << ' ' << v.auth_param);
}

} /* namespace http_field_parsers */

} /* namespace restinio */

