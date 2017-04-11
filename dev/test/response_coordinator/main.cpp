/*
	restinio
*/

/*!
	Tests for header objects.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <iterator>

#include <restinio/impl/response_coordinator.hpp>

using namespace restinio::impl;

TEST_CASE( "response_context_table" , "[response_context][response_context_table]" )
{
	SECTION( "empty" )
	{
		constexpr auto max_elements_count = 16UL;

		response_context_table_t table( max_elements_count );

		REQUIRE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
		REQUIRE_THROWS( table.pop_response_context() );
		REQUIRE_THROWS( table[ 0UL ] );
		REQUIRE_THROWS( table[ 1UL ] );
	}

	SECTION( "single" )
	{
		constexpr auto max_elements_count = 1UL;

		response_context_table_t table( max_elements_count );

		REQUIRE( table.empty() );
		REQUIRE_FALSE( table.is_full() );

		CHECK_NOTHROW( table.push_response_context( 42UL ) );
		CHECK_THROWS( table.push_response_context( 43UL ) );

		REQUIRE_FALSE( table.empty() );
		REQUIRE( table.is_full() );
		REQUIRE( &table[ 42UL ] == &table.front() );
		REQUIRE( &table[ 42UL ] == &table.back() );

		CHECK_NOTHROW( table[ 42UL ] );
		CHECK_THROWS( table[ 41UL ] );
		CHECK_THROWS( table[ 43UL ] );

		REQUIRE( table[ 42UL ].m_request_id == 42UL );
		REQUIRE_FALSE( table[ 42UL ].m_response_complete );

		CHECK_NOTHROW( table.pop_response_context() );
	}

	SECTION( "max_elements_count=4" )
	{
		constexpr auto max_elements_count = 4UL;

		response_context_table_t table( max_elements_count );

		REQUIRE( table.empty() );
		REQUIRE_FALSE( table.is_full() );

		CHECK_NOTHROW( table.push_response_context( 42UL ) );

		REQUIRE_FALSE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
		REQUIRE( &table[ 42UL ] == &table.front() );
		REQUIRE( &table[ 42UL ] == &table.back() );

		CHECK_NOTHROW( table.push_response_context( 43UL ) );

		REQUIRE_FALSE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
		REQUIRE( &table[ 42UL ] == &table.front() );
		REQUIRE( &table[ 43UL ] == &table.back() );

		CHECK_NOTHROW( table.push_response_context( 44UL ) );

		REQUIRE_FALSE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
		REQUIRE( &table[ 42UL ] == &table.front() );
		REQUIRE( &table[ 44UL ] == &table.back() );

		CHECK_NOTHROW( table.push_response_context( 45UL ) );

		REQUIRE_FALSE( table.empty() );
		REQUIRE( table.is_full() );
		REQUIRE( &table[ 42UL ] == &table.front() );
		REQUIRE( &table[ 45UL ] == &table.back() );

		CHECK_THROWS( table.push_response_context( 46UL ) );

		CHECK_NOTHROW( table.pop_response_context() );

		REQUIRE_FALSE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
		REQUIRE( &table[ 43UL ] == &table.front() );
		REQUIRE( &table[ 45UL ] == &table.back() );

		CHECK_NOTHROW( table.push_response_context( 46UL ) );
		REQUIRE_FALSE( table.empty() );
		REQUIRE( table.is_full() );
		REQUIRE( &table[ 43UL ] == &table.front() );
		REQUIRE( &table[ 46UL ] == &table.back() );

		CHECK_THROWS( table.push_response_context( 47UL ) );
		CHECK_NOTHROW( table.pop_response_context() );
		CHECK_NOTHROW( table.push_response_context( 47UL ) );
		REQUIRE_FALSE( table.empty() );
		REQUIRE( table.is_full() );
		REQUIRE( &table[ 44UL ] == &table.front() );
		REQUIRE( &table[ 47UL ] == &table.back() );

		CHECK_THROWS( table.push_response_context( 48UL ) );
		CHECK_NOTHROW( table.pop_response_context() );
		CHECK_NOTHROW( table.push_response_context( 48UL ) );
		REQUIRE_FALSE( table.empty() );
		REQUIRE( table.is_full() );
		REQUIRE( &table[ 45UL ] == &table.front() );
		REQUIRE( &table[ 48UL ] == &table.back() );

		CHECK_THROWS( table.push_response_context( 49UL ) );
		CHECK_NOTHROW( table.pop_response_context() );
		CHECK_NOTHROW( table.push_response_context( 49UL ) );
		REQUIRE_FALSE( table.empty() );
		REQUIRE( table.is_full() );
		REQUIRE( &table[ 46UL ] == &table.front() );
		REQUIRE( &table[ 49UL ] == &table.back() );

		CHECK_NOTHROW( table.pop_response_context() );
		REQUIRE_FALSE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
		REQUIRE( &table[ 47UL ] == &table.front() );
		REQUIRE( &table[ 49UL ] == &table.back() );

		CHECK_NOTHROW( table.pop_response_context() );
		REQUIRE_FALSE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
		REQUIRE( &table[ 48UL ] == &table.front() );
		REQUIRE( &table[ 49UL ] == &table.back() );

		CHECK_NOTHROW( table.pop_response_context() );
		REQUIRE_FALSE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
		REQUIRE( &table[ 49UL ] == &table.front() );
		REQUIRE( &table[ 49UL ] == &table.back() );

		CHECK_NOTHROW( table.pop_response_context() );
		REQUIRE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
	}
}
