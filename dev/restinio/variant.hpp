/*
	restinio
*/

/*!
	Adoption for std::variant (c++17).

	@since v.0.4.8
*/

#pragma once

#if defined(RESTINIO_EXTERNAL_VARIANT_LITE)
	#include <nonstd/variant.hpp>
#else
	#include "third_party/variant-lite/variant.hpp"
#endif

namespace restinio
{
	template< typename... Types >
	using variant_t = nonstd::variant< Types... >;

	using nonstd::holds_alternative;
	using nonstd::get;
	using nonstd::get_if;
	using nonstd::visit;

} /* namespace restinio */

