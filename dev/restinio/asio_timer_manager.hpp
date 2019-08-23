/*
	restinio
*/

/*!
	Timer factory implementation using asio timers.
*/

#pragma once

#include <memory>
#include <chrono>

#include <restinio/asio_include.hpp>

#include <restinio/utils/suppress_exceptions.hpp>

#include <restinio/timer_common.hpp>
#include <restinio/compiler_features.hpp>

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
			asio_ns::io_context & io_context,
			std::chrono::steady_clock::duration check_period )
			:	m_io_context{ io_context }
			,	m_check_period{ check_period }
		{}

		//! Timer guard for async operations.
		class timer_guard_t final
		{
			public:
				timer_guard_t(
					asio_ns::io_context & io_context,
					std::chrono::steady_clock::duration check_period ) noexcept
					:	m_operation_timer{ io_context }
					,	m_check_period{ check_period }
				{}

				//! Schedule timeouts check invocation.
				void
				schedule( tcp_connection_ctx_weak_handle_t weak_handle )
				{
					m_operation_timer.expires_after( m_check_period );
					m_operation_timer.async_wait(
							[ weak_handle = std::move( weak_handle ) ]( const auto & ec ){
									if( !ec )
									{
										if( auto h = weak_handle.lock() )
										{
											h->check_timeout( h );
										}
									}
								} );
				}

				//! Cancel timeout guard if any.
				/*!
				 * @note
				 * Since v.0.6.0 this method is noexcept.
				 */
				void
				cancel() noexcept
				{
					restinio::utils::suppress_exceptions_quietly(
							[this]{ m_operation_timer.cancel(); } );
				}

			private:
				asio_ns::steady_timer m_operation_timer;
				const std::chrono::steady_clock::duration m_check_period;
			//! \}
		};

		//! Create guard for connection.
		timer_guard_t
		create_timer_guard() const
		{
			return timer_guard_t{ m_io_context, m_check_period };
		}

		//! @name Start/stop timer manager.
		///@{
		void start() const noexcept {}
		void stop() const noexcept {}
		///@}

		struct factory_t final
		{
			//! Check period for timer events.
			const std::chrono::steady_clock::duration
				m_check_period{ std::chrono::seconds{ 1 } };

			factory_t() noexcept {}
			factory_t( std::chrono::steady_clock::duration check_period ) noexcept
				:	m_check_period{ check_period }
			{}

			//! Create an instance of timer manager.
			auto
			create( asio_ns::io_context & io_context ) const
			{
				return std::make_shared< asio_timer_manager_t >( io_context, m_check_period );
			}
		};

	private:
		//! An instanse of io_context to work with.
		asio_ns::io_context & m_io_context;

		//! Check period for timer events.
		const std::chrono::steady_clock::duration m_check_period;
};

} /* namespace restinio */
