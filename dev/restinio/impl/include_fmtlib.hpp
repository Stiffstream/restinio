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
#include <fmt/time.h>

#if defined(__GNUG__) || defined(__clang__)

#pragma GCC diagnostic pop

#endif
