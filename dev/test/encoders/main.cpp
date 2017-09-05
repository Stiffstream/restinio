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
	REQUIRE(
		restinio::impl::sha1::to_hex_string(
			restinio::impl::sha1::make_digest("sha", 3) ) ==
		"d8f4590320e1343a915b6394170650a8f35d6926" );

	REQUIRE(
		restinio::impl::sha1::to_hex_string(
			restinio::impl::sha1::make_digest("", 0) ) ==
		"da39a3ee5e6b4b0d3255bfef95601890afd80709" );

	REQUIRE(
		restinio::impl::sha1::to_hex_string(
			restinio::impl::sha1::make_digest(
				std::string{
					"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"} ) ) ==
		"f08f24908d682555111be7ff6f004e78283d989a" );

	REQUIRE(
		restinio::impl::sha1::to_hex_string(
			restinio::impl::sha1::make_digest(
				std::string{
					"one"} ) ) ==
		"fe05bcdcdc4928012781a5f1a2a77cbb5398e106" );

	REQUIRE(
		restinio::impl::sha1::to_hex_string(
			restinio::impl::sha1::make_digest(
				std::string{
					"onet"} ) ) ==
		"e7280c732e008396d4df9d63b8acd8dc3789a170" );

	REQUIRE(
		restinio::impl::sha1::to_hex_string(
			restinio::impl::sha1::make_digest(
				std::string{
					"onetw"} ) ) ==
		"32726a35183871f7a74467e48d5ad3d5055d9153" );

	REQUIRE(
		restinio::impl::sha1::to_hex_string(
			restinio::impl::sha1::make_digest(
				std::string{
					"onetwo"} ) ) ==
		"30ae97492ce1da88d0e7117ace0a60a6f9e1e0bc" );

	REQUIRE(
		restinio::impl::sha1::to_hex_string(
			restinio::impl::sha1::make_digest(
				std::string{
					"onetwot"} ) ) ==
		"2c61a030bbfa96ad4bafa0835b5c724ae14a4e80" );

	REQUIRE(
		restinio::impl::sha1::to_hex_string(
			restinio::impl::sha1::make_digest(
				std::string{
					"onetwoth"} ) ) ==
		"a9e4624901a4835426140fb45a01f4246021bd4d" );

	REQUIRE(
		restinio::impl::sha1::to_hex_string(
			restinio::impl::sha1::make_digest(
				std::string{
					"onetwothreefourfivesixseveneightnineten"} ) ) ==
		"47e1ea4ebbfdbef768c7c403d95dc2ee170c7cce" );

	REQUIRE(
		restinio::impl::sha1::to_hex_string(
			restinio::impl::sha1::make_digest(
				std::string{
					"The quick brown fox jumps over the lazy dog"} ) ) ==
		"2fd4e1c67a2d28fced849ee1bb76e7391b93eb12" );
}