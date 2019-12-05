
// Test #60 skipped
// "/(.*)/"
// null
// [["/match/anything",["/match/anything","/match/anything"]]]

// Test #61 skipped
// "/\\/(\\d+)/"
// null
// [["/abc",null],["/123",["/123","123"]]]

// Test #62 skipped
// "/test,/\\/(\\d+)/"
// null
// [["/test",["/test",null]]]

// Test #63 skipped
// "/:test(\\d+),/(.*)/"
// null
// [["/123",["/123","123",null]],["/abc",["/abc",null,"/abc"]]]

// Test #64 skipped
// "/:test,/route/:test"
// null
// [["/test",["/test","test",null]],["/route/test",["/route/test",null,"test"]]]

// Test #65 skipped
// "/^\\/([^\\/]+)$/,/^\\/route\\/([^\\/]+)$/"
// null
// [["/test",["/test","test",null]],["/route/test",["/route/test",null,"test"]]]

// Test #66 skipped
// "/(?:.*)/"
// null
// [["/anything/you/want",["/anything/you/want"]]]

// "/\\(testing\\)"
// null
// [["/testing",null],["/(testing)",["/(testing)"]]]
TEST_CASE( "Original tests #67", "[path2regex][original][generated][n67]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/\(testing\))route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/testing)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/(testing))target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/(testing))match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/.+*?=^!:${}[]|"
// null
// [["/.+*?=^!:${}[]|",["/.+*?=^!:${}[]|"]]]
TEST_CASE( "Original tests #68", "[path2regex][original][generated][n68]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/.+*?=^!:${}[]|)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/.+*?=^!:${}[]|)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/.+*?=^!:${}[]|)match" );

		REQUIRE( 0 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.empty() );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/test\\/:uid(u\\d+)?:cid(c\\d+)?"
// null
// [["/test",null],["/test/",["/test/",null,null]],["/test/u123",["/test/u123","u123",null]],["/test/c123",["/test/c123",null,"c123"]]]
TEST_CASE( "Original tests #69", "[path2regex][original][generated][n69]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/test\/:uid(u\d+)?:cid(c\d+)?)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test/)match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(uid)key" ) );
		REQUIRE( nps[0].first == R"key(uid)key" );
		REQUIRE( nps[0].second == R"value()value" );
		REQUIRE( params.has( R"key(cid)key" ) );
		REQUIRE( nps[1].first == R"key(cid)key" );
		REQUIRE( nps[1].second == R"value()value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/u123)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test/u123)match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(uid)key" ) );
		REQUIRE( nps[0].first == R"key(uid)key" );
		REQUIRE( nps[0].second == R"value(u123)value" );
		REQUIRE( params.has( R"key(cid)key" ) );
		REQUIRE( nps[1].first == R"key(cid)key" );
		REQUIRE( nps[1].second == R"value()value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/test/c123)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/test/c123)match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(uid)key" ) );
		REQUIRE( nps[0].first == R"key(uid)key" );
		REQUIRE( nps[0].second == R"value()value" );
		REQUIRE( params.has( R"key(cid)key" ) );
		REQUIRE( nps[1].first == R"key(cid)key" );
		REQUIRE( nps[1].second == R"value(c123)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/(apple-)?icon-:res(\\d+).png"
// null
// [["/icon-240.png",["/icon-240.png",null,"240"]],["/apple-icon-240.png",["/apple-icon-240.png","apple-","240"]]]
TEST_CASE( "Original tests #70", "[path2regex][original][generated][n70]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/(apple-)?icon-:res(\d+).png)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/icon-240.png)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/icon-240.png)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(res)key" ) );
		REQUIRE( nps[0].first == R"key(res)key" );
		REQUIRE( nps[0].second == R"value(240)value" );

		REQUIRE( 1 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( 1 == ips.size() );
		REQUIRE( ips.at( 0 ) == R"value()value" );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/apple-icon-240.png)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/apple-icon-240.png)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(res)key" ) );
		REQUIRE( nps[0].first == R"key(res)key" );
		REQUIRE( nps[0].second == R"value(240)value" );

		REQUIRE( 1 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( 1 == ips.size() );
		REQUIRE( ips.at( 0 ) == R"value(apple-)value" );
	}

}

