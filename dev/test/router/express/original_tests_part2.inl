
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
