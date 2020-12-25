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

	auto check_route_params = []( const auto & tuple ) {
			REQUIRE( 42 == std::get<0>(tuple) );
	};

	auto id_p = epr::non_negative_decimal_number_p<int>();

	router.add_handler(
		restinio::http_method_get(),
		epr::path_to_tuple( "/a-route/", id_p ),
		[&]( const auto &, auto p ){
			last_handler_called = 0;
			check_route_params( p );
			return request_accepted();
		} );

	router.add_handler(
		restinio::http_method_get(),
		epr::path_to_tuple( "/b-route/", id_p ),
		[&]( const auto &, auto p ){
			last_handler_called = 1;
			check_route_params( p );
			return request_accepted();
		} );

	router.add_handler(
		restinio::http_method_get(),
		epr::path_to_tuple( "/c-route/", id_p ),
		[&]( const auto &, auto p ){
			last_handler_called = 2;
			check_route_params( p );
			return request_accepted();
		} );

	router.add_handler(
		restinio::http_method_get(),
		epr::path_to_tuple( "/d-route/", id_p ),
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

TEST_CASE( "one parameter (no_user_data)" , "[path_to_tuple][no_user_data]" )
{
	tc_one_parameter< restinio::router::easy_parser_router_t >();
}

TEST_CASE( "one parameter (test_user_data)" , "[path_to_tuple][test_user_data]" )
{
	tc_one_parameter< restinio::router::generic_easy_parser_router_t<
			test::ud_factory_t
	> >();
}

template< typename Router >
void
tc_two_parameters()
{
	Router router;

	auto id_p = epr::non_negative_decimal_number_p<int>();

	router.add_handler(
		restinio::http_method_get(),
		epr::path_to_tuple( "/api/v1/books/", id_p, "/versions/", id_p ),
		[&]( const auto &, auto p ){
			REQUIRE( 123 == std::get<0>(p) );
			REQUIRE( 4386 == std::get<1>(p) );

			return request_accepted();
		} );

	REQUIRE( request_accepted() == router(
			create_fake_request( router, "/api/v1/books/123/versions/4386" ) ) );
	REQUIRE( request_accepted() == router(
			create_fake_request( router, "/api/v1/books/123/versions/4386/" ) ) );
}

TEST_CASE( "two parameters (no_user_data)" , "[path_to_tuple][no_user_data]" )
{
	tc_two_parameters< restinio::router::easy_parser_router_t >();
}

TEST_CASE( "two parameters (test_user_data)" , "[path_to_tuple][test_user_data]" )
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
		epr::path_to_tuple( "/api/v1/books" ),
		[&]( const auto &, auto p ){
			static_assert(std::is_same<std::tuple<>, decltype(p)>::value,
					"std::tuple<> is expected");

			return request_accepted();
		} );

	REQUIRE( request_accepted() == router(
			create_fake_request( router, "/api/v1/books" ) ) );
	REQUIRE( request_accepted() == router(
			create_fake_request( router, "/api/v1/books/" ) ) );
}

TEST_CASE( "no parameters (no_user_data)" , "[path_to_tuple][no_user_data]" )
{
	tc_no_parameters< restinio::router::easy_parser_router_t >();
}

TEST_CASE( "no parameters (test_user_data)" , "[path_to_tuple][test_user_data]" )
{
	tc_no_parameters< restinio::router::generic_easy_parser_router_t<
			test::ud_factory_t
	> >();
}

