/*
	restinio
*/

/*!
	Adoption for std::variant (c++17).

	@since v.0.4.8
*/

#pragma once

#include "third_party/variant-lite/variant.hpp"

namespace restinio
{
	template< class T >
	using variant_t = nonstd::variant< T >;

} /* namespace restinio */
