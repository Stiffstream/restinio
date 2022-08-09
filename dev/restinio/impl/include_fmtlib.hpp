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

// Workaround for absence of fmt::runtime in fmtlib prior to v8.0.0.
namespace restinio
{

namespace fmtlib_tools
{

#if FMT_VERSION < 80000
inline auto
runtime_format_string( const char * fmt_string ) noexcept
{
	return fmt_string;
}
#else
inline auto
runtime_format_string( const char * fmt_string )
{
	return fmt::runtime( fmt_string );
}
#endif

#if FMT_VERSION < 90000
template< typename T >
decltype(auto) streamed( T && v ) noexcept
{
	return std::forward<T>(v);
}
#else
template< typename T >
decltype(auto) streamed( T && v ) noexcept
{
	return fmt::streamed( std::forward<T>(v) );
}
#endif

} /* namespace fmtlib_tools */

} /* namespace restinio */

/*
 * NOTE: if RESTinio is used in project compiled with
 * FMT_ENFORCE_COMPILE_STRING then a format_string for fmt::format
 * and fmt::print function has to be specified via FMT_STRING macro.
 *
 * To cope with this a new macro RESTINIO_FMT_FORMAT_STRING(s) was
 * introduced in v.0.6.17. It's expanded into FMT_STRING(s) if
 * FMT_ENFORCE_COMPILE_STRING is set, otherwise it's just s.
 */

#if defined(FMT_ENFORCE_COMPILE_STRING)
	#define RESTINIO_FMT_FORMAT_STRING(s) FMT_STRING(s)
#else
	#define RESTINIO_FMT_FORMAT_STRING(s) s
#endif

#if defined(__GNUG__) || defined(__clang__)

#pragma GCC diagnostic pop

#endif
