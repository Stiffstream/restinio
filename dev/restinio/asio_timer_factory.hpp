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
			:	public std::enable_shared_from_this< timer_guard_t >
		{
			public:
				timer_guard_t( asio::io_context & io_context )
					:	m_operation_timer{ io_context }
				{}

				// Set new timeout guard.
				template <
						typename Executor,
						typename Callback_Func >
				void
				schedule_operation_timeout_callback(
					const Executor & executor,
					std::chrono::steady_clock::duration timeout,
					Callback_Func && f )
				{
					Executor callback_executor = executor;
					m_operation_timer.expires_after( timeout );
					m_operation_timer.async_wait(
							[ callback_executor,
								cb = std::move( f ),
								tag = ++m_current_timer_tag,
								ctx = shared_from_this() ]( const auto & ec ){
									if( !ec )
									{
										asio::post(
											callback_executor,
											[ cb = std::move( cb ), tag, ctx = std::move( ctx ) ]{
												if( tag == ctx->m_current_timer_tag )
													cb();
											} );
									}
								} );
				}

				// Cancel timeout guard if any.
				void
				cancel()
				{
					++m_current_timer_tag;
					m_operation_timer.cancel();
				}

			private:
				asio::steady_timer m_operation_timer;
				std::uint32_t m_current_timer_tag{ 0 };
			//! \}
		};

		using timer_guard_instance_t = std::shared_ptr< timer_guard_t >;

		// Create guard for connection.
		timer_guard_instance_t
		create_timer_guard( asio::io_context & io_context )
		{
			return std::make_shared< timer_guard_t >( io_context );
		}

		constexpr void
		start( asio::io_context & io_context ) {}

		constexpr void
		stop( asio::io_context & ) {}
};

} /* namespace restinio */
