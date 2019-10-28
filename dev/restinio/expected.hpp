/*
	restinio
*/

/*!
	Adoption for std::expected (c++20).

	@since v.0.6.1
*/

#pragma once

#include "third_party/optional-lite/expected.hpp"

namespace restinio
{
	template< typename T, typename E >
	using expected_t = nonstd::expected;

	using nonstd::make_expected;
	using nonstd::make_unexpected;

} /* namespace restinio */

