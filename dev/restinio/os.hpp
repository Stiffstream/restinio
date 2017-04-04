/*
	restinio
*/

/*!
	OS specific functions.
*/

#pragma once

namespace restinio
{

#if defined( _MSC_VER )
	#include "impl/os_win.inl"
#elif defined( __clang__ ) || defined( __GNUC__ )
	#include "impl/os_posix.inl"
#else
	#include "impl/os_unknown.inl"
#endif

} /* namespace restinio */
