/*
 * RESTinio
 */

/*!
 * \file
 * \brief A special wrapper around fmtlib include files.
 * \since
 * v.0.5.1.2
 */

#pragma once

// Because fmt can include `windows.h` we should include
// Asio's header before fmt.
//
// Since v.0.6.1
#include <restinio/asio_include.hpp>

#if defined(__GNUG__) || defined(__clang__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

#if defined(__clang__)
	#pragma GCC diagnostic ignored "-Wgnu-string-literal-operator-template"
#endif

#endif

#include <fmt/format.h>
#include <fmt/ostream.h>
#if FMT_VERSION < 60000
	#include <fmt/time.h>
#else
	#include <fmt/chrono.h>
#endif

#if defined(__GNUG__) || defined(__clang__)

#pragma GCC diagnostic pop

#endif
