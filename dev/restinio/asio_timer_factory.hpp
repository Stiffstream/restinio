/*
	restinio
*/

/*!
	Timer factory implementation using asio timers.
*/

#pragma once

#include <memory>
#include <chrono>

#include <asio.hpp>

#include <restinio/timer_common.hpp>

namespace restinio
{

//
// asio_timer_factory_t
//

//! Timer factory implementation using asio timers.
class asio_timer_factory_t
{
	public:
		//! Timer guard for async operations.
		class timer_guard_t final
		{
			public:
				timer_guard_t( asio::io_context & io_context )
					:	m_operation_timer{ io_context }
				{}

				// Set new timeout guard.
				void
				schedule_operation_timeout_callback(
					std::chrono::steady_clock::duration timeout,
					timer_invocation_tag_t tag,
					tcp_connection_ctx_weak_handle_t tcp_connection_ctx,
					timer_invocation_cb_t invocation_cb )
				{
					m_operation_timer.expires_after( timeout );
					m_operation_timer.async_wait(
							[ tag,
								tcp_connection_ctx = std::move( tcp_connection_ctx ),
								invocation_cb ]( const auto & ec ){
									if( !ec )
									{
										(*invocation_cb)( tag, std::move( tcp_connection_ctx ) );
									}
								} );
				}

				// Cancel timeout guard if any.
				void
				cancel()
				{
					m_operation_timer.cancel();
				}

			private:
				asio::steady_timer m_operation_timer;
			//! \}
		};

		// using timer_guard_instance_t = std::shared_ptr< timer_guard_t >;

		// Create guard for connection.
		timer_guard_t
		create_timer_guard( asio::io_context & io_context )
		{
			return timer_guard_t{ io_context };
		}

		constexpr void
		start( asio::io_context & io_context ) {}

		constexpr void
		stop( asio::io_context & ) {}
};

} /* namespace restinio */
