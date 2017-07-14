
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
