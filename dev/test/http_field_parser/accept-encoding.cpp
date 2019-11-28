
/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/accept-encoding.hpp>

namespace restinio
{

namespace http_field_parsers
{

bool
operator==( const accept_encoding_value_t::item_t & a,
	const accept_encoding_value_t::item_t & b ) noexcept
{
	return std::tie( a.content_coding, a.weight ) ==
			std::tie( b.content_coding, b.weight );
}

} /* namespace http_field_parsers */

} /* namespace restinio */

TEST_CASE( "Accept-Encoding", "[accept-encoding]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = accept_encoding_value_t::try_parse(
				"" );

		REQUIRE( result );

		REQUIRE( result->codings.empty() );
	}

	{
		const auto result = accept_encoding_value_t::try_parse(
				"compress" );

		REQUIRE( result );

		accept_encoding_value_t::item_container_t expected{
			{ "compress"s, qvalue_t{ qvalue_t::maximum } }
		};

		REQUIRE( expected == result->codings );
	}

	{
		const auto result = accept_encoding_value_t::try_parse(
				"compress, gzip" );

		REQUIRE( result );

		accept_encoding_value_t::item_container_t expected{
			{ "compress"s, qvalue_t{ qvalue_t::maximum } }
			, { "gzip"s, qvalue_t{ qvalue_t::maximum } }
		};

		REQUIRE( expected == result->codings );
	}

	{
		const auto result = accept_encoding_value_t::try_parse(
				"compress ; q=0.5  , gzip" );

		REQUIRE( result );

		accept_encoding_value_t::item_container_t expected{
			{ "compress"s, qvalue_t{ qvalue_t::trusted{500} } }
			, { "gzip"s, qvalue_t{ qvalue_t::maximum } }
		};

		REQUIRE( expected == result->codings );
	}

	{
		const auto result = accept_encoding_value_t::try_parse(
				"compress;q=0.5, gzip;q=1" );

		REQUIRE( result );

		accept_encoding_value_t::item_container_t expected{
			{ "compress"s, qvalue_t{ qvalue_t::trusted{500} } }
			, { "gzip"s, qvalue_t{ qvalue_t::maximum } }
		};

		REQUIRE( expected == result->codings );
	}

	{
		const auto result = accept_encoding_value_t::try_parse(
				"gzip;q=1.0, identity; q=0.5, *;q=0" );

		REQUIRE( result );

		accept_encoding_value_t::item_container_t expected{
			{ "gzip"s, qvalue_t{ qvalue_t::maximum } }
			, { "identity"s, qvalue_t{ qvalue_t::trusted{500} } }
			, { "*"s, qvalue_t{ qvalue_t::zero } }
		};

		REQUIRE( expected == result->codings );
	}

	{
		const auto result = accept_encoding_value_t::try_parse(
				"gzip;q=1.0, identity, *;q=0" );

		REQUIRE( result );

		accept_encoding_value_t::item_container_t expected{
			{ "gzip"s, qvalue_t{ qvalue_t::maximum } }
			, { "identity"s, qvalue_t{ qvalue_t::maximum } }
			, { "*"s, qvalue_t{ qvalue_t::zero } }
		};

		REQUIRE( expected == result->codings );
	}
}

