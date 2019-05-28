struct fake_connection_t : public restinio::impl::connection_base_t
{
	fake_connection_t() : restinio::impl::connection_base_t{ 0 }
	{}

	virtual void
	check_timeout( std::shared_ptr< tcp_connection_ctx_base_t > & ) override
	{}

	virtual void
	write_response_parts(
		request_id_t ,
		response_output_flags_t ,
		write_group_t ) override
	{}
};


request_handle_t
create_fake_request( std::string target, http_method_id_t method = http_method_get() )
{
	return
		std::make_shared< request_t >(
			0,
			http_request_header_t{ method, std::move( target ) },
			"",
			std::make_shared< fake_connection_t >(),
			restinio::endpoint_t{
				restinio::asio_ns::ip::make_address_v4("127.0.0.1"),
				3000 } );
			// restinio::impl::connection_handle_t{ new fake_connection_t} );
}

TEST_CASE( "Simple named param" , "[express][simple][named_params]" )
{

	int last_handler_called = -1;

	auto extract_last_handler_called = [&]{
		int result = last_handler_called;
		last_handler_called = -1;
		return result;
	};

	express_router_t router;

	route_params_t route_params{};

	auto check_route_params = [ & ]{
			const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( route_params );
			const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( route_params );

			REQUIRE_FALSE( nps.empty() );
			REQUIRE( ips.empty() );
			REQUIRE( nps[0].first =="id" );
			REQUIRE( nps[0].second == "42" );
			REQUIRE( restinio::cast_to< std::string >( route_params[ "id" ] ) == "42" );
			REQUIRE( restinio::cast_to< std::uint8_t >( route_params[ "id" ] ) == 42 );
			REQUIRE( restinio::cast_to< std::int8_t >( route_params[ "id" ] ) == 42 );
			REQUIRE( restinio::cast_to< short >( route_params[ "id" ] ) == 42 );
			REQUIRE( restinio::cast_to< unsigned short >( route_params[ "id" ] ) == 42 );
			REQUIRE( restinio::cast_to< int >( route_params[ "id" ] ) == 42 );
			REQUIRE( restinio::cast_to< unsigned int >( route_params[ "id" ] ) == 42 );
			REQUIRE( restinio::cast_to< float >( route_params[ "id" ] ) == Approx(42) );
			REQUIRE( restinio::cast_to< double >( route_params[ "id" ] ) == Approx(42) );
	};

	router.http_get(
		"/a-route/:id",
		[&]( auto , auto p ){
			last_handler_called = 0;
			route_params = std::move( p );
			return request_accepted();
		} );


	router.http_get(
		"/b-route/:id",
		[&]( auto , auto p ){
			last_handler_called = 1;
			route_params = std::move( p );
			return request_accepted();
		} );

	router.http_get(
		"/c-route/:id",
		[&]( auto , auto p ){
			last_handler_called = 2;
			route_params = std::move( p );
			return request_accepted();
		} );

	router.http_get(
		"/d-route/:id",
		[&]( auto , auto p ){
			last_handler_called = 3;
			route_params = std::move( p );
			return request_accepted();
		} );

	REQUIRE( request_rejected() == router( create_fake_request( "/xxx" ) ) );
	REQUIRE( -1 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/a-route/42" ) ) );
	REQUIRE( 0 == extract_last_handler_called() );
	check_route_params();

	REQUIRE( request_accepted() == router( create_fake_request( "/b-route/42" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );
	check_route_params();

	REQUIRE( request_accepted() == router( create_fake_request( "/c-route/42" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );
	check_route_params();

	REQUIRE( request_accepted() == router( create_fake_request( "/d-route/42" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
	check_route_params();
}

TEST_CASE( "Simple indexed param" , "[express][simple][indexed_params]" )
{
	int last_handler_called = -1;

	auto extract_last_handler_called = [&]{
		int result = last_handler_called;
		last_handler_called = -1;
		return result;
	};

	route_params_t route_params{};
	auto check_route_params = [ & ]{
		const auto & nps =
			restinio::router::impl::route_params_accessor_t::named_parameters( route_params );
		const auto & ips =
			restinio::router::impl::route_params_accessor_t::indexed_parameters( route_params );

		REQUIRE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( ips[ 0 ] == "42" );
	};

	express_router_t router;

	router.http_get(
		R"(/a-route/(\d+)/ending)",
		[&]( auto , auto p ){
			last_handler_called = 0;
			route_params = std::move( p );
			return request_accepted();
		} );


	router.http_get(
		R"(/b-route/(\d+)/ending)",
		[&]( auto , auto p ){
			last_handler_called = 1;
			route_params = std::move( p );
			return request_accepted();
		} );

	router.http_get(
		R"(/c-route/(\d+)/ending)",
		[&]( auto , auto p ){
			last_handler_called = 2;
			route_params = std::move( p );
			return request_accepted();
		} );

	router.http_get(
		R"(/d-route/(\d+)/ending)",
		[&]( auto , auto p ){
			last_handler_called = 3;
			route_params = std::move( p );
			return request_accepted();
		} );

	REQUIRE( request_rejected() == router( create_fake_request( "/xxx" ) ) );
	REQUIRE( -1 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/a-route/42/ending" ) ) );
	REQUIRE( 0 == extract_last_handler_called() );
	check_route_params();

	REQUIRE( request_accepted() == router( create_fake_request( "/b-route/42/ending" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );
	check_route_params();

	REQUIRE( request_accepted() == router( create_fake_request( "/c-route/42/ending" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );
	check_route_params();

	REQUIRE( request_accepted() == router( create_fake_request( "/d-route/42/ending" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
	check_route_params();
}

TEST_CASE( "Http methods" , "[express][simple][http_methods]" )
{

	http_method_id_t last_http_method = http_method_unknown();

	auto extract_last_http_method = [&]{
		http_method_id_t result = last_http_method;
		last_http_method = http_method_unknown();
		return result;
	};

	express_router_t router;

	router.http_delete(
		"/",
		[&]( auto , auto ){
			last_http_method = http_method_delete();
			return request_accepted();
		} );

	router.http_get(
		"/",
		[&]( auto , auto ){
			last_http_method = http_method_get();
			return request_accepted();
		} );

	router.http_head(
		"/",
		[&]( auto , auto ){
			last_http_method = http_method_head();
			return request_accepted();
		} );

	router.http_post(
		"/",
		[&]( auto , auto ){
			last_http_method = http_method_post();
			return request_accepted();
		} );

	router.http_put(
		"/",
		[&]( auto , auto ){
			last_http_method = http_method_put();
			return request_accepted();
		} );

	REQUIRE( request_rejected() == router( create_fake_request( "/xxx", http_method_delete() ) ) );
	REQUIRE( request_rejected() == router( create_fake_request( "/xxx", http_method_get() ) ) );
	REQUIRE( request_rejected() == router( create_fake_request( "/xxx", http_method_head() ) ) );
	REQUIRE( request_rejected() == router( create_fake_request( "/xxx", http_method_post() ) ) );
	REQUIRE( request_rejected() == router( create_fake_request( "/xxx", http_method_put() ) ) );

	REQUIRE( request_accepted() == router( create_fake_request( "/", http_method_delete() ) ) );
	REQUIRE( http_method_delete() == extract_last_http_method() );
	REQUIRE( request_accepted() == router( create_fake_request( "/", http_method_get() ) ) );
	REQUIRE( http_method_get() == extract_last_http_method() );
	REQUIRE( request_accepted() == router( create_fake_request( "/", http_method_head() ) ) );
	REQUIRE( http_method_head() == extract_last_http_method() );
	REQUIRE( request_accepted() == router( create_fake_request( "/", http_method_post() ) ) );
	REQUIRE( http_method_post() == extract_last_http_method() );
	REQUIRE( request_accepted() == router( create_fake_request( "/", http_method_put() ) ) );
	REQUIRE( http_method_put() == extract_last_http_method() );
}

TEST_CASE( "Many params" , "[express][named_params][indexed_params]" )
{
	int last_handler_called = -1;

	auto extract_last_handler_called = [&]{
		int result = last_handler_called;
		last_handler_called = -1;
		return result;
	};

	route_params_t route_params{};

	express_router_t router;

	router.http_get( R"(/:p1(\d+)/:p2([a-z]+)/:p3(\d{2}\.[AB]{0,2})/:opt([a-z]?))",
		[&]( auto , auto p ){
			last_handler_called = 0;
			route_params = std::move( p );
			return request_accepted();
		} );

	router.http_get( R"(/news/:year(\d{4})-:month(\d{2})-:day(\d{2}))",
		[&]( auto , auto p ){
			last_handler_called = 1;
			route_params = std::move( p );
			return request_accepted();
		} );

	router.http_get( R"(/events/(\d{4})-(\d{2})-(\d{2}))",
		[&]( auto , auto p ){
			last_handler_called = 2;
			route_params = std::move( p );
			return request_accepted();
		} );

	REQUIRE( request_accepted() == router( create_fake_request( "/717/abcd/99.AA/x" ) ) );
	REQUIRE( 0 == extract_last_handler_called() );

	{
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( route_params );
		REQUIRE( 4 == nps.size() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( route_params );
		REQUIRE( 0 == ips.size() );

		REQUIRE( route_params[ "p1" ] == "717" );
		REQUIRE( route_params[ "p2" ] == "abcd" );
		REQUIRE( route_params[ "p3" ] == "99.AA" );
		REQUIRE( route_params[ "opt" ] == "x" );
	}

	REQUIRE( request_accepted() == router( create_fake_request( "/news/2017-04-01" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );

	{
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( route_params );
		REQUIRE( 3 == nps.size() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( route_params );
		REQUIRE( 0 == ips.size() );

		REQUIRE( route_params[ "year" ] == "2017" );
		REQUIRE( route_params[ "month" ] == "04" );
		REQUIRE( route_params[ "day" ] == "01" );
	}

	REQUIRE( request_accepted() == router( create_fake_request( "/events/2017-06-03" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );

	{
		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( route_params );
		REQUIRE( 0 == nps.size() );

		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( route_params );
		REQUIRE( 3 == ips.size() );
		REQUIRE( route_params[ 0 ] == "2017" );
		REQUIRE( route_params[ 1 ] == "06" );
		REQUIRE( route_params[ 2 ] == "03" );
	}
}

TEST_CASE( "Non matched request handler" , "[express][non_matched_request_handler]" )
{
	int request_matched_type = -1; // -1 (uninitialized), 0 (not matched), 1 (matched).

	express_router_t router;

	router.http_delete(
		"/",
		[&]( auto , auto ){
			request_matched_type = 1;
			return request_accepted();
		} );

	router.http_get(
		"/",
		[&]( auto , auto ){
			request_matched_type = 1;
			return request_accepted();
		} );

	router.http_head(
		"/",
		[&]( auto , auto ){
			request_matched_type = 1;
			return request_accepted();
		} );

	router.http_post(
		"/",
		[&]( auto , auto ){
			request_matched_type = 1;
			return request_accepted();
		} );

	router.http_put(
		"/",
		[&]( auto , auto ){
			request_matched_type = 1;
			return request_accepted();
		} );

	router.non_matched_request_handler(
		[&]( auto ){
			request_matched_type = 0;
			return request_accepted();
		} );

	REQUIRE( request_accepted() == router( create_fake_request( "/xxx", http_method_delete() ) ) );
	REQUIRE( request_matched_type == 0 );
	request_matched_type = -1;

	REQUIRE( request_accepted() == router( create_fake_request( "/xxx", http_method_get() ) ) );
	REQUIRE( request_matched_type == 0 );
	request_matched_type = -1;

	REQUIRE( request_accepted() == router( create_fake_request( "/xxx", http_method_head() ) ) );
	REQUIRE( request_matched_type == 0 );
	request_matched_type = -1;

	REQUIRE( request_accepted() == router( create_fake_request( "/xxx", http_method_post() ) ) );
	REQUIRE( request_matched_type == 0 );
	request_matched_type = -1;

	REQUIRE( request_accepted() == router( create_fake_request( "/xxx", http_method_put() ) ) );
	REQUIRE( request_matched_type == 0 );
	request_matched_type = -1;

	REQUIRE( request_accepted() == router( create_fake_request( "/", http_method_delete() ) ) );
	REQUIRE( request_matched_type == 1 );
	request_matched_type = -1;


	REQUIRE( request_accepted() == router( create_fake_request( "/", http_method_get() ) ) );
	REQUIRE( request_matched_type == 1 );
	request_matched_type = -1;

	REQUIRE( request_accepted() == router( create_fake_request( "/", http_method_head() ) ) );
	REQUIRE( request_matched_type == 1 );
	request_matched_type = -1;

	REQUIRE( request_accepted() == router( create_fake_request( "/", http_method_post() ) ) );
	REQUIRE( request_matched_type == 1 );
	request_matched_type = -1;

	REQUIRE( request_accepted() == router( create_fake_request( "/", http_method_put() ) ) );
	REQUIRE( request_matched_type == 1 );
	request_matched_type = -1;
}

TEST_CASE( "Parameters cast" , "[express][parameters_cast]" )
{
	express_router_t router;

	route_params_t route_params{};

	router.http_get(
		"/:named_param/(.*)",
		[&]( auto , auto p ){
			route_params = std::move( p );
			return request_accepted();
		} );

	SECTION( "zero" )
	{
		REQUIRE( request_accepted() == router( create_fake_request( "/0/0" ) ) );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( route_params );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( route_params );
		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "0" );
		REQUIRE( route_params[ "named_param" ] == "0" );
		REQUIRE( restinio::cast_to< std::uint8_t >( route_params[ "named_param" ] ) == 0 );
		REQUIRE( restinio::cast_to< std::int8_t >( route_params[ "named_param" ] ) == 0 );
		REQUIRE( restinio::cast_to< short >( route_params[ "named_param" ] ) == 0 );
		REQUIRE( restinio::cast_to< unsigned short >( route_params[ "named_param" ] ) == 0 );
		REQUIRE( restinio::cast_to< int >( route_params[ "named_param" ] ) == 0 );
		REQUIRE( restinio::cast_to< unsigned int >( route_params[ "named_param" ] ) == 0 );
		REQUIRE( restinio::cast_to< float >( route_params[ "named_param" ] ) == Approx(0.0) );
		REQUIRE( restinio::cast_to< double >( route_params[ "named_param" ] ) == Approx(0.0) );
		REQUIRE( route_params[ 0 ] == "0" );
		REQUIRE( restinio::cast_to< std::uint8_t >( route_params[ 0 ] ) == 0 );
		REQUIRE( restinio::cast_to< std::int8_t >( route_params[ 0 ] ) == 0 );
		REQUIRE( restinio::cast_to< short >( route_params[ 0 ] ) == 0 );
		REQUIRE( restinio::cast_to< unsigned short >( route_params[ 0 ] ) == 0 );
		REQUIRE( restinio::cast_to< int >( route_params[ 0 ] ) == 0 );
		REQUIRE( restinio::cast_to< unsigned int >( route_params[ 0 ] ) == 0 );
		REQUIRE( restinio::cast_to< float >( route_params[ 0 ] ) == Approx(0.0) );
		REQUIRE( restinio::cast_to< double >( route_params[ 0 ] ) == Approx(0.0) );
	}

	SECTION( "int8" )
	{
		using int_type_t = std::int8_t;
		REQUIRE( request_accepted() == router( create_fake_request( "/-128/127" ) ) );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( route_params );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( route_params );

		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "-128" );
		REQUIRE( route_params[ "named_param" ] == "-128" );
		REQUIRE( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ) == -128 );

		REQUIRE( route_params[ 0 ] == "127" );
		REQUIRE( restinio::cast_to< int_type_t >( route_params[ 0 ] ) == 127 );

		REQUIRE( request_accepted() == router( create_fake_request( "/-129/128" ) ) );
		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "-129" );
		REQUIRE( route_params[ "named_param" ] == "-129" );
		REQUIRE_THROWS( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ) );
		REQUIRE( route_params[ 0 ] == "128" );
		REQUIRE_THROWS( restinio::cast_to< int_type_t >( route_params[ 0 ] ) );
	}

	SECTION( "uint8" )
	{
		using int_type_t = std::uint8_t;

		REQUIRE( request_accepted() == router( create_fake_request( "/0/255" ) ) );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( route_params );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( route_params );

		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "0" );
		REQUIRE( route_params[ "named_param" ] == "0" );
		REQUIRE( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ) == 0 );

		REQUIRE( route_params[ 0 ] == "255" );
		REQUIRE( restinio::cast_to< int_type_t >( route_params[ 0 ] ) == 255 );

		REQUIRE( request_accepted() == router( create_fake_request( "/-1/256" ) ) );
		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "-1" );
		REQUIRE( route_params[ "named_param" ] == "-1" );
		REQUIRE_THROWS( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ));
		REQUIRE( route_params[ 0 ] == "256" );
		REQUIRE_THROWS( restinio::cast_to< int_type_t >( route_params[ 0 ] ) );
	}

	SECTION( "int16" )
	{
		using int_type_t = std::int16_t;
		REQUIRE( request_accepted() == router( create_fake_request( "/-32768/32767" ) ) );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( route_params );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( route_params );

		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "-32768" );
		REQUIRE( route_params[ "named_param" ] == "-32768" );
		REQUIRE( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ) == -32768 );

		REQUIRE( route_params[ 0 ] == "32767" );
		REQUIRE( restinio::cast_to< int_type_t >( route_params[ 0 ] ) == 32767 );

		REQUIRE( request_accepted() == router( create_fake_request( "/-32769/32768" ) ) );
		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "-32769" );
		REQUIRE( route_params[ "named_param" ] == "-32769" );
		REQUIRE_THROWS( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ));
		REQUIRE( route_params[ 0 ] == "32768" );
		REQUIRE_THROWS( restinio::cast_to< int_type_t >( route_params[ 0 ] ) );
	}

	SECTION( "uint16" )
	{
		using int_type_t = std::uint16_t;

		REQUIRE( request_accepted() == router( create_fake_request( "/0/65535" ) ) );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( route_params );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( route_params );

		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "0" );
		REQUIRE( route_params[ "named_param" ] == "0" );
		REQUIRE( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ) == 0 );

		REQUIRE( route_params[ 0 ] == "65535" );
		REQUIRE( restinio::cast_to< int_type_t >( route_params[ 0 ] ) == 65535 );

		REQUIRE( request_accepted() == router( create_fake_request( "/-1/65536" ) ) );
		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "-1" );
		REQUIRE( route_params[ "named_param" ] == "-1" );
		REQUIRE_THROWS( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ));
		REQUIRE( route_params[ 0 ] == "65536" );
		REQUIRE_THROWS( restinio::cast_to< int_type_t >( route_params[ 0 ] ) );
	}

	SECTION( "int32" )
	{
		using int_type_t = std::int32_t;
		REQUIRE( request_accepted() == router( create_fake_request( "/-2147483648/2147483647" ) ) );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( route_params );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( route_params );

		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "-2147483648" );
		REQUIRE( route_params[ "named_param" ] == "-2147483648" );
		REQUIRE( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ) == -int_type_t{2147483647}-1 );

		REQUIRE( route_params[ 0 ] == "2147483647" );
		REQUIRE( restinio::cast_to< int_type_t >( route_params[ 0 ] ) == 2147483647 );

		REQUIRE( request_accepted() == router( create_fake_request( "/-2147483649/2147483648" ) ) );
		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "-2147483649" );
		REQUIRE( route_params[ "named_param" ] == "-2147483649" );
		REQUIRE_THROWS( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ));
		REQUIRE( route_params[ 0 ] == "2147483648" );
		REQUIRE_THROWS( restinio::cast_to< int_type_t >( route_params[ 0 ] ) );
	}

	SECTION( "uint32" )
	{
		using int_type_t = std::uint32_t;

		REQUIRE( request_accepted() == router( create_fake_request( "/0/4294967295" ) ) );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( route_params );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( route_params );

		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "0" );
		REQUIRE( route_params[ "named_param" ] == "0" );
		REQUIRE( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ) == 0UL );

		REQUIRE( route_params[ 0 ] == "4294967295" );
		REQUIRE( restinio::cast_to< int_type_t >( route_params[ 0 ] ) == 4294967295UL );

		REQUIRE( request_accepted() == router( create_fake_request( "/-1/4294967296" ) ) );
		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "-1" );
		REQUIRE( route_params[ "named_param" ] == "-1" );
		REQUIRE_THROWS( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ));
		REQUIRE( route_params[ 0 ] == "4294967296" );
		REQUIRE_THROWS( restinio::cast_to< int_type_t >( route_params[ 0 ] ) );
	}

	SECTION( "int64" )
	{
		using int_type_t = std::int64_t;
		REQUIRE( request_accepted() == router( create_fake_request( "/-9223372036854775808/9223372036854775807" ) ) );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( route_params );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( route_params );

		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "-9223372036854775808" );
		REQUIRE( route_params[ "named_param" ] == "-9223372036854775808" );

		// REQUIRE( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ) == (-9223372036854775808LL) );
		// To suppress the warning: integer constant is so large that it is unsigned.
		REQUIRE( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ) == std::numeric_limits<int_type_t>::min() );

		REQUIRE( route_params[ 0 ] == "9223372036854775807" );
		REQUIRE( restinio::cast_to< int_type_t >( route_params[ 0 ] ) == 9223372036854775807LL );

		REQUIRE( request_accepted() == router( create_fake_request( "/-9223372036854775809/9223372036854775808" ) ) );
		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "-9223372036854775809" );
		REQUIRE( route_params[ "named_param" ] == "-9223372036854775809" );
		REQUIRE_THROWS( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ));
		REQUIRE( route_params[ 0 ] == "9223372036854775808" );
		REQUIRE_THROWS( restinio::cast_to< int_type_t >( route_params[ 0 ] ) );
	}

	SECTION( "uint64" )
	{
		using int_type_t = std::uint64_t;

		REQUIRE( request_accepted() == router( create_fake_request( "/0/18446744073709551615" ) ) );

		const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( route_params );
		const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( route_params );

		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "0" );
		REQUIRE( route_params[ "named_param" ] == "0" );
		REQUIRE( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ) == 0ULL );

		REQUIRE( route_params[ 0 ] == "18446744073709551615" );
		REQUIRE( restinio::cast_to< int_type_t >( route_params[ 0 ] ) == 18446744073709551615ULL );

		REQUIRE( request_accepted() == router( create_fake_request( "/-1/18446744073709551616" ) ) );
		REQUIRE_FALSE( nps.empty() );
		REQUIRE_FALSE( ips.empty() );
		REQUIRE( nps[0].first =="named_param" );
		REQUIRE( nps[0].second == "-1" );
		REQUIRE( route_params[ "named_param" ] == "-1" );
		REQUIRE_THROWS( restinio::cast_to< int_type_t >( route_params[ "named_param" ] ));
		REQUIRE( route_params[ 0 ] == "18446744073709551616" );
		REQUIRE_THROWS( restinio::cast_to< int_type_t >( route_params[ 0 ] ) );
	}
}
