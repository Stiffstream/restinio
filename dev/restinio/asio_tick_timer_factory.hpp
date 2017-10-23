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

namespace restinio
{

namespace impl
{

//! Context wrapper for a timer logic.
template < typename Executor >
class timer_context_t : public std::enable_shared_from_this< timer_context_t< Executor > >
{
	public:
		timer_context_t(
			asio::io_context & io_context,
			std::chrono::steady_clock::duration tick,
			std::size_t initial_buckets )
			:	m_strand{ io_context.get_executor() }
			,	m_tick_timer{ io_context }
			,	m_tick{ std::move( tick ) }
			,	m_timers{ initial_buckets }
		{}

		//! Schedule or reschedule a given timer with specified callback.
		template < typename Callback_Func >
		void
		schedule_timer(
			void * timer_id,
			const Executor & executor,
			std::chrono::steady_clock::duration timeout,
			Callback_Func && cb )
		{
			Executor callback_executor = executor;
			asio::dispatch(
				m_strand,
				[ ctx = this->shared_from_this(),
					timer_id,
					callback_executor,
					timeout,
					cb = callback_t{ std::move( cb ) } ]{
					ctx->schedule_impl(
						timer_id,
						std::move( callback_executor ),
						std::move( timeout ),
						std::move( cb ) );
				} );
		}

		//! Cancel a given timer.
		void
		cancel_timer( void * timer_id )
		{
			// It is important to use post here:
			// beacause a chain of dispatch calls when expired timer is hit
			// causes cancel_impl() to execute which invalidates
			// iterator on expired timer table entry in
			// check_expired_timers() loop.
			asio::post(
				m_strand,
				[ timer_id, ctx = this->shared_from_this() ]{
					ctx->cancel_impl( timer_id );
				} );
		}

		void
		schedule_next_tick()
		{
			m_tick_timer.expires_after( m_tick );
			m_tick_timer.async_wait(
				asio::bind_executor(
					m_strand,
					[ ctx = this->shared_from_this() ]( const auto & ec ){
						if( !ec )
						{
							ctx->check_expired_timers();
							ctx->schedule_next_tick();
						}
					} ) );
		}

		void
		cancel_tick()
		{
			m_tick_timer.cancel();
		}

	private:
		using callback_t = std::function< void (void ) >;

		void
		schedule_impl(
			void * timer_id,
			Executor executor,
			std::chrono::steady_clock::duration timeout,
			callback_t cb )
		{
			auto it = m_timers.find( timer_id );
			if( m_timers.end() != it )
			{
				it->second.m_cb = std::move( cb );
				it->second.m_expired_after = std::chrono::steady_clock::now() + timeout;
			}
			else
			{
				typename timer_table_t::value_type
					p{ timer_id, timer_data_t{ timeout, std::move( executor ), std::move( cb ) } };
				m_timers.insert( std::move( p ) );
			}
		}

		void
		cancel_impl( void * timer_id )
		{
			m_timers.erase( timer_id );
		}

		//! Checks expired timers, execute them and remove them from table.
		void
		check_expired_timers()
		{
			auto it = m_timers.begin();
			const auto it_end = m_timers.end();

			const auto now = std::chrono::steady_clock::now();

			while( it_end != it )
			{
				auto & d = it->second;

				if( d.m_expired_after <= now )
				{
					asio::dispatch(
						d.m_cb_executor,
						[ cb = std::move( d.m_cb ) ]{
							cb();
						} );

					it = m_timers.erase( it );
				}
				else
				{
					++it;
				}
			}
		}

		Executor m_strand;
		asio::steady_timer m_tick_timer;
		const std::chrono::steady_clock::duration m_tick;

		struct timer_data_t
		{
			timer_data_t(
				std::chrono::steady_clock::duration timeout_from_now,
				Executor cb_executor,
				callback_t cb )
				:	m_expired_after( std::chrono::steady_clock::now() + timeout_from_now )
				,	m_cb_executor( std::move( cb_executor ) )
				,	m_cb{ std::move( cb ) }
			{}

			~timer_data_t()
			{}

			timer_data_t( const timer_data_t & ) = delete;
			const timer_data_t & operator = ( const timer_data_t & ) = delete;

			timer_data_t( timer_data_t && ) = default;
			timer_data_t & operator = ( timer_data_t && ) = delete;

			std::chrono::steady_clock::time_point m_expired_after;
			Executor m_cb_executor;
			callback_t m_cb;
		};

		using timer_table_t = std::unordered_map< void *, timer_data_t >;
		timer_table_t m_timers;
};

} /* namespace impl */

//
// asio_tick_timer_factory_t
//

//! Timer factory implementation using asio timers.
template < typename Executor = asio::strand< asio::executor > >
class asio_tick_timer_factory_t
{
	using timer_context_handle_t = std::shared_ptr< impl::timer_context_t< Executor > >;

	public:
		asio_tick_timer_factory_t(
			std::chrono::steady_clock::duration tick = std::chrono::seconds( 1 ),
			std::size_t initial_buckets = 64 )
			:	m_tick{ std::move( tick ) }
			,	m_initial_buckets{ initial_buckets }
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
					m_timer_context->schedule_timer(
						this,
						executor,
						timeout,
						[ cb = std::move( cb ),
							tag = ++m_current_timer_tag,
							ctx = this->shared_from_this() ]{
							if( tag == ctx->m_current_timer_tag )
							{
								cb();
							}
						} );
				}

				// Cancel timeout guard if any.
				void
				cancel()
				{
					m_timer_context->cancel_timer( this );
				}

			private:
				timer_context_handle_t m_timer_context;
				std::uint32_t m_current_timer_tag{ 0 };
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
			m_timer_context =
				std::make_shared< impl::timer_context_t< Executor > >(
					io_context,
					m_tick,
					m_initial_buckets );
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
		const std::size_t m_initial_buckets;

		timer_context_handle_t m_timer_context;
};

using mt_asio_tick_timer_factory_t = asio_tick_timer_factory_t< asio::strand< asio::executor > >;
using st_asio_tick_timer_factory_t = asio_tick_timer_factory_t< asio::executor >;

} /* namespace restinio */
