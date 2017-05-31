/*
	restinio
*/

/*!
	Tests for first match routers.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <iterator>

#include <restinio/router/express.hpp>

using namespace restinio;
using namespace restinio::router;

request_handle_t
create_fake_request( std::string target, http_method_t method = http_method_get() )
{
	return
		std::make_shared< request_t >(
			0,
			http_request_header_t{ method, std::move( target ) },
			"",
			connection_handle_t{} );
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
		[&]( auto r, auto p ){
			last_handler_called = 0;
			route_params = std::move( p );
			return request_accepted();
		} );


	router.http_get(
		"/b-route/:id",
		[&]( auto r, auto p ){
			last_handler_called = 1;
			route_params = std::move( p );
			return request_accepted();
		} );

	router.http_get(
		"/c-route/:id",
		[&]( auto r, auto p ){
			last_handler_called = 2;
			route_params = std::move( p );
			return request_accepted();
		} );

	router.http_get(
		"/d-route/:id",
		[&]( auto r, auto p ){
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
		[&]( auto r, auto p ){
			last_handler_called = 0;
			route_params = std::move( p );
			return request_accepted();
		} );


	router.http_get(
		R"(/b-route/(\d+)/ending)",
		[&]( auto r, auto p ){
			last_handler_called = 1;
			route_params = std::move( p );
			return request_accepted();
		} );

	router.http_get(
		R"(/c-route/(\d+)/ending)",
		[&]( auto r, auto p ){
			last_handler_called = 2;
			route_params = std::move( p );
			return request_accepted();
		} );

	router.http_get(
		R"(/d-route/(\d+)/ending)",
		[&]( auto r, auto p ){
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
		[&]( auto r, auto p ){
			last_handler_called = 0;
			route_params = std::move( p );
			return request_accepted();
		} );


	REQUIRE( request_accepted() == router( create_fake_request( "/717/abcd/99.AA/x" ) ) );
	REQUIRE( 0 == extract_last_handler_called() );

	const auto & nps = route_params.named_parameters();
	REQUIRE( 4 == route_params.named_parameters().size() );
	REQUIRE( route_params.named_parameters().at( "p1" ) == "717" );
	REQUIRE( route_params.named_parameters().at( "p2" ) == "abcd" );
	REQUIRE( route_params.named_parameters().at( "p3" ) == "99.AA" );
	REQUIRE( route_params.named_parameters().at( "opt" ) == "x" );
}

