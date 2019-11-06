/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/content-type.hpp>

TEST_CASE( "Content-Type", "[media-type][content-type]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = content_type_value_t::try_parse(
				"text/plain" );

		REQUIRE( result );

		REQUIRE( "text" == result->media_type.type );
		REQUIRE( "plain" == result->media_type.subtype );
		REQUIRE( result->media_type.parameters.empty() );
	}

	{
		const auto result = content_type_value_t::try_parse(
				"TexT/pLAIn" );

		REQUIRE( result );

		REQUIRE( "text" == result->media_type.type );
		REQUIRE( "plain" == result->media_type.subtype );
		REQUIRE( result->media_type.parameters.empty() );
	}

	{
		const auto result = content_type_value_t::try_parse(
				"text/*; CharSet=utf-8 ;    Alternative-Coding=\"Bla Bla Bla\"" );

		REQUIRE( result );

		REQUIRE( "text" == result->media_type.type );
		REQUIRE( "*" == result->media_type.subtype );

		media_type_value_t::parameter_container_t expected{
			{ "charset"s, "utf-8"s },
			{ "alternative-coding"s, "Bla Bla Bla"s }
		};
		REQUIRE( expected == result->media_type.parameters );
	}

	{
		const auto result = content_type_value_t::try_parse(
				"*/*;CharSet=utf-8;Alternative-Coding=\"Bla Bla Bla\";foO=BaZ" );

		REQUIRE( result );

		REQUIRE( "*" == result->media_type.type );
		REQUIRE( "*" == result->media_type.subtype );

		media_type_value_t::parameter_container_t expected{
			{ "charset"s, "utf-8"s },
			{ "alternative-coding"s, "Bla Bla Bla"s },
			{ "foo"s, "BaZ"s }
		};
		REQUIRE( expected == result->media_type.parameters );
	}
}

