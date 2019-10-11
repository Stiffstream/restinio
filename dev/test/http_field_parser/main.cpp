/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parser.hpp>

//#include <map>

namespace hfp = restinio::http_field_parser;

struct media_type_t
{
	std::string m_type;
	std::string m_subtype;
};

struct content_type_t
{
	media_type_t m_media_type;
//	std::map< std::string, std::string > m_parameters;
};

TEST_CASE( "token >> skip", "[token][skip]" )
{
	struct empty_type_t {};

	const auto result = hfp::try_parse_field_value< empty_type_t >(
			"multipart",
			hfp::rfc::token() >> hfp::skip() );

	REQUIRE( result.first );
}

TEST_CASE( "token+symbol+token >> skip", "[token+symbol+token][skip]" )
{
	struct empty_type_t {};

	{
		const auto result = hfp::try_parse_field_value< empty_type_t >(
				"multipart/form-data",
				hfp::rfc::token() >> hfp::skip(),
				hfp::symbol('/') >> hfp::skip(),
				hfp::rfc::token() >> hfp::skip() );

		REQUIRE( result.first );
	}

	{
		const auto result = hfp::try_parse_field_value< empty_type_t >(
				"multipart+form-data",
				hfp::rfc::token() >> hfp::skip(),
				hfp::symbol('/') >> hfp::skip(),
				hfp::rfc::token() >> hfp::skip() );

		REQUIRE( !result.first );
	}

	{
		const auto result = hfp::try_parse_field_value< empty_type_t >(
				"multipart/",
				hfp::rfc::token() >> hfp::skip(),
				hfp::symbol('/') >> hfp::skip(),
				hfp::rfc::token() >> hfp::skip() );

		REQUIRE( !result.first );
	}
}

TEST_CASE( "token+symbol+token >> into", "[token+symbol+token][into]" )
{
	const auto result = hfp::try_parse_field_value< media_type_t >(
			"multipart/form-data",
			hfp::rfc::token() >> &media_type_t::m_type,
			hfp::symbol('/') >> hfp::skip(),
			hfp::rfc::token() >> &media_type_t::m_subtype );

	REQUIRE( result.first );
	REQUIRE( "multipart" == result.second.m_type );
	REQUIRE( "form-data" == result.second.m_subtype );
}

#if 0
TEST_CASE( "Simple alternative" , "[simple][alternative]" )
{
	const auto do_parse = [](restinio::string_view_t what) {
		return hfp::try_parse_field_value< media_type_t >( what,
				hfp::alternatives(
						hfp::rfc::token( media_type_t::type ),
						hfp::symbol(
								'*',
								[]( auto & to ) { to.m_type = "*"; } )
				),
				hfp::rfc::delimiter( '/' ),
				hfp::alternatives(
						hfp::rfc::token( media_type_t::subtype ),
						hfp::symbol(
								'*',
								[]( auto & to ) { to.m_subtype = "*"; } )
				)
		);
	};

	{
		const char src[] = R"(multipart/form-data)";

		const auto result = do_parse( src );

		REQUIRE( result.first );
		REQUIRE( "multipart" == result.second.m_type );
		REQUIRE( "form-data" == result.second.m_subtype );
	}

	{
		const char src[] = R"(*/form-data)";

		const auto result = do_parse( src );

		REQUIRE( result.first );
		REQUIRE( "*" == result.second.m_type );
		REQUIRE( "form-data" == result.second.m_subtype );
	}

	{
		const char src[] = R"(multipart/*)";

		const auto result = do_parse( src );

		REQUIRE( result.first );
		REQUIRE( "multipart" == result.second.m_type );
		REQUIRE( "*" == result.second.m_subtype );
	}

	{
		const char src[] = R"(;/*)";

		const auto result = do_parse( src );

		REQUIRE( !result.first );
	}
}

