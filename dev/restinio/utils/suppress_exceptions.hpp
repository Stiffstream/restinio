/*
 * RESTinio
 */

/*!
 * @file
 * @brief Utilities for suppressing exceptions from some code block.
 *
 * @since v.0.6.0
 */

//FIXME: this file should be added to restinio/CMakeLists.txt

#pragma once

#include <exception>

#include <restinio/impl/include_fmtlib.hpp>

namespace restinio
{

namespace utils
{

namespace suppress_exceptions_details
{

//FIXME: document this!
template< typename Logger, typename Lambda >
void safe_log_error( Logger && logger, Lambda && lambda ) noexcept
{
	try
	{
		logger.error( std::forward<Lambda>(lambda) );
	}
	catch( ... ) {} // All exceptions should be ignored here.
}

} /* namespace suppress_exceptions_details */

//FIXME: document this!
template<
	typename Logger,
	typename Lambda >
void suppress_exceptions(
	Logger && logger,
	const char * block_description,
	Lambda && lambda ) noexcept
{
	using namespace suppress_exceptions_details;

	try
	{
		lambda();
	}
	catch( const std::exception & x )
	{
		safe_log_error( logger, [&] {
				return fmt::format( "an exception in '{}': {}",
						block_description, x.what() );
			} );
	}
	catch( ... )
	{
		safe_log_error( logger, [&] {
				return fmt::format( "an unknown exception in '{}'",
						block_description );
			} );
	}
}

//FIXME: document this!
template< typename Lambda >
void suppress_exceptions_quietly( Lambda && lambda ) noexcept
{
	try
	{
		lambda();
	}
	catch( ... ) {}
}

} /* namespace utils */

} /* namespace restinio */

