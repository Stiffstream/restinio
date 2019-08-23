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

#include <restinio/impl/include_fmtlib.hpp>

#include <restinio/null_logger.hpp>

#include <exception>

namespace restinio
{

namespace utils
{

//
// Wrappers for logging with suppressing of exceptions.
//

template< typename Logger, typename Message_Builder >
void
log_trace_noexcept( Logger && logger, Message_Builder && builder ) noexcept
{
	try { logger.trace( std::forward<Message_Builder>(builder) ); }
	catch( ... ) {}
}

template< typename Message_Builder >
void
log_trace_noexcept( null_logger_t &, Message_Builder && ) noexcept
{}

template< typename Logger, typename Message_Builder >
void
log_info_noexcept( Logger && logger, Message_Builder && builder ) noexcept
{
	try { logger.info( std::forward<Message_Builder>(builder) ); }
	catch( ... ) {}
}

template< typename Message_Builder >
void
log_info_noexcept( null_logger_t &, Message_Builder && ) noexcept
{}

template< typename Logger, typename Message_Builder >
void
log_warn_noexcept( Logger && logger, Message_Builder && builder ) noexcept
{
	try { logger.warn( std::forward<Message_Builder>(builder) ); }
	catch( ... ) {}
}

template< typename Message_Builder >
void
log_warn_noexcept( null_logger_t &, Message_Builder && ) noexcept
{}

template< typename Logger, typename Message_Builder >
void
log_error_noexcept( Logger && logger, Message_Builder && builder ) noexcept
{
	try { logger.error( std::forward<Message_Builder>(builder) ); }
	catch( ... ) {}
}

template< typename Message_Builder >
void
log_error_noexcept( null_logger_t &, Message_Builder && ) noexcept
{}

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
	try
	{
		lambda();
	}
	catch( const std::exception & x )
	{
		log_error_noexcept( logger, [&] {
				return fmt::format( "an exception in '{}': {}",
						block_description, x.what() );
			} );
	}
	catch( ... )
	{
		log_error_noexcept( logger, [&] {
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

