
/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/accept.hpp>

TEST_CASE( "Accept", "[media-type][accept]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = accept_value_t::try_parse(
				"" );

		REQUIRE( result );

		REQUIRE( result->items.empty() );
	}

	{
		const auto result = accept_value_t::try_parse(
				"text/" );

		REQUIRE( !result );
	}

	{
		const auto result = accept_value_t::try_parse(
				"/plain" );

		REQUIRE( !result );
	}

	{
		const auto result = accept_value_t::try_parse(
				"text/plain" );

		REQUIRE( result );

		REQUIRE( 1 == result->items.size() );

		const auto & item = result->items[0];

		REQUIRE( "text" == item.media_type.type );
		REQUIRE( "plain" == item.media_type.subtype );
		REQUIRE( item.media_type.parameters.empty() );
	}

	{
		const auto result = accept_value_t::try_parse(
				"text/*; CharSet=utf-8 ;    Alternative-Coding=\"Bla Bla Bla\"" );

		REQUIRE( result );

		REQUIRE( 1 == result->items.size() );

		const auto & item = result->items[0];

		REQUIRE( "text" == item.media_type.type );
		REQUIRE( "*" == item.media_type.subtype );

		media_type_value_t::parameter_container_t expected{
			{ "charset"s, "utf-8"s },
			{ "alternative-coding"s, "Bla Bla Bla"s }
		};
		REQUIRE( expected == item.media_type.parameters );
	}

	{
		const auto result = accept_value_t::try_parse(
				"text/*;CharSet=utf-8, application/json;charset=cp1251" );

		REQUIRE( result );

		REQUIRE( 2 == result->items.size() );

		{
			const auto & item = result->items[0];

			REQUIRE( "text" == item.media_type.type );
			REQUIRE( "*" == item.media_type.subtype );

			media_type_value_t::parameter_container_t expected{
				{ "charset"s, "utf-8"s },
			};
			REQUIRE( expected == item.media_type.parameters );
		}

		{
			const auto & item = result->items[1];

			REQUIRE( "application" == item.media_type.type );
			REQUIRE( "json" == item.media_type.subtype );

			media_type_value_t::parameter_container_t expected{
				{ "charset"s, "cp1251"s },
			};
			REQUIRE( expected == item.media_type.parameters );
		}
	}

	{
		const auto result = accept_value_t::try_parse(
				"text/plain;q=0.5;signed;signature-method=sha512, "
				"text/*;CharSet=utf-8, "
				"application/json;charset=cp1251" );

		REQUIRE( result );

		REQUIRE( 3 == result->items.size() );

		{
			const auto & item = result->items[0];
			REQUIRE( "text" == item.media_type.type );
			REQUIRE( "plain" == item.media_type.subtype );
			REQUIRE( item.media_type.parameters.empty() );

			REQUIRE( item.weight );
			REQUIRE( qvalue_t{ qvalue_t::untrusted{500} } == *item.weight );

			accept_value_t::item_t::accept_ext_container_t expected{
				{ "signed"s, restinio::nullopt },
				{ "signature-method"s, "sha512"s }
			};
			REQUIRE( expected == item.accept_params );
		}

		{
			const auto & item = result->items[1];

			REQUIRE( "text" == item.media_type.type );
			REQUIRE( "*" == item.media_type.subtype );

			media_type_value_t::parameter_container_t expected{
				{ "charset"s, "utf-8"s },
			};
			REQUIRE( expected == item.media_type.parameters );
		}

		{
			const auto & item = result->items[2];

			REQUIRE( "application" == item.media_type.type );
			REQUIRE( "json" == item.media_type.subtype );

			media_type_value_t::parameter_container_t expected{
				{ "charset"s, "cp1251"s },
			};
			REQUIRE( expected == item.media_type.parameters );
		}
	}
}
