/*
	restinio
*/

/*!
	Timers implementation with sobjectizer timers.
*/

#pragma once

#include <restinio/asio_include.hpp>
#include <restinio/compiler_features.hpp>

#include <so_5/all.hpp>

namespace restinio
{

namespace so5
{

#if defined(SO_5_VERSION)
	#if SO_5_VERSION < SO_5_VERSION_MAKE(6ull, 0ull, 0ull)
		#define RESTINIO_USE_SO_5_5
	#endif
#else
	#define RESTINIO_USE_SO_5_5
#endif

//
// msg_check_timer_t
//

//! Check timer.
struct msg_check_timer_t final : public so_5::message_t
{
	msg_check_timer_t( tcp_connection_ctx_weak_handle_t weak_handle )
		:	m_weak_handle{ std::move( weak_handle ) }
	{}

	tcp_connection_ctx_weak_handle_t m_weak_handle;
};

//
// so_timer_manager_t
//

#if defined(RESTINIO_USE_SO_5_5)

// The implementation of so_timer_manager for SO-5.5.
// SO-5.5 requires a reference to SObjectizer Environment for working with timers.

//! Timer factory implementation using timers from SObjectizer.
class so_timer_manager_t final
{
	public:
		so_timer_manager_t(
			so_5::environment_t & env,
			so_5::mbox_t mbox,
			std::chrono::steady_clock::duration check_period )
			:	m_env{ env }
			,	m_mbox{ std::move( mbox ) }
			,	m_check_period{ check_period }
		{}

		//! Timer guard for async operations.
		class timer_guard_t final
		{
			public:
				timer_guard_t(
					so_5::environment_t & env,
					so_5::mbox_t mbox,
					std::chrono::steady_clock::duration check_period )
					:	m_env{ env }
					,	m_mbox{ std::move( mbox ) }
					,	m_check_period{ check_period }
				{}

				//! Schedule timeout check invocation.
				void
				schedule( tcp_connection_ctx_weak_handle_t weak_handle )
				{
					if( !m_current_op_timer.is_active() )
					{
						m_current_op_timer = so_5::send_periodic< msg_check_timer_t >(
								m_env,
								m_mbox,
								m_check_period,
								m_check_period,
								std::move(weak_handle) );

					}
				}

				//! Cancel timeout guard if any.
				/*!
				 * @note
				 * Since v.0.6.0 this method is noexcept.
				 */
				void
				cancel() noexcept
				{
					RESTINIO_ENSURE_NOEXCEPT_CALL( m_current_op_timer.release() );
				}

			private:
				so_5::environment_t & m_env;
				const so_5::mbox_t m_mbox;

				so_5::timer_id_t m_current_op_timer;
				const std::chrono::steady_clock::duration m_check_period;
			//! \}
		};

		// Create guard for connection.
		timer_guard_t
		create_timer_guard()
		{
			return timer_guard_t{ m_env, m_mbox, m_check_period };
		}

		//! Start/stop timer manager.
		//! \{
		void start() const noexcept {}
		void stop() const noexcept {}
		//! \}

		struct factory_t
		{
			so_5::environment_t & m_env;
			so_5::mbox_t m_mbox;
			const std::chrono::steady_clock::duration m_check_period;

			factory_t(
				so_5::environment_t & env,
				so_5::mbox_t mbox,
				std::chrono::steady_clock::duration check_period = std::chrono::seconds{ 1 } )
				:	m_env{ env }
				,	m_mbox{ std::move( mbox ) }
				,	m_check_period{ check_period }
			{}

			auto
			create( asio_ns::io_context & ) const
			{
				return std::make_shared< so_timer_manager_t >( m_env, m_mbox, m_check_period );
			}
		};

	private:
		so_5::environment_t & m_env;
		so_5::mbox_t m_mbox;
		const std::chrono::steady_clock::duration m_check_period;
};

#else

// The implementation of so_timer_manager for SO-5.6 and newer.
// There is no need to hold a reference to SObjectizer Environment for working
// with timers.

//! Timer factory implementation using timers from SObjectizer.
class so_timer_manager_t final
{
	public:
		so_timer_manager_t(
			so_5::mbox_t mbox,
			std::chrono::steady_clock::duration check_period )
			:	m_mbox{ std::move( mbox ) }
			,	m_check_period{ check_period }
		{}

		//! Timer guard for async operations.
		class timer_guard_t final
		{
			public:
				timer_guard_t(
					so_5::mbox_t mbox,
					std::chrono::steady_clock::duration check_period )
					:	m_mbox{ std::move( mbox ) }
					,	m_check_period{ check_period }
				{}

				//! Schedule timeout check invocation.
				void
				schedule( tcp_connection_ctx_weak_handle_t weak_handle )
				{
					if( !m_current_op_timer.is_active() )
					{
						m_current_op_timer = so_5::send_periodic< msg_check_timer_t >(
								m_mbox,
								m_check_period,
								m_check_period,
								std::move(weak_handle) );

					}
				}

				//! Cancel timeout guard if any.
				/*!
				 * @note
				 * Since v.0.6.0 this method is noexcept.
				 */
				void
				cancel() noexcept
				{
					RESTINIO_ENSURE_NOEXCEPT_CALL( m_current_op_timer.release() );
				}

			private:
				const so_5::mbox_t m_mbox;

				so_5::timer_id_t m_current_op_timer;
				const std::chrono::steady_clock::duration m_check_period;
			//! \}
		};

		// Create guard for connection.
		timer_guard_t
		create_timer_guard()
		{
			return timer_guard_t{ m_mbox, m_check_period };
		}

		//! Start/stop timer manager.
		//! \{
		void start() const noexcept {}
		void stop() const noexcept {}
		//! \}

		struct factory_t
		{
			so_5::mbox_t m_mbox;
			const std::chrono::steady_clock::duration m_check_period;

			factory_t(
				so_5::mbox_t mbox,
				std::chrono::steady_clock::duration check_period = std::chrono::seconds{ 1 } )
				:	m_mbox{ std::move( mbox ) }
				,	m_check_period{ check_period }
			{}

			// This constructor is just for compatibility with previous versions.
			factory_t(
				so_5::environment_t &,
				so_5::mbox_t mbox,
				std::chrono::steady_clock::duration check_period = std::chrono::seconds{ 1 } )
				:	factory_t{ std::move(mbox), check_period }
			{}

			auto
			create( asio_ns::io_context & ) const
			{
				return std::make_shared< so_timer_manager_t >( m_mbox, m_check_period );
			}
		};

	private:
		so_5::mbox_t m_mbox;
		const std::chrono::steady_clock::duration m_check_period;
};
#endif

//
// a_timeout_handler_t
//

//! Agent that handles timeouts.
class a_timeout_handler_t final
	:	public so_5::agent_t
{
		typedef so_5::agent_t so_base_type_t;

	public:
		a_timeout_handler_t( context_t ctx )
			:	so_base_type_t{ std::move( ctx ) }
		{
			so_subscribe_self()
				.event(
					[]( const msg_check_timer_t & msg ){
						if( auto h = msg.m_weak_handle.lock() )
							h->check_timeout( h );
					} );
		}
};

} /* namespace so5 */

} /* namespace restinio */

