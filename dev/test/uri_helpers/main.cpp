/*
	restinio
*/

/*!
	Tests for settings parameters that have default constructor.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <restinio/uri_helpers.hpp>

using namespace restinio;

TEST_CASE( "Escape percent encoding" , "[escape][percent_encoding]" )
{
	{
		const std::string input_data{
			"0123456789"
			"abcdefghijklmnopqrstuvwxyz"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"-~._" };
		const std::string expected_result{ input_data };

		std::string result;

		REQUIRE_NOTHROW( result = restinio::escape_percent_encoding( input_data ) );

		REQUIRE( expected_result == result );
	}
	{
		const std::string input_data{
			"0123456789" " "
			"abcdefghijklmnopqrstuvwxyz" "\r\n"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ" ";"
			"-~._" };
		const std::string expected_result{
			"0123456789" "%20"
			"abcdefghijklmnopqrstuvwxyz" "%0D%0A"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ" "%3B"
			"-~._" };

		std::string result;

		REQUIRE_NOTHROW( result = restinio::escape_percent_encoding( input_data ) );

		REQUIRE( expected_result == result );
	}
}

TEST_CASE( "Unescape percent encoding" , "[unescape][percent_encoding]" )
{
	{
		const std::string input_data{
			"0123456789"
			"abcdefghijklmnopqrstuvwxyz"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"-~._" };
		const std::string expected_result{ input_data };

		std::string result;

		REQUIRE_NOTHROW( result = restinio::unescape_percent_encoding( input_data ) );

		REQUIRE( expected_result == result );
	}
	{
		const std::string input_data{
			"0123456789" "%20"
			"abcdefghijklmnopqrstuvwxyz" "%0D%0A"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ" "%3B"
			"-~._" };
		const std::string expected_result{
			"0123456789" " "
			"abcdefghijklmnopqrstuvwxyz" "\r\n"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ" ";"
			"-~._" };

		std::string result;

		REQUIRE_NOTHROW( result = restinio::unescape_percent_encoding( input_data ) );

		REQUIRE( expected_result == result );
	}

	{
		const std::string input_data{
			"0123456789" "%20"
			"abcdefghijklmnopqrstuvwxyz" "%0d%0a"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ" "%3b"
			"-~._" };
		const std::string expected_result{
			"0123456789" " "
			"abcdefghijklmnopqrstuvwxyz" "\r\n"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ" ";"
			"-~._" };

		std::string result;

		REQUIRE_NOTHROW( result = restinio::unescape_percent_encoding( input_data ) );

		REQUIRE( expected_result == result );
	}

	{
		const std::string input_data{
			"0123456789" "%20"
			"abcdefghijklmnopqrstuvwxyz" "%0D%ZA"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ" "%3B"
			"-~._" };

		std::string result;

		REQUIRE_THROWS( result = restinio::unescape_percent_encoding( input_data ) );
	}

	{
		const std::string input_data{
			"0123456789" "%20"
			"abcdefghijklmnopqrstuvwxyz" "%0D%" };

		std::string result;

		REQUIRE_THROWS( result = restinio::unescape_percent_encoding( input_data ) );
	}
	{
		const std::string input_data{
			"0123456789" "%20"
			"abcdefghijklmnopqrstuvwxyz" "%0D%0" };

		std::string result;

		REQUIRE_THROWS( result = restinio::unescape_percent_encoding( input_data ) );
	}
}



TEST_CASE( "Parse get params" , "[parse_get_params]" )
{
	{
		const std::string
			uri{ "/locations/25/avg?"
				"toDate=815875200&"
				"fromDate=1133136000&"
				"toAge=38&"
				"gender=f" };

		auto params = restinio::parse_get_params< std::map< std::string, std::string > >( uri );

		REQUIRE( 4 == params.size() );

		REQUIRE( 1 == params.count( "toDate" ) );
		REQUIRE( params[ "toDate" ] == "815875200" );
		REQUIRE( params[ "fromDate" ] == "1133136000" );
		REQUIRE( params[ "toAge" ] == "38" );
		REQUIRE( params[ "gender" ] == "f" );
	}
	{
		const std::string
			uri{ "/users/36/visits?"
				"country=%D0%9C%D0%B0%D0%BB%D1%8C%D1%82%D0%B0" };

		auto params = restinio::parse_get_params( uri );

		REQUIRE( 1 == params.size() );

		REQUIRE( 1 == params.count( "country" ) );
		REQUIRE( params[ "country" ] == "\xD0\x9C\xD0\xB0\xD0\xBB\xD1\x8C\xD1\x82\xD0\xB0" );
	}

	{
		const std::string
			uri{ "/users/36/visits?"
				"my%20name=my%20value" };

		auto params = restinio::parse_get_params( uri );

		REQUIRE( 1 == params.size() );

		REQUIRE( 1 == params.count( "my name" ) );
		REQUIRE( params[ "my name" ] == "my value" );
	}
}
