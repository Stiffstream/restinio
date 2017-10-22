/*
	restinio
*/

/*!
	No-op timer factory implementation.
*/

#pragma once

#include <chrono>

#include <asio.hpp>

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
		template <
				typename Executor,
				typename Callback >
		constexpr void
		schedule_operation_timeout_callback(
			const Executor & ,
			std::chrono::steady_clock::duration ,
			Callback && ) const
		{}

		// Cancel timeout guard if any.
		constexpr void
		cancel() const
		{}
	};

	struct timer_guard_instance_t
	{
		constexpr const timer_guard_t *
		operator ->() const noexcept
		{
			return static_cast< timer_guard_t * >( nullptr );
		}
	};

	// Create guard for connection.
	timer_guard_instance_t
	create_timer_guard( asio::io_context & )
	{
		return timer_guard_instance_t{};
	}

	constexpr void
	start( asio::io_context & ) {}

	constexpr void
	stop( asio::io_context & ) {}
};

} /* namespace restinio */
