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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/)target", params ) );
		REQUIRE( params.match() == R"match(/)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/route)target", params ) );
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test)target", params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/route)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/test/route)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test/)target", params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/test)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test/)target", params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test//)target", params ) );
		REQUIRE( params.match() == R"match(/test//)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test)target", params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/TEST)target", params ) );
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/test)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/TEST)target", params ) );
		REQUIRE( params.match() == R"match(/TEST)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test)target", params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/test/)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/TEST)target", params ) );
		REQUIRE( params.match() == R"match(/TEST)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/test)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test/)target", params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/test//)target", params ) );
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test)target", params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test/)target", params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test/route)target", params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/route)target", params ) );
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/test)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test/route)target", params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test//)target", params ) );
		REQUIRE( params.match() == R"match(/test//)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test//route)target", params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route)target", params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/route)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route/)target", params ) );
		REQUIRE( params.match() == R"match(/route/)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test)target", params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test/)target", params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test/route)target", params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/test)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test/)target", params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test//)target", params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test/route)target", params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test.json)target", params ) );
		REQUIRE( params.match() == R"match(/test.json)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/test.json.hbs)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test.json/route)target", params ) );
		REQUIRE( params.match() == R"match(/test.json)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route)target", params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route/)target", params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/route)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route/)target", params ) );
		REQUIRE( params.match() == R"match(/route/)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/test/route)target", params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route)target", params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/another)target", params ) );
		REQUIRE( params.match() == R"match(/another)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(another)value" );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/something/else)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route.json)target", params ) );
		REQUIRE( params.match() == R"match(/route.json)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route.json)value" );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/something%2Felse)target", params ) );
		REQUIRE( params.match() == R"match(/something%2Felse)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(something%2Felse)value" );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/something%2Felse%2Fmore)target", params ) );
		REQUIRE( params.match() == R"match(/something%2Felse%2Fmore)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(something%2Felse%2Fmore)value" );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/;,:@&=+$-_.!~*())target", params ) );
		REQUIRE( params.match() == R"match(/;,:@&=+$-_.!~*())match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(;,:@&=+$-_.!~*())value" );

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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route)target", params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/route/)target", params ) );
	}

}
