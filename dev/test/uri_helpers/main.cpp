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

	{
		const std::string input_data{ "+%20+" };
		const std::string expected_result{ "   "};

		std::string result;

		REQUIRE_NOTHROW( result = restinio::unescape_percent_encoding( input_data ) );
		REQUIRE( expected_result == result );
	}
}

TEST_CASE( "Parse query params" , "[parse_query_string]" )
{
	{
		const std::string
			uri{ "/locations/25/avg?"
				"toDate=815875200&"
				"fromDate=1133136000&"
				"toAge=38&"
				"gender=f" };

		auto params = restinio::parse_query_string< std::map< std::string, std::string > >( uri );

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

		auto params = restinio::parse_query_string( uri );

		REQUIRE( 1 == params.size() );

		REQUIRE( 1 == params.count( "country" ) );
		REQUIRE( params[ "country" ] == "\xD0\x9C\xD0\xB0\xD0\xBB\xD1\x8C\xD1\x82\xD0\xB0" );
	}

	{
		const std::string
			uri{ "/users/36/visits?"
				"my%20name=my%20value" };

		auto params = restinio::parse_query_string( uri );

		REQUIRE( 1 == params.size() );

		REQUIRE( 1 == params.count( "my name" ) );
		REQUIRE( params[ "my name" ] == "my value" );
	}

	{
		const std::string
			uri{ "/users/36/visits?"
				"k1=v1&k2=v2#fragment=value" };

		auto params = restinio::parse_query_string( uri );

		REQUIRE( 2 == params.size() );

		REQUIRE( params[ "k1" ] == "v1" );
		REQUIRE( params[ "k2" ] == "v2" );
	}
}

TEST_CASE( "Parse get params to std::multi_map" , "[parse_query_string_multi_map]" )
{
	using multimap_t = std::multimap< std::string, std::string >;

	{
		const std::string
			uri{ "/locations/25/avg?"
				"toDate=815875200&"
				"fromDate=1133136000&"
				"toAge=38&"
				"gender=f" };

		auto params = restinio::parse_query_string< multimap_t >( uri );

		REQUIRE( 4 == params.size() );

		REQUIRE( 1 == params.count( "toDate" ) );
		REQUIRE( params.find( "toDate" )->second == "815875200" );
		REQUIRE( 1 == params.count( "fromDate" ) );
		REQUIRE( params.find( "fromDate" )->second == "1133136000" );
		REQUIRE( 1 == params.count( "toAge" ) );
		REQUIRE( params.find( "toAge" )->second == "38" );
		REQUIRE( 1 == params.count( "gender" ) );
		REQUIRE( params.find( "gender" )->second == "f" );
	}
	{
		const std::string
			uri{ "/users/36/visits?"
				"country=%D0%9C%D0%B0%D0%BB%D1%8C%D1%82%D0%B0" };

		auto params = restinio::parse_query_string< multimap_t >( uri );

		REQUIRE( 1 == params.size() );

		REQUIRE( 1 == params.count( "country" ) );
		REQUIRE( params.find( "country" )->second == "\xD0\x9C\xD0\xB0\xD0\xBB\xD1\x8C\xD1\x82\xD0\xB0" );
	}

	{
		const std::string
			uri{ "/users/36/visits?"
				"my%20name=my%20value" };

		auto params = restinio::parse_query_string< multimap_t >( uri );

		REQUIRE( 1 == params.size() );

		REQUIRE( 1 == params.count( "my name" ) );
		REQUIRE( params.find( "my name" )->second == "my value" );
	}

	{
		const std::string
			uri{ "/users/36/visits?"
				"k1=v1&k2=v2#fragment=value" };

		auto params = restinio::parse_query_string< multimap_t >( uri );

		REQUIRE( 2 == params.size() );

		REQUIRE( 1 == params.count( "k1" ) );
		REQUIRE( params.find( "k1" )->second == "v1" );
		REQUIRE( 1 == params.count( "k2" ) );
		REQUIRE( params.find( "k2" )->second == "v2" );
	}

	{
		const std::string
			uri{ "/users/36/visits?"
				"k1=v1&k1=v2#fragment" };

		auto params = restinio::parse_query_string< multimap_t >( uri );

		REQUIRE( 2 == params.size() );

		REQUIRE( 2 == params.count( "k1" ) );
		auto it = params.find( "k1" );
		REQUIRE( it->second == "v1" );
		++it;
		REQUIRE( it->second == "v2" );
	}
}
