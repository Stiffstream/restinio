/*
	restinio
*/

/*!
	Tests for easy-parser based router DSL.
*/

#include <catch2/catch.hpp>

#include <restinio/router/easy_parser_router.hpp>

namespace ep = restinio::easy_parser;
namespace epr = restinio::router::easy_parser_router;

TEST_CASE("detect_result_tuple", "[detect_result_tuple]")
{
	using dsl_processor = epr::impl::dsl_processor<
			const char[8],
			ep::impl::non_negative_decimal_number_producer_t<int>,
			ep::impl::hexdigit_producer_t,
			std::string,
			ep::impl::digit_producer_t >;
	using T = typename dsl_processor::result_tuple;

#if 0
	using T2 = typename dsl_processor::clauses_tuple;
	restinio::utils::metaprogramming::impl::debug_print<T2>{};
#endif

	static_assert( std::is_same< T, std::tuple< int, char, char > >::value,
			"!Ok" );
}

TEST_CASE("simple path_to_tuple", "[path_to_tuple]")
{
	auto parser = epr::path_to_tuple(
			epr::exact( "/api/v1/books/" ),
			epr::non_negative_decimal_number_p<int>(),
			epr::exact( "/versions/" ),
			epr::non_negative_decimal_number_p<int>() );

	auto r = epr::try_parse( "/api/v1/books/124/versions/4325", parser );

	REQUIRE( r );
	REQUIRE( 124 == std::get<0>(*r) );
	REQUIRE( 4325 == std::get<1>(*r) );
}

TEST_CASE("path_to_tuple", "[path_to_tuple]")
{
	auto book_id_p = epr::non_negative_decimal_number_p<int>();
	auto version_id_p = epr::non_negative_decimal_number_p<int>();

	const restinio::string_view_t slash{ "/" };
	const std::string books_tag{ "books" };
	std::string versions_tag{ "versions" };

	auto parser = epr::path_to_tuple(
			slash, "api", slash, "v1", slash, books_tag, std::string{"/"},
			book_id_p,
			slash, versions_tag, restinio::string_view_t{ "/" },
			version_id_p );

	auto r = epr::try_parse( "/api/v1/books/124/versions/4325", parser );

	REQUIRE( r );
	REQUIRE( 124 == std::get<0>(*r) );
	REQUIRE( 4325 == std::get<1>(*r) );
}

TEST_CASE("normal path_to_tuple", "[path_to_tuple]")
{
	auto book_id_p = epr::non_negative_decimal_number_p<int>();
	auto version_id_p = epr::non_negative_decimal_number_p<int>();

	auto parser = epr::path_to_tuple(
			"/api/v1/books/",
			book_id_p,
			"/versions/",
			version_id_p );

	auto r = epr::try_parse( "/api/v1/books/124/versions/4325", parser );

	REQUIRE( r );
	REQUIRE( 124 == std::get<0>(*r) );
	REQUIRE( 4325 == std::get<1>(*r) );
}

