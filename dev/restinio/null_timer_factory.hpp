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
				typename EXECUTOR,
				typename CALLBACK_FUNC >
		constexpr void
		schedule_operation_timeout_callback(
			const EXECUTOR & ,
			std::chrono::steady_clock::duration ,
			CALLBACK_FUNC && ) const
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
	create_timer_guard( asio::io_service & )
	{
		return timer_guard_instance_t{};
	}
};

} /* namespace restinio */
