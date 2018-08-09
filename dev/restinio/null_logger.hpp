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
		template< typename Message_Builder >
		constexpr void
		trace( Message_Builder && ) const noexcept
		{}

		template< typename Message_Builder >
		constexpr void
		info( Message_Builder && ) const noexcept
		{}

		template< typename Message_Builder >
		constexpr void
		warn( Message_Builder && ) const noexcept
		{}

		template< typename Message_Builder >
		constexpr void
		error( Message_Builder && ) const noexcept
		{}
};

} /* namespace restinio */