// "/:foo/:bar"
// null
// [["/match/route",["/match/route","match","route"]]]
TEST_CASE( "Original tests #71", "[path2regex][original][generated][n71]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:foo/:bar)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/match/route)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/match/route)match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(foo)key" ) );
		REQUIRE( nps[0].first == R"key(foo)key" );
		REQUIRE( nps[0].second == R"value(match)value" );
		REQUIRE( params.has( R"key(bar)key" ) );
		REQUIRE( nps[1].first == R"key(bar)key" );
		REQUIRE( nps[1].second == R"value(route)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:foo(test\\)/bar"
// null
// []
TEST_CASE( "Original tests #72", "[path2regex][original][generated][n72]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:foo\(test\)/bar)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

}

// "/:remote([\\w\\-\\.]+)/:user([\\w\\-]+)"
// null
// [["/endpoint/user",["/endpoint/user","endpoint","user"]],["/endpoint/user-name",["/endpoint/user-name","endpoint","user-name"]],["/foo.bar/user-name",["/foo.bar/user-name","foo.bar","user-name"]]]
TEST_CASE( "Original tests #73", "[path2regex][original][generated][n73]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:remote([\w\-\.]+)/:user([\w\-]+))route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/endpoint/user)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/endpoint/user)match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(remote)key" ) );
		REQUIRE( nps[0].first == R"key(remote)key" );
		REQUIRE( nps[0].second == R"value(endpoint)value" );
		REQUIRE( params.has( R"key(user)key" ) );
		REQUIRE( nps[1].first == R"key(user)key" );
		REQUIRE( nps[1].second == R"value(user)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/endpoint/user-name)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/endpoint/user-name)match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(remote)key" ) );
		REQUIRE( nps[0].first == R"key(remote)key" );
		REQUIRE( nps[0].second == R"value(endpoint)value" );
		REQUIRE( params.has( R"key(user)key" ) );
		REQUIRE( nps[1].first == R"key(user)key" );
		REQUIRE( nps[1].second == R"value(user-name)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/foo.bar/user-name)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/foo.bar/user-name)match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(remote)key" ) );
		REQUIRE( nps[0].first == R"key(remote)key" );
		REQUIRE( nps[0].second == R"value(foo.bar)value" );
		REQUIRE( params.has( R"key(user)key" ) );
		REQUIRE( nps[1].first == R"key(user)key" );
		REQUIRE( nps[1].second == R"value(user-name)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:foo\\?"
// null
// [["/route?",["/route?","route"]]]
TEST_CASE( "Original tests #74", "[path2regex][original][generated][n74]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:foo\?)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/route?)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/route?)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(foo)key" ) );
		REQUIRE( nps[0].first == R"key(foo)key" );
		REQUIRE( nps[0].second == R"value(route)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:foo+baz"
// null
// [["/foobaz",["/foobaz","foo"]],["/foo/barbaz",["/foo/barbaz","foo/bar"]],["/baz",null]]
TEST_CASE( "Original tests #75", "[path2regex][original][generated][n75]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:foo+baz)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/foobaz)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/foobaz)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(foo)key" ) );
		REQUIRE( nps[0].first == R"key(foo)key" );
		REQUIRE( nps[0].second == R"value(foo)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/foo/barbaz)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/foo/barbaz)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(foo)key" ) );
		REQUIRE( nps[0].first == R"key(foo)key" );
		REQUIRE( nps[0].second == R"value(foo/bar)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/baz)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/:pre?baz"
// null
// [["/foobaz",["/foobaz","foo"]],["/baz",["/baz",null]]]
TEST_CASE( "Original tests #76", "[path2regex][original][generated][n76]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:pre?baz)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/foobaz)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/foobaz)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(pre)key" ) );
		REQUIRE( nps[0].first == R"key(pre)key" );
		REQUIRE( nps[0].second == R"value(foo)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/baz)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/baz)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(pre)key" ) );
		REQUIRE( nps[0].first == R"key(pre)key" );
		REQUIRE( nps[0].second == R"value()value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:foo\\(:bar?\\)"
// null
// [["/hello(world)",["/hello(world)","hello","world"]],["/hello()",["/hello()","hello",null]]]
TEST_CASE( "Original tests #77", "[path2regex][original][generated][n77]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:foo\(:bar?\))route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/hello(world))target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/hello(world))match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(foo)key" ) );
		REQUIRE( nps[0].first == R"key(foo)key" );
		REQUIRE( nps[0].second == R"value(hello)value" );
		REQUIRE( params.has( R"key(bar)key" ) );
		REQUIRE( nps[1].first == R"key(bar)key" );
		REQUIRE( nps[1].second == R"value(world)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/hello())target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/hello())match" );

		REQUIRE( 2 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 2 == nps.size() );
		REQUIRE( params.has( R"key(foo)key" ) );
		REQUIRE( nps[0].first == R"key(foo)key" );
		REQUIRE( nps[0].second == R"value(hello)value" );
		REQUIRE( params.has( R"key(bar)key" ) );
		REQUIRE( nps[1].first == R"key(bar)key" );
		REQUIRE( nps[1].second == R"value()value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}

}

// "/:postType(video|audio|text)(\\+.+)?"
// null
// [["/video",["/video","video",null]],["/video+test",["/video+test","video","+test"]],["/video+",null]]
TEST_CASE( "Original tests #78", "[path2regex][original][generated][n78]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:postType(video|audio|text)(\+.+)?)route",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/video)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/video)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(postType)key" ) );
		REQUIRE( nps[0].first == R"key(postType)key" );
		REQUIRE( nps[0].second == R"value(video)value" );

		REQUIRE( 1 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( 1 == ips.size() );
		REQUIRE( ips.at( 0 ) == R"value()value" );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/video+test)target" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == R"match(/video+test)match" );

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(postType)key" ) );
		REQUIRE( nps[0].first == R"key(postType)key" );
		REQUIRE( nps[0].second == R"value(video)value" );

		REQUIRE( 1 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( 1 == ips.size() );
		REQUIRE( ips.at( 0 ) == R"value(+test)value" );
	}

	{
		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ R"target(/video+)target" };
		REQUIRE_FALSE( rm.match_route( target_path, params ) );
	}

}

// "/:foo"
// null
// [["/café",["/café","café"]]]
TEST_CASE( "Original tests #79", "[path2regex][original][generated][n79]")
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			R"route(/:foo)route",
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

		REQUIRE( 1 == params.named_parameters_size() );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( 1 == nps.size() );
		REQUIRE( params.has( R"key(foo)key" ) );
		REQUIRE( nps[0].first == R"key(foo)key" );
		REQUIRE( nps[0].second == R"value(café)value" );

		REQUIRE( 0 == params.indexed_parameters_size() );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.empty() );
	}
}
