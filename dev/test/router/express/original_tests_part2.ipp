
// "/:test/"
// {"strict":true}
// [["/route/",["/route/","route"]],["/route//",null]]
TEST_CASE( "Original tests #20", "[path2regex][original][generated][n20]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test/)route",
			path2regex::options_t{}.strict( true ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

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

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route//)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/:test"
// {"end":false}
// [["/route.json",["/route.json","route.json"]],["/route//",["/route","route"]]]
TEST_CASE( "Original tests #21", "[path2regex][original][generated][n21]")
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

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route//)target" };
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

// "/:test?"
// null
// [["/route",["/route","route"]],["/route/nested",null],["/",["/",null]],["//",null]]
TEST_CASE( "Original tests #22", "[path2regex][original][generated][n22]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test?)route",
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

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route/nested)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value()value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(//)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/:test?"
// {"strict":true}
// [["/route",["/route","route"]],["/",null],["//",null]]
TEST_CASE( "Original tests #23", "[path2regex][original][generated][n23]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test?)route",
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

		restinio::router::impl::target_path_holder_t target_path{ R"target(/)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(//)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/:test?/"
// {"strict":true}
// [["/route",null],["/route/",["/route/","route"]],["/",["/",null]],["//",null]]
TEST_CASE( "Original tests #24", "[path2regex][original][generated][n24]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test?/)route",
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

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value()value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(//)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/:test?/bar"
// null
// [["/foo/bar",["/foo/bar","foo"]]]
TEST_CASE( "Original tests #25", "[path2regex][original][generated][n25]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test?/bar)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/foo/bar)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/foo/bar)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(foo)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:test?-bar"
// null
// [["/-bar",["/-bar",null]],["/foo-bar",["/foo-bar","foo"]]]
TEST_CASE( "Original tests #26", "[path2regex][original][generated][n26]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test?-bar)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/-bar)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/-bar)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value()value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/foo-bar)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/foo-bar)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(foo)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:test*-bar"
// null
// [["/-bar",["/-bar",null]],["/foo-bar",["/foo-bar","foo"]],["/foo/baz-bar",["/foo/baz-bar","foo/baz"]]]
TEST_CASE( "Original tests #27", "[path2regex][original][generated][n27]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test*-bar)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/-bar)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/-bar)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value()value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/foo-bar)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/foo-bar)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(foo)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/foo/baz-bar)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/foo/baz-bar)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(foo/baz)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:test+"
// null
// [["/",null],["/route",["/route","route"]],["/some/basic/route",["/some/basic/route","some/basic/route"]],["//",null]]
TEST_CASE( "Original tests #28", "[path2regex][original][generated][n28]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test+)route",
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
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

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

		restinio::router::impl::target_path_holder_t target_path{ R"target(/some/basic/route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/some/basic/route)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(some/basic/route)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(//)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/:test(\\d+)+"
// null
// [["/abc/456/789",null],["/123/456/789",["/123/456/789","123/456/789"]]]
TEST_CASE( "Original tests #29", "[path2regex][original][generated][n29]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test(\d+)+)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/abc/456/789)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/123/456/789)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/123/456/789)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(123/456/789)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/route.:ext(json|xml)+"
// null
// [["/route",null],["/route.json",["/route.json","json"]],["/route.xml.json",["/route.xml.json","xml.json"]],["/route.html",null]]
TEST_CASE( "Original tests #30", "[path2regex][original][generated][n30]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/route.:ext(json|xml)+)route",
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
		REQUIRE( params.has( R"key(ext)key" ) );
		REQUIRE( nps[0].first == R"key(ext)key" );
		REQUIRE( nps[0].second == R"value(json)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route.xml.json)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route.xml.json)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(ext)key" ) );
		REQUIRE( nps[0].first == R"key(ext)key" );
		REQUIRE( nps[0].second == R"value(xml.json)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route.html)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/:test*"
// null
// [["/",["/",null]],["//",null],["/route",["/route","route"]],["/some/basic/route",["/some/basic/route","some/basic/route"]]]
TEST_CASE( "Original tests #31", "[path2regex][original][generated][n31]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test*)route",
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

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value()value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(//)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

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

		restinio::router::impl::target_path_holder_t target_path{ R"target(/some/basic/route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/some/basic/route)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(some/basic/route)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/route.:ext([a-z]+)*"
// null
// [["/route",["/route",null]],["/route.json",["/route.json","json"]],["/route.json.xml",["/route.json.xml","json.xml"]],["/route.123",null]]
TEST_CASE( "Original tests #32", "[path2regex][original][generated][n32]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/route.:ext([a-z]+)*)route",
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
		REQUIRE( params.has( R"key(ext)key" ) );
		REQUIRE( nps[0].first == R"key(ext)key" );
		REQUIRE( nps[0].second == R"value()value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route.json)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route.json)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(ext)key" ) );
		REQUIRE( nps[0].first == R"key(ext)key" );
		REQUIRE( nps[0].second == R"value(json)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route.json.xml)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route.json.xml)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(ext)key" ) );
		REQUIRE( nps[0].first == R"key(ext)key" );
		REQUIRE( nps[0].second == R"value(json.xml)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route.123)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/:test(\\d+)"
// null
// [["/123",["/123","123"]],["/abc",null],["/123/abc",null]]
TEST_CASE( "Original tests #33", "[path2regex][original][generated][n33]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test(\d+))route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/123)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/123)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(123)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/abc)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/123/abc)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/:test(\\d+)"
// {"end":false}
// [["/123",["/123","123"]],["/abc",null],["/123/abc",["/123","123"]]]
TEST_CASE( "Original tests #34", "[path2regex][original][generated][n34]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test(\d+))route",
			path2regex::options_t{}.ending( false ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/123)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/123)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(123)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/abc)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/123/abc)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/123)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(123)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:test(.*)"
// null
// [["/anything/goes/here",["/anything/goes/here","anything/goes/here"]],["/;,:@&=/+$-_.!/~*()",["/;,:@&=/+$-_.!/~*()",";,:@&=/+$-_.!/~*()"]]]
TEST_CASE( "Original tests #35", "[path2regex][original][generated][n35]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test(.*))route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/anything/goes/here)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/anything/goes/here)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(anything/goes/here)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/;,:@&=/+$-_.!/~*())target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/;,:@&=/+$-_.!/~*())match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(;,:@&=/+$-_.!/~*())value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:route([a-z]+)"
// null
// [["/abcde",["/abcde","abcde"]],["/12345",null]]
TEST_CASE( "Original tests #36", "[path2regex][original][generated][n36]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:route([a-z]+))route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/abcde)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/abcde)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(route)key" ) );
		REQUIRE( nps[0].first == R"key(route)key" );
		REQUIRE( nps[0].second == R"value(abcde)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/12345)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/:route(this|that)"
// null
// [["/this",["/this","this"]],["/that",["/that","that"]],["/foo",null]]
TEST_CASE( "Original tests #37", "[path2regex][original][generated][n37]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:route(this|that))route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/this)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/this)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(route)key" ) );
		REQUIRE( nps[0].first == R"key(route)key" );
		REQUIRE( nps[0].second == R"value(this)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/that)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/that)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(route)key" ) );
		REQUIRE( nps[0].first == R"key(route)key" );
		REQUIRE( nps[0].second == R"value(that)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/foo)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/:path(abc|xyz)*"
// null
// [["/abc",["/abc","abc"]],["/abc/abc",["/abc/abc","abc/abc"]],["/xyz/xyz",["/xyz/xyz","xyz/xyz"]],["/abc/xyz",["/abc/xyz","abc/xyz"]],["/abc/xyz/abc/xyz",["/abc/xyz/abc/xyz","abc/xyz/abc/xyz"]],["/xyzxyz",null]]
TEST_CASE( "Original tests #38", "[path2regex][original][generated][n38]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:path(abc|xyz)*)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/abc)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/abc)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(path)key" ) );
		REQUIRE( nps[0].first == R"key(path)key" );
		REQUIRE( nps[0].second == R"value(abc)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/abc/abc)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/abc/abc)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(path)key" ) );
		REQUIRE( nps[0].first == R"key(path)key" );
		REQUIRE( nps[0].second == R"value(abc/abc)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/xyz/xyz)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/xyz/xyz)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(path)key" ) );
		REQUIRE( nps[0].first == R"key(path)key" );
		REQUIRE( nps[0].second == R"value(xyz/xyz)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/abc/xyz)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/abc/xyz)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(path)key" ) );
		REQUIRE( nps[0].first == R"key(path)key" );
		REQUIRE( nps[0].second == R"value(abc/xyz)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/abc/xyz/abc/xyz)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/abc/xyz/abc/xyz)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(path)key" ) );
		REQUIRE( nps[0].first == R"key(path)key" );
		REQUIRE( nps[0].second == R"value(abc/xyz/abc/xyz)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/xyzxyz)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "test"
// null
// [["test",["test"]],["/test",null]]
TEST_CASE( "Original tests #39", "[path2regex][original][generated][n39]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(test)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(test)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(test)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}
