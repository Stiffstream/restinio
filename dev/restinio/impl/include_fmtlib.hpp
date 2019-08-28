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

#if defined(__GNUG__) || defined(__clang__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

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
