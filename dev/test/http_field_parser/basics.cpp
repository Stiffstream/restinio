/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/easy_parser.hpp>

#include <restinio/helpers/http_field_parsers/basics.hpp>

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

TEST_CASE( "positive decimal number", "[positive_decimal_number_producer]" )
{
	using namespace restinio::http_field_parsers;
//	using namespace restinio::easy_parser;

	{
		const auto result =
			try_parse( "", positive_decimal_number_producer<int>() );

		REQUIRE( !result );
	}

	{
		const auto result =
			try_parse( "1", positive_decimal_number_producer<int>() );

		REQUIRE( result );
		REQUIRE( 1 == *result );
	}

	{
		const auto result =
			try_parse( "-1", positive_decimal_number_producer<int>() );

		REQUIRE( !result );
	}

	{
		const auto result =
			try_parse( "123456", positive_decimal_number_producer<int>() );

		REQUIRE( result );
		REQUIRE( 123456 == *result );
	}

	{
		const auto result =
			try_parse( "123456", positive_decimal_number_producer<unsigned long>() );

		REQUIRE( result );
		REQUIRE( 123456u == *result );
	}

	{
		const auto result =
			try_parse( "123456w",
					produce<int>(
							positive_decimal_number_producer<int>() >> as_result(),
							symbol('w')
					)
			);

		REQUIRE( result );
		REQUIRE( 123456 == *result );
	}

	{
		const auto result =
			try_parse( "123456w",
					produce<unsigned long>(
							positive_decimal_number_producer<unsigned long>() >>
									as_result(),
							symbol('w')
					)
			);

		REQUIRE( result );
		REQUIRE( 123456u == *result );
	}
}

TEST_CASE( "token", "[token]" )
{
	using namespace restinio::http_field_parsers;

	const auto try_parse = []( restinio::string_view_t what ) {
		return restinio::easy_parser::try_parse( what, token_producer() );
	};

	{
		const auto result = try_parse( "" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "," );

		REQUIRE( !result );
	}

	{
		const char * what = " multipart";
		const auto result = try_parse( what );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "multipart" );

		REQUIRE( result );
		REQUIRE( "multipart" == *result );
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

		REQUIRE( result );
		REQUIRE( result->empty() );
	}

	{
		const auto result = try_parse( "multipart" );

		REQUIRE( result );
		REQUIRE( "multipart" == *result );
	}

	{
		const auto result = try_parse( "MultiPart" );

		REQUIRE( result );
		REQUIRE( "multipart" == *result );
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

		REQUIRE( result );
		REQUIRE( "text" == result->first );
		REQUIRE( result->second.empty() );
	}

	{
		const auto result = try_parse( "text/*" );

		REQUIRE( result );
		REQUIRE( "text" == result->first );
		REQUIRE( "*" == result->second );
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

		REQUIRE( result );
		REQUIRE( "text" == result->first );
		REQUIRE( "plain" == result->second );
	}

	{
		const auto result = try_parse( "text/*" );

		REQUIRE( result );
		REQUIRE( "text" == result->first );
		REQUIRE( "*" == result->second );
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

		REQUIRE( result );
		REQUIRE( "text" == result->first );
		REQUIRE( "plain" == result->second );
	}

	{
		const auto result = try_parse( "text/plain;default" );

		REQUIRE( result );
		REQUIRE( "text" == result->first );
		REQUIRE( "plain" == result->second );
		REQUIRE( "default" == result->third );
	}

	{
		const auto result = try_parse( "text/plain;q" );

		REQUIRE( !result );
	}

	{
		const char * what = "text/plain;qq";
		const auto result = try_parse( what );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "text/plain;Q" );

		REQUIRE( result );
		REQUIRE( "text" == result->first );
		REQUIRE( "plain" == result->second );
		REQUIRE( "Q" == result->third );
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

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "text/plain;default" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "text/plain;q" );

		REQUIRE( result );
		REQUIRE( "text" == result->first );
		REQUIRE( "plain" == result->second );
		REQUIRE( "q" == result->third );
	}

	{
		const auto result = try_parse( "text/plain;qq" );

		REQUIRE( result );
		REQUIRE( "text" == result->first );
		REQUIRE( "plain" == result->second );
		REQUIRE( "qq" == result->third );
	}

	{
		const auto result = try_parse( "text/plain;Q" );

		REQUIRE( !result );
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
		REQUIRE( result );
		REQUIRE( "multipart" == result->m_type );
		REQUIRE( "form-data" == result->m_subtype );
	}

	{
		const auto result = try_parse( "multipart=form-data" );
		REQUIRE( result );
		REQUIRE( "multipart" == result->m_type );
		REQUIRE( "form-data" == result->m_subtype );
	}

	{
		const auto result = try_parse( "multipart[form-data" );
		REQUIRE( result );
		REQUIRE( "multipart" == result->m_type );
		REQUIRE( "form-data" == result->m_subtype );
	}

	{
		const char * what = "multipart(form-data";
		const auto result = try_parse( what );
		REQUIRE( !result );
	}
}

