/*
 * RESTinio
 */

/*!
 * @file
 * @brief Typedefs for default strands.
 * @since v.0.6.12
 */

#pragma once

#include <restinio/asio_include.hpp>

namespace restinio
{

/*!
 * @brief A typedef for the default strand type.
 *
 * @since v.0.6.12
 */
using default_strand_t = asio_ns::strand< default_asio_executor >;

/*!
 * @brief A typedef for no-op strand type.
 */
using noop_strand_t = default_asio_executor;

} /* namespace restinio */

