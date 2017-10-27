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
// null_timer_factory_t
//

//! Timer factory implementation using asio timers.
struct null_timer_factory_t
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
	create_timer_guard( asio::io_context & )
	{
		return timer_guard_t{};
	}

	constexpr void
	start( asio::io_context & ) {}

	constexpr void
	stop( asio::io_context & ) {}
};

} /* namespace restinio */
