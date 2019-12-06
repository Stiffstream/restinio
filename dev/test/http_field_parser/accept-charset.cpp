
/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/accept-charset.hpp>

namespace restinio
{

namespace http_field_parsers
{

bool
operator==( const accept_charset_value_t::item_t & a,
	const accept_charset_value_t::item_t & b ) noexcept
{
	return std::tie( a.charset, a.weight ) == std::tie( b.charset, b.weight );
}

} /* namespace http_field_parsers */

} /* namespace restinio */

TEST_CASE( "Accept-Charset", "[accept-charset]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = accept_charset_value_t::try_parse(
				"" );

		REQUIRE( !result );
	}

	{
		const auto result = accept_charset_value_t::try_parse(
				"iso-8859-1" );

		REQUIRE( result );

		accept_charset_value_t::item_container_t expected{
			{ "iso-8859-1"s, qvalue_t{ qvalue_t::maximum } }
		};

		REQUIRE( expected == result->charsets );
	}

	{
		const auto result = accept_charset_value_t::try_parse(
				"iso-8859-1, unicode-1-1" );

		REQUIRE( result );

		accept_charset_value_t::item_container_t expected{
			{ "iso-8859-1"s, qvalue_t{ qvalue_t::maximum } }
			, { "unicode-1-1"s, qvalue_t{ qvalue_t::maximum } }
		};

		REQUIRE( expected == result->charsets );
	}

	{
		const auto result = accept_charset_value_t::try_parse(
				"iso-8859-1 ; q=0.5  , cp1251" );

		REQUIRE( result );

		accept_charset_value_t::item_container_t expected{
			{ "iso-8859-1"s, qvalue_t{ qvalue_t::trusted{500} } }
			, { "cp1251"s, qvalue_t{ qvalue_t::maximum } }
		};

		REQUIRE( expected == result->charsets );
	}

	{
		const auto result = accept_charset_value_t::try_parse(
				"iso-8859-1;q=0.5, cp1251;q=1" );

		REQUIRE( result );

		accept_charset_value_t::item_container_t expected{
			{ "iso-8859-1"s, qvalue_t{ qvalue_t::trusted{500} } }
			, { "cp1251"s, qvalue_t{ qvalue_t::maximum } }
		};

		REQUIRE( expected == result->charsets );
	}

	{
		const auto result = accept_charset_value_t::try_parse(
				"iso-8859-1;q=1.0, cp1251; q=0.5, *;q=0" );

		REQUIRE( result );

		accept_charset_value_t::item_container_t expected{
			{ "iso-8859-1"s, qvalue_t{ qvalue_t::maximum } }
			, { "cp1251"s, qvalue_t{ qvalue_t::trusted{500} } }
			, { "*"s, qvalue_t{ qvalue_t::zero } }
		};

		REQUIRE( expected == result->charsets );
	}

	{
		const auto result = accept_charset_value_t::try_parse(
				"iso-8859-1;q=1.0, cp1251, *;q=0" );

		REQUIRE( result );

		accept_charset_value_t::item_container_t expected{
			{ "iso-8859-1"s, qvalue_t{ qvalue_t::maximum } }
			, { "cp1251"s, qvalue_t{ qvalue_t::maximum } }
			, { "*"s, qvalue_t{ qvalue_t::zero } }
		};

		REQUIRE( expected == result->charsets );
	}
}

