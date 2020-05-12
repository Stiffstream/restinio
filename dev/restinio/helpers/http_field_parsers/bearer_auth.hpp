/*
 * RESTinio
 */

/*!
 * @file
 * @brief Helpers for dealing with bearer authentification.
 *
 * @since v.0.6.7.1
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

namespace bearer_auth
{

//
// params_t
//
/*!
 * @brief Parameters for bearer authentification.
 *
 * @since v.0.6.7.1
 */
struct params_t
{
	//! Unique ID for a client.
	/*!
	 * Can't be empty.
	 */
	std::string client_id;

	//! Very Secret secret of a client.
	/*!
	 * Can't be empty.
	 */
	std::string client_secret;
};

//
// extraction_error_t
//
/*!
 * @brief Error codes for failures of extraction of bearer authentification
 * parameters.
 *
 * @since v.0.6.7.1
 */
enum class extraction_error_t
{
	//! There is no HTTP field with authentification parameters.
	no_auth_http_field,
	
	//! The HTTP field with authentification parameters can't be parsed.
	illegal_http_field_value,

	//! Different authentification scheme found.
	//! bearer authentification scheme is expected.
	not_bearer_auth_scheme,

	//! Invalid value of parameter for bearer authentification scheme.
	//! The single parameter in the form of b64token is expected.
	invalid_bearer_auth_param,

	//! Value of b64token parameter for bearer authentification can't be decoded.
	b64token_decode_error,

	//! Wrong format for id:secret in decoded parameter.
	//! Maybe there is no colon symbol.
	invalid_id_secret_pair,

	//! Empty id in id:secret pair.
	empty_id,

	//! Empty secret in id:secret pair.
	empty_secret,
};

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
	if( "bearer" != parsed_value.auth_scheme )
		return make_unexpected( extraction_error_t::not_bearer_auth_scheme );

	const auto * b64token = get_if<authorization_value_t::token68_t>(
			&parsed_value.auth_param );
	if( !b64token )
		return make_unexpected( extraction_error_t::invalid_bearer_auth_param );

	const auto unbase64_result =
			restinio::utils::base64::try_decode( b64token->value );
	if( !unbase64_result )
		return make_unexpected( extraction_error_t::b64token_decode_error );

	const std::string & id_secret = *unbase64_result;

	const auto first_colon = id_secret.find( ':' );
	if( std::string::npos == first_colon )
		return make_unexpected(
				extraction_error_t::invalid_id_secret_pair );
	if( 0u == first_colon )
		return make_unexpected( extraction_error_t::empty_id );
	if( id_secret.length() == first_colon + 1u )
		return make_unexpected( extraction_error_t::empty_secret );

	return params_t{
			id_secret.substr( 0u, first_colon ),
			id_secret.substr( first_colon + 1u )
	};
}

} /* namespace impl */

//
// try_extract_params
//
/*!
 * @brief Helper function for getting parameters of bearer authentification
 * from a request.
 *
 * This helper function is intended to be used for cases when authentification
 * parameters are stored inside a HTTP-field with a custom name. For example:
 * @code
 * auto on_request(restinio::request_handle_t & req) {
 * 	using namespace restinio::http_field_parsers::bearer_auth;
 * 	const auto auth_params = try_extract_params(*req, "X-My-Authorization");
 * 	if(auth_params) {
 * 		const std::string & id = auth_params->client_id;
 * 		const std::string & secret = auth_params->client_secret;
 * 		... // Do something with id and secret.
 * 	}
 * 	...
 * }
 * @endcode
 *
 * @since v.0.6.7.1
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
 * @brief Helper function for getting parameters of bearer authentification
 * from a request.
 *
 * Usage example:
 * @code
 * auto on_request(restinio::request_handle_t & req) {
 * 	using namespace restinio::http_field_parsers::bearer_auth;
 * 	const auto auth_params = try_extract_params(
 * 			*req, restinio::http_field::authorization);
 * 	if(auth_params) {
 * 		const std::string & id = auth_params->client_id;
 * 		const std::string & secret = auth_params->client_secret;
 * 		... // Do something with id and secret.
 * 	}
 * 	...
 * }
 * @endcode
 *
 * @since v.0.6.7.1
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

} /* namespace bearer_auth */

} /* namespace http_field_parsers */

} /* namespace restinio */

