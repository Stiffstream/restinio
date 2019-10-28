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
	unexpected_error
};

template< typename Handler >
enumeration_result_t
enumerate_parts_with_files(
	const request_t & /*req*/,
	Handler && /*handler*/ )
{
	return enumeration_result_t::unexpected_error;
}

} /* namespace file_upload */

} /* namespace restinio */

