/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/user-agent.hpp>

#include <tuple>

namespace restinio
{

namespace http_field_parsers
{

RESTINIO_NODISCARD
inline bool
operator==(
	const user_agent_value_t::product_t & a,
	const user_agent_value_t::product_t & b ) noexcept
{
	return std::tie(a.product, a.product_version) ==
			std::tie(b.product, b.product_version);
}

inline std::ostream &
operator<<(
	std::ostream & to,
	const user_agent_value_t::tail_item_t & i ) noexcept
{
	if( auto p = restinio::get_if<user_agent_value_t::product_t>(&i) ) {
		to << "[" << p->product;
		if( p->product_version )
			to << ", " << *(p->product_version);
		to << "]";
	}
	else if( auto p = restinio::get_if<std::string>(&i) ) {
		to << "(" << *p << ")";
	}

	return to;
}

} /* namespace http_field_parsers */

} /* namespace restinio */

TEST_CASE( "User-Agent Field", "[user-agent]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = user_agent_value_t::try_parse(
				"" );

		REQUIRE( !result );
	}

	{
		const auto result = user_agent_value_t::try_parse(
				"CERN-LineMode" );

		REQUIRE( result );

		REQUIRE( "CERN-LineMode" == result->product.product );
		REQUIRE( !result->product.product_version );
	}

	{
		const auto result = user_agent_value_t::try_parse(
				"CERN-LineMode/2.15" );

		REQUIRE( result );

		REQUIRE( "CERN-LineMode" == result->product.product );
		REQUIRE( "2.15" == *(result->product.product_version) );
	}

	{
		const auto result = user_agent_value_t::try_parse(
				"CERN-LineMode/2.15 libwww/2.17b3" );

		REQUIRE( result );

		REQUIRE( "CERN-LineMode" == result->product.product );
		REQUIRE( "2.15" == *(result->product.product_version) );

		std::vector< user_agent_value_t::tail_item_t > expected{
			user_agent_value_t::product_t{ "libwww"s, "2.17b3"s }
		};

		REQUIRE( result->tail == expected );
	}

	{
		const auto result = user_agent_value_t::try_parse(
				"Mozilla/5.0 (X11; Linux x86_64) "
				"AppleWebKit/537.36 (KHTML, like Gecko) "
				"Chrome/79.0.3945.130 Safari/537.36" );

		REQUIRE( result );

		REQUIRE( "Mozilla" == result->product.product );
		REQUIRE( "5.0" == *(result->product.product_version) );

		std::vector< user_agent_value_t::tail_item_t > expected{
			"X11; Linux x86_64"s,
			user_agent_value_t::product_t{ "AppleWebKit"s, "537.36"s },
			"KHTML, like Gecko"s,
			user_agent_value_t::product_t{ "Chrome"s, "79.0.3945.130"s },
			user_agent_value_t::product_t{ "Safari"s, "537.36"s }
		};

		REQUIRE( result->tail == expected );
	}

	{
		const auto result = user_agent_value_t::try_parse(
				"Mozilla/5.0 (X11; Linux x86_64) "
				"AppleWebKit (KHTML, like Gecko) "
				"Chrome/79.0.3945.130 Safari" );

		REQUIRE( result );

		REQUIRE( "Mozilla" == result->product.product );
		REQUIRE( "5.0" == *(result->product.product_version) );

		std::vector< user_agent_value_t::tail_item_t > expected{
			"X11; Linux x86_64"s,
			user_agent_value_t::product_t{ "AppleWebKit"s, restinio::nullopt },
			"KHTML, like Gecko"s,
			user_agent_value_t::product_t{ "Chrome"s, "79.0.3945.130"s },
			user_agent_value_t::product_t{ "Safari"s, restinio::nullopt }
		};

		REQUIRE( result->tail == expected );
	}
}

