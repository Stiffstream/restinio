/*
	restinio
*/

/*!
	Adoption for std::string_view (c++17).
*/

#pragma once

#if defined(RESTINIO_EXTERNAL_STRING_VIEW_LITE)
	#include <nonstd/string_view.hpp>
#else
	#include "third_party/string-view-lite/string_view.hpp"
#endif

namespace restinio
{
	using string_view_t = nonstd::string_view;

} /* namespace restinio */
