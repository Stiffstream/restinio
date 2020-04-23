/*
	restinio
*/

/*!
	Adoption for std::optional (c++17).

	@since v.0.4.4
*/

#pragma once

#if defined(RESTINIO_EXTERNAL_OPTIONAL_LITE)
	#include <nonstd/optional.hpp>
#else
	#include "third_party/optional-lite/optional.hpp"
#endif

namespace restinio
{
	template< class T >
	using optional_t = nonstd::optional< T >;

	using nonstd::nullopt;
	using nonstd::nullopt_t;

} /* namespace restinio */
