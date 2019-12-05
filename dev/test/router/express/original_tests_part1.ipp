// "/"
// null
// [["/",["/"]],["/route",null]]
TEST_CASE( "Original tests #0", "[path2regex][original][generated][n0]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/test"
// null
// [["/test",["/test"]],["/route",null],["/test/route",null],["/test/",["/test/"]]]
TEST_CASE( "Original tests #1", "[path2regex][original][generated][n1]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/route)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/test/"
// null
// [["/test",null],["/test/",["/test/"]],["/test//",["/test//"]]]
TEST_CASE( "Original tests #2", "[path2regex][original][generated][n2]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test/)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test//)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test//)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/test"
// {"sensitive":true}
// [["/test",["/test"]],["/TEST",null]]
TEST_CASE( "Original tests #3", "[path2regex][original][generated][n3]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test)route",
			path2regex::options_t{}.sensitive( true ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/TEST)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/TEST"
// {"sensitive":true}
// [["/test",null],["/TEST",["/TEST"]]]
TEST_CASE( "Original tests #4", "[path2regex][original][generated][n4]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/TEST)route",
			path2regex::options_t{}.sensitive( true ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/TEST)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/TEST)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/test"
// {"strict":true}
// [["/test",["/test"]],["/test/",null],["/TEST",["/TEST"]]]
TEST_CASE( "Original tests #5", "[path2regex][original][generated][n5]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test)route",
			path2regex::options_t{}.strict( true ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/TEST)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/TEST)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/test/"
// {"strict":true}
// [["/test",null],["/test/",["/test/"]],["/test//",null]]
TEST_CASE( "Original tests #6", "[path2regex][original][generated][n6]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test/)route",
			path2regex::options_t{}.strict( true ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test//)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/test"
// {"end":false}
// [["/test",["/test"]],["/test/",["/test/"]],["/test/route",["/test"]],["/route",null]]
TEST_CASE( "Original tests #7", "[path2regex][original][generated][n7]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test)route",
			path2regex::options_t{}.ending( false ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/test/"
// {"end":false}
// [["/test",null],["/test/route",["/test/"]],["/test//",["/test//"]],["/test//route",["/test/"]]]
TEST_CASE( "Original tests #8", "[path2regex][original][generated][n8]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test/)route",
			path2regex::options_t{}.ending( false ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test//)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test//)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test//route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:test"
// {"end":false}
// [["/route",["/route","route"]]]
TEST_CASE( "Original tests #9", "[path2regex][original][generated][n9]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test)route",
			path2regex::options_t{}.ending( false ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:test/"
// {"end":false}
// [["/route",null],["/route/",["/route/","route"]]]
TEST_CASE( "Original tests #10", "[path2regex][original][generated][n10]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test/)route",
			path2regex::options_t{}.ending( false ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route/)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/test"
// {"end":false,"strict":true}
// [["/test",["/test"]],["/test/",["/test"]],["/test/route",["/test"]]]
TEST_CASE( "Original tests #11", "[path2regex][original][generated][n11]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test)route",
			path2regex::options_t{}.strict( true ).ending( false ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/test/"
// {"end":false,"strict":true}
// [["/test",null],["/test/",["/test/"]],["/test//",["/test/"]],["/test/route",["/test/"]]]
TEST_CASE( "Original tests #12", "[path2regex][original][generated][n12]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test/)route",
			path2regex::options_t{}.strict( true ).ending( false ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test//)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/test.json"
// {"end":false,"strict":true}
// [["/test.json",["/test.json"]],["/test.json.hbs",null],["/test.json/route",["/test.json"]]]
TEST_CASE( "Original tests #13", "[path2regex][original][generated][n13]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test.json)route",
			path2regex::options_t{}.strict( true ).ending( false ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test.json)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test.json)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test.json.hbs)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test.json/route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test.json)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:test"
// {"end":false,"strict":true}
// [["/route",["/route","route"]],["/route/",["/route","route"]]]
TEST_CASE( "Original tests #14", "[path2regex][original][generated][n14]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test)route",
			path2regex::options_t{}.strict( true ).ending( false ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:test/"
// {"end":false,"strict":true}
// [["/route",null],["/route/",["/route/","route"]]]
TEST_CASE( "Original tests #15", "[path2regex][original][generated][n15]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test/)route",
			path2regex::options_t{}.strict( true ).ending( false ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route/)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// Test #16 skipped
// "/one,/two"
// null
// [["/one",["/one"]],["/two",["/two"]],["/three",null],["/one/two",null]]

// "/test"
// {"end":false}
// [["/test/route",["/test"]]]
TEST_CASE( "Original tests #17", "[path2regex][original][generated][n17]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test)route",
			path2regex::options_t{}.ending( false ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:test"
// null
// [["/route",["/route","route"]],["/another",["/another","another"]],["/something/else",null],["/route.json",["/route.json","route.json"]],["/something%2Felse",["/something%2Felse","something%2Felse"]],["/something%2Felse%2Fmore",["/something%2Felse%2Fmore","something%2Felse%2Fmore"]],["/;,:@&=+$-_.!~*()",["/;,:@&=+$-_.!~*()",";,:@&=+$-_.!~*()"]]]
TEST_CASE( "Original tests #18", "[path2regex][original][generated][n18]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/another)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/another)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(another)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/something/else)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route.json)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route.json)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route.json)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/something%2Felse)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/something%2Felse)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(something%2Felse)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		// A special check for normalization of incoming path before
		// searching the required pattern.
		restinio::router::impl::target_path_holder_t target_path{ R"target(/s%6fm%65thi%6Eg%2Fel%73e)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/something%2Felse)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(something%2Felse)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/something%2Felse%2Fmore)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/something%2Felse%2Fmore)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(something%2Felse%2Fmore)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/;,:@&=+$-_.!~*())target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/;,:@&=+$-_.!~*())match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(;,:@&=+$-_.!~*())value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}
}

// "/:test"
// {"strict":true}
// [["/route",["/route","route"]],["/route/",null]]
TEST_CASE( "Original tests #19", "[path2regex][original][generated][n19]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test)route",
			path2regex::options_t{}.strict( true ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route/)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}
}

