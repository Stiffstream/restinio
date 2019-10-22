/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/easy_parser.hpp>
#include <restinio/helpers/http_field_parsers/cache-control.hpp>
#include <restinio/helpers/http_field_parsers/media-type.hpp>
#include <restinio/helpers/http_field_parsers/content-type.hpp>
#include <restinio/helpers/http_field_parsers/content-encoding.hpp>
#include <restinio/helpers/http_field_parsers/accept.hpp>
#include <restinio/helpers/http_field_parsers/content-disposition.hpp>

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

TEST_CASE( "token", "[token]" )
{
	using namespace restinio::http_field_parsers;

	const auto try_parse = []( restinio::string_view_t what ) {
		return restinio::easy_parser::try_parse( what, token_producer() );
	};

	{
		const auto result = try_parse( "" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "," );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( " multipart" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "multipart" );

		REQUIRE( result.first );
		REQUIRE( "multipart" == result.second );
	}
}

TEST_CASE( "alternatives", "[token][alternatives]" )
{
	using namespace restinio::http_field_parsers;

	const auto try_parse = []( restinio::string_view_t what ) {
		return restinio::easy_parser::try_parse( what,
				produce< std::string >(
					alternatives(
						symbol(','),
						token_producer() >> to_lower() >> as_result() )
				)
			);
	};

	{
		const auto result = try_parse( "," );

		REQUIRE( result.first );
		REQUIRE( result.second.empty() );
	}

	{
		const auto result = try_parse( "multipart" );

		REQUIRE( result.first );
		REQUIRE( "multipart" == result.second );
	}

	{
		const auto result = try_parse( "MultiPart" );

		REQUIRE( result.first );
		REQUIRE( "multipart" == result.second );
	}
}

TEST_CASE( "maybe", "[token][maybe]" )
{
	using namespace restinio::http_field_parsers;

	using result_t = std::pair< std::string, std::string >;

	const auto try_parse = []( restinio::string_view_t what ) {
		return restinio::easy_parser::try_parse( what,
				produce< result_t >(
					token_producer() >> &result_t::first,
					maybe(
						symbol('/'),
						token_producer() >> &result_t::second
					)
				)
			);
	};

	{
		const auto result = try_parse( "text" );

		REQUIRE( result.first );
		REQUIRE( "text" == result.second.first );
		REQUIRE( result.second.second.empty() );
	}

	{
		const auto result = try_parse( "text/*" );

		REQUIRE( result.first );
		REQUIRE( "text" == result.second.first );
		REQUIRE( "*" == result.second.second );
	}
}

TEST_CASE( "sequence", "[token][sequence]" )
{
	using namespace restinio::http_field_parsers;

	using result_t = std::pair< std::string, std::string >;

	const auto try_parse = []( restinio::string_view_t what ) {
		return restinio::easy_parser::try_parse( what,
				produce< result_t >(
					sequence(
						token_producer() >> &result_t::first,
						symbol('/'),
						token_producer() >> &result_t::second
					)
				)
			);
	};

	{
		const auto result = try_parse( "text/plain" );

		REQUIRE( result.first );
		REQUIRE( "text" == result.second.first );
		REQUIRE( "plain" == result.second.second );
	}

	{
		const auto result = try_parse( "text/*" );

		REQUIRE( result.first );
		REQUIRE( "text" == result.second.first );
		REQUIRE( "*" == result.second.second );
	}
}

TEST_CASE( "not", "[token][not]" )
{
	using namespace restinio::http_field_parsers;

	struct result_t
	{
		std::string first;
		std::string second;
		std::string third;
	};

	const auto try_parse = []( restinio::string_view_t what ) {
		return restinio::easy_parser::try_parse( what,
				produce< result_t >(
					token_producer() >> &result_t::first,
					symbol('/'),
					token_producer() >> &result_t::second,
					not_clause(
						symbol(';'),
						symbol('q')
					),
					maybe(
						symbol(';'),
						token_producer() >> &result_t::third
					)
				)
			);
	};

	{
		const auto result = try_parse( "text/plain" );

		REQUIRE( result.first );
		REQUIRE( "text" == result.second.first );
		REQUIRE( "plain" == result.second.second );
	}

	{
		const auto result = try_parse( "text/plain;default" );

		REQUIRE( result.first );
		REQUIRE( "text" == result.second.first );
		REQUIRE( "plain" == result.second.second );
		REQUIRE( "default" == result.second.third );
	}

	{
		const auto result = try_parse( "text/plain;q" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "text/plain;qq" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "text/plain;Q" );

		REQUIRE( result.first );
		REQUIRE( "text" == result.second.first );
		REQUIRE( "plain" == result.second.second );
		REQUIRE( "Q" == result.second.third );
	}
}

