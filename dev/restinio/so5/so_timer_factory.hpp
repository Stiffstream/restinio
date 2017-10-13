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
struct msg_ckeck_timer_t : public so_5::message_t
{
	template < typename Checker >
	msg_ckeck_timer_t( Checker && checker )
		:	m_checker{ std::move( checker ) }
	{}

	std::function< void ( void ) > m_checker;
};

//
// so_timer_factory_t
//

//! Timer factory implementation using asio timers.
class so_timer_factory_t
{
	public:
		so_timer_factory_t(
			so_5::environment_t & env,
			so_5::mbox_t mbox )
			:	m_env{ env }
			,	m_mbox{ std::move( mbox ) }
		{}

		//! Timer guard for async operations.
		class timer_guard_t final
			:	public std::enable_shared_from_this< timer_guard_t >
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
					m_timeout_handler = std::move( f );

					std::weak_ptr< timer_guard_t > self_wp = shared_from_this();
					auto msg =
						std::make_unique< msg_ckeck_timer_t >(
							[ self_wp,
								executor = executor,
								tag = m_current_timer_tag](){

									if( auto ctx = self_wp.lock() )
									{
										ctx->init_timeout_check(
											executor,
											tag );
									}
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
					++m_current_timer_tag;
					m_current_op_timer.release();

					// Important to clear previous handler,
					// because it might inderectly reference itself.
					m_timeout_handler = timeout_handler_t{};
				}

				template< typename Executor >
				void
				init_timeout_check(
					const Executor & executor,
					std::uint32_t tag )
				{
					asio::post(
						executor,
						[ this, ctx = shared_from_this(), tag ](){
							if( tag == m_current_timer_tag )
								m_timeout_handler();
						} );
				}

			private:
				std::uint32_t m_current_timer_tag{ 0 };

				so_5::environment_t & m_env;
				const so_5::mbox_t m_mbox;

				so_5::timer_id_t m_current_op_timer;

				timeout_handler_t m_timeout_handler;
			//! \}
		};

		using timer_guard_instance_t = std::shared_ptr< timer_guard_t >;

		// Create guard for connection.
		timer_guard_instance_t
		create_timer_guard( asio::io_context & )
		{
			return std::make_shared< timer_guard_t >( m_env, m_mbox );
		}

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
