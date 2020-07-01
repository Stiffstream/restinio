/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/transfer-encoding.hpp>

TEST_CASE( "Transfer-Encoding", "[transfer-encoding]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = transfer_encoding_value_t::try_parse(
				"" );

		REQUIRE( !result );
	}

	{
		const auto result = transfer_encoding_value_t::try_parse(
				"compress/" );

		REQUIRE( !result );
	}

	{
		const auto result = transfer_encoding_value_t::try_parse(
				"compress" );

		REQUIRE( result );

		const transfer_encoding_value_t::value_container_t expected{
			{ transfer_encoding_value_t::compress }
		};

		REQUIRE( expected == result->values );
	}

	{
		const auto result = transfer_encoding_value_t::try_parse(
				"X-Compress" );

		REQUIRE( result );

		const transfer_encoding_value_t::value_container_t expected{
			{
				transfer_encoding_value_t::transfer_extension_t{
					"x-compress"s,
					parameter_with_mandatory_value_container_t{}
				}
			}
		};

		REQUIRE( expected == result->values );
	}

	{
		const auto result = transfer_encoding_value_t::try_parse(
				"compressed" );

		REQUIRE( result );

		const transfer_encoding_value_t::value_container_t expected{
			{
				transfer_encoding_value_t::transfer_extension_t{
					"compressed"s,
					parameter_with_mandatory_value_container_t{}
				}
			}
		};

		REQUIRE( expected == result->values );
	}

	{
		const auto result = transfer_encoding_value_t::try_parse(
				"gzip, X-Compress  ,     deflate" );

		REQUIRE( result );

		const transfer_encoding_value_t::value_container_t expected{
			{ transfer_encoding_value_t::gzip },
			{
				transfer_encoding_value_t::transfer_extension_t{
					"x-compress"s,
					parameter_with_mandatory_value_container_t{}
				}
			},
			{ transfer_encoding_value_t::deflate }
		};

		REQUIRE( expected == result->values );
	}
}

