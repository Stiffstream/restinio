/*
	restinio
*/

#include <catch2/catch_all.hpp>

#include <restinio/all.hpp>
#include <restinio/async_chain/fixed_size.hpp>
#include <restinio/async_chain/growable_size.hpp>

#include <so_5/all.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

#include "../../common/test_extra_data_factory.ipp"

using atomic_counter_t = std::atomic< unsigned int >;

template< typename Extra_Data_Factory >
struct msg_your_turn final : public so_5::message_t
{
	using unique_controller_t =
			restinio::async_chain::unique_async_handling_controller_t< Extra_Data_Factory >;

	unique_controller_t m_controller;

	msg_your_turn( unique_controller_t controller )
		: m_controller{ std::move(controller) }
	{}
};

template< typename Extra_Data_Factory >
class a_dummy_stage_t final : public so_5::agent_t
{
	atomic_counter_t & m_counter;
	const so_5::mbox_t m_next_stage;

public:
	a_dummy_stage_t(
		context_t ctx,
		atomic_counter_t & counter,
		so_5::mbox_t next_stage )
		: so_5::agent_t{ std::move(ctx) }
		, m_counter{ counter }
		, m_next_stage{ std::move(next_stage) }
	{}

	void
	so_define_agent() override
	{
		so_subscribe_self().event(
			[this]( so_5::mutable_mhood_t< msg_your_turn< Extra_Data_Factory > > cmd )
			{
				++m_counter;
				next( std::move(cmd->m_controller) );
			} );
	}
};

template< typename Extra_Data_Factory >
class a_response_maker_t final : public so_5::agent_t
{
	atomic_counter_t & m_counter;

public:
	a_response_maker_t(
		context_t ctx,
		atomic_counter_t & counter )
		: so_5::agent_t{ std::move(ctx) }
		, m_counter{ counter }
	{}

	void
	so_define_agent() override
	{
		so_subscribe_self().event(
			[this]( so_5::mutable_mhood_t< msg_your_turn< Extra_Data_Factory > > cmd )
			{
				++m_counter;

				const auto req = cmd->m_controller->request_handle();
				req->create_response()
					.append_header( "Server", "RESTinio utest server" )
					.append_header_date_field()
					.append_header( "Content-Type", "text/plain; charset=utf-8" )
					.set_body(
						restinio::const_buffer( req->header().method().c_str() ) )
					.done();
			} );
	}
};
template<
	typename Request_Handler,
	typename Extra_Data_Factory >
struct test_traits_t : public restinio::traits_t<
	restinio::asio_timer_manager_t, utest_logger_t >
{
	using request_handler_t = Request_Handler;
	using extra_data_factory_t = Extra_Data_Factory;
};

