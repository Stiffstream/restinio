/*
	restinio
*/

/*!
	Timer factory implementation using asio timers.
*/

#pragma once

#include <memory>
#include <chrono>
#include <unordered_map>

#include <asio.hpp>
#include <timertt/all.hpp>

namespace restinio
{

namespace impl
{

//! Context wrapper for a timer logic.
template < typename Timer_Manager >
class timer_context_t
	:	public std::enable_shared_from_this< timer_context_t< Timer_Manager > >
{
	public:
		using thread_safety_t = typename Timer_Manager::thread_safety;
		using timer_id_t = timertt::timer_object_holder< thread_safety_t >;

		template < typename... Args >
		timer_context_t(
			asio::io_context & io_context,
			std::chrono::steady_clock::duration tick,
			Args &&... args  )
			:	m_tick_timer{ io_context }
			,	m_tick{ std::move( tick ) }
			,	m_timer_manager{ std::forward< Args >( args )... }
		{}

		//! Schedule or reschedule a given timer with specified callback.
		template < typename Callback_Func >
		timer_id_t
		schedule_timer(
			std::chrono::steady_clock::duration timeout,
			Callback_Func && cb )
		{
			timer_id_t timer_id = m_timer_manager.allocate();
			m_timer_manager.activate(
				timer_id,
				timeout,
				std::move( cb ) );

			return timer_id;
		}

		//! Cancel a given timer.
		void
		cancel_timer( timer_id_t timer_id )
		{
			m_timer_manager.deactivate( std::move( timer_id ) );
		}

		void
		schedule_next_tick()
		{
			auto next_tp = std::chrono::steady_clock::now() + m_tick;

			auto n = m_timer_manager.nearest_time_point();
			if( std::get<0>( n ) )
			{
				const auto & alternative_next_tp = std::get<1>( n );

				if( alternative_next_tp < next_tp )
					next_tp = alternative_next_tp;
			}

			m_tick_timer.expires_at( next_tp );
			m_tick_timer.async_wait(
				[ ctx = this->shared_from_this() ]( const auto & ec ){
					if( !ec )
					{
						ctx->m_timer_manager.process_expired_timers();
						ctx->schedule_next_tick();
					}
				} );
		}

		void
		cancel_tick()
		{
			m_tick_timer.cancel();
		}

	private:
		asio::steady_timer m_tick_timer;
		const std::chrono::steady_clock::duration m_tick;
		Timer_Manager m_timer_manager;
};

} /* namespace impl */

//
// timertt_timer_factory_t
//

//! Timer factory implementation using asio timers.
template <
		typename Timer_Manager,
		typename Executor = asio::strand< asio::executor > >
class timertt_timer_factory_t
{
		using timer_context_t = impl::timer_context_t< Timer_Manager >;
		using timer_context_handle_t = std::shared_ptr< timer_context_t >;
		using timer_id_t = typename timer_context_t::timer_id_t;

	public:

		template < typename... Args >
		timertt_timer_factory_t(
			std::chrono::steady_clock::duration tick,
			Args &&... args )
			:	m_tick{ std::move( tick ) }
			,	m_context_factory{
				[ args... ](
					asio::io_context & io_context,
					std::chrono::steady_clock::duration tick ){
					return std::make_shared< timer_context_t >( io_context, tick, args... );
				} }
		{}

		//! Timer guard for async operations.
		class timer_guard_t final
			:	public std::enable_shared_from_this< timer_guard_t >
		{
			public:
				timer_guard_t( timer_context_handle_t timer_context )
					:	m_timer_context{ std::move( timer_context ) }
				{}

				// Set new timeout guard.
				template <
						typename Callback_Executor,
						typename Callback_Func >
				void
				schedule_operation_timeout_callback(
					const Callback_Executor & executor,
					std::chrono::steady_clock::duration timeout,
					Callback_Func && cb )
				{
					m_timer_id =
						m_timer_context->schedule_timer(
							timeout,
							[ cb_executor = executor,
								cb = std::move( cb ),
								tag = ++m_current_timer_tag,
								ctx = this->shared_from_this() ]{
									asio::dispatch(
										cb_executor,
										[ cb = std::move( cb ), tag, ctx = std::move( ctx ) ]{
											if( tag == ctx->m_current_timer_tag )
											{
												cb();
											}
										} );
							} );
				}

				// Cancel timeout guard if any.
				void
				cancel()
				{
					m_timer_context->cancel_timer( m_timer_id );
				}

			private:
				timer_context_handle_t m_timer_context;
				std::uint32_t m_current_timer_tag{ 0 };
				timer_id_t m_timer_id;
			//! \}
		};

		using timer_guard_instance_t = std::shared_ptr< timer_guard_t >;

		// Create guard for connection.
		timer_guard_instance_t
		create_timer_guard( asio::io_context & )
		{
			return std::make_shared< timer_guard_t >( m_timer_context );
		}

		void
		start( asio::io_context & io_context )
		{
			cancel_tick_if_needed();
			m_timer_context = m_context_factory( io_context, m_tick );
			m_timer_context->schedule_next_tick();
		}

		void
		stop( asio::io_context & )
		{
			cancel_tick_if_needed();
		}

	private:
		void
		cancel_tick_if_needed()
		{
			if( m_timer_context )
			{
				m_timer_context->cancel_tick();
				m_timer_context.reset();
			}
		}

		//! Tick duration.
		const std::chrono::steady_clock::duration m_tick;

		std::function< timer_context_handle_t(
			asio::io_context &,
			std::chrono::steady_clock::duration ) > m_context_factory;

		timer_context_handle_t m_timer_context;
};

struct timertt_noop_error_logger_t
{
	constexpr void
	operator () ( const std::string & what )
	{}
};

struct timertt_actor_exception_handler_t
{
	//! Handles exception.
	inline void
	operator()(
		//! An exception from timer actor.
		const std::exception & ex )
	{
		throw exception_t{ ex.what() };
	}
};

using st_timertt_wheel_timer_factory_t =
	timertt_timer_factory_t<
		timertt::timer_wheel_manager_template<
			timertt::thread_safety::unsafe,
			timertt_noop_error_logger_t,
			timertt_actor_exception_handler_t > >;

} /* namespace restinio */
