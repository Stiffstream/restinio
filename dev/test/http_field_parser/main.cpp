/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parser.hpp>
#include <restinio/helpers/http_field_parsers/cache-control.hpp>

//#include <map>

namespace hfp = restinio::http_field_parser;

struct media_type_t
{
	std::string m_type;
	std::string m_subtype;
};

bool
operator==( const media_type_t & a, const media_type_t & b ) noexcept
{
	return a.m_type == b.m_type && a.m_subtype == b.m_subtype;
}

std::ostream &
operator<<( std::ostream & to, const media_type_t & v )
{
	return (to << v.m_type << '/' << v.m_subtype);
}

struct content_type_t
{
	media_type_t m_media_type;
	std::map< std::string, std::string > m_parameters;
};

struct value_with_opt_params_t
{
	using param_t = std::pair< std::string, restinio::optional_t<std::string> >;
	using param_storage_t = std::vector< param_t >;

	std::string m_value;
	param_storage_t m_params;
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

TEST_CASE( "simple repeat (map target)", "[repeat][map][simple]" )
{
	struct pairs_holder_t
	{
		std::map< std::string, std::string > m_pairs;
	};

	const auto result = hfp::try_parse_field_value< pairs_holder_t >(
			";name1=value;name2=value2",
			hfp::repeat< std::map<std::string, std::string> >(
				0, hfp::N,
				hfp::symbol(';') >> hfp::skip(),
				hfp::rfc::token() >> &std::pair<std::string, std::string>::first,
				hfp::symbol('=') >> hfp::skip(),
				hfp::rfc::token() >> &std::pair<std::string, std::string>::second
			) >> &pairs_holder_t::m_pairs
		);

	REQUIRE( result.first );
	REQUIRE( 2 == result.second.m_pairs.size() );

	const std::map< std::string, std::string > expected{
			{ "name1", "value" }, { "name2", "value2" }
	};

	REQUIRE( expected == result.second.m_pairs );
}

TEST_CASE( "simple content_type", "[content_type][simple]" )
{
	const auto try_parse = [](restinio::string_view_t what) {
		return hfp::try_parse_field_value< content_type_t >(
			what,

			hfp::produce< media_type_t >(
				hfp::rfc::token() >> hfp::to_lower() >> &media_type_t::m_type,
				hfp::symbol('/') >> hfp::skip(),
				hfp::rfc::token() >> hfp::to_lower() >> &media_type_t::m_subtype
			) >> &content_type_t::m_media_type,

			hfp::repeat< std::map<std::string, std::string> >(
				0, hfp::N,
				hfp::symbol(';') >> hfp::skip(),
				hfp::rfc::ows() >> hfp::skip(),

				hfp::rfc::token() >> hfp::to_lower() >>
						&std::pair<std::string, std::string>::first,

				hfp::symbol('=') >> hfp::skip(),

				hfp::alternatives< std::string >(
					hfp::rfc::token() >> hfp::to_lower(),
					hfp::rfc::quoted_string()
				) >> &std::pair<std::string, std::string>::second

			) >> &content_type_t::m_parameters
		);
	};

	{
		const auto result = try_parse( "text/plain" );

		REQUIRE( result.first );
		REQUIRE( "text" == result.second.m_media_type.m_type );
		REQUIRE( "plain" == result.second.m_media_type.m_subtype );
		REQUIRE( result.second.m_parameters.empty() );
	}

	{
		const auto result = try_parse( "text/plain; charset=utf-8" );

		REQUIRE( result.first );
		REQUIRE( "text" == result.second.m_media_type.m_type );
		REQUIRE( "plain" == result.second.m_media_type.m_subtype );
		REQUIRE( !result.second.m_parameters.empty() );

		const std::map< std::string, std::string > expected{
				{ "charset", "utf-8" }
		};

		REQUIRE( expected == result.second.m_parameters );
	}

	{
		const auto result = try_parse( "text/plain;charset=utf-8" );

		REQUIRE( result.first );
		REQUIRE( "text" == result.second.m_media_type.m_type );
		REQUIRE( "plain" == result.second.m_media_type.m_subtype );
		REQUIRE( !result.second.m_parameters.empty() );

		const std::map< std::string, std::string > expected{
				{ "charset", "utf-8" }
		};

		REQUIRE( expected == result.second.m_parameters );
	}

	{
		const auto result = try_parse(
				"multipart/form-data; charset=utf-8; boundary=---123456" );

		REQUIRE( result.first );
		REQUIRE( "multipart" == result.second.m_media_type.m_type );
		REQUIRE( "form-data" == result.second.m_media_type.m_subtype );
		REQUIRE( !result.second.m_parameters.empty() );

		const std::map< std::string, std::string > expected{
				{ "charset", "utf-8" }, { "boundary", "---123456" }
		};

		REQUIRE( expected == result.second.m_parameters );
	}

	{
		const auto result = try_parse(
				R"(multipart/form-data; charset=utf-8; boundary="Text with space!")" );

		REQUIRE( result.first );
		REQUIRE( "multipart" == result.second.m_media_type.m_type );
		REQUIRE( "form-data" == result.second.m_media_type.m_subtype );
		REQUIRE( !result.second.m_parameters.empty() );

		const std::map< std::string, std::string > expected{
				{ "charset", "utf-8" }, { "boundary", "Text with space!" }
		};

		REQUIRE( expected == result.second.m_parameters );
	}

	{
		const auto result = try_parse(
				R"(multipart/form-data; charset=utf-8; boundary="Text with space!")" );

		REQUIRE( result.first );
		REQUIRE( "multipart" == result.second.m_media_type.m_type );
		REQUIRE( "form-data" == result.second.m_media_type.m_subtype );
		REQUIRE( !result.second.m_parameters.empty() );

		const std::map< std::string, std::string > expected{
				{ "charset", "utf-8" }, { "boundary", "Text with space!" }
		};

		REQUIRE( expected == result.second.m_parameters );
	}

	{
		const auto result = try_parse(
				R"(MultiPart/Form-Data; CharSet=utf-8; BOUNDARY="Text with space!")" );

		REQUIRE( result.first );
		REQUIRE( "multipart" == result.second.m_media_type.m_type );
		REQUIRE( "form-data" == result.second.m_media_type.m_subtype );
		REQUIRE( !result.second.m_parameters.empty() );

		const std::map< std::string, std::string > expected{
				{ "charset", "utf-8" }, { "boundary", "Text with space!" }
		};

		REQUIRE( expected == result.second.m_parameters );
	}
}

TEST_CASE( "sequence with optional", "[optional][simple]" )
{
	const auto try_parse = [](restinio::string_view_t what) {
		return hfp::try_parse_field_value< value_with_opt_params_t >(
			what,

			hfp::rfc::token() >> hfp::to_lower() >>
					&value_with_opt_params_t::m_value,

			hfp::repeat< value_with_opt_params_t::param_storage_t >(
				0, hfp::N,
				hfp::symbol(';') >> hfp::skip(),
				hfp::rfc::ows() >> hfp::skip(),

				hfp::rfc::token() >> hfp::to_lower() >>
						&value_with_opt_params_t::param_t::first,

				hfp::optional< std::string >(
					hfp::symbol('=') >> hfp::skip(),

					hfp::alternatives< std::string >(
						hfp::rfc::token() >> hfp::to_lower(),
						hfp::rfc::quoted_string()
					) >> hfp::as_result()

				) >> &value_with_opt_params_t::param_t::second

			) >> &value_with_opt_params_t::m_params
		);
	};

	{
		const auto result = try_parse("just-value");

		REQUIRE( result.first );
		REQUIRE( "just-value" == result.second.m_value );
		REQUIRE( result.second.m_params.empty() );
	}

	{
		const auto result = try_parse("just-value;one");

		REQUIRE( result.first );
		REQUIRE( "just-value" == result.second.m_value );

		REQUIRE( 1 == result.second.m_params.size() );

		REQUIRE( "one" == result.second.m_params[0].first );
		REQUIRE( !result.second.m_params[0].second );
	}

	{
		const auto result = try_parse("just-value;one; two=two;three;   "
				"four=\"four = 4\"");

		REQUIRE( result.first );
		REQUIRE( "just-value" == result.second.m_value );

		REQUIRE( 4 == result.second.m_params.size() );

		REQUIRE( "one" == result.second.m_params[0].first );
		REQUIRE( !result.second.m_params[0].second );

		REQUIRE( "two" == result.second.m_params[1].first );
		REQUIRE( result.second.m_params[1].second );
		REQUIRE( "two" == *(result.second.m_params[1].second) );

		REQUIRE( "three" == result.second.m_params[2].first );
		REQUIRE( !result.second.m_params[2].second );

		REQUIRE( "four" == result.second.m_params[3].first );
		REQUIRE( result.second.m_params[3].second );
		REQUIRE( "four = 4" == *(result.second.m_params[3].second) );
	}
}

TEST_CASE( "any_number_of", "[any_number_of]" )
{
	using namespace restinio::http_field_parser;

	const auto try_parse = []( restinio::string_view_t what ) {
		const auto media_type = produce< media_type_t >(
				rfc::token() >> to_lower() >> &media_type_t::m_type,
				symbol('/') >> skip(),
				rfc::token() >> to_lower() >> &media_type_t::m_subtype );

		return try_parse_field_value< std::vector< media_type_t > >(
				what,
				any_number_of< std::vector< media_type_t > >( media_type ) >> as_result() );
	};

	{
		const auto result = try_parse( "" );

		REQUIRE( result.first );
		REQUIRE( result.second.empty() );
	}

	{
		const auto result = try_parse( "," );

		REQUIRE( result.first );
		REQUIRE( result.second.empty() );
	}

	{
		const auto result = try_parse( ",,,," );

		REQUIRE( result.first );
		REQUIRE( result.second.empty() );
	}

	{
		const auto result = try_parse( ",  ,     ,    ,  " );

		REQUIRE( result.first );
		REQUIRE( result.second.empty() );
	}

	{
		const auto result = try_parse( "text/plain" );

		REQUIRE( result.first );

		std::vector< media_type_t > expected{
			{ "text", "plain" }
		};

		REQUIRE( expected == result.second );
	}

	{
		const auto result = try_parse( ", ,text/plain" );

		REQUIRE( result.first );

		std::vector< media_type_t > expected{
			{ "text", "plain" }
		};

		REQUIRE( expected == result.second );
	}

	{
		const auto result = try_parse( ", , text/plain , */*,, ,  ,   text/*," );

		REQUIRE( result.first );

		std::vector< media_type_t > expected{
			{ "text", "plain" },
			{ "*", "*" },
			{ "text", "*" }
		};

		REQUIRE( expected == result.second );
	}
}

TEST_CASE( "qvalue", "[qvalue]" )
{
	using namespace restinio::http_field_parser;
	using restinio::http_field_parser::rfc::qvalue_t;
	using untrusted = qvalue_t::untrusted;

	const auto try_parse = []( restinio::string_view_t what ) {
		return try_parse_field_value< rfc::qvalue_t >( what,
			rfc::qvalue() >> as_result() );
	};

	{
		const auto result = try_parse( "" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "Q" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "q" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "A" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "A=" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "Q=" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "q=" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "Q=0" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{0u}} == result.second );
	}

