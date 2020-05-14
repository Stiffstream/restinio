/*
 * RESTinio
 */

/*!
 * @file
 * @brief Definition of RESTINIO_VERSION macro
 *
 * @since v.0.6.1
 */

#pragma once

// The current version is 0.6.8
//
/*!
 * The major part of version number.
 *
 * If RESTinio's version is 0.6.0 then RESTINIO_VERSION_MAJOR==0.
 * If RESTinio's version is 1.2.4 then RESTINIO_VERSION_MAJOR==1.
 */
#define RESTINIO_VERSION_MAJOR 0ull

/*!
 * The minon part of version number.
 *
 * If RESTinio's version is 0.6.0 then RESTINIO_VERSION_MINOR==6.
 */
#define RESTINIO_VERSION_MINOR 6ull

/*!
 * The patch part of version number.
 *
 * If RESTinio's version is 0.6.23 then RESTINIO_VERSION_PATCH==23.
 */
#define RESTINIO_VERSION_PATCH 8ull

/*!
 * Helper macro for make single number representation of RESTinio's version.
 *
 * It can be used that way:
 * \code
 * // Some feature is available only from 1.2.4
 * #if RESTINIO_VERSION >= RESTINIO_VERSION_MAKE(1, 2, 4)
 * 	... // Some 1.2.4 (or above) specific code.
 * #endif
 * \endcode
 */
#define RESTINIO_VERSION_MAKE(major, minor, patch) \
	(((major) * 1000000ull) + \
	((minor) * 1000ull) + \
	(patch))

/*!
 * A single number representation of RESTinio version.
 *
 * For example it can be 6003ull for RESTinio-0.6.3.
 * Or 1004023ull for RESTinio-1.4.23.
 */
#define RESTINIO_VERSION RESTINIO_VERSION_MAKE( \
	RESTINIO_VERSION_MAJOR,\
	RESTINIO_VERSION_MINOR,\
	RESTINIO_VERSION_PATCH)