TEST_CASE( "and", "[token][and]" )
{
	using namespace restinio::http_field_parsers;

	struct result_t
	{
		std::string first;
		std::string second;
		std::string third;
	};

	const auto try_parse = []( restinio::string_view_t what ) {
		return restinio::easy_parser::try_parse( what,
				produce< result_t >(
					token_producer() >> &result_t::first,
					symbol('/'),
					token_producer() >> &result_t::second,
					and_clause(
						symbol(';'),
						symbol('q')
					),
					symbol(';'),
					token_producer() >> &result_t::third
				)
			);
	};

	{
		const auto result = try_parse( "text/plain" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "text/plain;default" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "text/plain;q" );

		REQUIRE( result.first );
		REQUIRE( "text" == result.second.first );
		REQUIRE( "plain" == result.second.second );
		REQUIRE( "q" == result.second.third );
	}

	{
		const auto result = try_parse( "text/plain;qq" );

		REQUIRE( result.first );
		REQUIRE( "text" == result.second.first );
		REQUIRE( "plain" == result.second.second );
		REQUIRE( "qq" == result.second.third );
	}

	{
		const auto result = try_parse( "text/plain;Q" );

		REQUIRE( !result.first );
	}
}

TEST_CASE( "alternatives with symbol", "[alternatives][symbol][field_setter]" )
{
	using namespace restinio::http_field_parsers;

	const auto try_parse = [](restinio::string_view_t what) {
		return restinio::easy_parser::try_parse(
			what,
			produce< media_type_t >(
				token_producer() >> &media_type_t::m_type,
				alternatives(
					symbol('/'),
					symbol('='),
					symbol('[')
				),
				token_producer() >> &media_type_t::m_subtype )
		);
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
	using namespace restinio::http_field_parsers;

	struct media_type_holder_t {
		media_type_t m_media;
	};

	const auto try_parse = [](restinio::string_view_t what) {
		return restinio::easy_parser::try_parse(
				what,
				produce< media_type_holder_t >(
					produce< media_type_t >(
						token_producer() >> &media_type_t::m_type,
						symbol('/'),
						token_producer() >> &media_type_t::m_subtype
					) >> &media_type_holder_t::m_media
				)
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

TEST_CASE( "simple repeat (vector target)", "[repeat][vector]" )
{
	using namespace restinio::http_field_parsers;

	struct pairs_holder_t
	{
		using value_t = std::pair<std::string, std::string>;
		using container_t = std::vector< value_t >;

		container_t m_pairs;
	};

	const auto result = restinio::easy_parser::try_parse(
			";name1=value;name2=value2",
			produce< pairs_holder_t >(
				produce< pairs_holder_t::container_t >(
					repeat( 0, N,
						produce< pairs_holder_t::value_t >(
							symbol(';'),
							token_producer() >> &pairs_holder_t::value_t::first,
							symbol('='),
							token_producer() >> &pairs_holder_t::value_t::second
						) >> to_container()
					)
				) >> &pairs_holder_t::m_pairs
			)
		);

	REQUIRE( result.first );
	REQUIRE( 2 == result.second.m_pairs.size() );
	REQUIRE( "name1" == result.second.m_pairs[0].first );
	REQUIRE( "value" == result.second.m_pairs[0].second );
	REQUIRE( "name2" == result.second.m_pairs[1].first );
	REQUIRE( "value2" == result.second.m_pairs[1].second );
}

TEST_CASE( "simple repeat (map target)", "[repeat][map]" )
{
	using namespace restinio::http_field_parsers;

	struct pairs_holder_t
	{
		using value_t = std::pair<std::string, std::string>;
		using container_t = std::map< std::string, std::string >;

		std::map< std::string, std::string > m_pairs;
	};

	const auto result = restinio::easy_parser::try_parse(
			";name1=value;name2=value2",
			produce< pairs_holder_t >(
				produce< pairs_holder_t::container_t >(
					repeat( 0, N,
						produce< pairs_holder_t::value_t >(
							symbol(';'),
							token_producer() >> &pairs_holder_t::value_t::first,
							symbol('='),
							token_producer() >> &pairs_holder_t::value_t::second
						) >> to_container()
					)
				) >> &pairs_holder_t::m_pairs
			)
		);

	REQUIRE( result.first );
	REQUIRE( 2 == result.second.m_pairs.size() );

	const std::map< std::string, std::string > expected{
			{ "name1", "value" }, { "name2", "value2" }
	};

	REQUIRE( expected == result.second.m_pairs );
}

TEST_CASE( "simple repeat (string)", "[repeat][string][symbol_producer]" )
{
	using namespace restinio::http_field_parsers;

	const auto try_parse = []( restinio::string_view_t what ) {
		return restinio::easy_parser::try_parse(
			what,
			produce< std::string >(
					repeat( 3, 7,
						symbol_producer('*') >> to_container()
					)
				)
			);
	};

	{
		const auto result = try_parse( "" );
		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "**" );
		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "***" );
		REQUIRE( result.first );
		REQUIRE( "***" == result.second );
	}

	{
		const auto result = try_parse( "*****" );
		REQUIRE( result.first );
		REQUIRE( "*****" == result.second );
	}

	{
		const auto result = try_parse( "*******" );
		REQUIRE( result.first );
		REQUIRE( "*******" == result.second );
	}

	{
		const auto result = try_parse( "********" );
		REQUIRE( !result.first );
	}
}

TEST_CASE( "simple content_type", "[content_type]" )
{
	using namespace restinio::http_field_parsers;

	const auto try_parse = [](restinio::string_view_t what) {
		return restinio::easy_parser::try_parse(
			what,
			produce< content_type_t >(
				produce< media_type_t >(
					token_producer() >> to_lower() >> &media_type_t::m_type,
					symbol('/'),
					token_producer() >> to_lower() >> &media_type_t::m_subtype
				) >> &content_type_t::m_media_type,

				produce< std::map<std::string, std::string> >(
					repeat( 0, N,
						produce< std::pair<std::string, std::string> >(
							symbol(';'),
							ows(),

							token_producer() >> to_lower() >>
									&std::pair<std::string, std::string>::first,

							symbol('='),

							produce< std::string >(
								alternatives(
									token_producer()
											>> to_lower()
											>> as_result(),
									quoted_string_producer()
											>> as_result()
								)
							) >> &std::pair<std::string, std::string>::second
						) >> to_container()
					)
				) >> &content_type_t::m_parameters
			)
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
	using namespace restinio::http_field_parsers;

	const auto try_parse = [](restinio::string_view_t what) {
		return restinio::easy_parser::try_parse(
			what,
			produce< value_with_opt_params_t >(
				token_producer() >> to_lower() >>
						&value_with_opt_params_t::m_value,

				produce< value_with_opt_params_t::param_storage_t >(
					repeat( 0, N,
						produce< value_with_opt_params_t::param_t >(
							symbol(';'),
							ows(),

							token_producer() >> to_lower() >>
									&value_with_opt_params_t::param_t::first,

							produce< restinio::optional_t<std::string> >(
								maybe(
									symbol('='),

									alternatives(
										token_producer() >> to_lower() >> as_result(),
										quoted_string_producer() >> as_result()
									)
								)
							) >> &value_with_opt_params_t::param_t::second
						) >> to_container()
					)
				) >> &value_with_opt_params_t::m_params
			)
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

TEST_CASE( "rollback on backtracking", "[rollback][alternative]" )
{
	using namespace restinio::http_field_parsers;

	struct accumulator_t {
		std::string m_one;
		std::string m_two;
		std::string m_three;
	};

	const auto try_parse = []( restinio::string_view_t what ) {
		return restinio::easy_parser::try_parse( what,
			produce< accumulator_t >(
				alternatives(
					sequence(
						symbol('1'), symbol('='),
						token_producer() >> &accumulator_t::m_one,
						symbol(';') ),
					sequence(
						symbol('1'), symbol('='),
						token_producer() >> &accumulator_t::m_one,
						symbol(','), symbol('2'), symbol('='),
						token_producer() >> &accumulator_t::m_two,
						symbol(';') ),
					sequence(
						symbol('1'), symbol('='),
						token_producer() >> &accumulator_t::m_one,
						symbol(','), symbol('2'), symbol('='),
						token_producer() >> &accumulator_t::m_two,
						symbol(','), symbol('3'), symbol('='),
						token_producer() >> &accumulator_t::m_three,
						symbol(';') ),
					sequence(
						symbol('1'), symbol('='),
						token_producer() >> skip(),
						symbol(','), symbol('2'), symbol('='),
						token_producer() >> skip(),
						symbol(','), symbol('3'), symbol('='),
						token_producer() >> &accumulator_t::m_three,
						symbol(','), symbol(',') )
					)
				)
			);
	};

	{
		const auto result = try_parse("1=a;");

		REQUIRE( result.first );
		REQUIRE( "a" == result.second.m_one );
	}

	{
		const auto result = try_parse("1=a2,2=b2,3=c2;");

		REQUIRE( result.first );
		REQUIRE( "a2" == result.second.m_one );
		REQUIRE( "b2" == result.second.m_two );
		REQUIRE( "c2" == result.second.m_three );
	}

	{
		const auto result = try_parse("1=aa,2=bb,3=cc,,");

		REQUIRE( result.first );
		REQUIRE( "" == result.second.m_one );
		REQUIRE( "" == result.second.m_two );
		REQUIRE( "cc" == result.second.m_three );
	}
}

TEST_CASE( "qvalue", "[qvalue]" )
{
	using namespace restinio::http_field_parsers;
	using untrusted = qvalue_t::untrusted;

	const auto try_parse = []( restinio::string_view_t what ) {
		return restinio::easy_parser::try_parse( what, qvalue_producer() );
	};

	{
		const auto result = try_parse( "" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "0" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{0u}} == result.second );
	}

	{
		const auto result = try_parse( "1" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( "0 " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{0u}} == result.second );
	}

	{
		const auto result = try_parse( "1 " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( "0." );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{0u}} == result.second );
	}

	{
		const auto result = try_parse( "1." );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( "0.000" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{0u}} == result.second );
	}

	{
		const auto result = try_parse( "0.1 " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{100u}} == result.second );
	}

	{
		const auto result = try_parse( "0.01 " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{10u}} == result.second );
	}

	{
		const auto result = try_parse( "0.001 " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1u}} == result.second );
	}

	{
		const auto result = try_parse( "1.000" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( "1.0  " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( "1.00  " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( "1.000  " );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1000u}} == result.second );
	}

	{
		const auto result = try_parse( "0.001" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{1u}} == result.second );
	}

	{
		const auto result = try_parse( "1.001" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "0.321" );

		REQUIRE( result.first );
		REQUIRE( qvalue_t{untrusted{321u}} == result.second );
		REQUIRE( "0.321" == result.second.as_string() );
	}
}

