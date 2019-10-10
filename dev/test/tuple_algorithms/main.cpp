/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/utils/tuple_algorithms.hpp>

namespace tup_algo = restinio::utils::tuple_algorithms;

TEST_CASE( "all_of non-const tuple" , "[all_of][non_const]" )
{
	int calls = 0;
	const auto less_than_10 = [&calls]( auto && v ) {
		++calls;
		return v < 10;
	};

	auto t1 = std::make_tuple( 2, 3, 4, 5 );
	REQUIRE( tup_algo::all_of( t1, less_than_10 ) );
	REQUIRE( 4 == calls );

	calls = 0;
	auto t2 = std::make_tuple( 3, 4, 10, 9, 0 );
	REQUIRE( !tup_algo::all_of( t2, less_than_10 ) );
	REQUIRE( 3 == calls );
}

TEST_CASE( "all_of const tuple" , "[all_of][const]" )
{
	int calls = 0;
	const auto less_than_10 = [&calls]( auto && v ) {
		++calls;
		return v < 10;
	};

	const auto t1 = std::make_tuple( 2, 3, 4, 5 );
	REQUIRE( tup_algo::all_of( t1, less_than_10 ) );
	REQUIRE( 4 == calls );

	calls = 0;
	const auto t2 = std::make_tuple( 3, 4, 10, 9, 0 );
	REQUIRE( !tup_algo::all_of( t2, less_than_10 ) );
	REQUIRE( 3 == calls );
}

TEST_CASE( "all_of rvalue-ref tuple" , "[all_of][rvalue]" )
{
	int calls = 0;
	const auto less_than_10 = [&calls]( auto && v ) {
		++calls;
		return v < 10;
	};

	REQUIRE( tup_algo::all_of( std::make_tuple( 2, 3, 4, 5 ), less_than_10 ) );
	REQUIRE( 4 == calls );

	calls = 0;
	REQUIRE( !tup_algo::all_of( std::make_tuple( 3, 4, 5, 9, 10 ), less_than_10 ) );
	REQUIRE( 5 == calls );
}

TEST_CASE( "any_of non-const tuple" , "[any_of][non_const]" )
{
	int calls = 0;
	const auto less_than_10 = [&calls]( auto && v ) {
		++calls;
		return v < 10;
	};

	auto t1 = std::make_tuple( 12, 3, 14, 15 );
	REQUIRE( tup_algo::any_of( t1, less_than_10 ) );
	REQUIRE( 2 == calls );

	calls = 0;
	auto t2 = std::make_tuple( 13, 14, 12, 19, 10 );
	REQUIRE( !tup_algo::any_of( t2, less_than_10 ) );
	REQUIRE( 5 == calls );
}

TEST_CASE( "any_of const tuple" , "[any_of][const]" )
{
	int calls = 0;
	const auto less_than_10 = [&calls]( auto && v ) {
		++calls;
		return v < 10;
	};

	const auto t1 = std::make_tuple( 12, 3, 14, 15 );
	REQUIRE( tup_algo::any_of( t1, less_than_10 ) );
	REQUIRE( 2 == calls );

	calls = 0;
	const auto t2 = std::make_tuple( 13, 14, 12, 19, 10 );
	REQUIRE( !tup_algo::any_of( t2, less_than_10 ) );
	REQUIRE( 5 == calls );
}

TEST_CASE( "any_of rvalue-ref tuple" , "[any_of][rvalue]" )
{
	int calls = 0;
	const auto less_than_10 = [&calls]( auto && v ) {
		++calls;
		return v < 10;
	};

	REQUIRE( tup_algo::any_of( std::make_tuple( 12, 3, 14, 15 ), less_than_10 ) );
	REQUIRE( 2 == calls );

	calls = 0;
	REQUIRE( !tup_algo::any_of( std::make_tuple( 13, 14, 12, 19, 10 ), less_than_10 ) );
	REQUIRE( 5 == calls );
}

