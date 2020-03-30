/*
 * RESTinio
 */

/**
 * @file
 * @brief The definition of the non_matched_request_handler type.
 *
 * @since v.0.6.6
 */

#pragma once

#include <restinio/request_handler.hpp>

#include <functional>

namespace restinio
{

namespace router
{

//
// unmatched_request_handler_t
//

using non_matched_request_handler_t =
		std::function< request_handling_status_t( request_handle_t ) >;

} /* namespace router */

} /* namespace restinio */

