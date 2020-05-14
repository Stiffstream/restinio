/*
 * RESTinio
 */

/*!
 * @file
 * @brief Helper functions for parsing values of HTTP-fields.
 *
 * @since v.0.6.8
 */

#pragma once

#include <restinio/helpers/easy_parser.hpp>

#include <restinio/http_headers.hpp>
#include <restinio/request_handler.hpp>
#include <restinio/variant.hpp>

#include <iostream>

namespace restinio
{

namespace http_field_parsers
{

//
// field_not_found_t
//
/*!
 * @brief A special type to be returned in the case if HTTP-field
 * isn't found in a request.
 *
 * @since v.0.6.8
 */
struct field_not_found_t {};

namespace try_extract_field_details
{

//
// result_variant_t
//
/*!
 * @brief Type of a variant to be returned as the result of attempt
 * to parse HTTP-field.
 *
 * @since v.0.6.8
 */
template< typename Parsed_Field_Type >
using result_variant_t = variant_t<
		Parsed_Field_Type,
		field_not_found_t,
		restinio::easy_parser::parse_error_t >;

//
// valid_field_type
//
template< typename, typename = restinio::utils::metaprogramming::void_t<> >
struct valid_field_type : public std::false_type {};

template< typename T >
struct valid_field_type<
		T,
		restinio::utils::metaprogramming::void_t<
			std::enable_if_t<
				std::is_same<
					expected_t< T, restinio::easy_parser::parse_error_t >,
					decltype(T::try_parse(std::declval<string_view_t>()))
				>::value,
				bool
			>
		>
	> : public std::true_type
{};

//
// try_extract_field_value_from
//
template< typename Parsed_Field_Type >
RESTINIO_NODISCARD
result_variant_t< Parsed_Field_Type >
try_extract_field_value_from(
	optional_t< string_view_t > opt_value,
	string_view_t default_value )
{
	static_assert( valid_field_type<Parsed_Field_Type>::value,
			"Parsed_Field_Type should have static try_parse method that "
			"accepts string_view_t and returns "
			"expected_t<Parsed_Field_Type, parse_error_t>" );

	if( !opt_value && default_value.empty() )
		return { field_not_found_t{} };

	string_view_t content = opt_value ? *opt_value : default_value;

	auto parse_result = Parsed_Field_Type::try_parse( content );
	if( parse_result )
		return { std::move(*parse_result) };
	else
		return { parse_result.error() };
}

} /* namespace try_extract_field_details */

//
// try_parse_field
//
/*!
 * @brief A helper function for extraction and parsing a value of
 * HTTP-field.
 *
 * This helper is intended to be used when HTTP-field is identified
 * by its name.
 *
 * Usage example:
 * @code
 * auto on_post(const restinio::request_handle_t & req) {
 * 	using namespace restinio::http_field_parsers;
 *
 * 	const auto auth_field = try_parse_field< authorization_value_t >(
 * 			req, "X-My-Authorization");
 * 	if(auto * auth = restinio::get_it<authorization_value_t>(&auth_field)) {
 * 		// X-My-Authorization is successfully parsed.
 * 		if("basic" == auth->auth_scheme) {
 * 			... // Dealing with basic authentification.
 * 		}
 * 		else if("bearer" == auth->auth_scheme) {
 * 			... // Dealing with bearer authentification.
 * 		}
 * 		else {
 * 			...
 * 		}
 * 	}
 * }
 * @endcode
 *
 * @tparam Parsed_Field_Type The type of field value to be received as the
 * result of successful parse if the field is present.
 *
 * @since v.0.6.8
 */
template< typename Parsed_Field_Type >
RESTINIO_NODISCARD
auto 
try_parse_field(
	//! A request that should hold a HTTP-field.
	const request_t & req,
	//! The name of HTTP-field to be extracted and parsed.
	string_view_t field_name,
	//! The default value to be used if HTTP-field is not found.
	//! If this value is not empty, then the variant with
	//! field_not_found_t won't be returned.
	string_view_t default_value = string_view_t{} )
{
	using namespace try_extract_field_details;

	return try_extract_field_value_from< Parsed_Field_Type >(
			req.header().opt_value_of( field_name ),
			default_value );
}

/*!
 * @brief A helper function for extraction and parsing a value of
 * HTTP-field.
 *
 * This helper is intended to be used when HTTP-field is identified
 * by its ID.
 *
 * Usage example:
 * @code
 * auto on_post(const restinio::request_handle_t & req) {
 * 	using namespace restinio::http_field_parsers;
 *
 * 	const auto auth_field = try_parse_field< authorization_value_t >(
 * 			req, restinio::http_field::authorization);
 * 	if(auto * auth = restinio::get_it<authorization_value_t>(&auth_field)) {
 * 		// Authorization is successfully parsed.
 * 		if("basic" == auth->auth_scheme) {
 * 			... // Dealing with basic authentification.
 * 		}
 * 		else if("bearer" == auth->auth_scheme) {
 * 			... // Dealing with bearer authentification.
 * 		}
 * 		else {
 * 			...
 * 		}
 * 	}
 * }
 * @endcode
 *
 * @tparam Parsed_Field_Type The type of field value to be received as the
 * result of successful parse if the field is present.
 *
 * @since v.0.6.8
 */
template< typename Parsed_Field_Type >
RESTINIO_NODISCARD
auto 
try_parse_field(
	//! A request that should hold a HTTP-field.
	const request_t & req,
	//! The ID of a HTTP-field to be extracted and parsed.
	http_field_t field_id,
	//! The default value to be used if HTTP-field is not found.
	//! If this value is not empty, then the variant with
	//! field_not_found_t won't be returned.
	string_view_t default_value = string_view_t{} )
{
	using namespace try_extract_field_details;

	return try_extract_field_value_from< Parsed_Field_Type >(
			req.header().opt_value_of( field_id ),
			default_value );
}

} /* namespace http_field_parsers */

} /* namespace restinio */

