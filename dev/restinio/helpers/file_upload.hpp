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
	illegal_boundary_value,
	content_disposition_field_parse_error,
	content_disposition_field_inappropriate_value,
	no_parts_found,
	no_files_found,
	terminated_by_handler,
	unexpected_error
};

namespace impl
{

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
//FIXME: document this!
using handling_result_t = restinio::multipart_body::handling_result_t;

//FIXME: should it be a class with private members and public getters
//or it can be a struct with public members?
//NOTE: parsed_part_t in multipart_body namespace is a simple struct.
//
// part_description_t
//
//FIXME: part_description_t
class part_description_t
{
	http_header_fields_t m_fields;
	string_view_t m_body;
	std::string m_name;
	optional_t< std::string > m_filename_star;
	optional_t< std::string > m_filename;

public:
	part_description_t(
		http_header_fields_t fields,
		string_view_t body,
		std::string name,
		optional_t< std::string > filename_star,
		optional_t< std::string > filename )
		:	m_fields{ std::move(fields) }
		,	m_body{ body }
		,	m_name{ std::move(name) }
		,	m_filename_star{ std::move(filename_star) }
		,	m_filename{ std::move(filename) }
	{}

	RESTINIO_NODISCARD
	const http_header_fields_t &
	fields() const noexcept { return m_fields; }

	RESTINIO_NODISCARD
	string_view_t
	body() const noexcept { return m_body; }

	RESTINIO_NODISCARD
	const std::string &
	name_parameter() const noexcept { return m_name; }

	RESTINIO_NODISCARD
	const optional_t<std::string> &
	filename_star_parameter() const noexcept { return m_filename_star; }

	RESTINIO_NODISCARD
	const optional_t<std::string> &
	filename_parameter() const noexcept { return m_filename; }
};

//
// analyze_part
//
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

template< typename Handler >
expected_t< std::size_t, enumeration_error_t >
enumerate_parts_with_files(
	const request_t & req,
	Handler && handler )
{
	//FIXME: there should be some static_assert that checks the possibility
	//to call the handler. It means the right argument type and the result
	//type should be checked.

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
			"multipart",
			"form-data" );

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

