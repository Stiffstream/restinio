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
// generic_non_matched_request_handler_t
//
//FIXME: document this!
template< typename User_Data >
using generic_non_matched_request_handler_t =
		std::function<
				request_handling_status_t( incoming_request_handle_t<User_Data> )
		>;
//
// non_matched_request_handler_t
//
//FIXME: document this!
using non_matched_request_handler_t =
		generic_non_matched_request_handler_t<
				no_user_data_factory_t::data_t
		>;

} /* namespace router */

} /* namespace restinio */

