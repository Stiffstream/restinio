
// "/café"
// null
// [["/café",["/café"]]]
TEST_CASE( "Original tests #80", "[path2regex][original][generated][n80]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/café)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/café)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/café)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "packages/"
// null
// [["packages",null],["packages/",["packages/"]]]
TEST_CASE( "Original tests #81", "[path2regex][original][generated][n81]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(packages/)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(packages)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(packages/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(packages/)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// ":domain.com"
// {"delimiter":"."}
// [["example.com",["example.com","example"]],["github.com",["github.com","github"]]]
TEST_CASE( "Original tests #82", "[path2regex][original][generated][n82]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(:domain.com)route",
			path2regex::options_t{}.delimiter( "." ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(example.com)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(example.com)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(domain)key" ) );
		REQUIRE( nps[0].first == R"key(domain)key" );
		REQUIRE( nps[0].second == R"value(example)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(github.com)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(github.com)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(domain)key" ) );
		REQUIRE( nps[0].first == R"key(domain)key" );
		REQUIRE( nps[0].second == R"value(github)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "mail.:domain.com"
// {"delimiter":"."}
// [["mail.example.com",["mail.example.com","example"]],["mail.github.com",["mail.github.com","github"]]]
TEST_CASE( "Original tests #83", "[path2regex][original][generated][n83]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(mail.:domain.com)route",
			path2regex::options_t{}.delimiter( "." ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(mail.example.com)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(mail.example.com)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(domain)key" ) );
		REQUIRE( nps[0].first == R"key(domain)key" );
		REQUIRE( nps[0].second == R"value(example)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(mail.github.com)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(mail.github.com)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(domain)key" ) );
		REQUIRE( nps[0].first == R"key(domain)key" );
		REQUIRE( nps[0].second == R"value(github)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "example.:ext"
// {"delimiter":"."}
// [["example.com",["example.com","com"]],["example.org",["example.org","org"]]]
TEST_CASE( "Original tests #84", "[path2regex][original][generated][n84]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(example.:ext)route",
			path2regex::options_t{}.delimiter( "." ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(example.com)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(example.com)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(ext)key" ) );
		REQUIRE( nps[0].first == R"key(ext)key" );
		REQUIRE( nps[0].second == R"value(com)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(example.org)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(example.org)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(ext)key" ) );
		REQUIRE( nps[0].first == R"key(ext)key" );
		REQUIRE( nps[0].second == R"value(org)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "this is"
// {"delimiter":" ","end":false}
// [["this is a test",["this is"]],["this isn't",null]]
TEST_CASE( "Original tests #85", "[path2regex][original][generated][n85]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(this is)route",
			path2regex::options_t{}.ending( false ).delimiter( " " ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(this is a test)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(this is)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(this isn't)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/test"
// {"endsWith":"?"}
// [["/test",["/test"]],["/test?query=string",["/test"]],["/test/?query=string",["/test/"]],["/testx",null]]
TEST_CASE( "Original tests #86", "[path2regex][original][generated][n86]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test)route",
			path2regex::options_t{}.ends_with( {"?"} ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test?query=string)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/?query=string)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/testx)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/test"
// {"endsWith":"?","strict":true}
// [["/test?query=string",["/test"]],["/test/?query=string",null]]
TEST_CASE( "Original tests #87", "[path2regex][original][generated][n87]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test)route",
			path2regex::options_t{}.strict( true ).ends_with( {"?"} ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test?query=string)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/?query=string)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "$:foo$:bar?"
// {"delimiters":"$"}
// [["$x",["$x","x",null]],["$x$y",["$x$y","x","y"]]]
TEST_CASE( "Original tests #88", "[path2regex][original][generated][n88]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route($:foo$:bar?)route",
			path2regex::options_t{}.delimiters( "$" ) );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target($x)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match($x)match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(foo)key" ) );
		REQUIRE( nps[0].first == R"key(foo)key" );
		REQUIRE( nps[0].second == R"value(x)value" );
		REQUIRE( params.has( R"key(bar)key" ) );
		REQUIRE( nps[1].first == R"key(bar)key" );
		REQUIRE( nps[1].second == R"value()value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target($x$y)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match($x$y)match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(foo)key" ) );
		REQUIRE( nps[0].first == R"key(foo)key" );
		REQUIRE( nps[0].second == R"value(x)value" );
		REQUIRE( params.has( R"key(bar)key" ) );
		REQUIRE( nps[1].first == R"key(bar)key" );
		REQUIRE( nps[1].second == R"value(y)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}
}