template< typename Extra_Data_Factory >
void
tc_fixed_size_chain()
{
	using http_server_t = restinio::http_server_t<
			test_traits_t<
					restinio::async_chain::fixed_size_chain_t<
							4u, Extra_Data_Factory>,
					Extra_Data_Factory >
	>;

	using your_turn_t = msg_your_turn< Extra_Data_Factory >;
	using dummy_stage_t = a_dummy_stage_t< Extra_Data_Factory >;
	using response_maker_t = a_response_maker_t< Extra_Data_Factory >;

	atomic_counter_t stages_completed{ 0u };

	std::array< so_5::mbox_t, 4u > destinations;

	so_5::wrapped_env_t sobjectizer{
		[&]( so_5::environment_t & env ) {
			env.introduce_coop(
				so_5::disp::active_obj::make_dispatcher( env ).binder(),
				[&]( so_5::coop_t & coop ) {
					destinations[ 3u ] = coop.make_agent< response_maker_t >(
							stages_completed )->so_direct_mbox();
					destinations[ 2u ] = coop.make_agent< dummy_stage_t >(
							stages_completed, destinations[ 3u ] )->so_direct_mbox();
					destinations[ 1u ] = coop.make_agent< dummy_stage_t >(
							stages_completed, destinations[ 2u ] )->so_direct_mbox();
					destinations[ 0u ] = coop.make_agent< dummy_stage_t >(
							stages_completed, destinations[ 1u ] )->so_direct_mbox();
				} );
		}
	};

	http_server_t http_server{
		restinio::own_io_context(),
		[&destinations]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[&destinations]( auto controller ) {
						so_5::send< so_5::mutable_msg< your_turn_t > >(
								destinations[ 0u ], std::move(controller) );
						return restinio::async_chain::ok();
					},
					[&destinations]( auto controller ) {
						so_5::send< so_5::mutable_msg< your_turn_t > >(
								destinations[ 1u ], std::move(controller) );
						return restinio::async_chain::ok();
					},
					[&destinations]( auto controller ) {
						so_5::send< so_5::mutable_msg< your_turn_t > >(
								destinations[ 2u ], std::move(controller) );
						return restinio::async_chain::ok();
					},
					[&destinations]( auto controller ) {
						so_5::send< so_5::mutable_msg< your_turn_t > >(
								destinations[ 3u ], std::move(controller) );
						return restinio::async_chain::ok();
					} );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( "GET" ) );

	other_thread.stop_and_join();
	sobjectizer.stop_then_join();

	REQUIRE( 4 == stages_completed );
}

TEST_CASE( "async_chain::so_5::fixed_size_chain (no_user_data)" ,
		"[async_chain][so_5][fixed_size_chain][no_user_data]" )
{
	tc_fixed_size_chain< restinio::no_extra_data_factory_t >();
}

TEST_CASE( "async_chain::so_5::fixed_size_chain (test_user_data)" ,
		"[async_chain][so_5][fixed_size_chain][test_user_data]" )
{
	tc_fixed_size_chain< test::ud_factory_t >();
}

template< typename Extra_Data_Factory >
void
tc_fixed_size_chain_accept_in_middle()
{
	using http_server_t = restinio::http_server_t<
			test_traits_t<
					restinio::async_chain::fixed_size_chain_t<
							4u, Extra_Data_Factory>,
					Extra_Data_Factory >
	>;

	using your_turn_t = msg_your_turn< Extra_Data_Factory >;
	using dummy_stage_t = a_dummy_stage_t< Extra_Data_Factory >;
	using response_maker_t = a_response_maker_t< Extra_Data_Factory >;

	atomic_counter_t stages_completed{ 0u };

	std::array< so_5::mbox_t, 4u > destinations;

	so_5::wrapped_env_t sobjectizer{
		[&]( so_5::environment_t & env ) {
			env.introduce_coop(
				so_5::disp::active_obj::make_dispatcher( env ).binder(),
				[&]( so_5::coop_t & coop ) {
					destinations[ 3u ] = coop.make_agent< dummy_stage_t >(
							stages_completed, destinations[ 0u ] )->so_direct_mbox();
					destinations[ 2u ] = coop.make_agent< dummy_stage_t >(
							stages_completed, destinations[ 3u ] )->so_direct_mbox();
					destinations[ 1u ] = coop.make_agent< response_maker_t >(
							stages_completed )->so_direct_mbox();
					destinations[ 0u ] = coop.make_agent< dummy_stage_t >(
							stages_completed, destinations[ 1u ] )->so_direct_mbox();
				} );
		}
	};

	http_server_t http_server{
		restinio::own_io_context(),
		[&destinations]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[&destinations]( auto controller ) {
						so_5::send< so_5::mutable_msg< your_turn_t > >(
								destinations[ 0u ], std::move(controller) );
						return restinio::async_chain::ok();
					},
					[&destinations]( auto controller ) {
						so_5::send< so_5::mutable_msg< your_turn_t > >(
								destinations[ 1u ], std::move(controller) );
						return restinio::async_chain::ok();
					},
					[&destinations]( auto controller ) {
						so_5::send< so_5::mutable_msg< your_turn_t > >(
								destinations[ 2u ], std::move(controller) );
						return restinio::async_chain::ok();
					},
					[&destinations]( auto controller ) {
						so_5::send< so_5::mutable_msg< your_turn_t > >(
								destinations[ 3u ], std::move(controller) );
						return restinio::async_chain::ok();
					} );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( "GET" ) );

	other_thread.stop_and_join();
	sobjectizer.stop_then_join();

	REQUIRE( 2 == stages_completed );
}

