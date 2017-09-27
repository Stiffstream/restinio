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
				timer_guard_t( asio::io_context & iosvc )
					:	m_operation_timer{ iosvc }
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
					cancel();
					m_operation_timer.expires_from_now( timeout );
					m_operation_timer.async_wait(
						asio::bind_executor(
							executor,
							[ this,
								cb = std::move( f ),
								tag = ++m_current_timer_tag,
								ctx = shared_from_this() ]( auto ec ){
								if( !ec && tag == m_current_timer_tag )
									cb();
							} ) );
				}

				// Cancel timeout guard if any.
				void
				cancel()
				{
					++m_current_timer_tag;
					m_operation_timer.cancel_one();
				}

			private:
				asio::steady_timer m_operation_timer;
				std::uint32_t m_current_timer_tag{ 0 };
			//! \}
		};

		using timer_guard_instance_t = std::shared_ptr< timer_guard_t >;

		// Create guard for connection.
		timer_guard_instance_t
		create_timer_guard( asio::io_context & iosvc )
		{
			return std::make_shared< timer_guard_t >( iosvc );
		}
};

} /* namespace restinio */
