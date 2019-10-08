/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parser.hpp>

namespace hfp = restinio::http_field_parser;

struct simple_setter_t
{
	template< typename T >
	void operator()( T & to, T && from ) const noexcept(noexcept(to=from))
	{
		to = std::forward<T>(from);
	}
};

struct media_type_t
{
	std::string m_type;
	std::string m_subtype;

	static void
	type( media_type_t & to, std::string && what )
	{
		to.m_type = std::move(what);
	}

	static void
	subtype( media_type_t & to, std::string && what )
	{
		to.m_subtype = std::move(what);
	}
};

TEST_CASE( "Simple" , "[simple]" )
{
	const char src[] = R"(multipart/form-data)";

	const auto result = hfp::try_parse_field_value< media_type_t >( src,
			hfp::rfc::token( media_type_t::type ),
			hfp::rfc::delimiter( '/' ),
			hfp::rfc::token( media_type_t::subtype ) );

	REQUIRE( result.first );
	REQUIRE( "multipart" == result.second.m_type );
	REQUIRE( "form-data" == result.second.m_subtype );
}

