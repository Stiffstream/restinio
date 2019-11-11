/*
 * RESTinio
 */

/*!
 * @file
 * @brief Various tools for simplification of file uploading.
 *
 * @since v.0.6.1
 */

#pragma once

#include <restinio/helpers/http_field_parsers/content-type.hpp>
#include <restinio/helpers/http_field_parsers/content-disposition.hpp>
#include <restinio/helpers/multipart_body.hpp>

#include <restinio/http_headers.hpp>
#include <restinio/request_handler.hpp>
#include <restinio/expected.hpp>

#include <iostream>

namespace restinio
{

namespace file_upload
{

//
// enumeration_error_t
//
/*!
 * @brief The result of an attempt to enumerate parts of a multipart body
 * that contains uploaded file.
 *
 * @since v.0.6.1
 */
enum class enumeration_error_t
{
	//! Content-Type field is not found.
	//! If Content-Type is absent there is no way to detect 'boundary'
	//! parameter.
	content_type_field_not_found,
	//! Unable to parse Content-Type field value.
	content_type_field_parse_error,
	//! Content-Type field value parsed but doesn't contain an appropriate
	//! value. For example there can be media-type different from 'multipart'
	//! or 'boundary' parameter can be absent.
	content_type_field_inappropriate_value,
	//! Value of 'boundary' parameter is invalid (for example it contains
	//! some illegal characters).
	illegal_boundary_value,
	//! Unable to parse Content-Disposition field.
	content_disposition_field_parse_error,
	//! Content-Disposition field value parsed but doesn't contain an
	//! appropriate value. For example, there is no 'name' parameter.
	content_disposition_field_inappropriate_value,
	//! No parts of a multipart body actually found.
	no_parts_found,
	//! No files found in the current part.
	//! For example, there is no Content-Disposition field for that part,
	//! or Content-Disposition hasn't 'filename' and 'filename*'
	//! parameters.
	no_files_found,
	//! Enumeration of parts was aborted by user-provided handler.
	//! This code is returned when user-provided handler returns
	//! handling_result_t::terminate_enumeration.
	terminated_by_handler,
	//! Some unexpected error encountered during the enumeration.
	unexpected_error
};

namespace impl
{

/*!
 * @brief Helper function for conversion from one enumeration_error
 * to another.
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
constexpr enumeration_error_t
translate_enumeration_error(
	restinio::multipart_body::enumeration_error_t original )
{
	using source = restinio::multipart_body::enumeration_error_t;
	using dest = enumeration_error_t;

	dest result = dest::unexpected_error;

	switch( original )
	{
		case source::content_type_field_not_found:
			result = dest::content_type_field_not_found; break;

		case source::content_type_field_parse_error:
			result = dest::content_type_field_parse_error; break;

		case source::content_type_field_inappropriate_value:
			result = dest::content_type_field_inappropriate_value; break;

		case source::illegal_boundary_value:
			result = dest::illegal_boundary_value; break;

		case source::no_parts_found:
			result = dest::no_parts_found; break;

		case source::terminated_by_handler:
			result = dest::terminated_by_handler; break;

		case source::unexpected_error:
			/* nothing to do */ break;
	}

	return result;
}

} /* namespace impl */

//
// handling_result_t
//
/*!
 * @brief The result to be returned from user-provided handler of
 * parts of multipart body.
 *
 * @since v.0.6.1
 */
using handling_result_t = restinio::multipart_body::handling_result_t;

//
// part_description_t
//
/*!
 * @brief A description of one part with an uploaded file.
 *
 * @note
 * Values of @a filename_star and @a filename are optional.
 * But at least one of them won't be empty.
 * Both of them can be non-empty.
 *
 * @since v.0.6.1
 */
