

	// Test #20
	{
		// "/:test/"
		// {"strict":true}
		// [["/route/",["/route/","route"]],["/route//",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test/)route",
				path2regex::options_t{}.strict( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route/)target", params ) );
			REQUIRE( params.match() == R"match(/route/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/route//)target", params ) );

	}


	// Test #21
	{
		// "/:test"
		// {"end":false}
		// [["/route.json",["/route.json","route.json"]],["/route//",["/route","route"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route.json)target", params ) );
			REQUIRE( params.match() == R"match(/route.json)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route.json)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route//)target", params ) );
			REQUIRE( params.match() == R"match(/route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #22
	{
		// "/:test?"
		// null
		// [["/route",["/route","route"]],["/route/nested",null],["/",["/",null]],["//",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test?)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route)target", params ) );
			REQUIRE( params.match() == R"match(/route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/route/nested)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/)target", params ) );
			REQUIRE( params.match() == R"match(/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value()value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(//)target", params ) );

	}


	// Test #23
	{
		// "/:test?"
		// {"strict":true}
		// [["/route",["/route","route"]],["/",null],["//",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test?)route",
				path2regex::options_t{}.strict( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route)target", params ) );
			REQUIRE( params.match() == R"match(/route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/)target", params ) );

		REQUIRE_FALSE( rm.match_route( R"target(//)target", params ) );

	}


	// Test #24
	{
		// "/:test?/"
		// {"strict":true}
		// [["/route",null],["/route/",["/route/","route"]],["/",["/",null]],["//",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test?/)route",
				path2regex::options_t{}.strict( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		REQUIRE_FALSE( rm.match_route( R"target(/route)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route/)target", params ) );
			REQUIRE( params.match() == R"match(/route/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/)target", params ) );
			REQUIRE( params.match() == R"match(/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value()value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(//)target", params ) );

	}


	// Test #25
	{
		// "/:test?/bar"
		// null
		// [["/foo/bar",["/foo/bar","foo"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test?/bar)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/foo/bar)target", params ) );
			REQUIRE( params.match() == R"match(/foo/bar)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(foo)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #26
	{
		// "/:test?-bar"
		// null
		// [["/-bar",["/-bar",null]],["/foo-bar",["/foo-bar","foo"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test?-bar)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/-bar)target", params ) );
			REQUIRE( params.match() == R"match(/-bar)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value()value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/foo-bar)target", params ) );
			REQUIRE( params.match() == R"match(/foo-bar)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(foo)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #27
	{
		// "/:test*-bar"
		// null
		// [["/-bar",["/-bar",null]],["/foo-bar",["/foo-bar","foo"]],["/foo/baz-bar",["/foo/baz-bar","foo/baz"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test*-bar)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/-bar)target", params ) );
			REQUIRE( params.match() == R"match(/-bar)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value()value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/foo-bar)target", params ) );
			REQUIRE( params.match() == R"match(/foo-bar)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(foo)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/foo/baz-bar)target", params ) );
			REQUIRE( params.match() == R"match(/foo/baz-bar)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(foo/baz)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #28
	{
		// "/:test+"
		// null
		// [["/",null],["/route",["/route","route"]],["/some/basic/route",["/some/basic/route","some/basic/route"]],["//",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test+)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		REQUIRE_FALSE( rm.match_route( R"target(/)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route)target", params ) );
			REQUIRE( params.match() == R"match(/route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/some/basic/route)target", params ) );
			REQUIRE( params.match() == R"match(/some/basic/route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(some/basic/route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(//)target", params ) );

	}


	// Test #29
	{
		// "/:test(\\d+)+"
		// null
		// [["/abc/456/789",null],["/123/456/789",["/123/456/789","123/456/789"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test(\d+)+)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		REQUIRE_FALSE( rm.match_route( R"target(/abc/456/789)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/123/456/789)target", params ) );
			REQUIRE( params.match() == R"match(/123/456/789)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(123/456/789)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #30
	{
		// "/route.:ext(json|xml)+"
		// null
		// [["/route",null],["/route.json",["/route.json","json"]],["/route.xml.json",["/route.xml.json","xml.json"]],["/route.html",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/route.:ext(json|xml)+)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		REQUIRE_FALSE( rm.match_route( R"target(/route)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route.json)target", params ) );
			REQUIRE( params.match() == R"match(/route.json)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "ext" ) > 0 );
			REQUIRE( nps.at( "ext" ) == R"value(json)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route.xml.json)target", params ) );
			REQUIRE( params.match() == R"match(/route.xml.json)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "ext" ) > 0 );
			REQUIRE( nps.at( "ext" ) == R"value(xml.json)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/route.html)target", params ) );

	}


	// Test #31
	{
		// "/:test*"
		// null
		// [["/",["/",null]],["//",null],["/route",["/route","route"]],["/some/basic/route",["/some/basic/route","some/basic/route"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test*)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/)target", params ) );
			REQUIRE( params.match() == R"match(/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value()value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(//)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route)target", params ) );
			REQUIRE( params.match() == R"match(/route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/some/basic/route)target", params ) );
			REQUIRE( params.match() == R"match(/some/basic/route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(some/basic/route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #32
	{
		// "/route.:ext([a-z]+)*"
		// null
		// [["/route",["/route",null]],["/route.json",["/route.json","json"]],["/route.json.xml",["/route.json.xml","json.xml"]],["/route.123",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/route.:ext([a-z]+)*)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route)target", params ) );
			REQUIRE( params.match() == R"match(/route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "ext" ) > 0 );
			REQUIRE( nps.at( "ext" ) == R"value()value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route.json)target", params ) );
			REQUIRE( params.match() == R"match(/route.json)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "ext" ) > 0 );
			REQUIRE( nps.at( "ext" ) == R"value(json)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route.json.xml)target", params ) );
			REQUIRE( params.match() == R"match(/route.json.xml)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "ext" ) > 0 );
			REQUIRE( nps.at( "ext" ) == R"value(json.xml)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/route.123)target", params ) );

	}


	// Test #33
	{
		// "/:test(\\d+)"
		// null
		// [["/123",["/123","123"]],["/abc",null],["/123/abc",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test(\d+))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/123)target", params ) );
			REQUIRE( params.match() == R"match(/123)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(123)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/abc)target", params ) );

		REQUIRE_FALSE( rm.match_route( R"target(/123/abc)target", params ) );

	}


	// Test #34
	{
		// "/:test(\\d+)"
		// {"end":false}
		// [["/123",["/123","123"]],["/abc",null],["/123/abc",["/123","123"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test(\d+))route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/123)target", params ) );
			REQUIRE( params.match() == R"match(/123)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(123)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/abc)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/123/abc)target", params ) );
			REQUIRE( params.match() == R"match(/123)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(123)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #35
	{
		// "/:test(.*)"
		// null
		// [["/anything/goes/here",["/anything/goes/here","anything/goes/here"]],["/;,:@&=/+$-_.!/~*()",["/;,:@&=/+$-_.!/~*()",";,:@&=/+$-_.!/~*()"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test(.*))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/anything/goes/here)target", params ) );
			REQUIRE( params.match() == R"match(/anything/goes/here)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(anything/goes/here)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/;,:@&=/+$-_.!/~*())target", params ) );
			REQUIRE( params.match() == R"match(/;,:@&=/+$-_.!/~*())match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(;,:@&=/+$-_.!/~*())value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #36
	{
		// "/:route([a-z]+)"
		// null
		// [["/abcde",["/abcde","abcde"]],["/12345",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:route([a-z]+))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/abcde)target", params ) );
			REQUIRE( params.match() == R"match(/abcde)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "route" ) > 0 );
			REQUIRE( nps.at( "route" ) == R"value(abcde)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/12345)target", params ) );

	}


	// Test #37
	{
		// "/:route(this|that)"
		// null
		// [["/this",["/this","this"]],["/that",["/that","that"]],["/foo",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:route(this|that))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/this)target", params ) );
			REQUIRE( params.match() == R"match(/this)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "route" ) > 0 );
			REQUIRE( nps.at( "route" ) == R"value(this)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/that)target", params ) );
			REQUIRE( params.match() == R"match(/that)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "route" ) > 0 );
			REQUIRE( nps.at( "route" ) == R"value(that)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/foo)target", params ) );

	}


	// Test #38
	{
		// "/:path(abc|xyz)*"
		// null
		// [["/abc",["/abc","abc"]],["/abc/abc",["/abc/abc","abc/abc"]],["/xyz/xyz",["/xyz/xyz","xyz/xyz"]],["/abc/xyz",["/abc/xyz","abc/xyz"]],["/abc/xyz/abc/xyz",["/abc/xyz/abc/xyz","abc/xyz/abc/xyz"]],["/xyzxyz",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:path(abc|xyz)*)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/abc)target", params ) );
			REQUIRE( params.match() == R"match(/abc)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "path" ) > 0 );
			REQUIRE( nps.at( "path" ) == R"value(abc)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/abc/abc)target", params ) );
			REQUIRE( params.match() == R"match(/abc/abc)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "path" ) > 0 );
			REQUIRE( nps.at( "path" ) == R"value(abc/abc)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/xyz/xyz)target", params ) );
			REQUIRE( params.match() == R"match(/xyz/xyz)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "path" ) > 0 );
			REQUIRE( nps.at( "path" ) == R"value(xyz/xyz)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/abc/xyz)target", params ) );
			REQUIRE( params.match() == R"match(/abc/xyz)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "path" ) > 0 );
			REQUIRE( nps.at( "path" ) == R"value(abc/xyz)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/abc/xyz/abc/xyz)target", params ) );
			REQUIRE( params.match() == R"match(/abc/xyz/abc/xyz)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "path" ) > 0 );
			REQUIRE( nps.at( "path" ) == R"value(abc/xyz/abc/xyz)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/xyzxyz)target", params ) );

	}


	// Test #39
	{
		// "test"
		// null
		// [["test",["test"]],["/test",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(test)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(test)target", params ) );
			REQUIRE( params.match() == R"match(test)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/test)target", params ) );

	}

