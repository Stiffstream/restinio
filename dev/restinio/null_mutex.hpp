/*
 * RESTinio
 */

/*!
 * @file
 * @brief Definition of null_mutex.
 * @since v.0.6.12
 */

#pragma once

namespace restinio
{

//
// null_mutex_t
//

/*!
 * @brief A class to be used as null_mutex.
 *
 * Provides an interface similar to std::mutex but does nothing.
 *
 * @since v.0.6.12
 */
struct null_mutex_t
{
	constexpr void lock() const noexcept {}

	constexpr bool try_lock() const noexcept { return true; }

	constexpr void unlock() const noexcept {}
};

} /* namespace restinio */

