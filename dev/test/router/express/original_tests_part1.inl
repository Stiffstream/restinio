
	// Test #0
	{
		// "/"
		// null
		// [["/",["/"]],["/route",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/)route",
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
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/route)target", params ) );

	}


	// Test #1
	{
		// "/test"
		// null
		// [["/test",["/test"]],["/route",null],["/test/route",null],["/test/",["/test/"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test)target", params ) );
			REQUIRE( params.match() == R"match(/test)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/route)target", params ) );

		REQUIRE_FALSE( rm.match_route( R"target(/test/route)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test/)target", params ) );
			REQUIRE( params.match() == R"match(/test/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #2
	{
		// "/test/"
		// null
		// [["/test",null],["/test/",["/test/"]],["/test//",["/test//"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test/)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		REQUIRE_FALSE( rm.match_route( R"target(/test)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test/)target", params ) );
			REQUIRE( params.match() == R"match(/test/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test//)target", params ) );
			REQUIRE( params.match() == R"match(/test//)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #3
	{
		// "/test"
		// {"sensitive":true}
		// [["/test",["/test"]],["/TEST",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test)route",
				path2regex::options_t{}.sensitive( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test)target", params ) );
			REQUIRE( params.match() == R"match(/test)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/TEST)target", params ) );

	}


	// Test #4
	{
		// "/TEST"
		// {"sensitive":true}
		// [["/test",null],["/TEST",["/TEST"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/TEST)route",
				path2regex::options_t{}.sensitive( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		REQUIRE_FALSE( rm.match_route( R"target(/test)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/TEST)target", params ) );
			REQUIRE( params.match() == R"match(/TEST)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #5
	{
		// "/test"
		// {"strict":true}
		// [["/test",["/test"]],["/test/",null],["/TEST",["/TEST"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test)route",
				path2regex::options_t{}.strict( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test)target", params ) );
			REQUIRE( params.match() == R"match(/test)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/test/)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/TEST)target", params ) );
			REQUIRE( params.match() == R"match(/TEST)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #6
	{
		// "/test/"
		// {"strict":true}
		// [["/test",null],["/test/",["/test/"]],["/test//",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test/)route",
				path2regex::options_t{}.strict( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		REQUIRE_FALSE( rm.match_route( R"target(/test)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test/)target", params ) );
			REQUIRE( params.match() == R"match(/test/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/test//)target", params ) );

	}


	// Test #7
	{
		// "/test"
		// {"end":false}
		// [["/test",["/test"]],["/test/",["/test/"]],["/test/route",["/test"]],["/route",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test)target", params ) );
			REQUIRE( params.match() == R"match(/test)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test/)target", params ) );
			REQUIRE( params.match() == R"match(/test/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test/route)target", params ) );
			REQUIRE( params.match() == R"match(/test)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/route)target", params ) );

	}


	// Test #8
	{
		// "/test/"
		// {"end":false}
		// [["/test",null],["/test/route",["/test/"]],["/test//",["/test//"]],["/test//route",["/test/"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test/)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		REQUIRE_FALSE( rm.match_route( R"target(/test)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test/route)target", params ) );
			REQUIRE( params.match() == R"match(/test/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test//)target", params ) );
			REQUIRE( params.match() == R"match(/test//)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test//route)target", params ) );
			REQUIRE( params.match() == R"match(/test/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #9
	{
		// "/:test"
		// {"end":false}
		// [["/route",["/route","route"]]]
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

			REQUIRE( rm.match_route( R"target(/route)target", params ) );
			REQUIRE( params.match() == R"match(/route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #10
	{
		// "/:test/"
		// {"end":false}
		// [["/route",null],["/route/",["/route/","route"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test/)route",
				path2regex::options_t{}.ending( false ) );

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

	}


	// Test #11
	{
		// "/test"
		// {"end":false,"strict":true}
		// [["/test",["/test"]],["/test/",["/test"]],["/test/route",["/test"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test)route",
				path2regex::options_t{}.strict( true ).ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test)target", params ) );
			REQUIRE( params.match() == R"match(/test)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test/)target", params ) );
			REQUIRE( params.match() == R"match(/test)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test/route)target", params ) );
			REQUIRE( params.match() == R"match(/test)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #12
	{
		// "/test/"
		// {"end":false,"strict":true}
		// [["/test",null],["/test/",["/test/"]],["/test//",["/test/"]],["/test/route",["/test/"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test/)route",
				path2regex::options_t{}.strict( true ).ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		REQUIRE_FALSE( rm.match_route( R"target(/test)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test/)target", params ) );
			REQUIRE( params.match() == R"match(/test/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test//)target", params ) );
			REQUIRE( params.match() == R"match(/test/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test/route)target", params ) );
			REQUIRE( params.match() == R"match(/test/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #13
	{
		// "/test.json"
		// {"end":false,"strict":true}
		// [["/test.json",["/test.json"]],["/test.json.hbs",null],["/test.json/route",["/test.json"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test.json)route",
				path2regex::options_t{}.strict( true ).ending( false ) );

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

		REQUIRE_FALSE( rm.match_route( R"target(/test.json.hbs)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test.json/route)target", params ) );
			REQUIRE( params.match() == R"match(/test.json)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #14
	{
		// "/:test"
		// {"end":false,"strict":true}
		// [["/route",["/route","route"]],["/route/",["/route","route"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test)route",
				path2regex::options_t{}.strict( true ).ending( false ) );

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

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route/)target", params ) );
			REQUIRE( params.match() == R"match(/route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #15
	{
		// "/:test/"
		// {"end":false,"strict":true}
		// [["/route",null],["/route/",["/route/","route"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test/)route",
				path2regex::options_t{}.strict( true ).ending( false ) );

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

	}



	// Test #17
	{
		// "/test"
		// {"end":false}
		// [["/test/route",["/test"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test/route)target", params ) );
			REQUIRE( params.match() == R"match(/test)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #18
	{
		// "/:test"
		// null
		// [["/route",["/route","route"]],["/another",["/another","another"]],["/something/else",null],["/route.json",["/route.json","route.json"]],["/something%2Felse",["/something%2Felse","something%2Felse"]],["/something%2Felse%2Fmore",["/something%2Felse%2Fmore","something%2Felse%2Fmore"]],["/;,:@&=+$-_.!~*()",["/;,:@&=+$-_.!~*()",";,:@&=+$-_.!~*()"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test)route",
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

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/another)target", params ) );
			REQUIRE( params.match() == R"match(/another)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(another)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/something/else)target", params ) );

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

			REQUIRE( rm.match_route( R"target(/something%2Felse)target", params ) );
			REQUIRE( params.match() == R"match(/something%2Felse)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(something%2Felse)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/something%2Felse%2Fmore)target", params ) );
			REQUIRE( params.match() == R"match(/something%2Felse%2Fmore)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(something%2Felse%2Fmore)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/;,:@&=+$-_.!~*())target", params ) );
			REQUIRE( params.match() == R"match(/;,:@&=+$-_.!~*())match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "test" ) > 0 );
			REQUIRE( nps.at( "test" ) == R"value(;,:@&=+$-_.!~*())value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #19
	{
		// "/:test"
		// {"strict":true}
		// [["/route",["/route","route"]],["/route/",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:test)route",
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

		REQUIRE_FALSE( rm.match_route( R"target(/route/)target", params ) );

	}
