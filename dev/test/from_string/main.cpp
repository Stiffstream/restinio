/*
	restinio
*/

/*!
	Tests for from_string functions.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <restinio/utils/from_string.hpp>

using namespace restinio::utils;

TEST_CASE( "int8" , "[int8]" )
{
	using type_t = std::int8_t;
	type_t v;
	std::string str;


	str = "-128";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == -128 );

	str = "-127";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == -127 );

	str = "-100";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == -100 );

	str = "-99";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == -99 );

	str = "-12";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == -12 );

	str = "-3";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == -3 );

	str = "-2";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == -2 );

	str = "-0";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 0 );

	str = "+2";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 2 );
	str = "2";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 2 );

	str = "+3";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 3 );
	str = "3";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 3 );

	str = "+13";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 13 );
	str = "13";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 13 );

	str = "+13";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 13 );
	str = "13";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 13 );

	str = "+97";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 97 );
	str = "97";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 97 );

	str = "+125";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 125 );
	str = "125";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 125 );

	str = "+126";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 126 );
	str = "126";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 126 );

	str = "+127";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 127 );
	str = "127";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 127 );


	str = "+128";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "128";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "-129";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "-150";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );

	str = "255";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "-200";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "+321";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "400";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );

	str = "1000";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "-1000";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "+1000";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
}

TEST_CASE( "uint8" , "[uint8]" )
{
	using type_t = std::uint8_t;
	type_t v;
	std::string str;

	str = "+0";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 0 );
	str = "0";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 0 );

	str = "+2";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 2 );
	str = "2";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 2 );

	str = "+3";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 3 );
	str = "3";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 3 );

	str = "+13";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 13 );
	str = "13";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 13 );

	str = "+13";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 13 );
	str = "13";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 13 );

	str = "+97";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 97 );
	str = "97";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 97 );

	str = "+125";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 125 );
	str = "125";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 125 );

	str = "+126";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 126 );
	str = "126";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 126 );

	str = "+127";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 127 );
	str = "127";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 127 );

	str = "+128";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 128 );
	str = "128";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 128 );

	str = "+255";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 255 );
	str = "255";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 255 );

	str = "+200";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 200 );
	str = "200";
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == 200 );


	str = "+328";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "328";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "-829";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "-1";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );

	str = "256";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "-200";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "+321";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "400";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );

	str = "1000";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "-1000";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "+1000";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
}
