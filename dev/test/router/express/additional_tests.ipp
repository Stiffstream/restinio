
TEST_CASE( "Path to regex" , "[path2regex][simple]" )
{
	{
		auto matcher_data =
			path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
				"/foo/:bar",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_get(),
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_named_params_buffer ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;

		{
			restinio::router::impl::target_path_holder_t target_path{ "/foo/42/q" };
			REQUIRE_FALSE( rm.match_route( target_path, params ) );
		}
		{
			restinio::router::impl::target_path_holder_t target_path{ "/oof/42" };
			REQUIRE_FALSE( rm.match_route( target_path, params ) );
		}

		restinio::router::impl::target_path_holder_t target_path{ "/foo/42" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == "/foo/42" );
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.size() == 1 );
		REQUIRE( nps[0].first == "bar" );
		REQUIRE( nps[0].second == "42" );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.size() == 0 );
	}

	{
		auto matcher_data =
			path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
				"/a-route/:id",
				path2regex::options_t{} );

		route_matcher_t
			rm{
				http_method_get(),
				std::move( matcher_data.m_regex ),
				std::move( matcher_data.m_named_params_buffer ),
				std::move( matcher_data.m_param_appender_sequence ) };

		route_params_t params;

		restinio::router::impl::target_path_holder_t target_path{ "/a-route/42" };
		REQUIRE( rm.match_route( target_path, params ) );
		REQUIRE( params.match() == "/a-route/42" );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
		REQUIRE( nps.size() == 1 );
		REQUIRE( nps[0].first == "id" );
		REQUIRE( nps[0].second == "42" );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
		REQUIRE( ips.size() == 0 );
	}
}

TEST_CASE( "Invalid path" , "[path2regex][invalid]" )
{
	auto try_to_create = []( const std::string & r ){
		auto matcher_data =
			path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
				r,
				path2regex::options_t{} );
	};

	REQUIRE_THROWS( try_to_create( R"(/:foo([123]+)" ) );
	REQUIRE_THROWS( try_to_create( R"(/:foo([123]+)))" ) );

	REQUIRE_THROWS( try_to_create( R"(/([123]+)" ) );
	REQUIRE_THROWS( try_to_create( R"(/([123]+)))" ) );

	REQUIRE_THROWS( try_to_create( R"(/:foo/:bar(\d+)" ) );
	REQUIRE_THROWS( try_to_create( R"(/:foo/:bar(\d+)))" ) );

	REQUIRE_THROWS( try_to_create( R"(/([123]+)/(\d+)" ) );
	REQUIRE_THROWS( try_to_create( R"(/([123]+)/(\d+)))" ) );

	REQUIRE_THROWS( try_to_create( R"(/:foo(?:[123]+)" ) );

	try
	{
		try_to_create( R"route(/:foo(test\)/bar)route" );
		REQUIRE( false );
	}
	catch( const restinio::exception_t & ex )
	{
		REQUIRE_THAT( ex.what(), Catch::Matchers::Contains( "pos 5:" ) );
	}

	try
	{
		try_to_create( R"route(/foo(test\)/bar)route" );
		REQUIRE( false );
	}
	catch( const restinio::exception_t & ex )
	{
		REQUIRE_THAT( ex.what(), Catch::Matchers::Contains( "pos 4:" ) );
	}

	try
	{
		try_to_create( R"route(/:foo\(test)/bar)route" );
		REQUIRE( false );
	}
	catch( const restinio::exception_t & ex )
	{
		REQUIRE_THAT( ex.what(), Catch::Matchers::Contains( "pos 11:" ) );
	}

	try
	{
		try_to_create( R"route(/foo\(test)/bar)route" );
		REQUIRE( false );
	}
	catch( const restinio::exception_t & ex )
	{
		REQUIRE_THAT( ex.what(), Catch::Matchers::Contains( "pos 10:" ) );
	}
}

TEST_CASE( "value_or" , "[value_or]" )
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			"/:to/:from/:age/:gender",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_get(),
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	route_params_t params;

	restinio::router::impl::target_path_holder_t target_path{ "/815875200/1133136000/38/f" };
	REQUIRE( rm.match_route( target_path, params ) );
	REQUIRE( restinio::value_or< std::uint32_t >( params, "to", 0L ) == 815875200L );
	REQUIRE( restinio::value_or< std::uint32_t >( params, "from", 0L ) == 1133136000UL );
	REQUIRE( restinio::value_or( params, "age", std::uint16_t{99} ) == 38 );
	REQUIRE( restinio::value_or( params, "gender", string_view_t{"m"} ) == "f" );

	REQUIRE( restinio::value_or<std::uint32_t>( params, "does_not_exits", 42UL ) == 42UL );
	REQUIRE( restinio::value_or( params, "pi", 3.14 ) == 3.14 );
	REQUIRE( restinio::value_or( params, "e", string_view_t{ "2.71828" } ) ==
															"2.71828" );
}

