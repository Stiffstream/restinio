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

//! Helperfunction to obtain raw `const char*` from string_view iterator.
/*!
	Some implementations of string_view_t have string_view_t::iterator
	type that is not 'const char *'. So we can't use iterator for
	initialization of std::cregex_iterator.
*/
inline const char * sv_it2ptr(string_view_t::iterator it)
{
	return std::addressof(*it);
}

} /* namespace restinio */
