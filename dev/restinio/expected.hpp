/*
	restinio
*/

/*!
	Adoption for std::expected (c++20).

	@since v.0.6.1
*/

#pragma once

#include <nonstd/expected.hpp>

namespace restinio
{
	template< typename T, typename E >
	using expected_t = nonstd::expected<T, E>;

	using nonstd::make_unexpected;

} /* namespace restinio */

