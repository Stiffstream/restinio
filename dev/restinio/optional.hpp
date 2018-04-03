/*
	restinio
*/

/*!
	Adoption for std::optional (c++17).

	@since v.0.4.4
*/

#pragma once

#include "third_party/optional-lite/optional.hpp"

namespace restinio
{
	template< class T >
	using optional_t = nonstd::optional< T >;

	using nonstd::nullopt;
	using nonstd::nullopt_t;

} /* namespace restinio */
