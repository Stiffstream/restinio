/*
	restinio
*/

#include <catch2/catch_all.hpp>

#include <restinio/helpers/http_field_parsers/cache-control.hpp>

TEST_CASE( "Cache-Control Field", "[cache-control]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = cache_control_value_t::try_parse(
				"" );

		REQUIRE( !result );
	}

	{
		const auto result = cache_control_value_t::try_parse(
				"," );

		REQUIRE( !result );
	}

	{
		const auto result = cache_control_value_t::try_parse(
				",, , ,   ,  " );

		REQUIRE( !result );
	}

	{
		const auto result = cache_control_value_t::try_parse(
				"max-age=5" );

		REQUIRE( result );

		cache_control_value_t::directive_container_t expected_directives{
			{ "max-age"s, "5"s },
		};

		REQUIRE( expected_directives == result->directives );
	}

	{
		const auto result = cache_control_value_t::try_parse(
				"max-age=5, no-transform, only-if-cached, min-fresh=20" );

		REQUIRE( result );

		cache_control_value_t::directive_container_t expected_directives{
			{ "max-age"s, "5"s },
			{ "no-transform"s, std::nullopt },
			{ "only-if-cached"s, std::nullopt },
			{ "min-fresh"s, "20"s }
		};

		REQUIRE( expected_directives == result->directives );
	}

	{
		const auto result = cache_control_value_t::try_parse(
				", ,  ,   , max-age=5, ,,, no-transform, only-if-cached, min-fresh=20,,,,    " );

		REQUIRE( result );

		cache_control_value_t::directive_container_t expected_directives{
			{ "max-age"s, "5"s },
			{ "no-transform"s, std::nullopt },
			{ "only-if-cached"s, std::nullopt },
			{ "min-fresh"s, "20"s }
		};

		REQUIRE( expected_directives == result->directives );
	}
}

