request_handle_t
create_fake_request( std::string target, http_method_t method = http_method_get() )
{
	return
		std::make_shared< request_t >(
			0,
			http_request_header_t{ method, std::move( target ) },
			"",
			restinio::impl::connection_handle_t{} );
}

TEST_CASE( "Simple named param" , "[express][simple][named_params]" )
{

	int last_handler_called = -1;

	auto extract_last_handler_called = [&]{
		int result = last_handler_called;
		last_handler_called = -1;
		return result;
	};

	route_params_t route_params{};
	auto check_route_params = [ & ]{
		const bool result =
			!route_params.named_parameters().empty() &&
			route_params.indexed_parameters().empty() &&
			0 != route_params.named_parameters().count( "id" ) &&
			route_params.named_parameters().at( "id" ) == "42";

		route_params.reset();

		return result;
	};

	express_router_t router;

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
	REQUIRE_FALSE( check_route_params() );

	REQUIRE( request_accepted() == router( create_fake_request( "/a-route/42" ) ) );
	REQUIRE( 0 == extract_last_handler_called() );
	REQUIRE( check_route_params() );

	REQUIRE( request_accepted() == router( create_fake_request( "/b-route/42" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );
	REQUIRE( check_route_params() );

	REQUIRE( request_accepted() == router( create_fake_request( "/c-route/42" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );
	REQUIRE( check_route_params() );

	REQUIRE( request_accepted() == router( create_fake_request( "/d-route/42" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
	REQUIRE( check_route_params() );
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
		const bool result =
			route_params.named_parameters().empty() &&
			!route_params.indexed_parameters().empty() &&
			route_params.indexed_parameters().at( 0 ) == "42";

		route_params.reset();

		return result;
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
	REQUIRE_FALSE( check_route_params() );

	REQUIRE( request_accepted() == router( create_fake_request( "/a-route/42/ending" ) ) );
	REQUIRE( 0 == extract_last_handler_called() );
	REQUIRE( check_route_params() );

	REQUIRE( request_accepted() == router( create_fake_request( "/b-route/42/ending" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );
	REQUIRE( check_route_params() );

	REQUIRE( request_accepted() == router( create_fake_request( "/c-route/42/ending" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );
	REQUIRE( check_route_params() );

	REQUIRE( request_accepted() == router( create_fake_request( "/d-route/42/ending" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
	REQUIRE( check_route_params() );
}

TEST_CASE( "Http methods" , "[express][simple][http_methods]" )
{

	http_method_t last_http_method = http_method_t::http_unknown;

	auto extract_last_http_method = [&]{
		http_method_t result = last_http_method;
		last_http_method = http_method_t::http_unknown;
		return result;
	};

	express_router_t router;

	router.http_delete(
		"/",
		[&]( auto , auto ){
			last_http_method = http_method_t::http_delete;
			return request_accepted();
		} );

	router.http_get(
		"/",
		[&]( auto , auto ){
			last_http_method = http_method_t::http_get;
			return request_accepted();
		} );

	router.http_head(
		"/",
		[&]( auto , auto ){
			last_http_method = http_method_t::http_head;
			return request_accepted();
		} );

	router.http_post(
		"/",
		[&]( auto , auto ){
			last_http_method = http_method_t::http_post;
			return request_accepted();
		} );

	router.http_put(
		"/",
		[&]( auto , auto ){
			last_http_method = http_method_t::http_put;
			return request_accepted();
		} );

	REQUIRE( request_rejected() == router( create_fake_request( "/xxx", http_method_t::http_delete ) ) );
	REQUIRE( request_rejected() == router( create_fake_request( "/xxx", http_method_t::http_get ) ) );
	REQUIRE( request_rejected() == router( create_fake_request( "/xxx", http_method_t::http_head ) ) );
	REQUIRE( request_rejected() == router( create_fake_request( "/xxx", http_method_t::http_post ) ) );
	REQUIRE( request_rejected() == router( create_fake_request( "/xxx", http_method_t::http_put ) ) );

	REQUIRE( request_accepted() == router( create_fake_request( "/", http_method_t::http_delete ) ) );
	REQUIRE( http_method_t::http_delete == extract_last_http_method() );
	REQUIRE( request_accepted() == router( create_fake_request( "/", http_method_t::http_get ) ) );
	REQUIRE( http_method_t::http_get == extract_last_http_method() );
	REQUIRE( request_accepted() == router( create_fake_request( "/", http_method_t::http_head ) ) );
	REQUIRE( http_method_t::http_head == extract_last_http_method() );
	REQUIRE( request_accepted() == router( create_fake_request( "/", http_method_t::http_post ) ) );
	REQUIRE( http_method_t::http_post == extract_last_http_method() );
	REQUIRE( request_accepted() == router( create_fake_request( "/", http_method_t::http_put ) ) );
	REQUIRE( http_method_t::http_put == extract_last_http_method() );
}


TEST_CASE( "Many params" , "[express][named_params]" )
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
		const auto & nps = route_params.named_parameters();
		REQUIRE( 4 == route_params.named_parameters().size() );

		const auto & ips = route_params.indexed_parameters();
		REQUIRE( 0 == ips.size() );

		REQUIRE( nps.at( "p1" ) == "717" );
		REQUIRE( nps.at( "p2" ) == "abcd" );
		REQUIRE( nps.at( "p3" ) == "99.AA" );
		REQUIRE( nps.at( "opt" ) == "x" );
	}

	REQUIRE( request_accepted() == router( create_fake_request( "/news/2017-04-01" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );

	{
		const auto & nps = route_params.named_parameters();
		REQUIRE( 3 == nps.size() );

		const auto & ips = route_params.indexed_parameters();
		REQUIRE( 0 == ips.size() );

		REQUIRE( nps.at( "year" ) == "2017" );
		REQUIRE( nps.at( "month" ) == "04" );
		REQUIRE( nps.at( "day" ) == "01" );
	}

	REQUIRE( request_accepted() == router( create_fake_request( "/events/2017-06-03" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );

	{
		const auto & nps = route_params.named_parameters();
		REQUIRE( 0 == route_params.named_parameters().size() );

		const auto & ips = route_params.indexed_parameters();
		REQUIRE( 3 == ips.size() );
		REQUIRE( ips.at( 0 ) == "2017" );
		REQUIRE( ips.at( 1 ) == "06" );
		REQUIRE( ips.at( 2 ) == "03" );
	}
}