TEST_CASE( "simple try_parse", "[try_parse]" )
{
	using namespace restinio::http_field_parsers;

	const char * content = "first,Second,Third;Four";
	const auto tokens = try_parse(
		content,
		produce<std::vector<std::string>>(
			token_producer() >> to_lower() >> to_container(),
			repeat( 0, N,
				alternatives(symbol(','), symbol(';')),
				token_producer() >> to_lower() >> to_container()
			)
		)
	);

	if(!tokens)
		std::cout << make_error_description(tokens.error(), content) << std::endl;

	REQUIRE( tokens );

	const std::vector<std::string> expected{
		"first", "second", "third", "four"
	};
	REQUIRE( expected == *tokens );
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
		REQUIRE( result );
		REQUIRE( "multipart" == result->m_media.m_type );
		REQUIRE( "form-data" == result->m_media.m_subtype );
	}

	{
		const auto result = try_parse( "*/form-data" );
		REQUIRE( result );
		REQUIRE( "*" == result->m_media.m_type );
		REQUIRE( "form-data" == result->m_media.m_subtype );
	}

	{
		const auto result = try_parse( "multipart/*" );
		REQUIRE( result );
		REQUIRE( "multipart" == result->m_media.m_type );
		REQUIRE( "*" == result->m_media.m_subtype );
	}

	{
		const auto result = try_parse( "*/*" );
		REQUIRE( result );
		REQUIRE( "*" == result->m_media.m_type );
		REQUIRE( "*" == result->m_media.m_subtype );
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

	REQUIRE( result );
	REQUIRE( 2 == result->m_pairs.size() );
	REQUIRE( "name1" == result->m_pairs[0].first );
	REQUIRE( "value" == result->m_pairs[0].second );
	REQUIRE( "name2" == result->m_pairs[1].first );
	REQUIRE( "value2" == result->m_pairs[1].second );
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

	REQUIRE( result );
	REQUIRE( 2 == result->m_pairs.size() );

	const std::map< std::string, std::string > expected{
			{ "name1", "value" }, { "name2", "value2" }
	};

	REQUIRE( expected == result->m_pairs );
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
		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "**" );
		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "***" );
		REQUIRE( result );
		REQUIRE( "***" == *result );
	}

	{
		const auto result = try_parse( "*****" );
		REQUIRE( result );
		REQUIRE( "*****" == *result );
	}

	{
		const auto result = try_parse( "*******" );
		REQUIRE( result );
		REQUIRE( "*******" == *result );
	}

	{
		const auto result = try_parse( "********" );
		REQUIRE( !result );
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

		REQUIRE( result );
		REQUIRE( "text" == result->m_media_type.m_type );
		REQUIRE( "plain" == result->m_media_type.m_subtype );
		REQUIRE( result->m_parameters.empty() );
	}

	{
		const auto result = try_parse( "text/plain; charset=utf-8" );

		REQUIRE( result );
		REQUIRE( "text" == result->m_media_type.m_type );
		REQUIRE( "plain" == result->m_media_type.m_subtype );
		REQUIRE( !result->m_parameters.empty() );

		const std::map< std::string, std::string > expected{
				{ "charset", "utf-8" }
		};

		REQUIRE( expected == result->m_parameters );
	}

	{
		const auto result = try_parse( "text/plain;charset=utf-8" );

		REQUIRE( result );
		REQUIRE( "text" == result->m_media_type.m_type );
		REQUIRE( "plain" == result->m_media_type.m_subtype );
		REQUIRE( !result->m_parameters.empty() );

		const std::map< std::string, std::string > expected{
				{ "charset", "utf-8" }
		};

		REQUIRE( expected == result->m_parameters );
	}

	{
		const auto result = try_parse(
				"multipart/form-data; charset=utf-8; boundary=---123456" );

		REQUIRE( result );
		REQUIRE( "multipart" == result->m_media_type.m_type );
		REQUIRE( "form-data" == result->m_media_type.m_subtype );
		REQUIRE( !result->m_parameters.empty() );

		const std::map< std::string, std::string > expected{
				{ "charset", "utf-8" }, { "boundary", "---123456" }
		};

		REQUIRE( expected == result->m_parameters );
	}

	{
		const auto result = try_parse(
				R"(multipart/form-data; charset=utf-8; boundary="Text with space!")" );

		REQUIRE( result );
		REQUIRE( "multipart" == result->m_media_type.m_type );
		REQUIRE( "form-data" == result->m_media_type.m_subtype );
		REQUIRE( !result->m_parameters.empty() );

		const std::map< std::string, std::string > expected{
				{ "charset", "utf-8" }, { "boundary", "Text with space!" }
		};

		REQUIRE( expected == result->m_parameters );
	}

	{
		const auto result = try_parse(
				R"(multipart/form-data; charset=utf-8; boundary="Text with space!")" );

		REQUIRE( result );
		REQUIRE( "multipart" == result->m_media_type.m_type );
		REQUIRE( "form-data" == result->m_media_type.m_subtype );
		REQUIRE( !result->m_parameters.empty() );

		const std::map< std::string, std::string > expected{
				{ "charset", "utf-8" }, { "boundary", "Text with space!" }
		};

		REQUIRE( expected == result->m_parameters );
	}

	{
		const auto result = try_parse(
				R"(MultiPart/Form-Data; CharSet=utf-8; BOUNDARY="Text with space!")" );

		REQUIRE( result );
		REQUIRE( "multipart" == result->m_media_type.m_type );
		REQUIRE( "form-data" == result->m_media_type.m_subtype );
		REQUIRE( !result->m_parameters.empty() );

		const std::map< std::string, std::string > expected{
				{ "charset", "utf-8" }, { "boundary", "Text with space!" }
		};

		REQUIRE( expected == result->m_parameters );
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

		REQUIRE( result );
		REQUIRE( "just-value" == result->m_value );
		REQUIRE( result->m_params.empty() );
	}

	{
		const auto result = try_parse("just-value;one");

		REQUIRE( result );
		REQUIRE( "just-value" == result->m_value );

		REQUIRE( 1 == result->m_params.size() );

		REQUIRE( "one" == result->m_params[0].first );
		REQUIRE( !result->m_params[0].second );
	}

	{
		const char * what = "just-value;one; two=two;three;   four=\"four = 4\"";
		const auto result = try_parse( what );

		REQUIRE( result );
		REQUIRE( "just-value" == result->m_value );

		REQUIRE( 4 == result->m_params.size() );

		REQUIRE( "one" == result->m_params[0].first );
		REQUIRE( !result->m_params[0].second );

		REQUIRE( "two" == result->m_params[1].first );
		REQUIRE( result->m_params[1].second );
		REQUIRE( "two" == *(result->m_params[1].second) );

		REQUIRE( "three" == result->m_params[2].first );
		REQUIRE( !result->m_params[2].second );

		REQUIRE( "four" == result->m_params[3].first );
		REQUIRE( result->m_params[3].second );
		REQUIRE( "four = 4" == *(result->m_params[3].second) );
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

		REQUIRE( result );
		REQUIRE( "a" == result->m_one );
	}

	{
		const auto result = try_parse("1=a2,2=b2,3=c2;");

		REQUIRE( result );
		REQUIRE( "a2" == result->m_one );
		REQUIRE( "b2" == result->m_two );
		REQUIRE( "c2" == result->m_three );
	}

	{
		const auto result = try_parse("1=aa,2=bb,3=cc,,");

		REQUIRE( result );
		REQUIRE( "" == result->m_one );
		REQUIRE( "" == result->m_two );
		REQUIRE( "cc" == result->m_three );
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

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "0" );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{0u}} == *result );
	}

	{
		const auto result = try_parse( "1" );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{1000u}} == *result );
	}

	{
		const auto result = try_parse( "0 " );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{0u}} == *result );
	}

	{
		const auto result = try_parse( "1 " );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{1000u}} == *result );
	}

	{
		const auto result = try_parse( "0." );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{0u}} == *result );
	}

	{
		const auto result = try_parse( "1." );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{1000u}} == *result );
	}

	{
		const auto result = try_parse( "0.000" );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{0u}} == *result );
	}

	{
		const auto result = try_parse( "0.1 " );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{100u}} == *result );
	}

	{
		const auto result = try_parse( "0.01 " );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{10u}} == *result );
	}

	{
		const auto result = try_parse( "0.001 " );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{1u}} == *result );
	}

	{
		const auto result = try_parse( "1.000" );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{1000u}} == *result );
	}

	{
		const auto result = try_parse( "1.0  " );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{1000u}} == *result );
	}

	{
		const auto result = try_parse( "1.00  " );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{1000u}} == *result );
	}

	{
		const auto result = try_parse( "1.000  " );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{1000u}} == *result );
	}

	{
		const auto result = try_parse( "0.001" );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{1u}} == *result );
	}

	{
		const auto result = try_parse( "1.001" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "0.321" );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{321u}} == *result );
		REQUIRE( "0.321" == result->as_string() );
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

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "q=0" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( ";Q" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( ";q" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( ";Q=" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( ";q=" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( ";Q=0" );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{0u}} == *result );
	}

	{
		const auto result = try_parse( ";q=0" );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{0u}} == *result );
	}

	{
		const auto result = try_parse( "    ;Q=0" );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{0u}} == *result );
	}

	{
		const auto result = try_parse( ";   q=0" );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{0u}} == *result );
	}

	{
		const auto result = try_parse( "       ;   q=0" );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{0u}} == *result );
	}

	{
		const auto result = try_parse( ";Q=1" );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{1000u}} == *result );
	}

	{
		const auto result = try_parse( ";q=1" );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{1000u}} == *result );
	}

	{
		const auto result = try_parse( ";q=1.0  " );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{1000u}} == *result );
	}

	{
		const auto result = try_parse( " ;   q=1.00  " );

		REQUIRE( result );
		REQUIRE( qvalue_t{untrusted{1000u}} == *result );
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

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "," );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( ",,,," );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( ",  ,     ,    ,  " );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "text/plain" );

		REQUIRE( result );

		std::vector< media_type_t > expected{
			{ "text", "plain" }
		};

		REQUIRE( expected == *result );
	}

	{
		const auto result = try_parse( ", ,text/plain" );

		REQUIRE( result );

		std::vector< media_type_t > expected{
			{ "text", "plain" }
		};

		REQUIRE( expected == *result );
	}

	{
		const auto result = try_parse( ", , text/plain , */*,, ,  ,   text/*," );

		REQUIRE( result );

		std::vector< media_type_t > expected{
			{ "text", "plain" },
			{ "*", "*" },
			{ "text", "*" }
		};

		REQUIRE( expected == *result );
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

		REQUIRE( result );
		REQUIRE( result->empty() );
	}

	{
		const auto result = try_parse( "," );

		REQUIRE( result );
		REQUIRE( result->empty() );
	}

	{
		const auto result = try_parse( ",,,," );

		REQUIRE( result );
		REQUIRE( result->empty() );
	}

	{
		const auto result = try_parse( ",  ,     ,    ,  " );

		REQUIRE( result );
		REQUIRE( result->empty() );
	}

	{
		const auto result = try_parse( "text/plain" );

		REQUIRE( result );

		std::vector< media_type_t > expected{
			{ "text", "plain" }
		};

		REQUIRE( expected == *result );
	}

	{
		const auto result = try_parse( ", ,text/plain" );

		REQUIRE( result );

		std::vector< media_type_t > expected{
			{ "text", "plain" }
		};

		REQUIRE( expected == *result );
	}

	{
		const auto result = try_parse( ", , text/plain , */*,, ,  ,   text/*," );

		REQUIRE( result );

		std::vector< media_type_t > expected{
			{ "text", "plain" },
			{ "*", "*" },
			{ "text", "*" }
		};

		REQUIRE( expected == *result );
	}
}

