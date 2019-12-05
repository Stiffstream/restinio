
// ":test"
// null
// [["route",["route","route"]],["/route",null],["route/",["route/","route"]]]
TEST_CASE( "Original tests #40", "[path2regex][original][generated][n40]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(:test)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(route)match" );

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

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(route/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(route/)match" );

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

// ":test"
// {"strict":true}
// [["route",["route","route"]],["/route",null],["route/",null]]
TEST_CASE( "Original tests #41", "[path2regex][original][generated][n41]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(:test)route",
			path2regex::options_t{}.strict( true ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(route)match" );

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

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(route/)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// ":test"
// {"end":false}
// [["route",["route","route"]],["/route",null],["route/",["route/","route"]],["route/foobar",["route","route"]]]
TEST_CASE( "Original tests #42", "[path2regex][original][generated][n42]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(:test)route",
			path2regex::options_t{}.ending( false ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(route)match" );

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

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(route/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(route/)match" );

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

		restinio::router::impl::target_path_holder_t target_path{ R"target(route/foobar)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(route)match" );

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

// ":test?"
// null
// [["route",["route","route"]],["/route",null],["",["",null]],["route/foobar",null]]
TEST_CASE( "Original tests #43", "[path2regex][original][generated][n43]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(:test?)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(route)match" );

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

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target()target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match()match" );

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

		restinio::router::impl::target_path_holder_t target_path{ R"target(route/foobar)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/test.json"
// null
// [["/test.json",["/test.json"]],["/route.json",null]]
TEST_CASE( "Original tests #44", "[path2regex][original][generated][n44]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test.json)route",
			path2regex::options_t{} );

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

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route.json)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/:test.json"
// null
// [["/.json",null],["/test.json",["/test.json","test"]],["/route.json",["/route.json","route"]],["/route.json.json",["/route.json.json","route.json"]]]
TEST_CASE( "Original tests #45", "[path2regex][original][generated][n45]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test.json)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/.json)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test.json)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test.json)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(test)value" );

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
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route.json.json)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route.json.json)match" );

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

}

// "/test.:format"
// null
// [["/test.html",["/test.html","html"]],["/test.hbs.html",null]]
TEST_CASE( "Original tests #46", "[path2regex][original][generated][n46]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test.:format)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test.html)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test.html)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(format)key" ) );
		REQUIRE( nps[0].first == R"key(format)key" );
		REQUIRE( nps[0].second == R"value(html)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test.hbs.html)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/test.:format+"
// null
// [["/test.html",["/test.html","html"]],["/test.hbs.html",["/test.hbs.html","hbs.html"]]]
TEST_CASE( "Original tests #47", "[path2regex][original][generated][n47]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test.:format+)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test.html)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test.html)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(format)key" ) );
		REQUIRE( nps[0].first == R"key(format)key" );
		REQUIRE( nps[0].second == R"value(html)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test.hbs.html)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test.hbs.html)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(format)key" ) );
		REQUIRE( nps[0].first == R"key(format)key" );
		REQUIRE( nps[0].second == R"value(hbs.html)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/test.:format"
// {"end":false}
// [["/test.html",["/test.html","html"]],["/test.hbs.html",null]]
TEST_CASE( "Original tests #48", "[path2regex][original][generated][n48]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test.:format)route",
			path2regex::options_t{}.ending( false ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test.html)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test.html)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(format)key" ) );
		REQUIRE( nps[0].first == R"key(format)key" );
		REQUIRE( nps[0].second == R"value(html)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test.hbs.html)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/test.:format."
// null
// [["/test.html.",["/test.html.","html"]],["/test.hbs.html",null]]
TEST_CASE( "Original tests #49", "[path2regex][original][generated][n49]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test.:format.)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test.html.)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test.html.)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(format)key" ) );
		REQUIRE( nps[0].first == R"key(format)key" );
		REQUIRE( nps[0].second == R"value(html)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test.hbs.html)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/:test.:format"
// null
// [["/route.html",["/route.html","route","html"]],["/route",null],["/route.html.json",["/route.html.json","route.html","json"]]]
TEST_CASE( "Original tests #50", "[path2regex][original][generated][n50]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test.:format)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route.html)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route.html)match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );
		REQUIRE( params.has( R"key(format)key" ) );
		REQUIRE( nps[1].first == R"key(format)key" );
		REQUIRE( nps[1].second == R"value(html)value" );

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

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route.html.json)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route.html.json)match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route.html)value" );
		REQUIRE( params.has( R"key(format)key" ) );
		REQUIRE( nps[1].first == R"key(format)key" );
		REQUIRE( nps[1].second == R"value(json)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:test.:format?"
// null
// [["/route",["/route","route",null]],["/route.json",["/route.json","route","json"]],["/route.json.html",["/route.json.html","route.json","html"]]]
TEST_CASE( "Original tests #51", "[path2regex][original][generated][n51]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test.:format?)route",
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

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );
		REQUIRE( params.has( R"key(format)key" ) );
		REQUIRE( nps[1].first == R"key(format)key" );
		REQUIRE( nps[1].second == R"value()value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route.json)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route.json)match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );
		REQUIRE( params.has( R"key(format)key" ) );
		REQUIRE( nps[1].first == R"key(format)key" );
		REQUIRE( nps[1].second == R"value(json)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route.json.html)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route.json.html)match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route.json)value" );
		REQUIRE( params.has( R"key(format)key" ) );
		REQUIRE( nps[1].first == R"key(format)key" );
		REQUIRE( nps[1].second == R"value(html)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:test.:format?"
// {"end":false}
// [["/route",["/route","route",null]],["/route.json",["/route.json","route","json"]],["/route.json.html",["/route.json.html","route.json","html"]]]
TEST_CASE( "Original tests #52", "[path2regex][original][generated][n52]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:test.:format?)route",
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

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );
		REQUIRE( params.has( R"key(format)key" ) );
		REQUIRE( nps[1].first == R"key(format)key" );
		REQUIRE( nps[1].second == R"value()value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route.json)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route.json)match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );
		REQUIRE( params.has( R"key(format)key" ) );
		REQUIRE( nps[1].first == R"key(format)key" );
		REQUIRE( nps[1].second == R"value(json)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route.json.html)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route.json.html)match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(test)key" ) );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route.json)value" );
		REQUIRE( params.has( R"key(format)key" ) );
		REQUIRE( nps[1].first == R"key(format)key" );
		REQUIRE( nps[1].second == R"value(html)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/test.:format(.*)z"
// {"end":false}
// [["/test.abc",null],["/test.z",["/test.z",""]],["/test.abcz",["/test.abcz","abc"]]]
TEST_CASE( "Original tests #53", "[path2regex][original][generated][n53]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test.:format(.*)z)route",
			path2regex::options_t{}.ending( false ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test.abc)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test.z)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test.z)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(format)key" ) );
		REQUIRE( nps[0].first == R"key(format)key" );
		REQUIRE( nps[0].second == R"value()value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test.abcz)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test.abcz)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(format)key" ) );
		REQUIRE( nps[0].first == R"key(format)key" );
		REQUIRE( nps[0].second == R"value(abc)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/(\\d+)"
// null
// [["/123",["/123","123"]],["/abc",null],["/123/abc",null]]
TEST_CASE( "Original tests #54", "[path2regex][original][generated][n54]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/(\d+))route",
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

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 1 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( 1 == ips.size() );
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

// "/(\\d+)"
// {"end":false}
// [["/123",["/123","123"]],["/abc",null],["/123/abc",["/123","123"]],["/123/",["/123/","123"]]]
TEST_CASE( "Original tests #55", "[path2regex][original][generated][n55]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/(\d+))route",
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

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 1 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( 1 == ips.size() );
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

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 1 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( 1 == ips.size() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/123/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/123/)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 1 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( 1 == ips.size() );
	}

}

// "/(\\d+)?"
// null
// [["/",["/",null]],["/123",["/123","123"]]]
TEST_CASE( "Original tests #56", "[path2regex][original][generated][n56]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/(\d+)?)route",
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

		REQUIRE( 1 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( 1 == ips.size() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/123)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/123)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 1 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( 1 == ips.size() );
	}

}

// "/(.*)"
// null
// [["/",["/",""]],["/route",["/route","route"]],["/route/nested",["/route/nested","route/nested"]]]
TEST_CASE( "Original tests #57", "[path2regex][original][generated][n57]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/(.*))route",
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

		REQUIRE( 1 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( 1 == ips.size() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 1 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( 1 == ips.size() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route/nested)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route/nested)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 1 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( 1 == ips.size() );
	}

}

// "/route\\(\\\\(\\d+\\\\)\\)"
// null
// [["/route(\\123\\)",["/route(\\123\\)","123\\"]]]
TEST_CASE( "Original tests #58", "[path2regex][original][generated][n58]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/route\(\\(\d+\\)\))route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route(\123\))target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route(\123\))match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 1 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( 1 == ips.size() );
	}

}

// Test #59 skipped
// "/.*/"
// null
// [["/match/anything",["/match/anything"]]]
