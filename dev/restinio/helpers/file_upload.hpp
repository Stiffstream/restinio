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
//FIXME: document this!
enum class enumeration_error_t
{
	content_type_field_not_found,
	content_type_field_parse_error,
	content_type_field_inappropriate_value,
	content_disposition_field_parse_error,
	content_disposition_field_inappropriate_value,
	no_parts_found,
	no_files_found,
	terminated_by_handler,
	unexpected_error
};

//
// handling_result_t
//
//FIXME: document this!
enum class handling_result_t
{
	continue_enumeration,
	stop_enumeration,
	terminate_enumeration
};

//
// part_description_t
//
//FIXME: part_description_t
class part_description_t
{
	http_header_fields_t m_fields;
	string_view_t m_body;
	string_view_t m_name;
	optional_t< string_view_t > m_filename_star;
	optional_t< string_view_t > m_filename;

public:
	part_description_t(
		http_header_fields_t fields,
		string_view_t body,
		string_view_t name,
		optional_t< string_view_t > filename_star,
		optional_t< string_view_t > filename )
		:	m_fields{ std::move(fields) }
		,	m_body{ body }
		,	m_name{ name }
		,	m_filename_star{ filename_star }
		,	m_filename{ filename }
	{}

	RESTINIO_NODISCARD
	const http_header_fields_t &
	fields() const noexcept { return m_fields; }

	RESTINIO_NODISCARD
	string_view_t
	body() const noexcept { return m_body; }

	RESTINIO_NODISCARD
	string_view_t
	name_parameter() const noexcept { return m_name; }

	RESTINIO_NODISCARD
	optional_t<string_view_t>
	filename_star_parameter() const noexcept { return m_filename_star; }

	RESTINIO_NODISCARD
	optional_t<string_view_t>
	filename_parameter() const noexcept { return m_filename; }
};

namespace impl
{

RESTINIO_NODISCARD
inline expected_t< std::string, enumeration_error_t >
detect_boundary_for_multipart_body(
	const request_t & req )
{
	namespace hfp = restinio::http_field_parsers;

	// Content-Type header file should be present.
	const auto content_type = req.header().opt_value_of(
			restinio::http_field::content_type );
	if( !content_type )
		return make_unexpected(
				enumeration_error_t::content_type_field_not_found );

	// Content-Type field should successfuly parsed and should
	// contain value multipart/form-data.
	const auto parse_result = hfp::content_type_value_t::try_parse(
			*content_type );
	if( !parse_result )
		return make_unexpected(
				enumeration_error_t::content_type_field_parse_error );

	const auto & media_type = parse_result->media_type;
	if( "multipart" != media_type.type
			&& "form-data" != media_type.subtype )
		return make_unexpected(
				enumeration_error_t::content_type_field_inappropriate_value );

	// `boundary` param should be present in parsed Content-Type value.
	const auto boundary = hfp::find_first(
			parse_result->media_type.parameters,
			"boundary" );
	if( !boundary )
		return make_unexpected(
				enumeration_error_t::content_type_field_inappropriate_value );
	
	//FIXME: boundary value should be checked!

	// Actual value of boundary mark can be created.
	std::string actual_boundary_mark;
	actual_boundary_mark.reserve( 2 + boundary->size() );
	actual_boundary_mark.append( "--" );
	actual_boundary_mark.append( boundary->data(), boundary->size() );

	return std::move(actual_boundary_mark);
}

//FIXME: maybe this function should be a part of public API?
//FIXME: or maybe there should be another version of try_analyze_part
//that accepts multipart_body::parsed_part_t?
RESTINIO_NODISCARD
inline expected_t< part_description_t, enumeration_error_t >
try_analyze_part( string_view_t part )
{
	namespace hfp = restinio::http_field_parsers;

	// The current part should be parsed to headers and 
	auto part_parse_result = restinio::multipart_body::try_parse_part( part );
	if( !part_parse_result )
		return make_unexpected( enumeration_error_t::unexpected_error );

	// Content-Disposition field should be present.
	const auto disposition_field =
			part_parse_result->fields.opt_value_of(
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
		return expected ? optional_t< string_view_t >{ *expected } :
				optional_t< string_view_t >{};
	};

	const auto filename_star = expected_to_optional( hfp::find_first(
			parsed_disposition->parameters, "filename*" ) );
	const auto filename = expected_to_optional( hfp::find_first(
			parsed_disposition->parameters, "filename" ) );

	// If there is no `filename*` nor `filename` then there is no file.
	if( !filename_star && !filename )
		return make_unexpected( enumeration_error_t::no_files_found );

	return part_description_t{
			std::move( part_parse_result->fields ),
			part_parse_result->body,
			*name,
			filename_star,
			filename
	};
}

template< typename Handler >
RESTINIO_NODISCARD
expected_t< std::size_t, enumeration_error_t >
enumerate_parts_of_request_body(
	const std::vector< string_view_t > & parts,
	Handler && handler )
{
	std::size_t files_found{ 0u };
	optional_t< enumeration_error_t > error;

	for( auto current_part : parts )
	{
		const auto analyzing_result = try_analyze_part( current_part );
		if( analyzing_result )
		{
			++files_found;

			const handling_result_t handler_ret_code = handler( *analyzing_result );
			if( handling_result_t::continue_enumeration != handler_ret_code )
			{
				if( handling_result_t::terminate_enumeration == handler_ret_code )
					error = enumeration_error_t::terminated_by_handler;

				break;
			}
		}
	}

	if( error )
		return make_unexpected( *error );

	if( 0u == files_found )
		return make_unexpected( enumeration_error_t::no_files_found );

	return files_found;
}

} /* namespace impl */

template< typename Handler >
expected_t< std::size_t, enumeration_error_t >
enumerate_parts_with_files(
	const request_t & req,
	Handler && handler )
{
	//FIXME: there should be some static_assert that checks the possibility
	//to call the handler. It means the right argument type and the result
	//type should be checked.

	const auto boundary = impl::detect_boundary_for_multipart_body( req );
	if( boundary )
	{
		const auto parts = restinio::multipart_body::split_multipart_body(
				req.body(),
				*boundary );

		if( parts.empty() )
			return make_unexpected(
					enumeration_error_t::no_parts_found );

		return impl::enumerate_parts_of_request_body(
				parts,
				std::forward<Handler>(handler) );
	}

	return make_unexpected( boundary.error() );
}

} /* namespace file_upload */

} /* namespace restinio */