TEST_CASE( "async_chain::so_5::fixed_size_chain_accept_in_middle (no_user_data)" ,
		"[async_chain][so_5][fixed_size_chain][no_user_data]" )
{
	tc_fixed_size_chain_accept_in_middle< restinio::no_extra_data_factory_t >();
}

TEST_CASE( "async_chain::so_5::fixed_size_chain_accept_in_middle (test_user_data)" ,
		"[async_chain][so_5][fixed_size_chain][test_user_data]" )
{
	tc_fixed_size_chain_accept_in_middle< test::ud_factory_t >();
}

template< typename Extra_Data_Factory >
void
tc_growable_size_chain()
{
	using request_handler_t =
			restinio::async_chain::growable_size_chain_t<
					Extra_Data_Factory
			>;

	using http_server_t = restinio::http_server_t<
			test_traits_t< request_handler_t, Extra_Data_Factory >
	>;

	using your_turn_t = msg_your_turn< Extra_Data_Factory >;
	using dummy_stage_t = a_dummy_stage_t< Extra_Data_Factory >;
	using response_maker_t = a_response_maker_t< Extra_Data_Factory >;

	atomic_counter_t stages_completed{ 0u };

	std::array< so_5::mbox_t, 4u > destinations;

	so_5::wrapped_env_t sobjectizer{
		[&]( so_5::environment_t & env ) {
			env.introduce_coop(
				so_5::disp::active_obj::make_dispatcher( env ).binder(),
				[&]( so_5::coop_t & coop ) {
					destinations[ 3u ] = coop.make_agent< response_maker_t >(
							stages_completed )->so_direct_mbox();
					destinations[ 2u ] = coop.make_agent< dummy_stage_t >(
							stages_completed, destinations[ 3u ] )->so_direct_mbox();
					destinations[ 1u ] = coop.make_agent< dummy_stage_t >(
							stages_completed, destinations[ 2u ] )->so_direct_mbox();
					destinations[ 0u ] = coop.make_agent< dummy_stage_t >(
							stages_completed, destinations[ 1u ] )->so_direct_mbox();
				} );
		}
	};

	typename request_handler_t::builder_t handler_builder;

	handler_builder.add(
			[&destinations]( auto controller ) {
				so_5::send< so_5::mutable_msg< your_turn_t > >(
						destinations[ 0u ], std::move(controller) );
				return restinio::async_chain::ok();
			} );
	handler_builder.add(
			[&destinations]( auto controller ) {
				so_5::send< so_5::mutable_msg< your_turn_t > >(
						destinations[ 1u ], std::move(controller) );
				return restinio::async_chain::ok();
			} );
	handler_builder.add(
			[&destinations]( auto controller ) {
				so_5::send< so_5::mutable_msg< your_turn_t > >(
						destinations[ 2u ], std::move(controller) );
				return restinio::async_chain::ok();
			} );
	handler_builder.add(
			[&destinations]( auto controller ) {
				so_5::send< so_5::mutable_msg< your_turn_t > >(
						destinations[ 3u ], std::move(controller) );
				return restinio::async_chain::ok();
			} );

	http_server_t http_server{
		restinio::own_io_context(),
		[&handler_builder]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler( handler_builder.release() );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( "GET" ) );

	other_thread.stop_and_join();
	sobjectizer.stop_then_join();

	REQUIRE( 4 == stages_completed );
}

