/*
	restinio
*/

/*!
	Ready to use loggers implementations.
*/

#pragma once

namespace restinio
{

//
// null_logger_t
//

//! No operation logger.
/*!
	Helps compiler to strip all operations associated with logging.
*/
class null_logger_t
{
	public:
		template< typename MSG_BUILDER >
		constexpr void
		trace( MSG_BUILDER && ) const
		{}

		template< typename MSG_BUILDER >
		constexpr void
		info( MSG_BUILDER && ) const
		{}

		template< typename MSG_BUILDER >
		constexpr void
		warn( MSG_BUILDER && ) const
		{}

		template< typename MSG_BUILDER >
		constexpr void
		error( MSG_BUILDER && ) const
		{}
};

} /* namespace restinio */
