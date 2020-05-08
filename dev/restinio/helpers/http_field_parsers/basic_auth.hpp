/*
 * RESTinio
 */

/*!
 * @file
 * @brief Helpers for dealing with basic authentification.
 *
 * @since v.0.6.7
 */

#pragma once

#include <restinio/helpers/http_field_parsers/authorization.hpp>

#include <restinio/utils/base64.hpp>

#include <restinio/http_headers.hpp>
#include <restinio/request_handler.hpp>
#include <restinio/expected.hpp>

#include <iostream>

namespace restinio
{

namespace http_field_parsers
{

namespace basic_auth
{

//
// params_t
//
/*!
 * @brief Parameters for basic authentification.
 *
 * @since v.0.6.7
 */
struct params_t
{
	//! Name of a user.
	/*!
	 * Can't be empty.
	 */
	std::string username;

	//! Password for a user.
	/*!
	 * Can be empty.
	 */
	std::string password;
};

//
// extraction_error_t
//
/*!
 * @brief Error codes for failures of extraction of basic authentification
 * parameters.
 *
 * @since v.0.6.7
 */
enum class extraction_error_t
{
	//! There is no HTTP field with authentification parameters.
	no_auth_http_field,
	
	//! The HTTP field with authentification parameters can't be parser.
	illegal_http_field_value,

	//! Different authentification scheme found.
	//! Basic authentification scheme is expected.
	not_basic_auth_scheme,

	//! Invalid value of parameter for basic authentification scheme.
	//! The single parameter in the form of token68 is expected.
	invalid_basic_auth_param,

	//! Value of token68 parameter for basic authentification can't be decoded.
	token68_decode_error,

	//! Wrong format for username:password in decoded token68 parameter.
	//! Maybe there is no colon symbol.
	invalid_username_password_pair,

	//! Empty user name in username:password pair.
	empty_username,
};

//
// try_extract_params
//
//FIXME: document this!
RESTINIO_NODISCARD
inline expected_t< params_t, extraction_error_t >
try_extract_params(
	const request_t & req,
	string_view_t auth_field_name )
{
	const auto opt_auth_field_value =
			req.header().opt_value_of( auth_field_name );
	if( !opt_auth_field_value )
		return make_unexpected( extraction_error_t::no_auth_http_field );

	const auto field_value_parse_result = authorization_value_t::try_parse(
			*opt_auth_field_value );
	if( !field_value_parse_result )
		return make_unexpected( extraction_error_t::illegal_http_field_value );

	const auto & parsed_value = *field_value_parse_result;
	if( "basic" != parsed_value.auth_scheme )
		return make_unexpected( extraction_error_t::not_basic_auth_scheme );

	const auto * token68 = get_if<authorization_value_t::token68_t>(
			&parsed_value.auth_param );
	if( !token68 )
		return make_unexpected( extraction_error_t::invalid_basic_auth_param );

	const auto unbase64_result =
			restinio::utils::base64::try_decode( token68->value );
	if( !unbase64_result )
		return make_unexpected( extraction_error_t::token68_decode_error );

	const std::string & username_password = *unbase64_result;
	const auto first_colon = username_password.find( ':' );
	if( std::string::npos == first_colon )
		return make_unexpected(
				extraction_error_t::invalid_username_password_pair );
	if( 0u == first_colon )
		return make_unexpected( extraction_error_t::empty_username );

	return params_t{
			username_password.substr( 0u, first_colon ),
			username_password.substr( first_colon + 1u )
	};
}

//FIXME: document this!
RESTINIO_NODISCARD
inline expected_t< params_t, extraction_error_t >
try_extract_params(
	const request_t & req,
	http_field_t auth_field_id )
{
	return try_extract_params( req, field_to_string( auth_field_id ) );
}

} /* namespace basic_auth */

} /* namespace http_field_parsers */

} /* namespace restinio */

