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
	
	//! The HTTP field with authentification parameters can't be parsed.
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
/*!
 * @brief Helper function for getting parameters of basic authentification
 * from an already parsed HTTP-field.
 *
 * @attention
 * This function doesn't check the content of
 * authorization_value_t::auth_scheme. It's expected that this field was
 * checked earlier.
 *
 * Usage example:
 * @code
 * auto on_request(restinio::request_handle_t & req) {
 * 	using namespace restinio::http_field_parsers;
 *		const auto opt_field = req.header().opt_value_of(
 * 			restinio::http_field::authorization);
 * 	if(opt_field) {
 * 		const auto parsed_field = authorization_value_t::try_parse(*opt_field);
 * 		if(parsed_field) {
 * 			if("basic" == parsed_field->auth_scheme) {
 * 				using namespace restinio::http_field_parsers::basic_auth;
 * 				const auto basic_params = try_extract_params(*parsed_field);
 * 				if(basic_params) {
 * 					const std::string & username = auth_params->username;
 * 					const std::string & password = auth_params->password;
 * 					... // Do something with username and password.
 * 				}
 * 			}
 * 			else if("bearer" == parsed_field->auth_scheme) {
 * 				... // Dealing with bearer authentification.
 * 			}
 * 			else {
 * 				... // Other authentification schemes.
 * 			}
 * 		}
 * 	}
 * 	...
 * }
 * @endcode
 *
 * @since v.0.6.8
 */
RESTINIO_NODISCARD
inline expected_t< params_t, extraction_error_t >
try_extract_params(
	const authorization_value_t & http_field )
{
	const auto * token68 = get_if<authorization_value_t::token68_t>(
			&http_field.auth_param );
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

namespace impl
{

RESTINIO_NODISCARD
inline expected_t< params_t, extraction_error_t >
perform_extraction_attempt(
	const optional_t< string_view_t > opt_field_value )
{
	if( !opt_field_value )
		return make_unexpected( extraction_error_t::no_auth_http_field );

	const auto field_value_parse_result = authorization_value_t::try_parse(
			*opt_field_value );
	if( !field_value_parse_result )
		return make_unexpected( extraction_error_t::illegal_http_field_value );

	const auto & parsed_value = *field_value_parse_result;
	if( "basic" != parsed_value.auth_scheme )
		return make_unexpected( extraction_error_t::not_basic_auth_scheme );

	return try_extract_params( parsed_value );
}

} /* namespace impl */

//
// try_extract_params
//
/*!
 * @brief Helper function for getting parameters of basic authentification
 * from a request.
 *
 * This helper function is intended to be used for cases when authentification
 * parameters are stored inside a HTTP-field with a custom name. For example:
 * @code
 * auto on_request(restinio::request_handle_t & req) {
 * 	using namespace restinio::http_field_parsers::basic_auth;
 * 	const auto auth_params = try_extract_params(*req, "X-My-Authorization");
 * 	if(auth_params) {
 * 		const std::string & username = auth_params->username;
 * 		const std::string & password = auth_params->password;
 * 		... // Do something with username and password.
 * 	}
 * 	...
 * }
 * @endcode
 *
 * @since v.0.6.7
 */
RESTINIO_NODISCARD
inline expected_t< params_t, extraction_error_t >
try_extract_params(
	//! A request that should hold a HTTP-field with authentification
	//! parameters.
	const request_t & req,
	//! The name of a HTTP-field with authentification parameters.
	string_view_t auth_field_name )
{
	return impl::perform_extraction_attempt(
			req.header().opt_value_of( auth_field_name ) );
}

/*!
 * @brief Helper function for getting parameters of basic authentification
 * from a request.
 *
 * Usage example:
 * @code
 * auto on_request(restinio::request_handle_t & req) {
 * 	using namespace restinio::http_field_parsers::basic_auth;
 * 	const auto auth_params = try_extract_params(
 * 			*req, restinio::http_field::authorization);
 * 	if(auth_params) {
 * 		const std::string & username = auth_params->username;
 * 		const std::string & password = auth_params->password;
 * 		... // Do something with username and password.
 * 	}
 * 	...
 * }
 * @endcode
 *
 * @since v.0.6.7
 */
RESTINIO_NODISCARD
inline expected_t< params_t, extraction_error_t >
try_extract_params(
	//! A request that should hold a HTTP-field with authentification
	//! parameters.
	const request_t & req,
	//! The ID of a HTTP-field with authentification parameters.
	http_field_t auth_field_id )
{
	return impl::perform_extraction_attempt(
			req.header().opt_value_of( auth_field_id ) );
}

} /* namespace basic_auth */

} /* namespace http_field_parsers */

} /* namespace restinio */

