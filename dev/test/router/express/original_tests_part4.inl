

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

