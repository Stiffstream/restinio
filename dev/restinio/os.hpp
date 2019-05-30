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
	#include "impl/os_win.ipp"
#elif (defined( __clang__ ) || defined( __GNUC__ )) && !defined(__WIN32__)
	#include "impl/os_posix.ipp"
#else
	#include "impl/os_unknown.ipp"
#endif

} /* namespace restinio */
