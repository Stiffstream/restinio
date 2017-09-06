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

char
to_char( int val )
{
	return static_cast<char>(val);
};

std::string
to_char_each( std::vector< int > source )
{
	std::string result;
	result.reserve( source.size() );

	for( const auto & val : source )
	{
		result.push_back( to_char(val) );
	}

	return result;
}

TEST_CASE(
	"Is base64 char" ,
	"[encoders][base64]" )
{
	REQUIRE( restinio::impl::base64::is_base64_char('A') );
	REQUIRE( restinio::impl::base64::is_base64_char('a') );
	REQUIRE( restinio::impl::base64::is_base64_char('+') );
	REQUIRE( restinio::impl::base64::is_base64_char('/') );

	REQUIRE_FALSE( restinio::impl::base64::is_base64_char('\n') );
}

TEST_CASE(
	"Base64 encode" ,
	"[encoders][base64]" )
{
	{
		std::string str{"Man"};
		REQUIRE( restinio::impl::base64::encode( str ) == "TWFu" );
	}
	{
		std::string str{"Many"};
		REQUIRE( restinio::impl::base64::encode( str ) == "TWFueQ==" );
	}
	{
		std::string str{"Money"};
		REQUIRE( restinio::impl::base64::encode( str ) == "TW9uZXk=" );
	}
	{
		std::string str( to_char_each( {
			0xb3, 0x7a, 0x4f, 0x2c,
			0xc0, 0x62, 0x4f, 0x16,
			0x90, 0xf6, 0x46, 0x06,
			0xcf, 0x38, 0x59, 0x45,
			0xb2, 0xbe, 0xc4, 0xea } ) );
		REQUIRE(
			restinio::impl::base64::encode( str ) ==
			"s3pPLMBiTxaQ9kYGzzhZRbK+xOo=" );
	}
}

TEST_CASE(
	"Base64 decode" ,
	"[encoders][base64]" )
{
	{
		std::string str{ "\r\n" };
		REQUIRE_THROWS( restinio::impl::base64::decode( str ) );
	}
	{
		std::string str{"TWFu"};
		REQUIRE( restinio::impl::base64::decode( str ) == "Man" );
	}
	{
		std::string str{"TWFueQ=="};
		REQUIRE( restinio::impl::base64::decode( str ) == "Many" );
	}
	{
		std::string str{"TW9uZXk="};
		REQUIRE( restinio::impl::base64::decode( str ) == "Money" );
	}
}

TEST_CASE(
	"SHA-1 encode" ,
	"[encoders][sha-1]" )
{
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

		REQUIRE( restinio::impl::sha1::to_hex_string( restinio::impl::sha1::make_digest(
			std::string{
				"dGhlIHNhbXBsZSBub25jZQ==258EAFA5-E914-47DA-95CA-C5AB0DC85B11"} ) ) ==
			"b37a4f2cc0624f1690f64606cf385945b2bec4ea" );

		REQUIRE(
			restinio::impl::sha1::to_hex_string(
				restinio::impl::sha1::make_digest(
					std::string{
						"1234567890"
						"1234567890"
						"1234567890"
						"1234567890"
						"1234567890"
						"1234567890"
						"123"} ) ) ==
			"98b4b1764ea88d6c3fa63b70799dbd0c03372d1a" );

		REQUIRE(
			restinio::impl::sha1::to_hex_string(
				restinio::impl::sha1::make_digest(
					std::string{
						"1234567890"
						"1234567890"
						"1234567890"
						"1234567890"
						"1234567890"
						"1234567890"
						"12345"} ) ) ==
			"586e902a0ea9ad0d3a53ba09269cbd1e5f90ff01" );

		REQUIRE(
			restinio::impl::sha1::to_hex_string(
				restinio::impl::sha1::make_digest(
					std::string{
						"abcdefghbcdefghicdefghijdefghijkefghijklfghijk"
						"lmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnop"
						"qrsmnopqrstnopqrstu"} ) ) ==
			"a49b2446" "a02c645b" "f419f995" "b6709125" "3a04a259" );
	}
	{
		restinio::impl::sha1::builder_t sha1_builder;

		for( auto i = 0 ; i < 1000000 ; ++i )
		{
			std::uint8_t buf[] = {0x61};
			sha1_builder.update( buf, 1 );
		}

		REQUIRE( restinio::impl::sha1::to_hex_string( sha1_builder.finish() ) ==
			"34aa973cd4c4daa4f61eeb2bdbad27316534016f" );
	}
	{
		restinio::impl::sha1::builder_t sha1_builder;

		for( auto i = 0 ; i < 10000 ; ++i )
		{
			sha1_builder.update(
				restinio::impl::sha1::as_uint8_ptr("1234567890"), 10 );
		}

		REQUIRE( restinio::impl::sha1::to_hex_string( sha1_builder.finish() ) ==
			"6afc512e5413a02921e949912c8b5d42f1ebdd80" );
	}
}