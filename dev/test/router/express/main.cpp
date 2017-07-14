/*
	restinio
*/

/*!
	Tests for express router.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <iterator>

#include <restinio/router/express.hpp>

using namespace restinio;
using namespace restinio::router;
using restinio::router::impl::route_matcher_t;

TEST_CASE( "Path to regex" , "[path2regex][simple]" )
{
	auto mather_data =
		path2regex::path2regex< route_params_t >(
			"/foo/:bar",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_t::http_get,
			std::move( mather_data.m_regex ),
			std::move( mather_data.m_param_appender_sequence ) };

	route_params_t params;

	REQUIRE_FALSE( rm.match_route( "/foo/42/q", params ) );
	REQUIRE_FALSE( rm.match_route( "/oof/42", params ) );

	REQUIRE( rm.match_route( "/foo/42", params ) );
	REQUIRE( params.match() == "/foo/42" );
	const auto & nps = params.named_parameters();
	REQUIRE( nps.size() == 1 );
	REQUIRE( nps.count( "bar" ) > 0 );
	REQUIRE( nps.at( "bar" ) == "42" );

	REQUIRE( params.indexed_parameters().size() == 0 );

}

// TEST_CASE( "Original tests" , "[path2regex][original][generated]" )
// {

// #include "original_tests.inl"

// }
