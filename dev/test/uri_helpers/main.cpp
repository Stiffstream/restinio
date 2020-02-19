/*
	restinio
*/

/*!
	Uri helpers.
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>

using namespace restinio;

TEST_CASE( "Escape percent encoding" , "[escape][percent_encoding]" )
{
	{
		const std::string input_data{
			"0123456789"
			"abcdefghijklmnopqrstuvwxyz"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"-~._" };
		const std::string& expected_result{ input_data };

		std::string result;

		REQUIRE_NOTHROW( result = restinio::utils::escape_percent_encoding( input_data ) );

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

		REQUIRE_NOTHROW( result = restinio::utils::escape_percent_encoding( input_data ) );

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
		const std::string& expected_result{ input_data };

		std::string result;

		REQUIRE_NOTHROW( result = restinio::utils::unescape_percent_encoding( input_data ) );

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

		REQUIRE_NOTHROW( result = restinio::utils::unescape_percent_encoding( input_data ) );

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

		REQUIRE_NOTHROW( result = restinio::utils::unescape_percent_encoding( input_data ) );

		REQUIRE( expected_result == result );
	}

	{
		const std::string input_data{
			"0123456789" "%20"
			"abcdefghijklmnopqrstuvwxyz" "%0D%ZA"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ" "%3B"
			"-~._" };

		std::string result;

		REQUIRE_THROWS( result = restinio::utils::unescape_percent_encoding( input_data ) );
	}

	{
		const std::string input_data{
			"0123456789" "%20"
			"abcdefghijklmnopqrstuvwxyz" "%0D%" };

		std::string result;

		REQUIRE_THROWS( result = restinio::utils::unescape_percent_encoding( input_data ) );
	}
	{
		const std::string input_data{
			"0123456789" "%20"
			"abcdefghijklmnopqrstuvwxyz" "%0D%0" };

		std::string result;

		REQUIRE_THROWS( result = restinio::utils::unescape_percent_encoding( input_data ) );
	}

	{
		const std::string input_data{ "+%20+" };
		const std::string expected_result{ "   "};

		std::string result;

		REQUIRE_NOTHROW( result = restinio::utils::unescape_percent_encoding( input_data ) );
		REQUIRE( expected_result == result );
	}
}

TEST_CASE( "In-place unescape percent encoding" , "[unescape][percent_encoding][inplace]" )
{
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

		std::string result = input_data;

		REQUIRE_NOTHROW( result.resize(
				restinio::utils::inplace_unescape_percent_encoding(
					&result[0], result.size() ) ) );

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

		std::string result = input_data;

		REQUIRE_NOTHROW( result.resize(
				restinio::utils::inplace_unescape_percent_encoding(
					&result[0], result.size() ) ) );

		REQUIRE( expected_result == result );
	}

	{
		const std::string input_data{
			"0123456789" "%20"
			"abcdefghijklmnopqrstuvwxyz" "%0D%ZA"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ" "%3B"
			"-~._" };

		std::string result = input_data;

		REQUIRE_THROWS( result.resize(
				restinio::utils::inplace_unescape_percent_encoding(
					&result[0], result.size() ) ) );
	}

	{
		const std::string input_data{
			"0123456789" "%20"
			"abcdefghijklmnopqrstuvwxyz" "%0D%" };

		std::string result;

		REQUIRE_THROWS( result = restinio::utils::unescape_percent_encoding( input_data ) );
	}
	{
		const std::string input_data{
			"0123456789" "%20"
			"abcdefghijklmnopqrstuvwxyz" "%0D%0" };

		std::string result = input_data;

		REQUIRE_THROWS( result.resize(
				restinio::utils::inplace_unescape_percent_encoding(
					&result[0], result.size() ) ) );
	}

	{
		const std::string input_data{ "+%20+" };
		const std::string expected_result{ "   "};

		std::string result = input_data;

		REQUIRE_NOTHROW( result.resize(
				restinio::utils::inplace_unescape_percent_encoding(
					&result[0], result.size() ) ) );
		REQUIRE( expected_result == result );
	}
}

TEST_CASE( "unreserved-chars: estimate capacity",
		"[unreserved_chars][estimate_required_capacity]" )
{
	namespace uri_norm = restinio::utils::uri_normalization;
	namespace uc = uri_norm::unreserved_chars;

	{
		const auto what = restinio::string_view_t{ "/just/a/test" };
		const auto r = uc::estimate_required_capacity( what );

		REQUIRE( what.size() == r );
	}

	{
		const auto what = restinio::string_view_t{ "/just/a/~test" };
		const auto r = uc::estimate_required_capacity( what );

		REQUIRE( what.size() == r );
	}

	{
		const auto what = restinio::string_view_t{ "/just/a/%7Etest" };
		const auto r = uc::estimate_required_capacity( what );

		REQUIRE( (what.size() - 2) == r );
	}

	{
		const auto what = restinio::string_view_t{ "/j%75st/%41/~test" };
		const auto r = uc::estimate_required_capacity( what );

		REQUIRE( (what.size() - 4) == r );
	}

	{
		const auto what = restinio::string_view_t{ "/j%75st/%4/~test" };
		REQUIRE_THROWS( uc::estimate_required_capacity( what ) );
	}

	{
		const auto what = restinio::string_view_t{ "/just/a/%C5%1Fest" };
		REQUIRE_THROWS( uc::estimate_required_capacity( what ) );
	}

	{
		const auto what = restinio::string_view_t{ "/something%2Felse" };
		const auto r = uc::estimate_required_capacity( what );

		REQUIRE( what.size() == r );
	}
}

TEST_CASE( "unreserved-chars: normalize",
		"[unreserved_chars][normalize_to]" )
{
	namespace uri_norm = restinio::utils::uri_normalization;
	namespace uc = uri_norm::unreserved_chars;

	{
		const auto what = restinio::string_view_t{ "/just/a/test" };
		std::vector< char > dest( what.size(), '\x00' );
		uc::normalize_to( what, dest.data() );

		REQUIRE( restinio::string_view_t{ dest.data(), dest.size() } == what );
	}

	{
		const auto what = restinio::string_view_t{ "/just/a/~test" };
		std::vector< char > dest( what.size(), '\x00' );
		uc::normalize_to( what, dest.data() );

		REQUIRE( restinio::string_view_t{ dest.data(), dest.size() } == what );
	}

	{
		const auto what = restinio::string_view_t{ "/just%2Fa%2F~test" };
		std::vector< char > dest( what.size(), '\x00' );
		uc::normalize_to( what, dest.data() );

		REQUIRE( restinio::string_view_t{ dest.data(), dest.size() } == what );
	}

	{
		const auto what = restinio::string_view_t{ "/just/a/%7Etest" };
		const auto expected = restinio::string_view_t{ "/just/a/~test" };
		std::vector< char > dest( expected.size(), '\x00' );
		uc::normalize_to( what, dest.data() );

		REQUIRE( restinio::string_view_t{ dest.data(), dest.size() } == expected );
	}

	{
		const auto what = restinio::string_view_t{ "/just%2Fa%2F%7Etest" };
		const auto expected = restinio::string_view_t{ "/just%2Fa%2F~test" };
		std::vector< char > dest( expected.size(), '\x00' );
		uc::normalize_to( what, dest.data() );

		REQUIRE( restinio::string_view_t{ dest.data(), dest.size() } == expected );
	}

	{
		const auto what = restinio::string_view_t{ "/j%75st/%41/~test" };
		const auto expected = restinio::string_view_t{ "/just/A/~test" };
		std::vector< char > dest( expected.size(), '\x00' );
		uc::normalize_to( what, dest.data() );

		REQUIRE( restinio::string_view_t{ dest.data(), dest.size() } == expected );
	}

	{
		const auto what = restinio::string_view_t{ "/j%75st/%4/~test" };
		std::vector< char > dest( what.size(), '\x00' );
		REQUIRE_THROWS( uc::normalize_to( what, dest.data() ) );
	}

	{
		const auto what = restinio::string_view_t{ "/just/a/%C5%BEest" };
		const auto expected = restinio::string_view_t{ "/just/a/%C5%BEest" };
		std::vector< char > dest( expected.size(), '\x00' );
		uc::normalize_to( what, dest.data() );

		REQUIRE( restinio::string_view_t{ dest.data(), dest.size() } == expected );
	}

	{
		const auto what = restinio::string_view_t{ "/something%2Felse" };
		const auto expected = restinio::string_view_t{ "/something%2Felse" };
		std::vector< char > dest( expected.size(), '\x00' );
		uc::normalize_to( what, dest.data() );

		REQUIRE( restinio::string_view_t{ dest.data(), dest.size() } == expected );
	}
}

TEST_CASE( "Parse query params" , "[parse_query]" )
{
	const auto case_1 = []( const restinio::string_view_t query )
	{
		auto params = restinio::parse_query( query );

		REQUIRE( 4 == params.size() );

		REQUIRE( params.has( "toDate" ) );
		REQUIRE( params.has( "fromDate" ) );
		REQUIRE( params.has( "toAge" ) );
		REQUIRE( params.has( "gender" ) );
		REQUIRE( restinio::cast_to< std::uint64_t >( params[ "toDate" ] ) == 815875200ULL );
		REQUIRE( restinio::cast_to< std::uint64_t >( params[ "fromDate" ] ) == 1133136000ULL );
		REQUIRE( restinio::cast_to< std::uint8_t >( params[ "toAge" ] ) == 38 );
		REQUIRE( restinio::cast_to< std::string >( params[ "gender" ] ) == "f" );
	};
	case_1(
			"toDate=815875200&"
			"fromDate=1133136000&"
			"toAge=38&"
			"gender=f" );
	case_1(
			"toDate=815875200;"
			"fromDate=1133136000;"
			"toAge=38;"
			"gender=f" );
	case_1(
			"toDate=815875200&"
			"fromDate=1133136000;"
			"toAge=38&"
			"gender=f" );

	{
		const restinio::string_view_t
			query{ "country=%D0%9C%D0%B0%D0%BB%D1%8C%D1%82%D0%B0" };

		auto params = restinio::parse_query( query );

		REQUIRE( 1 == params.size() );

		REQUIRE( params.has( "country" ) );
		REQUIRE( params[ "country" ] == "\xD0\x9C\xD0\xB0\xD0\xBB\xD1\x8C\xD1\x82\xD0\xB0" );
	}

	{
		const restinio::string_view_t
			query{ "my%20name=my%20value" };

		auto params = restinio::parse_query( query );

		REQUIRE( 1 == params.size() );

		REQUIRE( params.has( "my name" ) );
		REQUIRE( params[ "my name" ] == "my value" );
	}

	const auto case_2 = []( const restinio::string_view_t query )
	{
		auto params = restinio::parse_query( query );

		REQUIRE( 2 == params.size() );

		REQUIRE( params[ "k1" ] == "v1" );
		REQUIRE( params[ "k2" ] == "v2" );
	};
	case_2( "k1=v1&k2=v2" );
	case_2( "k1=v1;k2=v2" );

	{
		const restinio::string_view_t
			query{ "name=A*" };

		using traits_t = restinio::parse_query_traits::javascript_compatible;
		auto params = restinio::parse_query< traits_t >( query );

		REQUIRE( 1 == params.size() );

		REQUIRE( params.has( "name" ) );
		REQUIRE( params[ "name" ] == "A*" );
	}

	{
		const restinio::string_view_t
			query{ "one=%3A%2F%3F%23%5B%5D%40"
					"&two=!%24%26'()*%2B%2C%3B%3D"
					"&three=-.~_*"};

		using traits_t = restinio::parse_query_traits::javascript_compatible;
		auto params = restinio::parse_query< traits_t >( query );

		REQUIRE( 3 == params.size() );

		REQUIRE( params.has( "one" ) );
		REQUIRE( params[ "one" ] == ":/?#[]@" );

		REQUIRE( params.has( "two" ) );
		REQUIRE( params[ "two" ] == "!$&'()*+,;=" );

		REQUIRE( params.has( "three" ) );
		REQUIRE( params[ "three" ] == "-.~_*" );
	}

	{
		const restinio::string_view_t
			query{ "one=A*&two.=.value&_three=_value_&fo+ur=+4+" };

		using traits_t = restinio::parse_query_traits::x_www_form_urlencoded;
		auto params = restinio::parse_query< traits_t >( query );

		REQUIRE( 4 == params.size() );

		REQUIRE( params.has( "one" ) );
		REQUIRE( params[ "one" ] == "A*" );

		REQUIRE( params.has( "two." ) );
		REQUIRE( params[ "two." ] == ".value" );

		REQUIRE( params.has( "_three" ) );
		REQUIRE( params[ "_three" ] == "_value_" );

		REQUIRE( params.has( "fo ur" ) );
		REQUIRE( params[ "fo ur" ] == " 4 " );
	}

	{
		const restinio::string_view_t
			// semicolon can't be used as separator for x-www-form-urlencoded.
			query{ "one=A*;two.=.value" };

		using traits_t = restinio::parse_query_traits::x_www_form_urlencoded;
		REQUIRE_THROWS( restinio::parse_query< traits_t >( query ) );
	}

	{
		const restinio::string_view_t
			query{ "a=(&b=)&c=[&d=]&e=!&f=,&g=;&h='&i=@&par am=z" };

		using traits_t = restinio::parse_query_traits::relaxed;
		auto params = restinio::parse_query< traits_t >( query );

		REQUIRE( 10 == params.size() );

		REQUIRE( params.has( "a" ) );
		REQUIRE( params[ "a" ] == "(" );

		REQUIRE( params.has( "b" ) );
		REQUIRE( params[ "b" ] == ")" );

		REQUIRE( params.has( "c" ) );
		REQUIRE( params[ "c" ] == "[" );

		REQUIRE( params.has( "d" ) );
		REQUIRE( params[ "d" ] == "]" );

		REQUIRE( params.has( "e" ) );
		REQUIRE( params[ "e" ] == "!" );

		REQUIRE( params.has( "f" ) );
		REQUIRE( params[ "f" ] == "," );

		REQUIRE( params.has( "g" ) );
		REQUIRE( params[ "g" ] == ";" );

		REQUIRE( params.has( "h" ) );
		REQUIRE( params[ "h" ] == "'" );

		REQUIRE( params.has( "i" ) );
		REQUIRE( params[ "i" ] == "@" );

		REQUIRE( params.has( "par am" ) );
		REQUIRE( params[ "par am" ] == "z" );
	}
	{
		auto params = restinio::parse_query<
				restinio::parse_query_traits::relaxed >( "k1==&k2===&k3====" );

		REQUIRE( 3 == params.size() );

		REQUIRE( params[ "k1" ] == "=" );
		REQUIRE( params[ "k2" ] == "==" );
		REQUIRE( params[ "k3" ] == "===" );
	}
}

TEST_CASE( "Corner cases of query string",
		"[query_string][relaxed_traits][javascript_compatible_traits]" )
{
	using default_traits = restinio::parse_query_traits::restinio_defaults;
	using js_comp_traits = restinio::parse_query_traits::javascript_compatible;
	using relaxed_traits = restinio::parse_query_traits::relaxed;
	using restinio::parse_query;

	// param%00=123
	{
		const std::string query_to_check( "param%00=123" );
		const std::string expected_param_name( "param\0", 6u );
		const std::string expected_param_value( "123" );
		{
			auto params = parse_query< default_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}

		{
			auto params = parse_query< js_comp_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}

		{
			auto params = parse_query< relaxed_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}
	}

	// par%00am=123
	{
		const std::string query_to_check( "par%00am=123" );
		const std::string expected_param_name( "par\0am", 6u );
		const std::string expected_param_value( "123" );
		{
			auto params = parse_query< default_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}

		{
			auto params = parse_query< js_comp_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}

		{
			auto params = parse_query< relaxed_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}
	}

	// param+=123
	{
		const std::string query_to_check( "param+=123" );
		const std::string expected_param_name( "param ", 6u );
		const std::string expected_param_value( "123" );
		{
			auto params = parse_query< default_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}

		{
			auto params = parse_query< js_comp_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}

		{
			auto params = parse_query< relaxed_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}
	}
	// param.=123
	{
		const std::string query_to_check( "param.=123" );
		const std::string expected_param_name( "param.", 6u );
		const std::string expected_param_value( "123" );
		{
			auto params = parse_query< default_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}

		{
			auto params = parse_query< js_comp_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}

		{
			auto params = parse_query< relaxed_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}
	}
	// param[=123
	{
		const std::string query_to_check( "param[=123" );
		const std::string expected_param_name( "param[", 6u );
		const std::string expected_param_value( "123" );
		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			auto params = parse_query< relaxed_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}
	}
	// %00=123
	{
		const std::string query_to_check( "%00=123" );
		const std::string expected_param_name( "\0", 1u );
		const std::string expected_param_value( "123" );
		{
			auto params = parse_query< default_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}

		{
			auto params = parse_query< js_comp_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}

		{
			auto params = parse_query< relaxed_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}
	}
	// 1+2
	{
		const std::string query_to_check( "1+2" );
		const std::string expected_tag( "1 2" );
		{
			auto params = parse_query< default_traits >( query_to_check );

			REQUIRE( 0 == params.size() );
			REQUIRE( params.tag() == expected_tag );
		}

		{
			auto params = parse_query< default_traits >( query_to_check );

			REQUIRE( 0 == params.size() );
			REQUIRE( params.tag() == expected_tag );
		}

		{
			auto params = parse_query< default_traits >( query_to_check );

			REQUIRE( 0 == params.size() );
			REQUIRE( params.tag() == expected_tag );
		}
	}
	// param[]=123
	{
		const std::string query_to_check( "param[]=123" );
		const std::string expected_param_name( "param[]" );
		const std::string expected_param_value( "123" );
		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			auto params = parse_query< relaxed_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}
	}
	// param[]xxx=123
	{
		const std::string query_to_check( "param[]xxx=123" );
		const std::string expected_param_name( "param[]xxx" );
		const std::string expected_param_value( "123" );
		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			auto params = parse_query< relaxed_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}
	}
	// [xxx&param=123
	{
		const std::string query_to_check( "[xxx&param=123" );
		const std::string expected_param_name( "[xxx&param" );
		const std::string expected_param_value( "123" );
		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			auto params = parse_query< relaxed_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}
	}
	// par am=123
	{
		const std::string query_to_check( "par am=123" );
		const std::string expected_param_name( "par am" );
		const std::string expected_param_value( "123" );
		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			auto params = parse_query< relaxed_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}
	}
	// param=12%3
	{
		const std::string query_to_check( "param=12%3" );
		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}
	}
	// par%am=123
	{
		const std::string query_to_check( "par%am=123" );
		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}
	}
	// %para%00m=123
	{
		const std::string query_to_check( "%para%00m=123" );
		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}
	}
	// +=123
	{
		const std::string query_to_check( "+=123" );
		const std::string expected_param_name( " " );
		const std::string expected_param_value( "123" );
		{
			auto params = parse_query< default_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}

		{
			auto params = parse_query< js_comp_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}

		{
			auto params = parse_query< relaxed_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}
	}
	// param%80=123
	{
		const std::string query_to_check( "param%80=123" );
		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< js_comp_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< relaxed_traits >( query_to_check ) );
		}
	}
	// param%FF=123
	{
		const std::string query_to_check( "param%FF=123" );
		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< js_comp_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< relaxed_traits >( query_to_check ) );
		}
	}
	// param=123%80
	{
		const std::string query_to_check( "param=123%80" );
		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< js_comp_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< relaxed_traits >( query_to_check ) );
		}
	}
	// param=123%FF
	{
		const std::string query_to_check( "param=123%FF" );
		{
			REQUIRE_THROWS( parse_query< default_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< js_comp_traits >( query_to_check ) );
		}

		{
			REQUIRE_THROWS( parse_query< relaxed_traits >( query_to_check ) );
		}
	}
	// p=Привет
	{
		const std::string query_to_check( "p=%D0%9F%D1%80%D0%B8%D0%B2%D0%B5%D1%82" );
		const std::string expected_param_name( "p" );
		const std::string expected_param_value( "Привет" );
		{
			auto params = parse_query< default_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}

		{
			auto params = parse_query< js_comp_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}

		{
			auto params = parse_query< relaxed_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}
	}
	// p=𒈙
	{
		const std::string query_to_check( "p=%F0%92%88%99" );
		const std::string expected_param_name( "p" );
		const std::string expected_param_value( "𒈙" );
		{
			auto params = parse_query< default_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}

		{
			auto params = parse_query< js_comp_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}

		{
			auto params = parse_query< relaxed_traits >( query_to_check );

			REQUIRE( 1 == params.size() );
			REQUIRE( params.has( expected_param_name ) );
			REQUIRE( params[ expected_param_name ] == expected_param_value );
		}
	}
}

TEST_CASE( "Parse get params to std::multi_map" , "[parse_query_multi_map]" )
{
	{
		const restinio::string_view_t
			query{ "toDate=815875200&"
				"fromDate=1133136000&"
				"toAge=38&"
				"gender=f" };

		auto params = restinio::parse_query( query );

		REQUIRE( 4 == params.size() );

		REQUIRE( params.has( "toDate" ) );
		REQUIRE( restinio::cast_to< std::int32_t >(params[ "toDate" ] ) == 815875200L );
		REQUIRE( params.has( "fromDate" ) );
		REQUIRE( restinio::cast_to< std::uint32_t >(params[ "fromDate" ] ) == 1133136000UL );
		REQUIRE( params.has( "toAge" ) );
		REQUIRE( restinio::cast_to< std::int8_t >(params[ "toAge" ] ) == 38 );
		REQUIRE( params.has( "gender" ) );
		REQUIRE( params[ "gender" ] == "f" );
	}
	{
		const restinio::string_view_t
			query{ "country=%D0%9C%D0%B0%D0%BB%D1%8C%D1%82%D0%B0" };

		auto params = restinio::parse_query( query );

		REQUIRE( 1 == params.size() );

		REQUIRE( params.has( "country" ) );
		REQUIRE( restinio::cast_to< std::string >( params[ "country" ] ) == "\xD0\x9C\xD0\xB0\xD0\xBB\xD1\x8C\xD1\x82\xD0\xB0" );
	}

	{
		const restinio::string_view_t
			query{ "my%20name=my%20value" };

		auto params = restinio::parse_query( query );

		REQUIRE( 1 == params.size() );

		REQUIRE( params.has( "my name" ) );
		REQUIRE( params[ "my name"] == "my value" );
	}

	{
		const restinio::string_view_t query{ "k1=v1&k2=v2" };

		auto params = restinio::parse_query( query );

		REQUIRE( 2 == params.size() );

		REQUIRE( params.has( "k1" ) );
		REQUIRE( params[ "k1" ] == "v1" );
		REQUIRE( params.has( "k2" ) );
		REQUIRE( params[ "k2" ] == "v2" );
	}
}

TEST_CASE( "value_or" , "[value_or]" )
{
	const restinio::string_view_t
		query{ "toDate=815875200&"
			"fromDate=1133136000&"
			"toAge=38&"
			"gender=f" };

	auto params = restinio::parse_query( query );

	REQUIRE( restinio::value_or< std::uint32_t >( params, "toDate", 0 ) == 815875200UL );
	REQUIRE( restinio::value_or< std::uint32_t >( params, "fromDate", 0 ) == 1133136000UL );
	REQUIRE( restinio::value_or( params, "toAge", std::uint16_t{99} ) == 38 );
	REQUIRE( restinio::value_or( params, "gender", restinio::string_view_t{"m"} ) == "f" );

	REQUIRE( restinio::value_or< std::uint32_t >( params, "does_not_exits", 42 ) == 42UL );
	REQUIRE( restinio::value_or( params, "pi", 3.14 ) == 3.14 );
	REQUIRE( restinio::value_or( params, "e", restinio::string_view_t{ "2.71828" } ) ==
															"2.71828" );
}

TEST_CASE( "opt_value" , "[opt_value]" )
{
	const restinio::string_view_t
		query{ "toDate=815875200&"
			"fromDate=1133136000&"
			"toAge=38&"
			"gender=f" };

	auto params = restinio::parse_query( query );

	REQUIRE( *restinio::opt_value< std::uint32_t >( params, "toDate" ) == 815875200UL );
	REQUIRE( *opt_value< std::uint32_t >( params, "fromDate" ) == 1133136000UL );
	REQUIRE( *opt_value< int >( params, "toAge" ) == 38 );
	REQUIRE( *opt_value< std::string >( params, "gender" ) == "f" );

	REQUIRE_FALSE( restinio::opt_value< std::uint32_t >( params, "does_not_exits" ) );
	REQUIRE_FALSE( restinio::opt_value< double >( params, "pi" ) );
	REQUIRE_FALSE( restinio::opt_value< std::string >( params, "e" ) );
}

TEST_CASE( "Query string with only web-beacon" , "[web-beacon]" )
{
	{
		const restinio::string_view_t
			query{ "a=b&123456" };

		REQUIRE_THROWS( restinio::parse_query( query ) );
	}

	{
		const restinio::string_view_t
			query{ "123456" };

		auto params = restinio::parse_query( query );

		auto tag = params.tag();
		REQUIRE( tag );
		REQUIRE( *tag == "123456" );
	}

	{
		const restinio::string_view_t
			query{ "12%33456" };

		auto params = restinio::parse_query( query );

		auto tag = params.tag();
		REQUIRE( tag );
		REQUIRE( *tag == "123456" );
	}

	{
		const restinio::string_view_t
			query{ "12%33+456" };

		auto params = restinio::parse_query( query );

		auto tag = params.tag();
		REQUIRE( tag );
		REQUIRE( *tag == "123 456" );
	}

	{
		const restinio::string_view_t
			query{ "" };

		auto params = restinio::parse_query( query );

		auto tag = params.tag();
		REQUIRE( !tag );
	}
}

