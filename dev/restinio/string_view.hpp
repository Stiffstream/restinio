/*
	restinio
*/

/*!
	Adoption for std::string_view (c++17).
*/

#pragma once

#include <memory>

#include "third_party/string-view-lite/string_view.hpp"

namespace restinio
{
	using string_view_t = nonstd::string_view;

} /* namespace restinio */
