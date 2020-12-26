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
/*!
 * @brief A generic type of handler for non-matched requests.
 *
 * Since v.0.6.13 some extra-data can be incorporated into a request
 * object. In that case request-handler receives a parameter of type
 * `generic_request_handle_t<Extra_Data>`. The name
 * generic_non_matched_request_handler_t describes a type of
 * generic handler that can be parametrized by a @a User_Type.
 *
 * @tparam Extra_Data The type of extra-data incorporated into a
 * request object.
 *
 * @since v.0.6.13
 */
template< typename Extra_Data >
using generic_non_matched_request_handler_t =
		std::function<
				request_handling_status_t( generic_request_handle_t<Extra_Data> )
		>;
//
// non_matched_request_handler_t
//
/*!
 * @brief A type of handler for non-matched requests for a case when
 * default extra-data-factory is specified in the server's traits.
 *
 * Since v.0.6.13 the name non_matched_request_handler_t is just
 * an alias for generic_non_matched_request_handler_t.
 */
using non_matched_request_handler_t =
		generic_non_matched_request_handler_t<
				no_extra_data_factory_t::data_t
		>;

} /* namespace router */

} /* namespace restinio */

