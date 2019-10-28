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

#include <restinio/http_headers.hpp>
#include <restinio/request_handler.hpp>
#include <restinio/expected.hpp>

#include <iostream>

namespace restinio
{

namespace file_upload
{

enum class enumeration_result_t
{
	success,
	content_type_field_not_found,
	content_type_field_parse_error,
	content_type_field_inappropriate_value,
	unexpected_error
};

namespace impl
{

RESTINIO_NODISCARD
inline expected_t< std::string, enumeration_result_t >
detect_boundary_for_multipart_body(
	const request_t & req )
{
	namespace hfp = restinio::http_field_parsers;

	// Content-Type header file should be present.
	const auto content_type = req.header().opt_value_of(
			restinio::http_field::content_type );
	if( !content_type )
		return make_unexpected(
				enumeration_result_t::content_type_field_not_found );

	// Content-Type field should successfuly parsed and should
	// contain value multipart/form-data.
	const auto parse_result = hfp::content_type_value_t::try_parse(
			*content_type );
	if( !parse_result.first )
		return make_unexpected(
				enumeration_result_t::content_type_field_parse_error );

	const auto & media_type = parse_result.second.m_media_type;
	if( "multipart" != media_type.m_type
			&& "form-data" != media_type.m_subtype )
		return make_unexpected(
				enumeration_result_t::content_type_field_inappropriate_value );

	// `boundary` param should be present in parsed Content-Type value.
	const auto boundary = hfp::find_first(
			parse_result.second.m_media_type.m_parameters,
			"boundary" );
	if( !boundary )
		return make_unexpected(
				enumeration_result_t::content_type_field_inappropriate_value );
	
	//FIXME: boundary value should be checked!

	// Actual value of boundary mark can be created.
	std::string actual_boundary_mark;
	actual_boundary_mark.reserve( 2 + boundary->size() );
	actual_boundary_mark.append( "--" );
	actual_boundary_mark.append( boundary->data(), boundary->size() );

	return std::move(actual_boundary_mark);
}

} /* namespace impl */

template< typename Handler >
enumeration_result_t
enumerate_parts_with_files(
	const request_t & req,
	Handler && /*handler*/ )
{
	const auto boundary = impl::detect_boundary_for_multipart_body( req );
	if( boundary )
	{
		//FIXME: implement this!
		return enumeration_result_t::unexpected_error;
	}

	return boundary.error();
}

} /* namespace file_upload */

} /* namespace restinio */