TEST_CASE( "weight", "[qvalue][weight]" )
{
	using namespace restinio::http_field_parsers;
	using untrusted = qvalue_t::untrusted;

	const auto try_parse = []( restinio::string_view_t what ) {
		return restinio::easy_parser::try_parse( what, weight_producer() );
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
		const auto result = try_parse( ";Q" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( ";q" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( ";Q=" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( ";q=" );

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

TEST_CASE( "non_empty_comma_separated_list_producer",
		"[non_empty_comma_separated_list_producer]" )
{
	using namespace restinio::http_field_parsers;

	const auto try_parse = []( restinio::string_view_t what ) {
		const auto media_type = produce< media_type_t >(
				token_producer() >> to_lower() >> &media_type_t::m_type,
				symbol('/'),
				token_producer() >> to_lower() >> &media_type_t::m_subtype );

		return restinio::easy_parser::try_parse(
				what,
				non_empty_comma_separated_list_producer<
						std::vector< media_type_t > >( media_type )
			);
	};

	{
		const auto result = try_parse( "" );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( "," );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( ",,,," );

		REQUIRE( !result.first );
	}

	{
		const auto result = try_parse( ",  ,     ,    ,  " );

		REQUIRE( !result.first );
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

TEST_CASE( "maybe_empty_comma_separated_list_producer",
		"[maybe_empty_comma_separated_list_producer]" )
{
	using namespace restinio::http_field_parsers;

	const auto try_parse = []( restinio::string_view_t what ) {
		const auto media_type = produce< media_type_t >(
				token_producer() >> to_lower() >> &media_type_t::m_type,
				symbol('/'),
				token_producer() >> to_lower() >> &media_type_t::m_subtype );

		return restinio::easy_parser::try_parse(
				what,
				maybe_empty_comma_separated_list_producer<
						std::vector< media_type_t > >( media_type ) 
			);
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

TEST_CASE( "Media-Type", "[media-type]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = media_type_value_t::try_parse(
				"" );

		REQUIRE( !result.first );
	}

	{
		const auto result = media_type_value_t::try_parse(
				"text/" );

		REQUIRE( !result.first );
	}

	{
		const auto result = media_type_value_t::try_parse(
				"/plain" );

		REQUIRE( !result.first );
	}

	{
		const auto result = media_type_value_t::try_parse(
				"text/plain" );

		REQUIRE( result.first );

		REQUIRE( "text" == result.second.m_type );
		REQUIRE( "plain" == result.second.m_subtype );
		REQUIRE( result.second.m_parameters.empty() );
	}

	{
		const auto result = media_type_value_t::try_parse(
				"TexT/pLAIn" );

		REQUIRE( result.first );

		REQUIRE( "text" == result.second.m_type );
		REQUIRE( "plain" == result.second.m_subtype );
		REQUIRE( result.second.m_parameters.empty() );
	}

	{
		const auto result = media_type_value_t::try_parse(
				"text/*; CharSet=utf-8 ;    Alternative-Coding=\"Bla Bla Bla\"" );

		REQUIRE( result.first );

		REQUIRE( "text" == result.second.m_type );
		REQUIRE( "*" == result.second.m_subtype );

		media_type_value_t::parameter_container_t expected{
			{ "charset"s, "utf-8"s },
			{ "alternative-coding"s, "Bla Bla Bla"s }
		};
		REQUIRE( expected == result.second.m_parameters );
	}

	{
		const auto result = media_type_value_t::try_parse(
				"*/*;CharSet=utf-8;Alternative-Coding=\"Bla Bla Bla\";foO=BaZ" );

		REQUIRE( result.first );

		REQUIRE( "*" == result.second.m_type );
		REQUIRE( "*" == result.second.m_subtype );

		media_type_value_t::parameter_container_t expected{
			{ "charset"s, "utf-8"s },
			{ "alternative-coding"s, "Bla Bla Bla"s },
			{ "foo"s, "BaZ"s }
		};
		REQUIRE( expected == result.second.m_parameters );
	}
}

TEST_CASE( "Content-Type", "[media-type][content-type]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = content_type_value_t::try_parse(
				"text/plain" );

		REQUIRE( result.first );

		REQUIRE( "text" == result.second.m_media_type.m_type );
		REQUIRE( "plain" == result.second.m_media_type.m_subtype );
		REQUIRE( result.second.m_media_type.m_parameters.empty() );
	}

	{
		const auto result = content_type_value_t::try_parse(
				"TexT/pLAIn" );

		REQUIRE( result.first );

		REQUIRE( "text" == result.second.m_media_type.m_type );
		REQUIRE( "plain" == result.second.m_media_type.m_subtype );
		REQUIRE( result.second.m_media_type.m_parameters.empty() );
	}

	{
		const auto result = content_type_value_t::try_parse(
				"text/*; CharSet=utf-8 ;    Alternative-Coding=\"Bla Bla Bla\"" );

		REQUIRE( result.first );

		REQUIRE( "text" == result.second.m_media_type.m_type );
		REQUIRE( "*" == result.second.m_media_type.m_subtype );

		media_type_value_t::parameter_container_t expected{
			{ "charset"s, "utf-8"s },
			{ "alternative-coding"s, "Bla Bla Bla"s }
		};
		REQUIRE( expected == result.second.m_media_type.m_parameters );
	}

	{
		const auto result = content_type_value_t::try_parse(
				"*/*;CharSet=utf-8;Alternative-Coding=\"Bla Bla Bla\";foO=BaZ" );

		REQUIRE( result.first );

		REQUIRE( "*" == result.second.m_media_type.m_type );
		REQUIRE( "*" == result.second.m_media_type.m_subtype );

		media_type_value_t::parameter_container_t expected{
			{ "charset"s, "utf-8"s },
			{ "alternative-coding"s, "Bla Bla Bla"s },
			{ "foo"s, "BaZ"s }
		};
		REQUIRE( expected == result.second.m_media_type.m_parameters );
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

TEST_CASE( "Content-Encoding", "[content-encoding]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = content_encoding_value_t::try_parse(
				"" );

		REQUIRE( !result.first );
	}

	{
		const auto result = content_encoding_value_t::try_parse(
				"compress/" );

		REQUIRE( !result.first );
	}

	{
		const auto result = content_encoding_value_t::try_parse(
				"compress" );

		REQUIRE( result.first );

		const content_encoding_value_t::value_container_t expected{
			"compress"s
		};

		REQUIRE( expected == result.second.m_values );
	}

	{
		const auto result = content_encoding_value_t::try_parse(
				"X-Compress" );

		REQUIRE( result.first );

		const content_encoding_value_t::value_container_t expected{
			"x-compress"s
		};

		REQUIRE( expected == result.second.m_values );
	}

	{
		const auto result = content_encoding_value_t::try_parse(
				"gzip, X-Compress  ,     deflate" );

		REQUIRE( result.first );

		const content_encoding_value_t::value_container_t expected{
			"gzip"s,
			"x-compress"s,
			"deflate"s
		};

		REQUIRE( expected == result.second.m_values );
	}
}

TEST_CASE( "Accept", "[media-type][accept]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = accept_value_t::try_parse(
				"" );

		REQUIRE( result.first );

		REQUIRE( result.second.m_items.empty() );
	}

	{
		const auto result = accept_value_t::try_parse(
				"text/" );

		REQUIRE( !result.first );
	}

	{
		const auto result = accept_value_t::try_parse(
				"/plain" );

		REQUIRE( !result.first );
	}

	{
		const auto result = accept_value_t::try_parse(
				"text/plain" );

		REQUIRE( result.first );

		REQUIRE( 1 == result.second.m_items.size() );

		const auto & item = result.second.m_items[0];

		REQUIRE( "text" == item.m_media_type.m_type );
		REQUIRE( "plain" == item.m_media_type.m_subtype );
		REQUIRE( item.m_media_type.m_parameters.empty() );
	}

	{
		const auto result = accept_value_t::try_parse(
				"text/*; CharSet=utf-8 ;    Alternative-Coding=\"Bla Bla Bla\"" );

		REQUIRE( result.first );

		REQUIRE( 1 == result.second.m_items.size() );

		const auto & item = result.second.m_items[0];

		REQUIRE( "text" == item.m_media_type.m_type );
		REQUIRE( "*" == item.m_media_type.m_subtype );

		media_type_value_t::parameter_container_t expected{
			{ "charset"s, "utf-8"s },
			{ "alternative-coding"s, "Bla Bla Bla"s }
		};
		REQUIRE( expected == item.m_media_type.m_parameters );
	}

	{
		const auto result = accept_value_t::try_parse(
				"text/*;CharSet=utf-8, application/json;charset=cp1251" );

		REQUIRE( result.first );

		REQUIRE( 2 == result.second.m_items.size() );

		{
			const auto & item = result.second.m_items[0];

			REQUIRE( "text" == item.m_media_type.m_type );
			REQUIRE( "*" == item.m_media_type.m_subtype );

			media_type_value_t::parameter_container_t expected{
				{ "charset"s, "utf-8"s },
			};
			REQUIRE( expected == item.m_media_type.m_parameters );
		}

		{
			const auto & item = result.second.m_items[1];

			REQUIRE( "application" == item.m_media_type.m_type );
			REQUIRE( "json" == item.m_media_type.m_subtype );

			media_type_value_t::parameter_container_t expected{
				{ "charset"s, "cp1251"s },
			};
			REQUIRE( expected == item.m_media_type.m_parameters );
		}
	}

	{
		const auto result = accept_value_t::try_parse(
				"text/plain;q=0.5;signed;signature-method=sha512, "
				"text/*;CharSet=utf-8, "
				"application/json;charset=cp1251" );

		REQUIRE( result.first );

		REQUIRE( 3 == result.second.m_items.size() );

		{
			const auto & item = result.second.m_items[0];
			REQUIRE( "text" == item.m_media_type.m_type );
			REQUIRE( "plain" == item.m_media_type.m_subtype );
			REQUIRE( item.m_media_type.m_parameters.empty() );

			REQUIRE( item.m_weight );
			REQUIRE( qvalue_t{ qvalue_t::untrusted{500} } ==
					*item.m_weight );

			accept_value_t::item_t::accept_ext_container_t expected{
				{ "signed"s, restinio::nullopt },
				{ "signature-method"s, "sha512"s }
			};
			REQUIRE( expected == item.m_accept_params );
		}

		{
			const auto & item = result.second.m_items[1];

			REQUIRE( "text" == item.m_media_type.m_type );
			REQUIRE( "*" == item.m_media_type.m_subtype );

			media_type_value_t::parameter_container_t expected{
				{ "charset"s, "utf-8"s },
			};
			REQUIRE( expected == item.m_media_type.m_parameters );
		}

		{
			const auto & item = result.second.m_items[2];

			REQUIRE( "application" == item.m_media_type.m_type );
			REQUIRE( "json" == item.m_media_type.m_subtype );

			media_type_value_t::parameter_container_t expected{
				{ "charset"s, "cp1251"s },
			};
			REQUIRE( expected == item.m_media_type.m_parameters );
		}
	}
}

TEST_CASE( "Content-Disposition", "[content-disposition]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = content_disposition_value_t::try_parse(
				"form-data" );

		REQUIRE( result.first );

		REQUIRE( "form-data" == result.second.m_value );
		REQUIRE( result.second.m_parameters.empty() );
	}

	{
		const auto result = content_disposition_value_t::try_parse(
				"form-data; name=some-name" );

		REQUIRE( result.first );

		REQUIRE( "form-data" == result.second.m_value );

		content_disposition_value_t::parameter_container_t expected{
			{ "name"s, "some-name"s },
		};
		REQUIRE( expected == result.second.m_parameters );
	}

	{
		const auto result = content_disposition_value_t::try_parse(
				"form-data; name=some-name  ;  filename=\"file\"" );

		REQUIRE( result.first );

		REQUIRE( "form-data" == result.second.m_value );

		content_disposition_value_t::parameter_container_t expected{
			{ "name"s, "some-name"s },
			{ "filename"s, "file"s },
		};
		REQUIRE( expected == result.second.m_parameters );
	}

	{
		const auto result = content_disposition_value_t::try_parse(
				"form-data; name=some-name  ;  filename=\"file\""
				";filename*=\"another name\"");

		REQUIRE( result.first );

		REQUIRE( "form-data" == result.second.m_value );

		content_disposition_value_t::parameter_container_t expected{
			{ "name"s, "some-name"s },
			{ "filename"s, "file"s },
			{ "filename*"s, "another name"s },
		};
		REQUIRE( expected == result.second.m_parameters );
	}
}

