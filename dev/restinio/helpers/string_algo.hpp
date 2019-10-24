/*
 * RESTinio
 */

/*!
 * @file
 * @brief Various string-related algorithms.
 *
 * @since v.0.6.1
 */

#pragma once

#include <restinio/string_view.hpp>

namespace restinio
{

namespace string_algo
{

bool starts_with(
	const string_view_t & where,
	const string_view_t & what ) noexcept
{
	return where.size() >= what.size() &&
			0 == where.compare(0u, what.size(), what);
}

bool ends_with(
	const string_view_t & where,
	const string_view_t & what ) noexcept
{
	return where.size() >= what.size() && 0 == where.compare(
			where.size() - what.size(), what.size(), what);
}

} /* namespace string_algo */

} /* namespace restinio */