struct part_description_t
{
	//! HTTP-fields local for that part.
	/*!
	 * @note
	 * It can be empty if no HTTP-fields are found for that part.
	 */
	http_header_fields_t fields;
	//! The body of that part.
	string_view_t body;
	//! The value of Content-Disposition's 'name' parameter.
	std::string name;
	//! The value of Content-Disposition's 'filename*' parameter.
	/*!
	 * This field has the value only of 'filename*' parameter was
	 * found in Content-Disposition field.
	 *
	 * @attention
	 * If that field is presend then it is the original value extracted
	 * from Content-Disposition without any transformation. It means
	 * that this field will hold values defined in RFC5987 like:
	 * `utf-8'en-US'A%20some%20filename.txt`
	 */
	optional_t< std::string > filename_star;
	//! The value of Content-Disposition's 'filename' parameter.
	/*!
	 * This field has the value only of 'filename' parameter was
	 * found in Content-Disposition field.
	 */
	optional_t< std::string > filename;
};

//
// analyze_part
//
/*!
 * @brief Helper function for analyzing an already parsed part of
 * a multipart body for presence of an uploaded file.
 *
 * This function returns an instance of part_description_t if an
 * uploaded file is found in @a parsed_part.
 *
 * If an uploaded file isn't found or any error detected during analysis
 * of @a parsed_part then enumeration_error_t returned.
 *
 * Usage example:
 * @code
 * auto on_post(const restinio::request_handle_t & req) {
 * 	using namespace restinio::multipart_body;
 * 	using namespace restinio::file_upload;
 *
 * 	const auto result = enumerate_parts( *req,
 * 		[](parsed_part_t part) {
 * 			// Try to find an uploaded file in that part.
 * 			const auto uploaded_file = analyze_part(part);
 * 			if(uploaded_file) {
 * 				... // Some handling of the file content.
 * 			}
 * 			return handling_result_t::continue_enumeration;
 * 		},
 * 		"multipart", "form-data" );
 * 	if(result) {
 * 		... // Producing positive response.
 * 	}
 * 	else {
 * 		... // Producing negative response.
 * 	}
 * 	return restinio::request_accepted();
 * }
 * @endcode
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline expected_t< part_description_t, enumeration_error_t >
analyze_part( restinio::multipart_body::parsed_part_t parsed_part )
{
	namespace hfp = restinio::http_field_parsers;

	// Content-Disposition field should be present.
	const auto disposition_field = parsed_part.fields.opt_value_of(
			restinio::http_field::content_disposition );
	if( !disposition_field )
		return make_unexpected( enumeration_error_t::no_files_found );

	// Content-Disposition should have value `form-data` with
	// `name` and `filename*`/`filename` parameters.
	const auto parsed_disposition = hfp::content_disposition_value_t::
			try_parse( *disposition_field );
	if( !parsed_disposition )
		return make_unexpected(
				enumeration_error_t::content_disposition_field_parse_error );
	if( "form-data" != parsed_disposition->value )
		return make_unexpected( enumeration_error_t::no_files_found );

	const auto name = hfp::find_first(
			parsed_disposition->parameters, "name" );
	if( !name )
		return make_unexpected(
				enumeration_error_t::content_disposition_field_inappropriate_value );
	const auto expected_to_optional = []( auto expected ) {
		return expected ?
				optional_t< std::string >{ std::string{
						expected->data(),
						expected->size()
				} }
				: optional_t< std::string >{};
	};

	auto filename_star = expected_to_optional( hfp::find_first(
			parsed_disposition->parameters, "filename*" ) );
	auto filename = expected_to_optional( hfp::find_first(
			parsed_disposition->parameters, "filename" ) );

	// If there is no `filename*` nor `filename` then there is no file.
	if( !filename_star && !filename )
		return make_unexpected( enumeration_error_t::no_files_found );

	return part_description_t{
			std::move( parsed_part.fields ),
			parsed_part.body,
			std::string{ name->data(), name->size() },
			std::move(filename_star),
			std::move(filename)
	};
}

namespace impl
{

//
// valid_handler_type
//
template< typename, typename = restinio::utils::metaprogramming::void_t<> >
struct valid_handler_type : public std::false_type {};

template< typename T >
struct valid_handler_type<
		T,
		restinio::utils::metaprogramming::void_t<
			std::enable_if_t<
				std::is_same<
					handling_result_t,
					decltype(std::declval<T>()(std::declval<part_description_t>()))
				>::value,
				bool
			>
		>
	> : public std::true_type
{};

} /* namespace impl */

