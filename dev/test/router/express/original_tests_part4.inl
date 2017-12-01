	// Test #67
	{
		// "/\\(testing\\)"
		// null
		// [["/testing",null],["/(testing)",["/(testing)"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/\(testing\))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

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


	// Test #68
	{
		// "/.+*?=^!:${}[]|"
		// null
		// [["/.+*?=^!:${}[]|",["/.+*?=^!:${}[]|"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/.+*?=^!:${}[]|)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

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


	// Test #69
	{
		// "/test\\/:uid(u\\d+)?:cid(c\\d+)?"
		// null
		// [["/test",null],["/test/",["/test/",null,null]],["/test/u123",["/test/u123","u123",null]],["/test/c123",["/test/c123",null,"c123"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test\/:uid(u\d+)?:cid(c\d+)?)route",
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
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "uid" ) > 0 );
			REQUIRE( nps.at( "uid" ) == R"value()value" );

			REQUIRE( nps.count( "cid" ) > 0 );
			REQUIRE( nps.at( "cid" ) == R"value()value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test/u123)target", params ) );
			REQUIRE( params.match() == R"match(/test/u123)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "uid" ) > 0 );
			REQUIRE( nps.at( "uid" ) == R"value(u123)value" );

			REQUIRE( nps.count( "cid" ) > 0 );
			REQUIRE( nps.at( "cid" ) == R"value()value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test/c123)target", params ) );
			REQUIRE( params.match() == R"match(/test/c123)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "uid" ) > 0 );
			REQUIRE( nps.at( "uid" ) == R"value()value" );

			REQUIRE( nps.count( "cid" ) > 0 );
			REQUIRE( nps.at( "cid" ) == R"value(c123)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #70
	{
		// "/(apple-)?icon-:res(\\d+).png"
		// null
		// [["/icon-240.png",["/icon-240.png",null,"240"]],["/apple-icon-240.png",["/apple-icon-240.png","apple-","240"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/(apple-)?icon-:res(\d+).png)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

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


	// Test #71
	{
		// "/:foo/:bar"
		// null
		// [["/match/route",["/match/route","match","route"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:foo/:bar)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

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


	// Test #72
	{
		// "/:foo(test\\)/bar"
		// null
		// []
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:foo(test\)/bar)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
	}


	// Test #73
	{
		// "/:remote([\\w-.]+)/:user([\\w-]+)"
		// null
		// [["/endpoint/user",["/endpoint/user","endpoint","user"]],["/endpoint/user-name",["/endpoint/user-name","endpoint","user-name"]],["/foo.bar/user-name",["/foo.bar/user-name","foo.bar","user-name"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:remote([\w-.]+)/:user([\w-]+))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

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


	// Test #74
	{
		// "/:foo\\?"
		// null
		// [["/route?",["/route?","route"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:foo\?)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

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


	// Test #75
	{
		// "/:foo+baz"
		// null
		// [["/foobaz",["/foobaz","foo"]],["/foo/barbaz",["/foo/barbaz","foo/bar"]],["/baz",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:foo+baz)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

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


	// Test #76
	{
		// "/:pre?baz"
		// null
		// [["/foobaz",["/foobaz","foo"]],["/baz",["/baz",null]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:pre?baz)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

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


	// Test #77
	{
		// "/:foo\\(:bar?\\)"
		// null
		// [["/hello(world)",["/hello(world)","hello","world"]],["/hello()",["/hello()","hello",null]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:foo\(:bar?\))route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

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


	// Test #78
	{
		// "/:postType(video|audio|text)(\\+.+)?"
		// null
		// [["/video",["/video","video",null]],["/video+test",["/video+test","video","+test"]],["/video+",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:postType(video|audio|text)(\+.+)?)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

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


	// Test #79
	{
		// "/:foo"
		// null
		// [["/café",["/café","café"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/:foo)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

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
