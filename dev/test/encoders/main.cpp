/*
	restinio
*/

/*!
	Tests for settings parameters that have default constructor.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <restinio/impl/base64.hpp>
#include <restinio/impl/sha1.hpp>

using namespace restinio;

TEST_CASE(
	"Is base64 char" ,
	"[encoders][base64]" )
{
	REQUIRE( is_base64_char('A') );
	REQUIRE( is_base64_char('a') );
	REQUIRE( is_base64_char('+') );
	REQUIRE( is_base64_char('/') );

	REQUIRE_FALSE( is_base64_char('\n') );
}

TEST_CASE(
	"Base64 encode" ,
	"[encoders][base64]" )
{
	{
		std::string str{"Man"};
		REQUIRE( base64_encode( str ) == "TWFu" );
	}
	{
		std::string str{"Many"};
		REQUIRE( base64_encode( str ) == "TWFueQ==" );
	}
	{
		std::string str{"Money"};
		REQUIRE( base64_encode( str ) == "TW9uZXk=" );
	}
}

TEST_CASE(
	"Base64 decode" ,
	"[encoders][base64]" )
{
	{
		std::string str{ "\r\n" };
		REQUIRE_THROWS( base64_decode( str ) );
	}
	{
		std::string str{"TWFu"};
		REQUIRE( base64_decode( str ) == "Man" );
	}
	{
		std::string str{"TWFueQ=="};
		REQUIRE( base64_decode( str ) == "Many" );
	}
	{
		std::string str{"TW9uZXk="};
		REQUIRE( base64_decode( str ) == "Money" );
	}
}

TEST_CASE(
	"SHA-1 encode" ,
	"[encoders][sha-1]" )
{
	auto digest = restinio::impl::sha1::make_digest("sha", 3);

	std::cout << restinio::impl::sha1::to_hex_string( digest ) << std::endl;

}