TEST_CASE( "comment producer", "[comment_producer]" )
{
	using namespace restinio::http_field_parsers;

	const auto try_parse = []( restinio::string_view_t what ) {
		return restinio::easy_parser::try_parse(
				what,
				comment_producer() 
			);
	};

	{
		const auto result = try_parse("");

		REQUIRE( !result );
	}

	{
		const auto result = try_parse("(");

		REQUIRE( !result );
	}

	{
		const auto result = try_parse(")");

		REQUIRE( !result );
	}

	{
		const auto result = try_parse("()");

		REQUIRE( result );
		REQUIRE( "" == *result );
	}

	{
		const auto result = try_parse("(a)");

		REQUIRE( result );
		REQUIRE( "a" == *result );
	}

	{
		const auto result = try_parse("(abc(def)ghk)");

		REQUIRE( result );
		REQUIRE( "abcdefghk" == *result );
	}

	{
		const auto result = try_parse("(abc(def ghk)");

		REQUIRE( !result );
	}

	{
		const auto result = try_parse("(abc\\(def\\)ghk)");

		REQUIRE( result );
		REQUIRE( "abc(def)ghk" );
	}

	{
		const auto result = try_parse("(a(b(c)d(e)((f))))");

		REQUIRE( result );
		REQUIRE( "abcdef" == *result );
	}
}

