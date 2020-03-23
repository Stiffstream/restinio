/*
	restinio
*/

/*!
	Tests for express router.
*/

#include <catch2/catch.hpp>

#include <iterator>

#include <restinio/all.hpp>
#include <restinio/router/easy_parser_router.hpp>

using namespace restinio;

using router_t = restinio::router::easy_parser_router_t;
namespace epr = restinio::router::easy_parser_router;

#include "../fake_connection_and_request.ipp"

TEST_CASE( "one parameter" , "[path_to_params]" )
{
	int last_handler_called = -1;

	auto extract_last_handler_called = [&]{
		int result = last_handler_called;
		last_handler_called = -1;
		return result;
	};

	router_t router;

	auto check_route_params = []( const auto & v ) {
			REQUIRE( 42 == v );
	};

	auto id_p = epr::non_negative_decimal_number_p<int>();

	router.add_handler(
		restinio::http_method_get(),
		epr::path_to_params( "/a-route/", id_p ),
		[&]( const auto &, auto p ){
			last_handler_called = 0;
			check_route_params( p );
			return request_accepted();
		} );


	router.add_handler(
		restinio::http_method_get(),
		epr::path_to_params( "/b-route/", id_p ),
		[&]( const auto &, auto p ){
			last_handler_called = 1;
			check_route_params( p );
			return request_accepted();
		} );

	router.add_handler(
		restinio::http_method_get(),
		epr::path_to_params( "/c-route/", id_p ),
		[&]( const auto &, auto p ){
			last_handler_called = 2;
			check_route_params( p );
			return request_accepted();
		} );

	router.add_handler(
		restinio::http_method_get(),
		epr::path_to_params( "/d-route/", id_p ),
		[&]( const auto &, auto p ) {
			last_handler_called = 3;
			check_route_params( p );
			return request_accepted();
		} );

	REQUIRE( request_rejected() == router( create_fake_request( "/xxx" ) ) );
	REQUIRE( -1 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/a-route/42" ) ) );
	REQUIRE( 0 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/b-route/42" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/c-route/42" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/d-route/42" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
}

TEST_CASE( "one parameter and http_* methods" , "[path_to_params]" )
{
	int last_handler_called = -1;

	auto extract_last_handler_called = [&]{
		int result = last_handler_called;
		last_handler_called = -1;
		return result;
	};

	router_t router;

	auto check_route_params = []( const auto & v ) {
			REQUIRE( 42 == v );
	};

	auto id_p = epr::non_negative_decimal_number_p<int>();

	router.http_get(
		epr::path_to_params( "/a-route/", id_p ),
		[&]( const auto &, auto p ){
			last_handler_called = 0;
			check_route_params( p );
			return request_accepted();
		} );


	router.http_get(
		epr::path_to_params( "/b-route/", id_p ),
		[&]( const auto &, auto p ){
			last_handler_called = 1;
			check_route_params( p );
			return request_accepted();
		} );

	router.http_get(
		epr::path_to_params( "/c-route/", id_p ),
		[&]( const auto &, auto p ){
			last_handler_called = 2;
			check_route_params( p );
			return request_accepted();
		} );

	router.http_get(
		epr::path_to_params( "/d-route/", id_p ),
		[&]( const auto &, auto p ) {
			last_handler_called = 3;
			check_route_params( p );
			return request_accepted();
		} );

	REQUIRE( request_rejected() == router( create_fake_request( "/xxx" ) ) );
	REQUIRE( -1 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/a-route/42" ) ) );
	REQUIRE( 0 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/b-route/42" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/c-route/42" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/d-route/42" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
}

TEST_CASE( "two parameters" , "[path_to_params]" )
{
	router_t router;

	auto id_p = epr::non_negative_decimal_number_p<int>();

	router.add_handler(
		restinio::http_method_get(),
		epr::path_to_params( "/api/v1/books/", id_p, "/versions/", id_p ),
		[&]( const auto &, auto book_id, auto ver_id ){
			REQUIRE( 123 == book_id );
			REQUIRE( 4386 == ver_id );

			return request_accepted();
		} );

	REQUIRE( request_accepted() == router(
			create_fake_request( "/api/v1/books/123/versions/4386" ) ) );
	REQUIRE( request_accepted() == router(
			create_fake_request( "/api/v1/books/123/versions/4386/" ) ) );
}

TEST_CASE( "no parameters" , "[path_to_params]" )
{
	router_t router;

	router.add_handler(
		restinio::http_method_get(),
		epr::path_to_params( "/api/v1/books" ),
		[&]( const auto & ){
			return request_accepted();
		} );

	REQUIRE( request_accepted() == router(
			create_fake_request( "/api/v1/books" ) ) );
	REQUIRE( request_accepted() == router(
			create_fake_request( "/api/v1/books/" ) ) );
}

TEST_CASE( "Http method matchers" , "[express][http_method_matchers]" )
{
	http_method_id_t last_http_method = http_method_unknown();

	auto extract_last_http_method = [&]{
		http_method_id_t result = last_http_method;
		last_http_method = http_method_unknown();
		return result;
	};

	SECTION( "any_of_methods" )
	{
		router_t router;

		router.add_handler(
			restinio::router::any_of_methods(
				http_method_get(),
				http_method_post(),
				http_method_delete() ),
			epr::path_to_params( "/user" ),
			[&]( const auto & req ){
				last_http_method = req->header().method();
				return request_accepted();
			} );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_delete() ) ) );
		REQUIRE( http_method_delete() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_get() ) ) );
		REQUIRE( http_method_get() == extract_last_http_method() );

		REQUIRE( request_rejected() == router(
			create_fake_request( "/user", http_method_head() ) ) );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_post() ) ) );
		REQUIRE( http_method_post() == extract_last_http_method() );

		REQUIRE( request_rejected() == router(
			create_fake_request( "/user", http_method_put() ) ) );
	}

	SECTION( "dynamic_any_of_methods-1" )
	{
		router_t router;

		restinio::router::dynamic_any_of_methods_matcher_t matcher;
		matcher.add( http_method_get() );
		matcher.add( http_method_post() );
		matcher.add( http_method_head() );
		matcher.add( http_method_delete() );

		router.add_handler(
			matcher,
			epr::path_to_params( "/user" ),
			[&]( const auto & req ){
				last_http_method = req->header().method();
				return request_accepted();
			} );

		router.add_handler(
			matcher,
			epr::path_to_params( "/status" ),
			[&]( const auto & req ){
				last_http_method = req->header().method();
				return request_accepted();
			} );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_delete() ) ) );
		REQUIRE( http_method_delete() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_get() ) ) );
		REQUIRE( http_method_get() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_head() ) ) );
		REQUIRE( http_method_head() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_post() ) ) );
		REQUIRE( http_method_post() == extract_last_http_method() );

		REQUIRE( request_rejected() == router(
			create_fake_request( "/user", http_method_put() ) ) );
		REQUIRE( request_rejected() == router(
			create_fake_request( "/user", http_method_copy() ) ) );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/status", http_method_delete() ) ) );
		REQUIRE( http_method_delete() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/status", http_method_get() ) ) );
		REQUIRE( http_method_get() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/status", http_method_head() ) ) );
		REQUIRE( http_method_head() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/status", http_method_post() ) ) );
		REQUIRE( http_method_post() == extract_last_http_method() );

		REQUIRE( request_rejected() == router(
			create_fake_request( "/status", http_method_put() ) ) );
		REQUIRE( request_rejected() == router(
			create_fake_request( "/status", http_method_copy() ) ) );
	}

	SECTION( "dynamic_any_of_methods-2" )
	{
		router_t router;

		restinio::router::dynamic_any_of_methods_matcher_t matcher;
		matcher.add( http_method_get() );
		matcher.add( http_method_post() );
		matcher.add( http_method_head() );
		matcher.add( http_method_delete() );

		router.add_handler(
			std::move(matcher),
			epr::path_to_params( "/user" ),
			[&]( const auto & req ){
				last_http_method = req->header().method();
				return request_accepted();
			} );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_delete() ) ) );
		REQUIRE( http_method_delete() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_get() ) ) );
		REQUIRE( http_method_get() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_head() ) ) );
		REQUIRE( http_method_head() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_post() ) ) );
		REQUIRE( http_method_post() == extract_last_http_method() );

		REQUIRE( request_rejected() == router(
			create_fake_request( "/user", http_method_put() ) ) );
		REQUIRE( request_rejected() == router(
			create_fake_request( "/user", http_method_copy() ) ) );
	}

	SECTION( "none_of_methods" )
	{
		router_t router;

		router.add_handler(
			restinio::router::none_of_methods(
				http_method_get(),
				http_method_post(),
				http_method_delete(),
				http_method_copy(),
				http_method_lock(),
				http_method_move() ),
			epr::path_to_params( "/user" ),
			[&]( const auto & req ){
				last_http_method = req->header().method();
				return request_accepted();
			} );

		REQUIRE( request_rejected() == router(
			create_fake_request( "/user", http_method_delete() ) ) );

		REQUIRE( request_rejected() == router(
			create_fake_request( "/user", http_method_get() ) ) );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_head() ) ) );
		REQUIRE( http_method_head() == extract_last_http_method() );

		REQUIRE( request_rejected() == router(
			create_fake_request( "/user", http_method_post() ) ) );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_put() ) ) );
		REQUIRE( http_method_put() == extract_last_http_method() );
	}

	SECTION( "dynamic_none_of_methods-1" )
	{
		router_t router;

		restinio::router::dynamic_none_of_methods_matcher_t matcher;
		matcher.add( http_method_get() );
		matcher.add( http_method_post() );
		matcher.add( http_method_copy() );
		matcher.add( http_method_delete() );

		router.add_handler(
			matcher,
			epr::path_to_params( "/user" ),
			[&]( const auto & req ){
				last_http_method = req->header().method();
				return request_accepted();
			} );

		REQUIRE( request_rejected() == router(
			create_fake_request( "/user", http_method_delete() ) ) );

		REQUIRE( request_rejected() == router(
			create_fake_request( "/user", http_method_get() ) ) );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_head() ) ) );
		REQUIRE( http_method_head() == extract_last_http_method() );

		REQUIRE( request_rejected() == router(
			create_fake_request( "/user", http_method_post() ) ) );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_put() ) ) );
		REQUIRE( http_method_put() == extract_last_http_method() );
	}

	SECTION( "dynamic_none_of_methods-2" )
	{
		router_t router;

		restinio::router::dynamic_none_of_methods_matcher_t matcher;
		matcher.add( http_method_get() );
		matcher.add( http_method_post() );
		matcher.add( http_method_copy() );
		matcher.add( http_method_delete() );

		router.add_handler(
			std::move(matcher),
			epr::path_to_params( "/user" ),
			[&]( const auto & req ){
				last_http_method = req->header().method();
				return request_accepted();
			} );

		REQUIRE( request_rejected() == router(
			create_fake_request( "/user", http_method_delete() ) ) );

		REQUIRE( request_rejected() == router(
			create_fake_request( "/user", http_method_get() ) ) );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_head() ) ) );
		REQUIRE( http_method_head() == extract_last_http_method() );

		REQUIRE( request_rejected() == router(
			create_fake_request( "/user", http_method_post() ) ) );

		REQUIRE( request_accepted() == router(
			create_fake_request( "/user", http_method_put() ) ) );
		REQUIRE( http_method_put() == extract_last_http_method() );
	}
}

