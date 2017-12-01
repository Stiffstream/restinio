

	// Test #80
	{
		// "/café"
		// null
		// [["/café",["/café"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/café)route",
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
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #81
	{
		// "packages/"
		// null
		// [["packages",null],["packages/",["packages/"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(packages/)route",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		REQUIRE_FALSE( rm.match_route( R"target(packages)target", params ) );

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(packages/)target", params ) );
			REQUIRE( params.match() == R"match(packages/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}


	// Test #82
	{
		// ":domain.com"
		// {"delimiter":"."}
		// [["example.com",["example.com","example"]],["github.com",["github.com","github"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(:domain.com)route",
				path2regex::options_t{}.delimiter( "." ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

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


	// Test #83
	{
		// "mail.:domain.com"
		// {"delimiter":"."}
		// [["mail.example.com",["mail.example.com","example"]],["mail.github.com",["mail.github.com","github"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(mail.:domain.com)route",
				path2regex::options_t{}.delimiter( "." ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

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


	// Test #84
	{
		// "example.:ext"
		// {"delimiter":"."}
		// [["example.com",["example.com","com"]],["example.org",["example.org","org"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(example.:ext)route",
				path2regex::options_t{}.delimiter( "." ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

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


	// Test #85
	{
		// "this is"
		// {"delimiter":" ","end":false}
		// [["this is a test",["this is"]],["this isn't",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(this is)route",
				path2regex::options_t{}.ending( false ).delimiter( " " ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

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


	// Test #86
	{
		// "/test"
		// {"endsWith":"?"}
		// [["/test",["/test"]],["/test?query=string",["/test"]],["/test/?query=string",["/test/"]],["/testx",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test)route",
				path2regex::options_t{}.ends_with( {"?"} ) );

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

			REQUIRE( rm.match_route( R"target(/test?query=string)target", params ) );
			REQUIRE( params.match() == R"match(/test)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test/?query=string)target", params ) );
			REQUIRE( params.match() == R"match(/test/)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/testx)target", params ) );

	}


	// Test #87
	{
		// "/test"
		// {"endsWith":"?","strict":true}
		// [["/test?query=string",["/test"]],["/test/?query=string",null]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route(/test)route",
				path2regex::options_t{}.strict( true ).ends_with( {"?"} ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target(/test?query=string)target", params ) );
			REQUIRE( params.match() == R"match(/test)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( nps.empty() );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		REQUIRE_FALSE( rm.match_route( R"target(/test/?query=string)target", params ) );

	}


	// Test #88
	{
		// "$:foo$:bar?"
		// {"delimiters":"$"}
		// [["$x",["$x","x",null]],["$x$y",["$x$y","x","y"]]]
		auto matcher_data =
			path2regex::path2regex< route_params_t, regex_engine_t >(
				R"route($:foo$:bar?)route",
				path2regex::options_t{}.delimiters( "$" ) );

		route_matcher_t
			rm{
				http_method_t::http_get,
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;
		{
			params.reset();

			REQUIRE( rm.match_route( R"target($x)target", params ) );
			REQUIRE( params.match() == R"match($x)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "foo" ) > 0 );
			REQUIRE( nps.at( "foo" ) == R"value(x)value" );

			REQUIRE( nps.count( "bar" ) > 0 );
			REQUIRE( nps.at( "bar" ) == R"value()value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

		{
			params.reset();

			REQUIRE( rm.match_route( R"target($x$y)target", params ) );
			REQUIRE( params.match() == R"match($x$y)match" );

			const auto & nps = params.named_parameters();
			REQUIRE( 2 == nps.size() );

			REQUIRE( nps.count( "foo" ) > 0 );
			REQUIRE( nps.at( "foo" ) == R"value(x)value" );

			REQUIRE( nps.count( "bar" ) > 0 );
			REQUIRE( nps.at( "bar" ) == R"value(y)value" );

			const auto & ips = params.indexed_parameters();
			REQUIRE( ips.empty() );
		}

	}
