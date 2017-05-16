/*
	restinio
*/

/*!
	Tests for header objects.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <iterator>

#include <restinio/router/first_match.hpp>

using namespace restinio;
using namespace restinio::router;

TEST_CASE( "Exact match" , "[first_match][exact]" )
{
	REQUIRE( true );
}

