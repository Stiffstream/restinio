/*
	restinio
*/

/*!
	Tests for from_string functions.
*/

#include <catch2/catch.hpp>

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

TEST_CASE( "int16" , "[int16]" )
{
	using type_t = std::int16_t;
	type_t v;
	std::string str;

	auto i = std::numeric_limits< type_t >::min();
	do
	{
		str = std::to_string( i );
		REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
		REQUIRE( v == i );

	} while( i++ != std::numeric_limits< type_t >::max() );

	str = "+99999";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "-1233214884845664";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "--129";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "15d";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );

	str = "";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "-";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "+";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "400*400";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );

	str = "-+1000";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "+-1000";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "32769";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
}

TEST_CASE( "uint16" , "[uint16]" )
{
	using type_t = std::uint16_t;
	type_t v;
	std::string str;

	auto i = std::numeric_limits< type_t >::min();
	do
	{
		str = std::to_string( i );
		REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
		REQUIRE( v == i );

	} while( i++ != std::numeric_limits< type_t >::max() );

	str = "+99999";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "-1";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "-129";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "3#15";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );

	str = "";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "-";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "+";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "400*400";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );

	str = "asas000";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "+-1000";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
	str = "65536";
	REQUIRE_THROWS( v = from_string< type_t >( str ) );
}

TEST_CASE( "int32" , "[int32]" )
{
	using type_t = std::int32_t;
	type_t v;
	std::string str;

	auto i = std::numeric_limits< type_t >::min();
	str = std::to_string( i );
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == i );

	do
	{
		type_t r = std::rand() % 200000;
		if( i > 0 )
			i += std::min< type_t >( r, std::numeric_limits< type_t >::max() - i );
		else
			i += r;

		str = std::to_string( i );

		REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
		REQUIRE( v == i );
	} while( i != std::numeric_limits< type_t >::max() );
}

TEST_CASE( "uint32" , "[uint32]" )
{
	using type_t = std::uint32_t;
	type_t v;
	std::string str;

	auto i = std::numeric_limits< type_t >::min();
	str = std::to_string( i );
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == i );

	do
	{
		type_t r = std::rand() % 200000;
		i += std::min< type_t >( r, std::numeric_limits< type_t >::max() - i );

		str = std::to_string( i );

		REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
		REQUIRE( v == i );
	} while( i != std::numeric_limits< type_t >::max() );
}

TEST_CASE( "int64" , "[int64]" )
{
	using type_t = std::int64_t;
	type_t v;
	std::string str;

	auto i = std::numeric_limits< type_t >::min();
	str = std::to_string( i );
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == i );

	do
	{
		type_t r = std::rand() % 200000;
		r <<= 32;
		if( i > 0 )
			i += std::min< type_t >( r, std::numeric_limits< type_t >::max() - i );
		else
			i += r;

		str = std::to_string( i );

		REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
		REQUIRE( v == i );
	} while( i != std::numeric_limits< type_t >::max() );
}

TEST_CASE( "uint64" , "[uint64]" )
{
	using type_t = std::uint64_t;
	type_t v;
	std::string str;

	auto i = std::numeric_limits< type_t >::min();
	str = std::to_string( i );
	REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
	REQUIRE( v == i );

	do
	{
		type_t r = std::rand() % 200000;
		r <<= 32;
		i += std::min< type_t >( r, std::numeric_limits< type_t >::max() - i );

		str = std::to_string( i );

		REQUIRE_NOTHROW( v = from_string< type_t >( str ) );
		REQUIRE( v == i );
	} while( i != std::numeric_limits< type_t >::max() );
}
