/*
	restinio
*/

/*!
	No-op timer factory implementation.
*/

#pragma once

#include <chrono>

#include <asio.hpp>

#include <restinio/timer_common.hpp>

namespace restinio
{

//
// null_timer_manager_t
//

//! Timer factory implementation using asio timers.
struct null_timer_manager_t final
	:	public std::enable_shared_from_this< null_timer_manager_t >
{
	//! Timer guard for async operations.
	struct timer_guard_t
	{
		// Set new timeout guard.
		template <typename... Args >
		constexpr void
		schedule_operation_timeout_callback( Args &&... ) const
		{}

		// Cancel timeout guard if any.
		constexpr void
		cancel() const
		{}
	};

	// Create guard for connection.
	constexpr timer_guard_t
	create_timer_guard() const
	{
		return timer_guard_t{};
	}

	struct factory_t
	{
		auto
		create( asio::io_context & )
		{
			return std::make_shared< null_timer_manager_t >();
		}
	};
};

} /* namespace restinio */
