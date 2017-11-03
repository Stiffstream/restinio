/*
	restinio
*/

/*!
	Timers implementation with sobjectizer timers.
*/

#pragma once

#include <asio.hpp>

#include <so_5/all.hpp>

namespace restinio
{

namespace so5
{

//
// msg_ckeck_timer_t
//

//! Check timer.
struct msg_ckeck_timer_t final : public so_5::message_t
{
	template < typename Checker >
	msg_ckeck_timer_t( Checker && checker )
		:	m_checker{ std::move( checker ) }
	{}

	std::function< void ( void ) > m_checker;
};

//
// so_timer_manager_t
//

//! Timer factory implementation using asio timers.
class so_timer_manager_t final
{
	public:
		so_timer_manager_t(
			so_5::environment_t & env,
			so_5::mbox_t mbox )
			:	m_env{ env }
			,	m_mbox{ std::move( mbox ) }
		{}

		//! Timer guard for async operations.
		class timer_guard_t final
		{
			public:
				using timeout_handler_t = std::function< void ( void ) >;

				timer_guard_t(
					so_5::environment_t & env,
					so_5::mbox_t mbox )
					:	m_env{ env }
					,	m_mbox{ std::move( mbox ) }
				{}

				// Set new timeout guard.
				void
				schedule_operation_timeout_callback(
					std::chrono::steady_clock::duration timeout,
					timer_invocation_tag_t tag,
					tcp_connection_ctx_weak_handle_t tcp_connection_ctx,
					timer_invocation_cb_t invocation_cb )
				{
					cancel();

					auto msg =
						std::make_unique< msg_ckeck_timer_t >(
							[ tag,
								tcp_connection_ctx = std::move( tcp_connection_ctx ),
								invocation_cb]() mutable {
									(*invocation_cb)( tag, std::move( tcp_connection_ctx ) );
								} );

					m_current_op_timer =
						m_env.schedule_timer(
							std::move( msg ),
							m_mbox,
							std::move( timeout ),
							std::chrono::steady_clock::duration::zero() );
				}

				// Cancel timeout guard if any.
				void
				cancel()
				{
					m_current_op_timer.release();
				}

			private:
				so_5::environment_t & m_env;
				const so_5::mbox_t m_mbox;

				so_5::timer_id_t m_current_op_timer;
			//! \}
		};

		// Create guard for connection.
		timer_guard_t
		create_timer_guard()
		{
			return timer_guard_t{ m_env, m_mbox };
		}

		//! Start/stop timer manager.
		//! \{
		void start() const {}
		void stop() const {}
		//! \}

		struct factory_t
		{
			so_5::environment_t & m_env;
			so_5::mbox_t m_mbox;

			factory_t( so_5::environment_t & env,so_5::mbox_t mbox )
				:	m_env{ env }
				,	m_mbox{ std::move( mbox ) }
			{}

			auto
			create( asio::io_context & ) const
			{
				return std::make_shared< so_timer_manager_t >( m_env, m_mbox );
			}
		};

	private:
		so_5::environment_t & m_env;
		so_5::mbox_t m_mbox;
};

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
					[]( const msg_ckeck_timer_t & msg ){
						msg.m_checker();
					} );
		}
};

} /* namespace so5 */

} /* namespace restinio */
