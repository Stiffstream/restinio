/*
	restinio
*/

/*!
	Adoption for std::expected (c++20).

	@since v.0.6.1
*/

#pragma once

#if defined(RESTINIO_EXTERNAL_EXPECTED_LITE)
	#include <nonstd/expected.hpp>
#else
	#include "third_party/expected-lite/expected.hpp"
#endif

namespace restinio
{
	template< typename T, typename E >
	using expected_t = nonstd::expected<T, E>;

//FIXME: is this symbol actually present in expected-lite?
//	using nonstd::make_expected;
	using nonstd::make_unexpected;

} /* namespace restinio */

