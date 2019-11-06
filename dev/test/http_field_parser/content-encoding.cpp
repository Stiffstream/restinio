/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/content-encoding.hpp>

TEST_CASE( "Content-Encoding", "[content-encoding]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = content_encoding_value_t::try_parse(
				"" );

		REQUIRE( !result );
	}

	{
		const auto result = content_encoding_value_t::try_parse(
				"compress/" );

		REQUIRE( !result );
	}

	{
		const auto result = content_encoding_value_t::try_parse(
				"compress" );

		REQUIRE( result );

		const content_encoding_value_t::value_container_t expected{
			"compress"s
		};

		REQUIRE( expected == result->values );
	}

	{
		const auto result = content_encoding_value_t::try_parse(
				"X-Compress" );

		REQUIRE( result );

		const content_encoding_value_t::value_container_t expected{
			"x-compress"s
		};

		REQUIRE( expected == result->values );
	}

	{
		const auto result = content_encoding_value_t::try_parse(
				"gzip, X-Compress  ,     deflate" );

		REQUIRE( result );

		const content_encoding_value_t::value_container_t expected{
			"gzip"s,
			"x-compress"s,
			"deflate"s
		};

		REQUIRE( expected == result->values );
	}
}

