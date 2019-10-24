/*
 * RESTinio
 */

/*!
 * @file
 * @brief Various tools for working with multipart bodies.
 *
 * @since v.0.6.1
 */

#pragma once

#include <restinio/helpers/string_algo.hpp>

#include <iostream>

namespace restinio
{

namespace multipart_body
{

//
// split_multipart_body
//
//FIXME: document this!
inline std::vector< string_view_t >
split_multipart_body(
	string_view_t body,
	string_view_t boundary )
{
	using namespace restinio::string_algo;

	std::vector< string_view_t > result;
	std::vector< string_view_t > tmp_result;

	const string_view_t eol{ "\r\n" };
	const string_view_t last_separator{ "--\r\n" };

	// Find the first boundary.
	auto boundary_pos = body.find( boundary );
	if( string_view_t::npos == boundary_pos )
		// There is no initial separator in the body.
		return result;

	// The first body can be at the very begining of the body or
	// there should be CRLF before the initial boundary.
	if( boundary_pos != 0u &&
			(boundary_pos < eol.size() ||
			body.substr( boundary_pos - eol.size(), eol.size() ) != eol) )
		return result;

	auto remaining_body = body.substr( boundary_pos + boundary.size() );
	if( starts_with( remaining_body, last_separator ) )
		// The start boundary is the last boundary.
		return result;

	while( starts_with( remaining_body, eol ) )
	{
		remaining_body = remaining_body.substr( eol.size() );

		boundary_pos = remaining_body.find( boundary );
		if( string_view_t::npos == boundary_pos )
			return result;

		// There should be CRLF before the next boundary.
		if( boundary_pos < eol.size() ||
				remaining_body.substr( boundary_pos - eol.size(), eol.size() ) != eol )
			return result;

		tmp_result.push_back(
				remaining_body.substr( 0u, boundary_pos - eol.size() ) );

		remaining_body = remaining_body.substr( boundary_pos + boundary.size() );
		// Is this boundary the last one?
		if( starts_with( remaining_body, last_separator ) )
		{
			// Yes, our iteration can be stopped and we can return the result.
			swap( tmp_result, result );
			return result;
		}
	}

	// We didn't find the last boundary. Or some error encountered in the format
	// of the body.
	//
	// Empty result should be returned.
	return result;
}

} /* namespace multipart_body */

} /* namespace restinio */

