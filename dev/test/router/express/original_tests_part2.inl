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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route/)target", params ) );
		REQUIRE( params.match() == R"match(/route/)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/route//)target", params ) );
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route.json)target", params ) );
		REQUIRE( params.match() == R"match(/route.json)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route.json)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route//)target", params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		const auto & ips = params.indexed_parameters();
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route)target", params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/route/nested)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/)target", params ) );
		REQUIRE( params.match() == R"match(/)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value()value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(//)target", params ) );
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route)target", params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(//)target", params ) );
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

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/)target", params ) );
		REQUIRE( params.match() == R"match(/)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value()value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(//)target", params ) );
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/foo/bar)target", params ) );
		REQUIRE( params.match() == R"match(/foo/bar)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(foo)value" );

		const auto & ips = params.indexed_parameters();
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/-bar)target", params ) );
		REQUIRE( params.match() == R"match(/-bar)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value()value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/foo-bar)target", params ) );
		REQUIRE( params.match() == R"match(/foo-bar)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(foo)value" );

		const auto & ips = params.indexed_parameters();
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/-bar)target", params ) );
		REQUIRE( params.match() == R"match(/-bar)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value()value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/foo-bar)target", params ) );
		REQUIRE( params.match() == R"match(/foo-bar)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(foo)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/foo/baz-bar)target", params ) );
		REQUIRE( params.match() == R"match(/foo/baz-bar)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(foo/baz)value" );

		const auto & ips = params.indexed_parameters();
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route)target", params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/some/basic/route)target", params ) );
		REQUIRE( params.match() == R"match(/some/basic/route)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(some/basic/route)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(//)target", params ) );
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/abc/456/789)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/123/456/789)target", params ) );
		REQUIRE( params.match() == R"match(/123/456/789)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(123/456/789)value" );

		const auto & ips = params.indexed_parameters();
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

		REQUIRE( rm.match_route( R"target(/route.json)target", params ) );
		REQUIRE( params.match() == R"match(/route.json)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(ext)key" );
		REQUIRE( nps[0].second == R"value(json)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route.xml.json)target", params ) );
		REQUIRE( params.match() == R"match(/route.xml.json)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(ext)key" );
		REQUIRE( nps[0].second == R"value(xml.json)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/route.html)target", params ) );
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/)target", params ) );
		REQUIRE( params.match() == R"match(/)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value()value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(//)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route)target", params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/some/basic/route)target", params ) );
		REQUIRE( params.match() == R"match(/some/basic/route)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(some/basic/route)value" );

		const auto & ips = params.indexed_parameters();
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route)target", params ) );
		REQUIRE( params.match() == R"match(/route)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(ext)key" );
		REQUIRE( nps[0].second == R"value()value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route.json)target", params ) );
		REQUIRE( params.match() == R"match(/route.json)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(ext)key" );
		REQUIRE( nps[0].second == R"value(json)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/route.json.xml)target", params ) );
		REQUIRE( params.match() == R"match(/route.json.xml)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(ext)key" );
		REQUIRE( nps[0].second == R"value(json.xml)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/route.123)target", params ) );
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/123)target", params ) );
		REQUIRE( params.match() == R"match(/123)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(123)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/abc)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/123/abc)target", params ) );
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/123)target", params ) );
		REQUIRE( params.match() == R"match(/123)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(123)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/abc)target", params ) );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/123/abc)target", params ) );
		REQUIRE( params.match() == R"match(/123)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(123)value" );

		const auto & ips = params.indexed_parameters();
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/anything/goes/here)target", params ) );
		REQUIRE( params.match() == R"match(/anything/goes/here)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(anything/goes/here)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/;,:@&=/+$-_.!/~*())target", params ) );
		REQUIRE( params.match() == R"match(/;,:@&=/+$-_.!/~*())match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(test)key" );
		REQUIRE( nps[0].second == R"value(;,:@&=/+$-_.!/~*())value" );

		const auto & ips = params.indexed_parameters();
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/abcde)target", params ) );
		REQUIRE( params.match() == R"match(/abcde)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(route)key" );
		REQUIRE( nps[0].second == R"value(abcde)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/12345)target", params ) );
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/this)target", params ) );
		REQUIRE( params.match() == R"match(/this)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(route)key" );
		REQUIRE( nps[0].second == R"value(this)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/that)target", params ) );
		REQUIRE( params.match() == R"match(/that)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(route)key" );
		REQUIRE( nps[0].second == R"value(that)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/foo)target", params ) );
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/abc)target", params ) );
		REQUIRE( params.match() == R"match(/abc)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(path)key" );
		REQUIRE( nps[0].second == R"value(abc)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/abc/abc)target", params ) );
		REQUIRE( params.match() == R"match(/abc/abc)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(path)key" );
		REQUIRE( nps[0].second == R"value(abc/abc)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/xyz/xyz)target", params ) );
		REQUIRE( params.match() == R"match(/xyz/xyz)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(path)key" );
		REQUIRE( nps[0].second == R"value(xyz/xyz)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/abc/xyz)target", params ) );
		REQUIRE( params.match() == R"match(/abc/xyz)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(path)key" );
		REQUIRE( nps[0].second == R"value(abc/xyz)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(/abc/xyz/abc/xyz)target", params ) );
		REQUIRE( params.match() == R"match(/abc/xyz/abc/xyz)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( 1 == nps.size() );
		REQUIRE( nps[0].first == R"key(path)key" );
		REQUIRE( nps[0].second == R"value(abc/xyz/abc/xyz)value" );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/xyzxyz)target", params ) );
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
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		REQUIRE( rm.match_route( R"target(test)target", params ) );
		REQUIRE( params.match() == R"match(test)match" );

		const auto & nps = params.named_parameters();
		REQUIRE( nps.empty() );

		const auto & ips = params.indexed_parameters();
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		REQUIRE_FALSE( rm.match_route( R"target(/test)target", params ) );
	}

}