	{
		const auto result = try_parse( "q=0" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{0u}} == result.second );
	}

	{
		const auto result = try_parse( "Q=1" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( "q=1" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( "Q=0 " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{0u}} == result.second );
	}

	{
		const auto result = try_parse( "q=0 " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{0u}} == result.second );
	}

	{
		const auto result = try_parse( "Q=1 " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( "q=1 " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( "q=0." );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{0u}} == result.second );
	}

	{
		const auto result = try_parse( "q=1." );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( "q=0.000" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{0u}} == result.second );
	}

	{
		const auto result = try_parse( "q=0.1 " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{100u}} == result.second );
	}

	{
		const auto result = try_parse( "q=0.01 " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{10u}} == result.second );
	}

	{
		const auto result = try_parse( "q=0.001 " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1u}} == result.second );
	}

	{
		const auto result = try_parse( "q=1.000" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( "q=1.0  " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( "q=1.00  " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( "q=1.000  " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( "q=0.001" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1u}} == result.second );
	}

	{
		const auto result = try_parse( "q=1.001" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "q=0.321" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{321u}} == result.second );
		REQUIRE( "0.321" == result.second.as_string() );
	}
}

TEST_CASE( "weight", "[qvalue][weight]" )
{
	using namespace restinio::http_field_parser;
	using restinio::http_field_parser::rfc::qvalue_t;
	using untrusted = qvalue_t::untrusted;

	const auto try_parse = []( restinio::string_view_t what ) {
		return try_parse_field_value< rfc::qvalue_t >( what,
			rfc::weight() >> as_result() );
	};

	{
		const auto result = try_parse( "Q=0" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "q=0" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( ";Q=0" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{0u}} == result.second );
	}

	{
		const auto result = try_parse( ";q=0" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{0u}} == result.second );
	}

	{
		const auto result = try_parse( "    ;Q=0" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{0u}} == result.second );
	}

	{
		const auto result = try_parse( ";   q=0" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{0u}} == result.second );
	}

	{
		const auto result = try_parse( "       ;   q=0" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{0u}} == result.second );
	}

	{
		const auto result = try_parse( ";Q=1" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( ";q=1" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( ";q=1.0  " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( " ;   q=1.00  " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}
}

TEST_CASE( "Cache-Control Field", "[cache-control]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = cache_control_value_t::try_parse(
				"" );

		REQUIRE( !result.first );
	}

	{
		const auto result = cache_control_value_t::try_parse(
				"," );

		REQUIRE( !result.first );
	}

	{
		const auto result = cache_control_value_t::try_parse(
				",, , ,   ,  " );

		REQUIRE( !result.first );
	}

	{
		const auto result = cache_control_value_t::try_parse(
				"max-age=5" );

		REQUIRE( result.first );

		cache_control_value_t::directive_container_t expected_directives{
			{ "max-age"s, "5"s },
		};

		REQUIRE( expected_directives == result.second.m_directives );
	}

	{
		const auto result = cache_control_value_t::try_parse(
				"max-age=5, no-transform, only-if-cached, min-fresh=20" );

		REQUIRE( result.first );

		cache_control_value_t::directive_container_t expected_directives{
			{ "max-age"s, "5"s },
			{ "no-transform"s, restinio::nullopt },
			{ "only-if-cached"s, restinio::nullopt },
			{ "min-fresh"s, "20"s }
		};

		REQUIRE( expected_directives == result.second.m_directives );
	}

	{
		const auto result = cache_control_value_t::try_parse(
				", ,  ,   , max-age=5, ,,, no-transform, only-if-cached, min-fresh=20,,,,    " );

		REQUIRE( result.first );

		cache_control_value_t::directive_container_t expected_directives{
			{ "max-age"s, "5"s },
			{ "no-transform"s, restinio::nullopt },
			{ "only-if-cached"s, restinio::nullopt },
			{ "min-fresh"s, "20"s }
		};

		REQUIRE( expected_directives == result.second.m_directives );
	}
}

