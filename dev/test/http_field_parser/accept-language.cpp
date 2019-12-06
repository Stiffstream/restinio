
/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/accept-language.hpp>

namespace restinio
{

namespace http_field_parsers
{

bool
operator==( const accept_language_value_t::item_t & a,
	const accept_language_value_t::item_t & b ) noexcept
{
	return std::tie( a.language_range, a.weight )
			== std::tie( b.language_range, b.weight );
}

std::ostream &
operator<<( std::ostream & to, const accept_language_value_t::item_t & a )
{
	return (to << a.language_range << ";" << a.weight.as_string());
}

} /* namespace http_field_parsers */

} /* namespace restinio */

TEST_CASE( "Accept-Language", "[accept-language]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = accept_language_value_t::try_parse(
				"" );

		REQUIRE( !result );
	}

	{
		const auto result = accept_language_value_t::try_parse(
				"aaaabbbbc" );

		REQUIRE( !result );
	}

	{
		const auto result = accept_language_value_t::try_parse(
				"a1" );

		REQUIRE( !result );
	}

	{
		const auto result = accept_language_value_t::try_parse(
				"12345678" );

		REQUIRE( !result );
	}

	{
		const auto result = accept_language_value_t::try_parse(
				"aaaabbbb-" );

		REQUIRE( !result );
	}

	{
		const auto result = accept_language_value_t::try_parse(
				"aaaabbbb-aaaabbbbc" );

		REQUIRE( !result );
	}

	{
		const auto result = accept_language_value_t::try_parse(
				"en" );

		REQUIRE( result );

		accept_language_value_t::item_container_t expected{
			{ "en"s, qvalue_t{ qvalue_t::maximum } }
		};

		REQUIRE( expected == result->languages );
	}

	{
		const auto result = accept_language_value_t::try_parse(
				"aaaabbbb-12345678-aaaa1111" );

		REQUIRE( result );

		accept_language_value_t::item_container_t expected{
			{ "aaaabbbb-12345678-aaaa1111"s, qvalue_t{ qvalue_t::maximum } }
		};

		REQUIRE( expected == result->languages );
	}

	{
		const auto result = accept_language_value_t::try_parse(
				"en, en-US" );

		REQUIRE( result );

		accept_language_value_t::item_container_t expected{
			{ "en"s, qvalue_t{ qvalue_t::maximum } }
			, { "en-US"s, qvalue_t{ qvalue_t::maximum } }
		};

		REQUIRE( expected == result->languages );
	}

	{
		const auto result = accept_language_value_t::try_parse(
				"en-US ; q=0.5  , ru-RU" );

		REQUIRE( result );

		accept_language_value_t::item_container_t expected{
			{ "en-US"s, qvalue_t{ qvalue_t::trusted{500} } }
			, { "ru-RU"s, qvalue_t{ qvalue_t::maximum } }
		};

		REQUIRE( expected == result->languages );
	}

	{
		const auto result = accept_language_value_t::try_parse(
				"en-US;q=0.5, ru-RU;q=1" );

		REQUIRE( result );

		accept_language_value_t::item_container_t expected{
			{ "en-US"s, qvalue_t{ qvalue_t::trusted{500} } }
			, { "ru-RU"s, qvalue_t{ qvalue_t::maximum } }
		};

		REQUIRE( expected == result->languages );
	}

	{
		const auto result = accept_language_value_t::try_parse(
				"zh-cmn-Hans-CN;q=1.0, de-CH-1901; q=0.5, *;q=0" );

		REQUIRE( result );

		accept_language_value_t::item_container_t expected{
			{ "zh-cmn-Hans-CN"s, qvalue_t{ qvalue_t::maximum } }
			, { "de-CH-1901"s, qvalue_t{ qvalue_t::trusted{500} } }
			, { "*"s, qvalue_t{ qvalue_t::zero } }
		};

		REQUIRE( expected == result->languages );
	}

	{
		const auto result = accept_language_value_t::try_parse(
				"az-Arab-x-AZE-derbend;q=1.0, es-419, *;q=0" );

		REQUIRE( result );

		accept_language_value_t::item_container_t expected{
			{ "az-Arab-x-AZE-derbend"s, qvalue_t{ qvalue_t::maximum } }
			, { "es-419"s, qvalue_t{ qvalue_t::maximum } }
			, { "*"s, qvalue_t{ qvalue_t::zero } }
		};

		REQUIRE( expected == result->languages );
	}
}