TEST_CASE( "Simple content_type value", "[simple][content-type]" )
{
	const char src[] = R"(multipart/form-data; boundary=---12345)";

	const auto result = hfp::try_parse_field_value< content_type_parsed_value_t >(
			src,
			hfp::rfc::token( content_type_parsed_value_t::type ),
			hfp::rfc::delimiter( '/' ),
			hfp::rfc::token( content_type_parsed_value_t::subtype ),
			hfp::any_occurences_of< std::vector< parameter_t > >(
					content_type_parsed_value_t::parameters,
					hfp::rfc::semicolon(),
					hfp::rfc::ows(),
					hfp::rfc::token( parameter_t::name ),
					hfp::rfc::delimiter( '=' ),
					hfp::rfc::token( parameter_t::value ) )
			);

	REQUIRE( result.first );
	REQUIRE( "multipart" == result.second.m_media_type.m_type );
	REQUIRE( "form-data" == result.second.m_media_type.m_subtype );

	REQUIRE( 1u == result.second.m_parameters.size() );
	REQUIRE( "boundary" == result.second.m_parameters[0].m_name );
	REQUIRE( "---12345" == result.second.m_parameters[0].m_value );
}

TEST_CASE( "Simple content_type value (two-params)", "[simple][content-type][two-params]" )
{
	const char src[] = R"(multipart/form-data; boundary=---12345; another=value)";

	const auto result = hfp::try_parse_field_value< content_type_parsed_value_t >(
			src,
			hfp::rfc::token( content_type_parsed_value_t::type ),
			hfp::rfc::delimiter( '/' ),
			hfp::rfc::token( content_type_parsed_value_t::subtype ),
			hfp::any_occurences_of< std::vector< parameter_t > >(
					content_type_parsed_value_t::parameters,
					hfp::rfc::semicolon(),
					hfp::rfc::ows(),
					hfp::rfc::token( parameter_t::name ),
					hfp::rfc::delimiter( '=' ),
					hfp::rfc::token( parameter_t::value ) )
			);

	REQUIRE( result.first );
	REQUIRE( "multipart" == result.second.m_media_type.m_type );
	REQUIRE( "form-data" == result.second.m_media_type.m_subtype );

	REQUIRE( 2u == result.second.m_parameters.size() );
	REQUIRE( "boundary" == result.second.m_parameters[0].m_name );
	REQUIRE( "---12345" == result.second.m_parameters[0].m_value );
	REQUIRE( "another" == result.second.m_parameters[1].m_name );
	REQUIRE( "value" == result.second.m_parameters[1].m_value );
}

TEST_CASE( "Simple content_type value (trailing spaces)", "[simple][content-type][trailing-spaces]" )
{
	const char src[] = R"(multipart/form-data; boundary=---12345  )";

	const auto result = hfp::try_parse_field_value< content_type_parsed_value_t >(
			src,
			hfp::rfc::token( content_type_parsed_value_t::type ),
			hfp::rfc::delimiter( '/' ),
			hfp::rfc::token( content_type_parsed_value_t::subtype ),
			hfp::any_occurences_of< std::vector< parameter_t > >(
					content_type_parsed_value_t::parameters,
					hfp::rfc::semicolon(),
					hfp::rfc::ows(),
					hfp::rfc::token( parameter_t::name ),
					hfp::rfc::delimiter( '=' ),
					hfp::rfc::token( parameter_t::value ) )
			);

	REQUIRE( result.first );
	REQUIRE( "multipart" == result.second.m_media_type.m_type );
	REQUIRE( "form-data" == result.second.m_media_type.m_subtype );

	REQUIRE( 1u == result.second.m_parameters.size() );
	REQUIRE( "boundary" == result.second.m_parameters[0].m_name );
	REQUIRE( "---12345" == result.second.m_parameters[0].m_value );
}

TEST_CASE( "Simple content_type value (trailing garbage)", "[simple][content-type][trailing-garbage]" )
{
	const char src[] = R"(multipart/form-data; boundary=---12345; bla-bla-bla)";

	const auto result = hfp::try_parse_field_value< content_type_parsed_value_t >(
			src,
			hfp::rfc::token( content_type_parsed_value_t::type ),
			hfp::rfc::delimiter( '/' ),
			hfp::rfc::token( content_type_parsed_value_t::subtype ),
			hfp::any_occurences_of< std::vector< parameter_t > >(
					content_type_parsed_value_t::parameters,
					hfp::rfc::semicolon(),
					hfp::rfc::ows(),
					hfp::rfc::token( parameter_t::name ),
					hfp::rfc::delimiter( '=' ),
					hfp::rfc::token( parameter_t::value ) )
			);

	REQUIRE( !result.first );
}

#endif

