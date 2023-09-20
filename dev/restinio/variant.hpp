/*
	restinio
*/

/*!
	Adoption for std::variant (c++17).

	@since v.0.4.8
*/

#pragma once

#include <variant>

namespace restinio
{
	template< typename... Types >
	using variant_t = std::variant< Types... >;

	using std::holds_alternative;
	using std::get;
	using std::get_if;
	using std::visit;

} /* namespace restinio */

