/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/media-type.hpp>

TEST_CASE( "Media-Type", "[media-type]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = media_type_value_t::try_parse(
				"" );

		REQUIRE( !result );
	}

	{
		const auto result = media_type_value_t::try_parse(
				"text/" );

		REQUIRE( !result );
	}

	{
		const auto result = media_type_value_t::try_parse(
				"/plain" );

		REQUIRE( !result );
	}

	{
		const auto result = media_type_value_t::try_parse(
				"text/plain" );

		REQUIRE( result );

		REQUIRE( "text" == result->type );
		REQUIRE( "plain" == result->subtype );
		REQUIRE( result->parameters.empty() );
	}

	{
		const auto result = media_type_value_t::try_parse(
				"TexT/pLAIn" );

		REQUIRE( result );

		REQUIRE( "text" == result->type );
		REQUIRE( "plain" == result->subtype );
		REQUIRE( result->parameters.empty() );
	}

	{
		const auto result = media_type_value_t::try_parse(
				"text/*; CharSet=utf-8 ;    Alternative-Coding=\"Bla Bla Bla\"" );

		REQUIRE( result );

		REQUIRE( "text" == result->type );
		REQUIRE( "*" == result->subtype );

		media_type_value_t::parameter_container_t expected{
			{ "charset"s, "utf-8"s },
			{ "alternative-coding"s, "Bla Bla Bla"s }
		};
		REQUIRE( expected == result->parameters );
	}

	{
		const auto result = media_type_value_t::try_parse(
				"*/*;CharSet=utf-8;Alternative-Coding=\"Bla Bla Bla\";foO=BaZ" );

		REQUIRE( result );

		REQUIRE( "*" == result->type );
		REQUIRE( "*" == result->subtype );

		media_type_value_t::parameter_container_t expected{
			{ "charset"s, "utf-8"s },
			{ "alternative-coding"s, "Bla Bla Bla"s },
			{ "foo"s, "BaZ"s }
		};
		REQUIRE( expected == result->parameters );
	}
}