/*!
 * @brief A helper function for enumeration of parts of a multipart body
 * those contain uploaded files.
 *
 * This function:
 *
 * - finds Content-Type field for @a req;
 * - parses Content-Type field, checks the media-type and extracts
 *   the value of 'boundary' parameter. The extracted 'boundary'
 *   parameter is checked for validity;
 * - splits the body of @a req using value of 'boundary' parameter;
 * - enumerates every part of body, parses every part and tries to
 *   find a Content-Disposition field with appropriate 'name' and
 *   'filename*'/'filename' parameters;
 * - if a part with appropriate Content-Disposition is found the
 *   @a handler is called for it.
 *
 * Enumeration stops if @a handler returns handling_result_t::stop_enumeration
 * or handling_result_t::terminate_enumeration. If @a handler returns
 * handling_result_t::terminate_enumeration the enumerate_parts() returns
 * enumeration_error_t::terminated_by_handler error code.
 *
 * A handler passed as @a handler argument should be a function or
 * lambda/functor with one of the following formats:
 * @code
 * handling_result_t(part_description_t part);
 * handling_result_t(part_description_t && part);
 * handling_result_t(const part_description_t & part);
 * @endcode
 * Note that enumerate_parts_with_files() passes part_description_t instance to
 * @a handler as rvalue reference. And this reference will be invalidaded after
 * the return from @a handler.
 *
 * Usage example:
 * @code
 * auto on_post(const restinio::request_handle_t & req) {
 * 	using namespace restinio::file_upload;
 *
 * 	const auto result = enumerate_parts_with_files( *req,
 * 		[](part_description_t part) {
 * 			... // Some actions with the current part.
 * 			return handling_result_t::continue_enumeration;
 * 		},
 * 		"multipart", "form-data" );
 * 	if(result) {
 * 		... // Producing positive response.
 * 	}
 * 	else {
 * 		... // Producing negative response.
 * 	}
 * 	return restinio::request_accepted();
 * }
 * @endcode
 *
 * @return the count of parts passed to @a handler or
 * error code in the case if some error is detected.
 *
 * @since v.0.6.1
 */
template< typename Handler >
expected_t< std::size_t, enumeration_error_t >
enumerate_parts_with_files(
	//! Request to be processed.
	const request_t & req,
	//! Handler to be called for every part with uploaded file.
	Handler && handler,
	//! The value of 'type' part of media-type in Content-Type field.
	//! Please note: the special value '*' is not supported here.
	string_view_t expected_media_type = string_view_t{"multipart"},
	//! The value of 'subtype' part of media-type in Content-Type field.
	string_view_t expected_media_subtype = string_view_t{"form-data"} )
{
	static_assert(
			impl::valid_handler_type< std::decay_t<Handler> >::value,
			"Handler should be callable object, "
			"should accept part_description_t by value, const or rvalue reference, "
			"and should return handling_result_t" );

	std::size_t files_found{ 0u };
	optional_t< enumeration_error_t > error;

	const auto result = restinio::multipart_body::enumerate_parts( req,
			[&handler, &files_found, &error]
			( restinio::multipart_body::parsed_part_t part )
			{
				auto part_description = analyze_part( std::move(part) );
				if( part_description )
				{
					++files_found;

					return handler( std::move(*part_description) );
				}
				else if( enumeration_error_t::no_files_found ==
						part_description.error() )
				{
					return handling_result_t::continue_enumeration;
				}
				else
				{
					error = part_description.error();
					return handling_result_t::terminate_enumeration;
				}
			},
			expected_media_type,
			expected_media_subtype );

	if( error )
		return make_unexpected( *error );
	else if( !result )
		return make_unexpected(
				impl::translate_enumeration_error( result.error() ) );
	else
		return files_found;
}

} /* namespace file_upload */

} /* namespace restinio */