TEST_CASE( "async_chain::so_5::growable_size_chain (no_user_data)" ,
		"[async_chain][so_5][growable_size_chain][no_user_data]" )
{
	tc_growable_size_chain< restinio::no_extra_data_factory_t >();
}

TEST_CASE( "gasync_chain::so_5::rowable_size_chain (test_user_data)" ,
		"[async_chain][so_5][growable_size_chain][test_user_data]" )
{
	tc_growable_size_chain< test::ud_factory_t >();
}

template< typename Extra_Data_Factory >
void
tc_growable_size_chain_accept_in_middle()
{
	using request_handler_t =
			restinio::async_chain::growable_size_chain_t<
					Extra_Data_Factory
			>;

	using http_server_t = restinio::http_server_t<
			test_traits_t< request_handler_t, Extra_Data_Factory >
	>;

	using your_turn_t = msg_your_turn< Extra_Data_Factory >;
	using dummy_stage_t = a_dummy_stage_t< Extra_Data_Factory >;
	using response_maker_t = a_response_maker_t< Extra_Data_Factory >;

	atomic_counter_t stages_completed{ 0u };

	std::array< so_5::mbox_t, 4u > destinations;

	so_5::wrapped_env_t sobjectizer{
		[&]( so_5::environment_t & env ) {
			env.introduce_coop(
				so_5::disp::active_obj::make_dispatcher( env ).binder(),
				[&]( so_5::coop_t & coop ) {
					destinations[ 3u ] = coop.make_agent< dummy_stage_t >(
							stages_completed, destinations[ 0u ] )->so_direct_mbox();
					destinations[ 2u ] = coop.make_agent< dummy_stage_t >(
							stages_completed, destinations[ 3u ] )->so_direct_mbox();
					destinations[ 1u ] = coop.make_agent< response_maker_t >(
							stages_completed )->so_direct_mbox();
					destinations[ 0u ] = coop.make_agent< dummy_stage_t >(
							stages_completed, destinations[ 1u ] )->so_direct_mbox();
				} );
		}
	};

	typename request_handler_t::builder_t handler_builder;

	handler_builder.add(
			[&destinations]( auto controller ) {
				so_5::send< so_5::mutable_msg< your_turn_t > >(
						destinations[ 0u ], std::move(controller) );
				return restinio::async_chain::ok();
			} );
	handler_builder.add(
			[&destinations]( auto controller ) {
				so_5::send< so_5::mutable_msg< your_turn_t > >(
						destinations[ 1u ], std::move(controller) );
				return restinio::async_chain::ok();
			} );
	handler_builder.add(
			[&destinations]( auto controller ) {
				so_5::send< so_5::mutable_msg< your_turn_t > >(
						destinations[ 2u ], std::move(controller) );
				return restinio::async_chain::ok();
			} );
	handler_builder.add(
			[&destinations]( auto controller ) {
				so_5::send< so_5::mutable_msg< your_turn_t > >(
						destinations[ 3u ], std::move(controller) );
				return restinio::async_chain::ok();
			} );

	http_server_t http_server{
		restinio::own_io_context(),
		[&handler_builder]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler( handler_builder.release() );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( "GET" ) );

	other_thread.stop_and_join();
	sobjectizer.stop_then_join();

	REQUIRE( 2 == stages_completed );
}

TEST_CASE( "async_chain::so_5::growable_size_chain_accept_in_middle (no_user_data)" ,
		"[async_chain][so_5][growable_size_chain][no_user_data]" )
{
	tc_growable_size_chain_accept_in_middle< restinio::no_extra_data_factory_t >();
}

TEST_CASE( "gasync_chain::so_5::rowable_size_chain_accept_in_middle (test_user_data)" ,
		"[async_chain][so_5][growable_size_chain][test_user_data]" )
{
	tc_growable_size_chain_accept_in_middle< test::ud_factory_t >();
}

