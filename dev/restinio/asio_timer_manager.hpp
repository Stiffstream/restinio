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
// asio_timer_manager_t
//

//! Timer factory implementation using asio timers.
class asio_timer_manager_t final
	:	public std::enable_shared_from_this< asio_timer_manager_t >
{
	public:
		asio_timer_manager_t( asio::io_context & io_context )
			:	m_io_context( io_context )
		{}

		//! Timer guard for async operations.
		class timer_guard_t final
		{
			public:
				timer_guard_t( asio::io_context & io_context )
					:	m_operation_timer{ io_context }
				{}

				// Guard operation.
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

		// Create guard for connection.
		timer_guard_t
		create_timer_guard()
		{
			return timer_guard_t{ m_io_context };
		}

		//! Start/stop timer manager.
		//! \{
		void start() const {}
		void stop() const {}
		//! \}

		struct factory_t
		{
			auto
			create( asio::io_context & io_context ) const
			{
				return std::make_shared< asio_timer_manager_t >( io_context );
			}
		};

	private:
		asio::io_context & m_io_context;
};

} /* namespace restinio */
