
	// Test #40
	{
		// ":test"
		// null
		// [["route",["route","route"]],["/route",null],["route/",["route/","route"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(:test)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(route)target", params ) );
			REQUIRE( params.match() == R"match(route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/route)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(route/)target", params ) );
			REQUIRE( params.match() == R"match(route/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #41
	{
		// ":test"
		// {"strict":true}
		// [["route",["route","route"]],["/route",null],["route/",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(:test)route",
				path2regex::options_t{}.strict( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(route)target", params ) );
			REQUIRE( params.match() == R"match(route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/route)target", params ) );

		REQUIRE_FALSE( rm.match_route( R"target(route/)target", params ) );

	}


	// Test #42
	{
		// ":test"
		// {"end":false}
		// [["route",["route","route"]],["/route",null],["route/",["route/","route"]],["route/foobar",["route","route"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(:test)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(route)target", params ) );
			REQUIRE( params.match() == R"match(route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/route)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(route/)target", params ) );
			REQUIRE( params.match() == R"match(route/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(route/foobar)target", params ) );
			REQUIRE( params.match() == R"match(route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #43
	{
		// ":test?"
		// null
		// [["route",["route","route"]],["/route",null],["",["",null]],["route/foobar",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(:test?)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(route)target", params ) );
			REQUIRE( params.match() == R"match(route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/route)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target()target", params ) );
			REQUIRE( params.match() == R"match()match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value()value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(route/foobar)target", params ) );

	}


	// Test #44
	{
		// "/test.json"
		// null
		// [["/test.json",["/test.json"]],["/route.json",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test.json)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test.json)target", params ) );
			REQUIRE( params.match() == R"match(/test.json)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/route.json)target", params ) );

	}


	// Test #45
	{
		// "/:test.json"
		// null
		// [["/.json",null],["/test.json",["/test.json","test"]],["/route.json",["/route.json","route"]],["/route.json.json",["/route.json.json","route.json"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test.json)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		REQUIRE_FALSE( rm.match_route( R"target(/.json)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test.json)target", params ) );
			REQUIRE( params.match() == R"match(/test.json)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(test)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route.json)target", params ) );
			REQUIRE( params.match() == R"match(/route.json)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route.json.json)target", params ) );
			REQUIRE( params.match() == R"match(/route.json.json)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route.json)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #46
	{
		// "/test.:format"
		// null
		// [["/test.html",["/test.html","html"]],["/test.hbs.html",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test.:format)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test.html)target", params ) );
			REQUIRE( params.match() == R"match(/test.html)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "format" ) > 0 );
			REQUIRE( nps.at( "format" ) == R"value(html)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/test.hbs.html)target", params ) );

	}


	// Test #47
	{
		// "/test.:format+"
		// null
		// [["/test.html",["/test.html","html"]],["/test.hbs.html",["/test.hbs.html","hbs.html"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test.:format+)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test.html)target", params ) );
			REQUIRE( params.match() == R"match(/test.html)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "format" ) > 0 );
			REQUIRE( nps.at( "format" ) == R"value(html)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test.hbs.html)target", params ) );
			REQUIRE( params.match() == R"match(/test.hbs.html)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "format" ) > 0 );
			REQUIRE( nps.at( "format" ) == R"value(hbs.html)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #48
	{
		// "/test.:format"
		// {"end":false}
		// [["/test.html",["/test.html","html"]],["/test.hbs.html",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test.:format)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test.html)target", params ) );
			REQUIRE( params.match() == R"match(/test.html)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "format" ) > 0 );
			REQUIRE( nps.at( "format" ) == R"value(html)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/test.hbs.html)target", params ) );

	}


	// Test #49
	{
		// "/test.:format."
		// null
		// [["/test.html.",["/test.html.","html"]],["/test.hbs.html",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test.:format.)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test.html.)target", params ) );
			REQUIRE( params.match() == R"match(/test.html.)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "format" ) > 0 );
			REQUIRE( nps.at( "format" ) == R"value(html)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/test.hbs.html)target", params ) );

	}


	// Test #50
	{
		// "/:test.:format"
		// null
		// [["/route.html",["/route.html","route","html"]],["/route",null],["/route.html.json",["/route.html.json","route.html","json"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test.:format)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route.html)target", params ) );
			REQUIRE( params.match() == R"match(/route.html)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			REQUIRE( nps.count( "format" ) > 0 );
			REQUIRE( nps.at( "format" ) == R"value(html)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/route)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route.html.json)target", params ) );
			REQUIRE( params.match() == R"match(/route.html.json)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route.html)value" );

			REQUIRE( nps.count( "format" ) > 0 );
			REQUIRE( nps.at( "format" ) == R"value(json)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #51
	{
		// "/:test.:format?"
		// null
		// [["/route",["/route","route",null]],["/route.json",["/route.json","route","json"]],["/route.json.html",["/route.json.html","route.json","html"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test.:format?)route",
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
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			REQUIRE( nps.count( "format" ) > 0 );
			REQUIRE( nps.at( "format" ) == R"value()value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route.json)target", params ) );
			REQUIRE( params.match() == R"match(/route.json)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			REQUIRE( nps.count( "format" ) > 0 );
			REQUIRE( nps.at( "format" ) == R"value(json)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route.json.html)target", params ) );
			REQUIRE( params.match() == R"match(/route.json.html)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route.json)value" );

			REQUIRE( nps.count( "format" ) > 0 );
			REQUIRE( nps.at( "format" ) == R"value(html)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #52
	{
		// "/:test.:format?"
		// {"end":false}
		// [["/route",["/route","route",null]],["/route.json",["/route.json","route","json"]],["/route.json.html",["/route.json.html","route.json","html"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test.:format?)route",
				path2regex::options_t{}.ending( false ) );

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
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			REQUIRE( nps.count( "format" ) > 0 );
			REQUIRE( nps.at( "format" ) == R"value()value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route.json)target", params ) );
			REQUIRE( params.match() == R"match(/route.json)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			REQUIRE( nps.count( "format" ) > 0 );
			REQUIRE( nps.at( "format" ) == R"value(json)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route.json.html)target", params ) );
			REQUIRE( params.match() == R"match(/route.json.html)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route.json)value" );

			REQUIRE( nps.count( "format" ) > 0 );
			REQUIRE( nps.at( "format" ) == R"value(html)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #53
	{
		// "/test.:format(.*)z"
		// {"end":false}
		// [["/test.abc",null],["/test.z",["/test.z",""]],["/test.abcz",["/test.abcz","abc"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test.:format(.*)z)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		REQUIRE_FALSE( rm.match_route( R"target(/test.abc)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test.z)target", params ) );
			REQUIRE( params.match() == R"match(/test.z)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "format" ) > 0 );
			REQUIRE( nps.at( "format" ) == R"value()value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test.abcz)target", params ) );
			REQUIRE( params.match() == R"match(/test.abcz)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "format" ) > 0 );
			REQUIRE( nps.at( "format" ) == R"value(abc)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #54
	{
		// "/(\\d+)"
		// null
		// [["/123",["/123","123"]],["/abc",null],["/123/abc",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/(\d+))route",
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
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/abc)target", params ) );

		REQUIRE_FALSE( rm.match_route( R"target(/123/abc)target", params ) );

	}


	// Test #55
	{
		// "/(\\d+)"
		// {"end":false}
		// [["/123",["/123","123"]],["/abc",null],["/123/abc",["/123","123"]],["/123/",["/123/","123"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/(\d+))route",
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
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/abc)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/123/abc)target", params ) );
			REQUIRE( params.match() == R"match(/123)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/123/)target", params ) );
			REQUIRE( params.match() == R"match(/123/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
		}

	}


	// Test #56
	{
		// "/(\\d+)?"
		// null
		// [["/",["/",null]],["/123",["/123","123"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/(\d+)?)route",
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
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/123)target", params ) );
			REQUIRE( params.match() == R"match(/123)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
		}

	}


	// Test #57
	{
		// "/(.*)"
		// null
		// [["/",["/",""]],["/route",["/route","route"]],["/route/nested",["/route/nested","route/nested"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/(.*))route",
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
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route)target", params ) );
			REQUIRE( params.match() == R"match(/route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route/nested)target", params ) );
			REQUIRE( params.match() == R"match(/route/nested)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
		}

	}


	// Test #58
	{
		// "/route\\(\\\\(\\d+\\\\)\\)"
		// null
		// [["/route(\\123\\)",["/route(\\123\\)","123\\"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/route\(\\(\d+\\)\))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route(\123\))target", params ) );
			REQUIRE( params.match() == R"match(/route(\123\))match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
		}

	}


