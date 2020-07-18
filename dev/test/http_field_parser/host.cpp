
/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/host.hpp>

namespace restinio
{

namespace http_field_parsers
{

bool
operator==( const raw_host_value_t & a, const raw_host_value_t & b ) noexcept
{
	return std::tie( a.host, a.port ) == std::tie( b.host, b.port );
}

} /* namespace http_field_parsers */

} /* namespace restinio */

TEST_CASE( "Raw value of Host", "[raw-host]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = raw_host_value_t::try_parse(
				"" );

		REQUIRE( !result );
	}

	{
		const auto result = raw_host_value_t::try_parse(
				"localhost" );

		REQUIRE( result );

		raw_host_value_t expected{
				raw_host_value_t::reg_name_t{ "localhost"s },
				restinio::nullopt
			};

		REQUIRE( expected == *result );
	}

	{
		const auto result = raw_host_value_t::try_parse(
				"localhost:80" );

		REQUIRE( result );

		raw_host_value_t expected{
				raw_host_value_t::reg_name_t{ "localhost"s },
				80u
			};

		REQUIRE( expected == *result );
	}

	{
		const auto result = raw_host_value_t::try_parse(
				"127.0.0.1" );

		REQUIRE( result );

		raw_host_value_t expected{
				raw_host_value_t::ipv4_address_t{ "127.0.0.1"s },
				restinio::nullopt
			};

		REQUIRE( expected == *result );
	}

	{
		const auto result = raw_host_value_t::try_parse(
				"127.0.0.1:80" );

		REQUIRE( result );

		raw_host_value_t expected{
				raw_host_value_t::ipv4_address_t{ "127.0.0.1"s },
				80u
			};

		REQUIRE( expected == *result );
	}

	{
		const auto result = raw_host_value_t::try_parse(
				"255.255.255.255" );

		REQUIRE( result );

		raw_host_value_t expected{
				raw_host_value_t::ipv4_address_t{ "255.255.255.255"s },
				restinio::nullopt
			};

		REQUIRE( expected == *result );
	}

	{
		const auto result = raw_host_value_t::try_parse(
				"255.205.255.255:80" );

		REQUIRE( result );

		raw_host_value_t expected{
				raw_host_value_t::ipv4_address_t{ "255.205.255.255"s },
				80u
			};

		REQUIRE( expected == *result );
	}

	{
		const auto result = raw_host_value_t::try_parse(
				"245.205.245.221:80" );

		REQUIRE( result );

		raw_host_value_t expected{
				raw_host_value_t::ipv4_address_t{ "245.205.245.221"s },
				80u
			};

		REQUIRE( expected == *result );
	}

	{
		const auto result = raw_host_value_t::try_parse(
				"256.275.245.221:80" );

		REQUIRE( result );

		raw_host_value_t expected{
				raw_host_value_t::reg_name_t{ "256.275.245.221"s },
				80u
			};

		REQUIRE( expected == *result );
	}

	{
		const auto result = raw_host_value_t::try_parse(
				"[::]:80" );

		REQUIRE( result );

		raw_host_value_t expected{
				raw_host_value_t::ipv6_address_t{ "::"s },
				80u
			};

		REQUIRE( expected == *result );
	}

	{
		const auto result = raw_host_value_t::try_parse(
				"[::1]:80" );

		REQUIRE( result );

		raw_host_value_t expected{
				raw_host_value_t::ipv6_address_t{ "::1"s },
				80u
			};

		REQUIRE( expected == *result );
	}

	{
		const auto result = raw_host_value_t::try_parse(
				"[2001:0DB8:85A3:0000:0000:8A2E:0370:7334]:443" );

		REQUIRE( result );

		raw_host_value_t expected{
				raw_host_value_t::ipv6_address_t{ "2001:0db8:85a3:0000:0000:8a2e:0370:7334"s },
				443u
			};

		REQUIRE( expected == *result );
	}

	{
		const auto result = raw_host_value_t::try_parse(
				"[2001:db8::1:0:0:1]:443" );

		REQUIRE( result );

		raw_host_value_t expected{
				raw_host_value_t::ipv6_address_t{ "2001:db8::1:0:0:1"s },
				443u
			};

		REQUIRE( expected == *result );
	}

	{
		const auto result = raw_host_value_t::try_parse(
				"[::ffff:192.0.2.128]:443" );

		REQUIRE( result );

		raw_host_value_t expected{
				raw_host_value_t::ipv6_address_t{ "::ffff:192.0.2.128"s },
				443u
			};

		REQUIRE( expected == *result );
	}
}

