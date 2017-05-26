
	{
		// "/"
		// null
		// [["/",["/"]],["/route",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/test"
		// null
		// [["/test",["/test"]],["/route",null],["/test/route",null],["/test/",["/test/"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/test/"
		// null
		// [["/test",["/test"]],["/test/",["/test/"]],["/test//",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test/)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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

		REQUIRE_FALSE( rm.match_route( R"target(/test//)target", params ) );

	}


	{
		// "/test"
		// {"sensitive":true}
		// [["/test",["/test"]],["/TEST",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test)route",
				path2regex::options_t{}.sensitive( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/TEST"
		// {"sensitive":true}
		// [["/test",null],["/TEST",["/TEST"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/TEST)route",
				path2regex::options_t{}.sensitive( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/test"
		// {"strict":true}
		// [["/test",["/test"]],["/test/",null],["/TEST",["/TEST"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test)route",
				path2regex::options_t{}.strict( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/test/"
		// {"strict":true}
		// [["/test",null],["/test/",["/test/"]],["/test//",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test/)route",
				path2regex::options_t{}.strict( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/test"
		// {"end":false}
		// [["/test",["/test"]],["/test/",["/test/"]],["/test/route",["/test"]],["/route",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/test/"
		// {"end":false}
		// [["/test/route",["/test"]],["/test//",["/test"]],["/test//route",["/test"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test/)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test//)target", params ) );
			REQUIRE( params.match() == R"match(/test)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test//route)target", params ) );
			REQUIRE( params.match() == R"match(/test)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	{
		// "/:test"
		// {"end":false}
		// [["/route",["/route","route"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test/"
		// {"end":false}
		// [["/route",["/route","route"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test/)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/test"
		// {"end":false,"strict":true}
		// [["/test",["/test"]],["/test/",["/test"]],["/test/route",["/test"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test)route",
				path2regex::options_t{}.strict( true ).ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/test/"
		// {"end":false,"strict":true}
		// [["/test",null],["/test/",["/test/"]],["/test//",["/test/"]],["/test/route",["/test/"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test/)route",
				path2regex::options_t{}.strict( true ).ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/test.json"
		// {"end":false,"strict":true}
		// [["/test.json",["/test.json"]],["/test.json.hbs",null],["/test.json/route",["/test.json"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test.json)route",
				path2regex::options_t{}.strict( true ).ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test"
		// {"end":false,"strict":true}
		// [["/route",["/route","route"]],["/route/",["/route","route"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test)route",
				path2regex::options_t{}.strict( true ).ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test/"
		// {"end":false,"strict":true}
		// [["/route",null],["/route/",["/route/","route"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test/)route",
				path2regex::options_t{}.strict( true ).ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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



	{
		// "/test"
		// {"end":false}
		// [["/test/route",["/test"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test"
		// null
		// [["/route",["/route","route"]],["/another",["/another","another"]],["/something/else",null],["/route.json",["/route.json","route.json"]],["/something%2Felse",["/something%2Felse","something%2Felse"]],["/something%2Felse%2Fmore",["/something%2Felse%2Fmore","something%2Felse%2Fmore"]],["/;,:@&=+$-_.!~*()",["/;,:@&=+$-_.!~*()",";,:@&=+$-_.!~*()"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test"
		// {"strict":true}
		// [["/route",["/route","route"]],["/route/",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test)route",
				path2regex::options_t{}.strict( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test/"
		// {"strict":true}
		// [["/route/",["/route/","route"]],["/route//",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test/)route",
				path2regex::options_t{}.strict( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test"
		// {"end":false}
		// [["/route.json",["/route.json","route.json"]],["/route//",["/route","route"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test?"
		// null
		// [["/route",["/route","route"]],["/route/nested",null],["/",["/",null]],["//",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test?)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test?"
		// {"strict":true}
		// [["/route",["/route","route"]],["/",null],["//",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test?)route",
				path2regex::options_t{}.strict( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test?/"
		// {"strict":true}
		// [["/route",null],["/route/",["/route/","route"]],["/",["/",null]],["//",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test?/)route",
				path2regex::options_t{}.strict( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test?/bar"
		// null
		// [["/foo/bar",["/foo/bar","foo"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test?/bar)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test?-bar"
		// null
		// [["/-bar",["/-bar",null]],["/foo-bar",["/foo-bar","foo"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test?-bar)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test+"
		// null
		// [["/",null],["/route",["/route","route"]],["/some/basic/route",["/some/basic/route","some/basic/route"]],["//",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test+)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test(\\d+)+"
		// null
		// [["/abc/456/789",null],["/123/456/789",["/123/456/789","123/456/789"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test(\d+)+)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/route.:ext(json|xml)+"
		// null
		// [["/route",null],["/route.json",["/route.json","json"]],["/route.xml.json",["/route.xml.json","xml.json"]],["/route.html",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/route.:ext(json|xml)+)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test*"
		// null
		// [["/",["/",null]],["//",null],["/route",["/route","route"]],["/some/basic/route",["/some/basic/route","some/basic/route"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test*)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/route.:ext([a-z]+)*"
		// null
		// [["/route",["/route",null]],["/route.json",["/route.json","json"]],["/route.json.xml",["/route.json.xml","json.xml"]],["/route.123",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/route.:ext([a-z]+)*)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test(\\d+)"
		// null
		// [["/123",["/123","123"]],["/abc",null],["/123/abc",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test(\d+))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test(\\d+)"
		// {"end":false}
		// [["/123",["/123","123"]],["/abc",null],["/123/abc",["/123","123"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test(\d+))route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test(.*)"
		// null
		// [["/anything/goes/here",["/anything/goes/here","anything/goes/here"]],["/;,:@&=/+$-_.!/~*()",["/;,:@&=/+$-_.!/~*()",";,:@&=/+$-_.!/~*()"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test(.*))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:route([a-z]+)"
		// null
		// [["/abcde",["/abcde","abcde"]],["/12345",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:route([a-z]+))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:route(this|that)"
		// null
		// [["/this",["/this","this"]],["/that",["/that","that"]],["/foo",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:route(this|that))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:path(abc|xyz)*"
		// null
		// [["/abc",["/abc","abc"]],["/abc/abc",["/abc/abc","abc/abc"]],["/xyz/xyz",["/xyz/xyz","xyz/xyz"]],["/abc/xyz",["/abc/xyz","abc/xyz"]],["/abc/xyz/abc/xyz",["/abc/xyz/abc/xyz","abc/xyz/abc/xyz"]],["/xyzxyz",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:path(abc|xyz)*)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "test"
		// null
		// [["test",["test"]],["/test",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(test)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// ":test"
		// null
		// [["route",["route","route"]],["/route",null],["route/",["route/","route"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(:test)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// ":test"
		// {"strict":true}
		// [["route",["route","route"]],["/route",null],["route/",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(:test)route",
				path2regex::options_t{}.strict( true ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// ":test"
		// {"end":false}
		// [["route",["route","route"]],["/route",null],["route/",["route/","route"]],["route/foobar",["route","route"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(:test)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// ":test?"
		// null
		// [["route",["route","route"]],["/route",null],["",["",null]],["route/foobar",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(:test?)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/test.json"
		// null
		// [["/test.json",["/test.json"]],["/route.json",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test.json)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test.json"
		// null
		// [["/.json",null],["/test.json",["/test.json","test"]],["/route.json",["/route.json","route"]],["/route.json.json",["/route.json.json","route.json"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test.json)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/test.:format"
		// null
		// [["/test.html",["/test.html","html"]],["/test.hbs.html",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test.:format)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/test.:format+"
		// null
		// [["/test.html",["/test.html","html"]],["/test.hbs.html",["/test.hbs.html","hbs.html"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test.:format+)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/test.:format"
		// {"end":false}
		// [["/test.html",["/test.html","html"]],["/test.hbs.html",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test.:format)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/test.:format."
		// null
		// [["/test.html.",["/test.html.","html"]],["/test.hbs.html",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test.:format.)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test.:format"
		// null
		// [["/route.html",["/route.html","route","html"]],["/route",null],["/route.html.json",["/route.html.json","route.html","json"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test.:format)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test.:format?"
		// null
		// [["/route",["/route","route",null]],["/route.json",["/route.json","route","json"]],["/route.json.html",["/route.json.html","route.json","html"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test.:format?)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/:test.:format?"
		// {"end":false}
		// [["/route",["/route","route",null]],["/route.json",["/route.json","route","json"]],["/route.json.html",["/route.json.html","route.json","html"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:test.:format?)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/test.:format(.*)z"
		// {"end":false}
		// [["/test.abc",null],["/test.z",["/test.z",""]],["/test.abcz",["/test.abcz","abc"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/test.:format(.*)z)route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/(\\d+)"
		// null
		// [["/123",["/123","123"]],["/abc",null],["/123/abc",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/(\d+))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/(\\d+)"
		// {"end":false}
		// [["/123",["/123","123"]],["/abc",null],["/123/abc",["/123","123"]],["/123/",["/123/","123"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/(\d+))route",
				path2regex::options_t{}.ending( false ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/(\\d+)?"
		// null
		// [["/",["/",null]],["/123",["/123","123"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/(\d+)?)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/(.*)"
		// null
		// [["/",["/",""]],["/route",["/route","route"]],["/route/nested",["/route/nested","route/nested"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/(.*))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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


	{
		// "/route\\(\\\\(\\d+\\\\)\\)"
		// null
		// [["/route(\\123\\)",["/route(\\123\\)","123\\"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/route\(\\(\d+\\)\))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

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










	{
		// "/\\(testing\\)"
		// null
		// [["/testing",null],["/(testing)",["/(testing)"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/\(testing\))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		REQUIRE_FALSE( rm.match_route( R"target(/testing)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/(testing))target", params ) );
			REQUIRE( params.match() == R"match(/(testing))match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	{
		// "/.+\\*?=^!:${}[]|"
		// null
		// [["/.+*?=^!:${}[]|",["/.+*?=^!:${}[]|"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/.+\*?=^!:${}[]|)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/.+*?=^!:${}[]|)target", params ) );
			REQUIRE( params.match() == R"match(/.+*?=^!:${}[]|)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	{
		// "/*"
		// null
		// [["",null],["/",["/",""]],["/foo/bar",["/foo/bar","foo/bar"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/*)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		REQUIRE_FALSE( rm.match_route( R"target()target", params ) );

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

			REQUIRE( rm.match_route( R"target(/foo/bar)target", params ) );
			REQUIRE( params.match() == R"match(/foo/bar)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
		}

	}


	{
		// "/foo/*"
		// null
		// [["",null],["/test",null],["/foo",null],["/foo/",["/foo/",""]],["/foo/bar",["/foo/bar","bar"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/foo/*)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		REQUIRE_FALSE( rm.match_route( R"target()target", params ) );

		REQUIRE_FALSE( rm.match_route( R"target(/test)target", params ) );

		REQUIRE_FALSE( rm.match_route( R"target(/foo)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/foo/)target", params ) );
			REQUIRE( params.match() == R"match(/foo/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/foo/bar)target", params ) );
			REQUIRE( params.match() == R"match(/foo/bar)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
		}

	}


	{
		// "/:foo/*"
		// null
		// [["",null],["/test",null],["/foo",null],["/foo/",["/foo/","foo",""]],["/foo/bar",["/foo/bar","foo","bar"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:foo/*)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		REQUIRE_FALSE( rm.match_route( R"target()target", params ) );

		REQUIRE_FALSE( rm.match_route( R"target(/test)target", params ) );

		REQUIRE_FALSE( rm.match_route( R"target(/foo)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/foo/)target", params ) );
			REQUIRE( params.match() == R"match(/foo/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "foo" ) > 0 );
			REQUIRE( nps.at( "foo" ) == R"value(foo)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
			REQUIRE( ips.at( 0 ) == R"value()value" );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/foo/bar)target", params ) );
			REQUIRE( params.match() == R"match(/foo/bar)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "foo" ) > 0 );
			REQUIRE( nps.at( "foo" ) == R"value(foo)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
			REQUIRE( ips.at( 0 ) == R"value(bar)value" );
		}

	}


	{
		// "/(apple-)?icon-:res(\\d+).png"
		// null
		// [["/icon-240.png",["/icon-240.png",null,"240"]],["/apple-icon-240.png",["/apple-icon-240.png","apple-","240"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/(apple-)?icon-:res(\d+).png)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/icon-240.png)target", params ) );
			REQUIRE( params.match() == R"match(/icon-240.png)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "res" ) > 0 );
			REQUIRE( nps.at( "res" ) == R"value(240)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
			REQUIRE( ips.at( 0 ) == R"value()value" );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/apple-icon-240.png)target", params ) );
			REQUIRE( params.match() == R"match(/apple-icon-240.png)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "res" ) > 0 );
			REQUIRE( nps.at( "res" ) == R"value(240)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
			REQUIRE( ips.at( 0 ) == R"value(apple-)value" );
		}

	}


	{
		// "/:foo/:bar"
		// null
		// [["/match/route",["/match/route","match","route"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:foo/:bar)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/match/route)target", params ) );
			REQUIRE( params.match() == R"match(/match/route)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "foo" ) > 0 );
			REQUIRE( nps.at( "foo" ) == R"value(match)value" );

			REQUIRE( nps.count( "bar" ) > 0 );
			REQUIRE( nps.at( "bar" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	{
		// "/:foo(test\\)/bar"
		// null
		// []
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:foo(test\)/bar)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
	}


	{
		// "/:remote([\\w-.]+)/:user([\\w-]+)"
		// null
		// [["/endpoint/user",["/endpoint/user","endpoint","user"]],["/endpoint/user-name",["/endpoint/user-name","endpoint","user-name"]],["/foo.bar/user-name",["/foo.bar/user-name","foo.bar","user-name"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:remote([\w-.]+)/:user([\w-]+))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/endpoint/user)target", params ) );
			REQUIRE( params.match() == R"match(/endpoint/user)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "remote" ) > 0 );
			REQUIRE( nps.at( "remote" ) == R"value(endpoint)value" );

			REQUIRE( nps.count( "user" ) > 0 );
			REQUIRE( nps.at( "user" ) == R"value(user)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/endpoint/user-name)target", params ) );
			REQUIRE( params.match() == R"match(/endpoint/user-name)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "remote" ) > 0 );
			REQUIRE( nps.at( "remote" ) == R"value(endpoint)value" );

			REQUIRE( nps.count( "user" ) > 0 );
			REQUIRE( nps.at( "user" ) == R"value(user-name)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/foo.bar/user-name)target", params ) );
			REQUIRE( params.match() == R"match(/foo.bar/user-name)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "remote" ) > 0 );
			REQUIRE( nps.at( "remote" ) == R"value(foo.bar)value" );

			REQUIRE( nps.count( "user" ) > 0 );
			REQUIRE( nps.at( "user" ) == R"value(user-name)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	{
		// "/:foo\\?"
		// null
		// [["/route?",["/route?","route"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:foo\?)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/route?)target", params ) );
			REQUIRE( params.match() == R"match(/route?)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "foo" ) > 0 );
			REQUIRE( nps.at( "foo" ) == R"value(route)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	{
		// "/:foo+baz"
		// null
		// [["/foobaz",["/foobaz","foo"]],["/foo/barbaz",["/foo/barbaz","foo/bar"]],["/baz",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:foo+baz)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/foobaz)target", params ) );
			REQUIRE( params.match() == R"match(/foobaz)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "foo" ) > 0 );
			REQUIRE( nps.at( "foo" ) == R"value(foo)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/foo/barbaz)target", params ) );
			REQUIRE( params.match() == R"match(/foo/barbaz)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "foo" ) > 0 );
			REQUIRE( nps.at( "foo" ) == R"value(foo/bar)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/baz)target", params ) );

	}


	{
		// "/:pre?baz"
		// null
		// [["/foobaz",["/foobaz","foo"]],["/baz",["/baz",null]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:pre?baz)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/foobaz)target", params ) );
			REQUIRE( params.match() == R"match(/foobaz)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "pre" ) > 0 );
			REQUIRE( nps.at( "pre" ) == R"value(foo)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/baz)target", params ) );
			REQUIRE( params.match() == R"match(/baz)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "pre" ) > 0 );
			REQUIRE( nps.at( "pre" ) == R"value()value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	{
		// "/:foo\\(:bar?\\)"
		// null
		// [["/hello(world)",["/hello(world)","hello","world"]],["/hello()",["/hello()","hello",null]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:foo\(:bar?\))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/hello(world))target", params ) );
			REQUIRE( params.match() == R"match(/hello(world))match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "foo" ) > 0 );
			REQUIRE( nps.at( "foo" ) == R"value(hello)value" );

			REQUIRE( nps.count( "bar" ) > 0 );
			REQUIRE( nps.at( "bar" ) == R"value(world)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/hello())target", params ) );
			REQUIRE( params.match() == R"match(/hello())match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "foo" ) > 0 );
			REQUIRE( nps.at( "foo" ) == R"value(hello)value" );

			REQUIRE( nps.count( "bar" ) > 0 );
			REQUIRE( nps.at( "bar" ) == R"value()value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	{
		// "/:postType(video|audio|text)(\\+.+)?"
		// null
		// [["/video",["/video","video",null]],["/video+test",["/video+test","video","+test"]],["/video+",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:postType(video|audio|text)(\+.+)?)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/video)target", params ) );
			REQUIRE( params.match() == R"match(/video)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "postType" ) > 0 );
			REQUIRE( nps.at( "postType" ) == R"value(video)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
			REQUIRE( ips.at( 0 ) == R"value()value" );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/video+test)target", params ) );
			REQUIRE( params.match() == R"match(/video+test)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "postType" ) > 0 );
			REQUIRE( nps.at( "postType" ) == R"value(video)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( 1 == ips.size() );
			REQUIRE( ips.at( 0 ) == R"value(+test)value" );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/video+)target", params ) );

	}


	{
		// "/:foo"
		// null
		// [["/café",["/café","café"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(/:foo)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/café)target", params ) );
			REQUIRE( params.match() == R"match(/café)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "foo" ) > 0 );
			REQUIRE( nps.at( "foo" ) == R"value(café)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	{
		// ":domain.com"
		// {"delimiter":"."}
		// [["example.com",["example.com","example"]],["github.com",["github.com","github"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(:domain.com)route",
				path2regex::options_t{}.delimiter( "." ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(example.com)target", params ) );
			REQUIRE( params.match() == R"match(example.com)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "domain" ) > 0 );
			REQUIRE( nps.at( "domain" ) == R"value(example)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(github.com)target", params ) );
			REQUIRE( params.match() == R"match(github.com)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "domain" ) > 0 );
			REQUIRE( nps.at( "domain" ) == R"value(github)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	{
		// "mail.:domain.com"
		// {"delimiter":"."}
		// [["mail.example.com",["mail.example.com","example"]],["mail.github.com",["mail.github.com","github"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(mail.:domain.com)route",
				path2regex::options_t{}.delimiter( "." ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(mail.example.com)target", params ) );
			REQUIRE( params.match() == R"match(mail.example.com)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "domain" ) > 0 );
			REQUIRE( nps.at( "domain" ) == R"value(example)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(mail.github.com)target", params ) );
			REQUIRE( params.match() == R"match(mail.github.com)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "domain" ) > 0 );
			REQUIRE( nps.at( "domain" ) == R"value(github)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	{
		// "example.:ext"
		// {"delimiter":"."}
		// [["example.com",["example.com","com"]],["example.org",["example.org","org"]]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(example.:ext)route",
				path2regex::options_t{}.delimiter( "." ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(example.com)target", params ) );
			REQUIRE( params.match() == R"match(example.com)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "ext" ) > 0 );
			REQUIRE( nps.at( "ext" ) == R"value(com)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(example.org)target", params ) );
			REQUIRE( params.match() == R"match(example.org)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 1 == nps.size() );

			REQUIRE( nps.count( "ext" ) > 0 );
			REQUIRE( nps.at( "ext" ) == R"value(org)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	{
		// "this is"
		// {"delimiter":" ","end":false}
		// [["this is a test",["this is"]],["this isn't",null]]
		auto mather_data =
			path2regex::path2regex< route_params_t >(
				R"route(this is)route",
				path2regex::options_t{}.ending( false ).delimiter( " " ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( mather_data.m_regex ),
				std::move( mather_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(this is a test)target", params ) );
			REQUIRE( params.match() == R"match(this is)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(this isn't)target", params ) );

	}

