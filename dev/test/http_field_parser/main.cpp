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

TEST_CASE( "alternatives with symbol", "[alternatives][symbol][into]" )
{
	const auto try_parse = [](restinio::string_view_t what) {
		return hfp::try_parse_field_value< media_type_t >(
			what,
			hfp::rfc::token() >> &media_type_t::m_type,
			hfp::alternatives<char>(
				hfp::symbol('/'),
				hfp::symbol('='),
				hfp::symbol('[')
			) >> hfp::skip(),
			hfp::rfc::token() >> &media_type_t::m_subtype );
	};

	{
		const auto result = try_parse( "multipart/form-data" );
		REQUIRE( result.first );
		REQUIRE( "multipart" == result.second.m_type );
		REQUIRE( "form-data" == result.second.m_subtype );
	}

	{
		const auto result = try_parse( "multipart=form-data" );
		REQUIRE( result.first );
		REQUIRE( "multipart" == result.second.m_type );
		REQUIRE( "form-data" == result.second.m_subtype );
	}

	{
		const auto result = try_parse( "multipart[form-data" );
		REQUIRE( result.first );
		REQUIRE( "multipart" == result.second.m_type );
		REQUIRE( "form-data" == result.second.m_subtype );
	}

	{
		const auto result = try_parse( "multipart(form-data" );
		REQUIRE( !result.first );
	}
}

TEST_CASE( "produce media_type", "[produce][media_type]" )
{
	struct media_type_holder_t {
		media_type_t m_media;
	};

	const auto try_parse = [](restinio::string_view_t what) {
		return hfp::try_parse_field_value< media_type_holder_t >(
				what,
				hfp::produce< media_type_t >(
					hfp::rfc::token() >> &media_type_t::m_type,
					hfp::symbol('/') >> hfp::skip(),
					hfp::rfc::token() >> &media_type_t::m_subtype
				) >> &media_type_holder_t::m_media
			);
	};

	{
		const auto result = try_parse( "multipart/form-data" );
		REQUIRE( result.first );
		REQUIRE( "multipart" == result.second.m_media.m_type );
		REQUIRE( "form-data" == result.second.m_media.m_subtype );
	}

	{
		const auto result = try_parse( "*/form-data" );
		REQUIRE( result.first );
		REQUIRE( "*" == result.second.m_media.m_type );
		REQUIRE( "form-data" == result.second.m_media.m_subtype );
	}

	{
		const auto result = try_parse( "multipart/*" );
		REQUIRE( result.first );
		REQUIRE( "multipart" == result.second.m_media.m_type );
		REQUIRE( "*" == result.second.m_media.m_subtype );
	}

	{
		const auto result = try_parse( "*/*" );
		REQUIRE( result.first );
		REQUIRE( "*" == result.second.m_media.m_type );
		REQUIRE( "*" == result.second.m_media.m_subtype );
	}
}

TEST_CASE( "simple repeat (vector target)", "[repeat][vector][simple]" )
{
	struct pairs_holder_t
	{
		std::vector< std::pair<std::string, std::string> > m_pairs;
	};

	const auto result = hfp::try_parse_field_value< pairs_holder_t >(
			";name1=value;name2=value2",
			hfp::repeat< std::vector< std::pair<std::string, std::string> > >(
				0, hfp::N,
				hfp::symbol(';') >> hfp::skip(),
				hfp::rfc::token() >> &std::pair<std::string, std::string>::first,
				hfp::symbol('=') >> hfp::skip(),
				hfp::rfc::token() >> &std::pair<std::string, std::string>::second
			) >> &pairs_holder_t::m_pairs
		);

	REQUIRE( result.first );
	REQUIRE( 2 == result.second.m_pairs.size() );
	REQUIRE( "name1" == result.second.m_pairs[0].first );
	REQUIRE( "value" == result.second.m_pairs[0].second );
	REQUIRE( "name2" == result.second.m_pairs[1].first );
	REQUIRE( "value2" == result.second.m_pairs[1].second );
}

