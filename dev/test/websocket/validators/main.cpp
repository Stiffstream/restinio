/*
	restinio
*/

/*!
	Tests for specific websocket protocol validations.
*/

#define CATCH_CONFIG_MAIN

#include <bitset>

#include <catch/catch.hpp>

#include <restinio/websocket/impl/utf8.hpp>

#include <test/common/pub.hpp>
#include <test/websocket/common/pub.hpp>

using namespace restinio;
using namespace restinio::websocket;
using namespace restinio::websocket::impl;

TEST_CASE(
	"UTF-8 check" ,
	"[validators][utf-8]" )
{
	{
		// Hello-µ@ßöäüàá-UTF-8!!
		std::string str{ to_char_each({
				0x48,
				0x65,
				0x6c,
				0x6c,
				0x6f,
				0x2d,
				0xc2,
				0xb5,
				0x40,
				0xc3,
				0x9f,
				0xc3,
				0xb6,
				0xc3,
				0xa4,
				0xc3,
				0xbc,
				0xc3,
				0xa0,
				0xc3,
				0xa1,
				0x2d,
				0x55,
				0x54,
				0x46,
				0x2d,
				0x38,
				0x21,
				0x21
			}) };

		// for( auto ch: str )
		// {
		// 	std::cout << std::bitset<8>(ch) << std::endl;
		// }

		REQUIRE( restinio::websocket::impl::check_utf8_is_correct( str ) == true );
	}
	{
		std::string str{ to_char_each({
				0xce,
				0xba,
				0xe1,
				0xbd,
				0xb9,
				0xcf,
				0x83,
				0xce,
				0xbc,
				0xce,
				0xb5,
				0xed,
				0xa0,
				0x80,
				0x65,
				0x64,
				0x69,
				0x74,
				0x65,
				0x64
			}) };

		REQUIRE( restinio::websocket::impl::check_utf8_is_correct( str ) == false );
	}
	{
		std::string str{ to_char_each({
				0xf8, 0x88, 0x80, 0x80, 0x80
			}) };

		REQUIRE( restinio::websocket::impl::check_utf8_is_correct( str ) == false );
	}
	{
		std::string str{ to_char_each({
				0xed, 0x9f, 0xbf
			}) };

		REQUIRE( restinio::websocket::impl::check_utf8_is_correct( str ) == true );
	}
	{
		std::string str{ to_char_each({
				0xf0, 0x90, 0x80, 0x80
			}) };

		for( auto ch: str )
		{
			std::cout << std::bitset<8>(ch) << std::endl;
		}

		REQUIRE( restinio::websocket::impl::check_utf8_is_correct( str ) == true );
	}
}
