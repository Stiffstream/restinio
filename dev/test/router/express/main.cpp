/*
	restinio
*/

/*!
	Tests for express router engine.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <iterator>

#include <restinio/router/express.hpp>

#include "usings.inl"

TEST_CASE( "Path to regex" , "[path2regex][simple]" )
{
	auto matcher_data =
		path2regex::path2regex< restinio::router::impl::route_params_appender_t, regex_engine_t >(
			"/foo/:bar",
			path2regex::options_t{} );

	route_matcher_t
		rm{
			http_method_t::http_get,
			std::move( matcher_data.m_regex ),
			std::move( matcher_data.m_named_params_buffer ),
			std::move( matcher_data.m_param_appender_sequence ) };

	route_params_t params;

	REQUIRE_FALSE( rm.match_route( "/foo/42/q", params ) );
	REQUIRE_FALSE( rm.match_route( "/oof/42", params ) );

	REQUIRE( rm.match_route( "/foo/42", params ) );
	REQUIRE( params.match() == "/foo/42" );
	const auto & nps = restinio::router::impl::route_params_accessor_t::named_parameters( params );
	REQUIRE( nps.size() == 1 );
	REQUIRE( nps[0].first == "bar" );
	REQUIRE( nps[0].second == "42" );

	const auto & ips = restinio::router::impl::route_params_accessor_t::indexed_parameters( params);
	REQUIRE( ips.size() == 0 );
}


