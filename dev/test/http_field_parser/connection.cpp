/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/connection.hpp>

TEST_CASE( "Connection", "[connection]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = connection_value_t::try_parse(
				"" );

		REQUIRE( !result );
	}

	{
		const auto result = connection_value_t::try_parse(
				"close/" );

		REQUIRE( !result );
	}

	{
		const auto result = connection_value_t::try_parse(
				"close" );

		REQUIRE( result );

		const connection_value_t::value_container_t expected{
			"close"s
		};

		REQUIRE( expected == result->values );
	}

	{
		const auto result = connection_value_t::try_parse(
				"Close" );

		REQUIRE( result );

		const connection_value_t::value_container_t expected{
			"close"s
		};

		REQUIRE( expected == result->values );
	}

	{
		const auto result = connection_value_t::try_parse(
				"keep-alive, Keep-Alive  ,     My-Hop-By-Hop-Header" );

		REQUIRE( result );

		const connection_value_t::value_container_t expected{
			"keep-alive"s,
			"keep-alive"s,
			"my-hop-by-hop-header"s
		};

		REQUIRE( expected == result->values );
	}
}

