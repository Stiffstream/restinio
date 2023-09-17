/*
	restinio
*/

/*!
	Adoption for std::optional (c++17).

	@since v.0.4.4
*/

#pragma once

#include <optional>

namespace restinio
{
	template< class T >
	using optional_t = std::optional< T >;

	using std::nullopt;
	using std::nullopt_t;

} /* namespace restinio */
