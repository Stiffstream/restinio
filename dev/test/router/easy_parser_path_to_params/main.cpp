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

namespace epr = restinio::router::easy_parser_router;

#include "../fake_connection_and_request.ipp"
#include "../../common/test_extra_data_factory.ipp"

template< typename Router >
void
tc_one_parameter()
{
	int last_handler_called = -1;

	auto extract_last_handler_called = [&]{
		int result = last_handler_called;
		last_handler_called = -1;
		return result;
	};

	Router router;

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

	REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/xxx" ) ) );
	REQUIRE( -1 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router(
			create_fake_request( router, "/a-route/42" ) ) );
	REQUIRE( 0 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router(
			create_fake_request( router, "/b-route/42" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router(
			create_fake_request( router, "/c-route/42" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router(
			create_fake_request( router, "/d-route/42" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
}

TEST_CASE( "one parameter (no_user_data)" , "[path_to_params][no_user_data]" )
{
	tc_one_parameter< restinio::router::easy_parser_router_t >();
}

TEST_CASE( "one parameter (test_user_data)" , "[path_to_params][test_user_data]" )
{
	tc_one_parameter< restinio::router::generic_easy_parser_router_t<
		test::ud_factory_t
	> >();
}

template< typename Router >
void
tc_one_parameter_and_http_methods()
{
	int last_handler_called = -1;

	auto extract_last_handler_called = [&]{
		int result = last_handler_called;
		last_handler_called = -1;
		return result;
	};

	Router router;

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

	REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/xxx" ) ) );
	REQUIRE( -1 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router(
			create_fake_request( router, "/a-route/42" ) ) );
	REQUIRE( 0 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router(
			create_fake_request( router, "/b-route/42" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router(
			create_fake_request( router, "/c-route/42" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router(
			create_fake_request( router, "/d-route/42" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
}

TEST_CASE( "one parameter and http_* methods (no_user_data)" ,
	"[path_to_params][no_user_data]" )
{
	tc_one_parameter_and_http_methods<
		restinio::router::easy_parser_router_t
	>();
}

TEST_CASE( "one parameter and http_* methods (test_user_data)" ,
	"[path_to_params][test_user_data]" )
{
	tc_one_parameter_and_http_methods<
		restinio::router::generic_easy_parser_router_t< test::ud_factory_t >
	>();
}

template< typename Router >
void
tc_two_parameters()
{
	Router router;

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
			create_fake_request( router, "/api/v1/books/123/versions/4386" ) ) );
	REQUIRE( request_accepted() == router(
			create_fake_request( router, "/api/v1/books/123/versions/4386/" ) ) );
}

TEST_CASE( "two parameters (no_user_data)" , "[path_to_params][no_user_data]" )
{
	tc_two_parameters< restinio::router::easy_parser_router_t >();
}

TEST_CASE( "two parameters (test_user_data)" , "[path_to_params][test_user_data]" )
{
	tc_two_parameters< restinio::router::generic_easy_parser_router_t<
		test::ud_factory_t
	> >();
}

template< typename Router >
void
tc_no_parameters()
{
	Router router;

	router.add_handler(
		restinio::http_method_get(),
		epr::path_to_params( "/api/v1/books" ),
		[&]( const auto & ){
			return request_accepted();
		} );

	REQUIRE( request_accepted() == router(
			create_fake_request( router, "/api/v1/books" ) ) );
	REQUIRE( request_accepted() == router(
			create_fake_request( router, "/api/v1/books/" ) ) );
}

TEST_CASE( "no parameters (no_user_data)" , "[path_to_params][no_user_data]" )
{
	tc_no_parameters< restinio::router::easy_parser_router_t >();
}

TEST_CASE( "no parameters (test_user_data)" , "[path_to_params][test_user_data]" )
{
	tc_no_parameters< restinio::router::generic_easy_parser_router_t<
		test::ud_factory_t
	> >();
}

template< typename Router >
void
tc_http_method_matchers()
{
	http_method_id_t last_http_method = http_method_unknown();

	auto extract_last_http_method = [&]{
		http_method_id_t result = last_http_method;
		last_http_method = http_method_unknown();
		return result;
	};

	SECTION( "any_of_methods" )
	{
		Router router;

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
			create_fake_request( router, "/user", http_method_delete() ) ) );
		REQUIRE( http_method_delete() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/user", http_method_get() ) ) );
		REQUIRE( http_method_get() == extract_last_http_method() );

		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/user", http_method_head() ) ) );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/user", http_method_post() ) ) );
		REQUIRE( http_method_post() == extract_last_http_method() );

		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/user", http_method_put() ) ) );
	}

	SECTION( "dynamic_any_of_methods-1" )
	{
		Router router;

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
			create_fake_request( router, "/user", http_method_delete() ) ) );
		REQUIRE( http_method_delete() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/user", http_method_get() ) ) );
		REQUIRE( http_method_get() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/user", http_method_head() ) ) );
		REQUIRE( http_method_head() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/user", http_method_post() ) ) );
		REQUIRE( http_method_post() == extract_last_http_method() );

		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/user", http_method_put() ) ) );
		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/user", http_method_copy() ) ) );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/status", http_method_delete() ) ) );
		REQUIRE( http_method_delete() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/status", http_method_get() ) ) );
		REQUIRE( http_method_get() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/status", http_method_head() ) ) );
		REQUIRE( http_method_head() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/status", http_method_post() ) ) );
		REQUIRE( http_method_post() == extract_last_http_method() );

		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/status", http_method_put() ) ) );
		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/status", http_method_copy() ) ) );
	}

	SECTION( "dynamic_any_of_methods-2" )
	{
		Router router;

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
			create_fake_request( router, "/user", http_method_delete() ) ) );
		REQUIRE( http_method_delete() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/user", http_method_get() ) ) );
		REQUIRE( http_method_get() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/user", http_method_head() ) ) );
		REQUIRE( http_method_head() == extract_last_http_method() );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/user", http_method_post() ) ) );
		REQUIRE( http_method_post() == extract_last_http_method() );

		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/user", http_method_put() ) ) );
		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/user", http_method_copy() ) ) );
	}

	SECTION( "none_of_methods" )
	{
		Router router;

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

		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/user", http_method_delete() ) ) );

		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/user", http_method_get() ) ) );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/user", http_method_head() ) ) );
		REQUIRE( http_method_head() == extract_last_http_method() );

		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/user", http_method_post() ) ) );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/user", http_method_put() ) ) );
		REQUIRE( http_method_put() == extract_last_http_method() );
	}

	SECTION( "dynamic_none_of_methods-1" )
	{
		Router router;

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

		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/user", http_method_delete() ) ) );

		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/user", http_method_get() ) ) );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/user", http_method_head() ) ) );
		REQUIRE( http_method_head() == extract_last_http_method() );

		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/user", http_method_post() ) ) );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/user", http_method_put() ) ) );
		REQUIRE( http_method_put() == extract_last_http_method() );
	}

	SECTION( "dynamic_none_of_methods-2" )
	{
		Router router;

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

		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/user", http_method_delete() ) ) );

		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/user", http_method_get() ) ) );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/user", http_method_head() ) ) );
		REQUIRE( http_method_head() == extract_last_http_method() );

		REQUIRE( request_not_handled() == router(
			create_fake_request( router, "/user", http_method_post() ) ) );

		REQUIRE( request_accepted() == router(
			create_fake_request( router, "/user", http_method_put() ) ) );
		REQUIRE( http_method_put() == extract_last_http_method() );
	}
}

TEST_CASE( "Http method matchers (no_user_data)" ,
		"[easy_parser][http_method_matchers][no_user_data]" )
{
	tc_http_method_matchers< restinio::router::easy_parser_router_t >();
}

TEST_CASE( "Http method matchers (test_user_data)" ,
		"[easy_parser][http_method_matchers][test_user_data]" )
{
	tc_http_method_matchers< restinio::router::generic_easy_parser_router_t<
		test::ud_factory_t
	> >();
}

