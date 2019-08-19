/*
 * RESTinio
 */

/*!
 * @file
 * @brief Utilities for suppressing exceptions from some code block.
 *
 * @since v.0.6.0
 */

#pragma once

#include <exception>

#include <restinio/impl/include_fmtlib.hpp>

namespace restinio
{

namespace utils
{

namespace suppress_exceptions_details
{

/*!
 * @brief Helper function for logging error with suppressing possible
 * exceptions.
 * 
 * @since v.0.6.0
 */
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

/*!
 * @brief Helper function for execution a block of code with
 * suppression of any exceptions raised inside that block.
 *
 * Exceptions caught are logged via \a logger. Exceptions thrown during
 * this logging are suppressed.
 *
 * @since v.0.6.0
 */
template<
	typename Logger,
	typename Lambda >
void suppress_exceptions(
	//! Logger to be used.
	Logger && logger,
	//! Description of the block of code.
	//! Will be used for logging about exceptions caught.
	const char * block_description,
	//! Block of code for execution.
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

/*!
 * @brief Helper function for execution a block of code with
 * suppression of any exceptions raised inside that block.
 *
 * All exceptions are simply intercepted. Nothing is logged in the
 * case of an exception thrown.
 *
 * @since v.0.6.0
 */
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

