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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

#if defined(__clang__)
	#pragma GCC diagnostic ignored "-Wgnu-string-literal-operator-template"
#endif

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/time.h>

#pragma GCC diagnostic pop

