
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

