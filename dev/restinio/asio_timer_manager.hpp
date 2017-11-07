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
		asio_timer_manager_t(
			asio::io_context & io_context,
			std::chrono::steady_clock::duration check_period )
			:	m_io_context( io_context )
			,	m_check_period{ check_period }
		{}

		//! Timer guard for async operations.
		class timer_guard_t final
		{
			public:
				timer_guard_t(
					asio::io_context & io_context,
					std::chrono::steady_clock::duration check_period )
					:	m_operation_timer{ io_context }
					,	m_check_period{ check_period }
				{}

				// Guard operation.
				void
				schedule_timeout_check_invocation(
					tcp_connection_ctx_weak_handle_t weak_handle )
				{
					m_operation_timer.expires_after( m_check_period );
					m_operation_timer.async_wait(
							[ weak_handle = std::move( weak_handle ) ]( const auto & ec ){
									if( !ec )
									{
										if( auto h = weak_handle.lock() )
										{
											h->check_timeout();
										}
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
				const std::chrono::steady_clock::duration m_check_period;
			//! \}
		};

		// Create guard for connection.
		timer_guard_t
		create_timer_guard()
		{
			return timer_guard_t{ m_io_context, m_check_period };
		}

		//! Start/stop timer manager.
		//! \{
		void start() const {}
		void stop() const {}
		//! \}

		struct factory_t
		{
			factory_t() {}
			factory_t( std::chrono::steady_clock::duration check_period )
				:	m_check_period{ check_period }
			{}

			auto
			create( asio::io_context & io_context ) const
			{
				return std::make_shared< asio_timer_manager_t >( io_context, m_check_period );
			}

			const std::chrono::steady_clock::duration m_check_period{ std::chrono::seconds{ 1 } };
		};

	private:
		asio::io_context & m_io_context;
		const std::chrono::steady_clock::duration m_check_period;
};

} /* namespace restinio */
