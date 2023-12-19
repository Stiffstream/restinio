/*!
 * @file
 * @brief Helper macros for detection of compiler/platform.
 */

#pragma once

/*!
 * @def RESTINIO_OS_WIN32
 * @brief A marker for Win32 platform.
 * 
 * @note This marker is also defined for Win64 platform.
 */

/*!
 * @def RESTINIO_OS_WIN64
 * @brief A marker for Win64 platform.
 */

/*!
 * @def RESTINIO_OS_WINDOWS
 * @brief A marker for Windows platform.
 *
 * Defined if RESTINIO_OS_WIN32 or RESTINIO_OS_WIN64 are defined.
 */

/*!
 * @def RESTINIO_OS_UNIX
 * @brief A marker for Unix platforms, but not macOS/iOS.
 */

/*!
 * @def RESTINIO_OS_APPLE
 * @brief A marker for macOS/iOS.
 */

#if defined( _WIN64 )
	#define RESTINIO_OS_WIN64
#endif

#if defined( _WIN32 )
	#define RESTINIO_OS_WIN32
#endif

#if defined( RESTINIO_OS_WIN32 ) || defined( RESTINIO_OS_WIN64 )
	#define RESTINIO_OS_WINDOWS
#endif

#if defined(unix) || defined(__unix__) || defined(__unix)
	#define RESTINIO_OS_UNIX
#endif

#if defined(__APPLE__)
	#define RESTINIO_OS_APPLE
#endif

