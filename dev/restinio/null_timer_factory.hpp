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
		void
		schedule_operation_timeout_callback(
			const EXECUTOR & ,
			std::chrono::steady_clock::duration ,
			CALLBACK_FUNC && ) const
		{}

		// Cancel timeout guard if any.
		void
		cancel() const
		{}
	};

	using timer_guard_instance_t = std::shared_ptr< timer_guard_t >;

	// Create guard for connection.
	timer_guard_instance_t
	create_timer_guard( asio::io_service & )
	{
		return std::make_shared< timer_guard_t >();
	}
};

} /* namespace restinio */
