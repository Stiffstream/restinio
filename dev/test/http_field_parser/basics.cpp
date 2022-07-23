/*
	restinio
*/

#define NOMINMAX

#include <catch2/catch.hpp>

#include <fmt/format.h>

#include <restinio/impl/include_fmtlib.hpp>

#include <restinio/helpers/easy_parser.hpp>

#include <restinio/helpers/http_field_parsers/basics.hpp>

#include <restinio/variant.hpp>

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

TEST_CASE( "non-negative decimal number", "[non_negative_decimal_number_p]" )
{
	using namespace restinio::http_field_parsers;
//	using namespace restinio::easy_parser;

	{
		const auto result =
			try_parse( "", non_negative_decimal_number_p<int>() );

		REQUIRE( !result );
	}

	{
		const auto result =
			try_parse( "1", non_negative_decimal_number_p<int>() );

		REQUIRE( result );
		REQUIRE( 1 == *result );
	}

	{
		const auto result =
			try_parse( "-1", non_negative_decimal_number_p<int>() );

		REQUIRE( !result );
	}

	{
		const auto result =
			try_parse( "123456", non_negative_decimal_number_p<int>() );

		REQUIRE( result );
		REQUIRE( 123456 == *result );
	}

	{
		const auto result =
			try_parse( "123456", non_negative_decimal_number_p<unsigned long>() );

		REQUIRE( result );
		REQUIRE( 123456u == *result );
	}

	{
		const auto result =
			try_parse( "123456w",
					produce<int>(
							non_negative_decimal_number_p<int>() >> as_result(),
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
							non_negative_decimal_number_p<unsigned long>() >>
									as_result(),
							symbol('w')
					)
			);

		REQUIRE( result );
		REQUIRE( 123456u == *result );
	}

	{
		const auto result =
			try_parse( "1234",
					non_negative_decimal_number_p<unsigned long>(
							expected_digits(6) )
			);

		REQUIRE( !result );
	}

	{
		const auto result =
			try_parse( "1234",
					non_negative_decimal_number_p<unsigned long>(
							expected_digits(2, 4) )
			);

		REQUIRE( result );
		REQUIRE( 1234u == *result );
	}

	{
		const auto result =
			try_parse( "1234",
					non_negative_decimal_number_p<unsigned long>(
							expected_digits(2, 6) )
			);

		REQUIRE( result );
		REQUIRE( 1234u == *result );
	}

	{
		const auto result =
			try_parse( "1234567",
					produce<unsigned long>(
						non_negative_decimal_number_p<unsigned long>(
								expected_digits(2, 6) ) >> as_result(),
						digit()
					)
			);

		REQUIRE( result );
		REQUIRE( 123456u == *result );
	}
}

TEST_CASE( "hexadecimal number", "[hexadecimal_number_p]" )
{
	using namespace restinio::http_field_parsers;
//	using namespace restinio::easy_parser;

	{
		const auto result =
			try_parse( "", hexadecimal_number_p<unsigned int>() );

		REQUIRE( !result );
	}

	{
		const auto result =
			try_parse( "1", hexadecimal_number_p<unsigned int>() );

		REQUIRE( result );
		REQUIRE( 1u == *result );
	}

	{
		const auto result =
			try_parse( "-1", hexadecimal_number_p<unsigned int>() );

		REQUIRE( !result );
	}

	{
		const auto result =
			try_parse( "1234", hexadecimal_number_p<unsigned int>() );

		REQUIRE( result );
		REQUIRE( 0x1234u == *result );
	}

	{
		const auto result =
			try_parse( "123456", hexadecimal_number_p<unsigned long>() );

		REQUIRE( result );
		REQUIRE( 0x123456u == *result );
	}

	{
		const auto result =
			try_parse( "1234w",
					produce<unsigned int>(
							hexadecimal_number_p<unsigned int>() >> as_result(),
							symbol('w')
					)
			);

		REQUIRE( result );
		REQUIRE( 0x1234 == *result );
	}

	{
		const auto result =
			try_parse( "aFbDw",
					produce<unsigned int>(
							hexadecimal_number_p<unsigned int>() >> as_result(),
							symbol('w')
					)
			);

		REQUIRE( result );
		REQUIRE( 0xafbd == *result );
	}

	{
		const auto result =
			try_parse( "aFbDe",
					produce<unsigned int>(
							hexadecimal_number_p<unsigned int>(
								expected_digits(4) ) >> as_result(),
							hexdigit()
					)
			);

		REQUIRE( result );
		REQUIRE( 0xafbd == *result );
	}

	{
		const auto result =
			try_parse( "aFbD",
					produce< std::pair<unsigned int, unsigned int> >(
						hexadecimal_number_p<unsigned int>( expected_digits(2) )
							>> &std::pair<unsigned int, unsigned int>::first,
						hexadecimal_number_p<unsigned int>( expected_digits(2) )
							>> &std::pair<unsigned int, unsigned int>::second
					)
			);

		REQUIRE( result );
		REQUIRE( std::make_pair(0xAFu, 0xBDu) == *result );
	}
}

TEST_CASE( "decimal number", "[decimal_number_p]" )
{
	using namespace restinio::http_field_parsers;
//	using namespace restinio::easy_parser;

	{
		const auto result =
			try_parse( "", decimal_number_p<int>() );

		REQUIRE( !result );
	}

	{
		const auto result =
			try_parse( "1", decimal_number_p<int>() );

		REQUIRE( result );
		REQUIRE( 1 == *result );
	}

	{
		const auto result =
			try_parse( "+1", decimal_number_p<int>() );

		REQUIRE( result );
		REQUIRE( 1 == *result );
	}

	{
		const auto result =
			try_parse( "-1", decimal_number_p<int>() );

		REQUIRE( result );
		REQUIRE( -1 == *result );
	}

	{
		const auto result =
			try_parse( "123456", decimal_number_p<int>() );

		REQUIRE( result );
		REQUIRE( 123456 == *result );
	}

	{
		const auto result =
			try_parse( "-123456", decimal_number_p<int>() );

		REQUIRE( result );
		REQUIRE( -123456 == *result );
	}

	{
		const auto result =
			try_parse( "123456w",
					produce<int>(
							decimal_number_p<int>() >> as_result(),
							symbol('w')
					)
			);

		REQUIRE( result );
		REQUIRE( 123456 == *result );
	}

	{
		const auto result =
			try_parse( "-32768", decimal_number_p<std::int16_t>() );

		REQUIRE( result );
		REQUIRE( -32768 == *result );
	}

	{
		const auto result =
			try_parse( "-32769", decimal_number_p<std::int16_t>() );

		REQUIRE( !result );
	}

	{
		const auto result =
			try_parse( "32767", decimal_number_p<std::int16_t>() );

		REQUIRE( result );
		REQUIRE( 32767 == *result );
	}

	{
		const auto result =
			try_parse( "32768", decimal_number_p<std::int16_t>() );

		REQUIRE( !result );
	}

	{
		const auto result =
			try_parse( "2",
					decimal_number_p<std::int16_t>( expected_digits(2) ) );

		REQUIRE( !result );
	}

	{
		const auto result =
			try_parse( "-22",
					decimal_number_p<std::int16_t>( expected_digits(2) ) );

		REQUIRE( result );
		REQUIRE( -22 == *result );
	}

	{
		const auto result =
			try_parse( "+22",
					decimal_number_p<std::int16_t>( expected_digits(2) ) );

		REQUIRE( result );
		REQUIRE( 22 == *result );
	}

	{
		const auto result =
			try_parse( "22",
					decimal_number_p<std::int16_t>( expected_digits(2) ) );

		REQUIRE( result );
		REQUIRE( 22 == *result );
	}

	{
		const auto result =
			try_parse( "2234",
					decimal_number_p<std::int16_t>( expected_digits(2, 6) ) );

		REQUIRE( result );
		REQUIRE( 2234 == *result );
	}

	{
		const auto result =
			try_parse( "2234",
					produce<std::int16_t>(
						decimal_number_p<std::int16_t>( expected_digits(2, 3) )
							>> as_result(),
						digit()
					)
			);

		REQUIRE( result );
		REQUIRE( 223 == *result );
	}
}

TEST_CASE( "any_symbol", "[any_symbol]" )
{
	using namespace restinio::easy_parser;

	struct indicator {
		char class_;
		char level_;
	};

	auto parser = produce< indicator >(
			any_symbol_p() >> &indicator::class_,
			digit_p() >> &indicator::level_ );

	{
		const auto r = try_parse( "c", parser );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse( "1", parser );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse( "1c", parser );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse( "cc", parser );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse( "c1", parser );

		REQUIRE( r );
		REQUIRE( r->class_ == 'c' );
		REQUIRE( r->level_ == '1' );
	}
}

TEST_CASE( "any_symbol with to_lower", "[any_symbol][to_lower]" )
{
	using namespace restinio::easy_parser;

	struct indicator {
		char class_;
		char level_;
	};

	auto parser = produce< indicator >(
			any_symbol_p() >> to_lower() >> &indicator::class_,
			digit_p() >> &indicator::level_ );

	{
		const auto r = try_parse( "c", parser );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse( "1", parser );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse( "1c", parser );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse( "cc", parser );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse( "c1", parser );

		REQUIRE( r );
		REQUIRE( r->class_ == 'c' );
		REQUIRE( r->level_ == '1' );
	}

	{
		const auto r = try_parse( "C1", parser );

		REQUIRE( r );
		REQUIRE( r->class_ == 'c' );
		REQUIRE( r->level_ == '1' );
	}

	{
		const auto r = try_parse( "D6", parser );

		REQUIRE( r );
		REQUIRE( r->class_ == 'd' );
		REQUIRE( r->level_ == '6' );
	}
}

TEST_CASE( "clause by reference", "[clause]" )
{
	using namespace restinio::easy_parser;

	struct dummy_t {};

	auto minus = symbol( '-' );
	auto plus = symbol( '+' );
	auto flag = caseless_symbol( 'F' );

	const auto parser = produce<dummy_t>(
			alternatives(
					sequence( minus, flag, alternatives( minus, plus ) ),
					sequence( plus, flag, alternatives( minus, plus ) ),
					sequence( flag, alternatives( minus, plus ) )
			)
		);

	{
		const auto r = try_parse( "-f+", parser );
		REQUIRE( r );
	}
}

TEST_CASE( "just_result (chunk_size case)", "[just_result]" )
{
	using namespace restinio::easy_parser;

	struct chunk_size_t
	{
		std::size_t count_{ 1u };
		std::size_t multiplier_{ 1u };
	};

	const auto total = [](const chunk_size_t & c) {
		return c.count_ * c.multiplier_;
	};

	constexpr std::size_t one_byte = 1u;
	constexpr std::size_t one_kib = 1024u * one_byte;
	constexpr std::size_t one_mib = 1024u * one_kib;

	auto parser = produce<chunk_size_t>(
			non_negative_decimal_number_p<std::size_t>()
					>> &chunk_size_t::count_,
			maybe(
				produce<std::size_t>(
					alternatives(
						caseless_symbol_p('b') >> just_result(one_byte),
						caseless_symbol_p('k') >> just_result(one_kib),
						caseless_symbol_p('m') >> just_result(one_mib)
					)
				) >> &chunk_size_t::multiplier_
			)
		);

	{
		const auto result = try_parse( "196", parser );

		REQUIRE( result );
		REQUIRE( 196u == total(*result) );
	}

	{
		const auto result = try_parse( "196B", parser );

		REQUIRE( result );
		REQUIRE( 196u == total(*result) );
	}

	{
		const auto result = try_parse( "10k", parser );

		REQUIRE( result );
		REQUIRE( (10u * one_kib) == total(*result) );
	}

	{
		const auto result = try_parse( "200K", parser );

		REQUIRE( result );
		REQUIRE( (200u * one_kib) == total(*result) );
	}

	{
		const auto result = try_parse( "400M", parser );

		REQUIRE( result );
		REQUIRE( (400u * one_mib) == total(*result) );
	}
}

TEST_CASE( "just_result (string case)", "[just_result]" )
{
	using namespace restinio::easy_parser;
	using namespace std::string_literals;

	auto parser = produce<std::string>(
			alternatives(
				caseless_exact_p("Fem") >> just_result("Female"s),
				caseless_exact_p("Mal") >> just_result("Male"s)
			)
		);

	{
		const auto result = try_parse( "fem", parser );

		REQUIRE( result );
		REQUIRE( "Female"s == *result );
	}

	{
		const auto result = try_parse( "FEM", parser );

		REQUIRE( result );
		REQUIRE( "Female"s == *result );
	}

	{
		const auto result = try_parse( "mal", parser );

		REQUIRE( result );
		REQUIRE( "Male"s == *result );
	}

	{
		const auto result = try_parse( "MAL", parser );

		REQUIRE( result );
		REQUIRE( "Male"s == *result );
	}
}

TEST_CASE( "convert", "[convert_transformer]" )
{
	using namespace restinio::easy_parser;
	using namespace std::string_literals;

	const auto parser = produce<std::string>(
			alternatives(
					symbol_p('f')
						>> convert([](char c) { return std::string(1u, c); })
						>> as_result(),
					symbol_p('m')
						>> convert([](char c) { return std::string(1u, c); })
						>> as_result()
			)
		);

	{
		const auto r = try_parse( "f", parser );
		REQUIRE( r );
		REQUIRE( "f"s == *r );
	}

	{
		const auto r = try_parse( "m", parser );
		REQUIRE( r );
		REQUIRE( "m"s == *r );
	}
}

TEST_CASE( "convert with failures", "[convert_transformer]" )
{
	using namespace restinio::easy_parser;
	using namespace std::string_literals;

	const auto parser = produce<std::string>(
			any_symbol_p()
				>> convert([](char c) ->
						restinio::expected_t< std::string, error_reason_t >
					{
						if( 'f' == c || 'm' == c )
							return std::string(1u, c);
						else
							return restinio::make_unexpected(
									error_reason_t::unexpected_character );
					})
				>> as_result()
		);

	{
		const auto r = try_parse( "f", parser );
		REQUIRE( r );
		REQUIRE( "f"s == *r );
	}

	{
		const auto r = try_parse( "m", parser );
		REQUIRE( r );
		REQUIRE( "m"s == *r );
	}

	{
		const auto r = try_parse( "z", parser );
		REQUIRE( !r );
	}
}

TEST_CASE( "convert generic case", "[convert_transformer]" )
{
	using namespace restinio::easy_parser;
	using namespace std::string_literals;

	const auto parser = produce<std::string>(
			alternatives(
					symbol_p('f')
						>> convert([](auto c) { return std::string(1u, c); })
						>> as_result(),
					symbol_p('m')
						>> convert([](auto c) { return std::string(1u, c); })
						>> as_result()
			)
		);

	{
		const auto r = try_parse( "f", parser );
		REQUIRE( r );
		REQUIRE( "f"s == *r );
	}

	{
		const auto r = try_parse( "m", parser );
		REQUIRE( r );
		REQUIRE( "m"s == *r );
	}
}

TEST_CASE( "GUID parser", "[hexdigit]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	const auto try_parse = []( restinio::string_view_t what ) {
		const auto parser = produce<std::string>(
			repeat(8u, 8u, hexdigit_p() >> to_container()),
			symbol_p('-') >> to_container(),
			repeat(3u, 3u,
				repeat(4u, 4u, hexdigit_p() >> to_container()),
				symbol_p('-') >> to_container()),
			repeat(12u, 12u, hexdigit_p() >> to_container())
		);
		return restinio::easy_parser::try_parse( what, parser );
	};

	{
		const auto result = try_parse( "" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse(
				"12345678:0000-1111-2222-123456789abc" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse(
				"12345678a0000-1111-2222-123456789abc" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse(
				"1234567x-0000-1111-2222-123456789abc" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse(
				"12345678-0000-1111-2222-123456789abc" );

		REQUIRE( result );
		REQUIRE( "12345678-0000-1111-2222-123456789abc"s == *result );
	}
}

TEST_CASE( "GUID parser to std::array", "[hexdigit][std::array]" )
{
	using namespace restinio::http_field_parsers;

	using uuid_image_t = std::array< char, 36 >;

	const auto make_from_string = []( restinio::string_view_t what ) {
		uuid_image_t result;
		std::copy( what.begin(), what.end(), &result[0] );
		return result;
	};

	const auto try_parse = []( restinio::string_view_t what ) {
		const auto parser = produce<uuid_image_t>(
			repeat(8u, 8u, hexdigit_p() >> to_container()),
			symbol_p('-') >> to_container(),
			repeat(3u, 3u,
				repeat(4u, 4u, hexdigit_p() >> to_container()),
				symbol_p('-') >> to_container()),
			repeat(12u, 12u, hexdigit_p() >> to_container())
		);
		return restinio::easy_parser::try_parse( what, parser );
	};

	{
		const auto result = try_parse( "" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse(
				"12345678:0000-1111-2222-123456789abc" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse(
				"12345678a0000-1111-2222-123456789abc" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse(
				"1234567x-0000-1111-2222-123456789abc" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse(
				"12345678-0000-1111-2222-123456789abc" );

		REQUIRE( result );
		REQUIRE( make_from_string( "12345678-0000-1111-2222-123456789abc" )
				== *result );
	}
}

TEST_CASE( "GUID parser to string", "[hexdigit][expected_digits]" )
{
	using namespace restinio::http_field_parsers;

	struct uuid_t
	{
		std::uint32_t time_low_;
		std::uint16_t time_mid_;
		std::uint16_t time_hi_and_version_;
		std::uint8_t clock_seq_hi_and_res_;
		std::uint8_t clock_seq_low_;
		std::array< std::uint8_t, 6 > node_;
	};

	const auto to_string = []( const uuid_t & v ) {
		return fmt::format(
				RESTINIO_FMT_FORMAT_STRING(
					"{:08x}-{:04x}-{:04x}-{:02x}{:02x}-"
					"{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}" ),
				v.time_low_, v.time_mid_, v.time_hi_and_version_,
				v.clock_seq_hi_and_res_, v.clock_seq_low_,
				v.node_[0], v.node_[1], v.node_[2], v.node_[3],
				v.node_[4], v.node_[5] );
	};

	const auto try_parse = []( restinio::string_view_t what ) {
		const auto x_uint32_p =
				hexadecimal_number_p<std::uint32_t>(expected_digits(8));
		const auto x_uint16_p =
				hexadecimal_number_p<std::uint16_t>(expected_digits(4));
		const auto x_uint8_p =
				hexadecimal_number_p<std::uint8_t>(expected_digits(2));

		const auto parser = produce<uuid_t>(
			x_uint32_p >> &uuid_t::time_low_,
			symbol('-'),
			x_uint16_p >> &uuid_t::time_mid_,
			symbol('-'),
			x_uint16_p >> &uuid_t::time_hi_and_version_,
			symbol('-'),
			x_uint8_p >> &uuid_t::clock_seq_hi_and_res_,
			x_uint8_p >> &uuid_t::clock_seq_low_,
			symbol('-'),
			produce< std::array<std::uint8_t, 6> >(
				repeat( 6, 6, x_uint8_p >> to_container() )
			) >> &uuid_t::node_
		);
		return restinio::easy_parser::try_parse( what, parser );
	};

	{
		const auto result = try_parse( "" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse(
				"12345678:0000-1111-2222-123456789abc" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse(
				"12345678a0000-1111-2222-123456789abc" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse(
				"1234567x-0000-1111-2222-123456789abc" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse(
				"12345678-0000-1111-2222-123456789ABC" );

		REQUIRE( result );
		REQUIRE( "12345678-0000-1111-2222-123456789abc" == to_string( *result ) );
	}
}

TEST_CASE( "to_lower and std::array", "[hexdigit][to_lower][std::array]" )
{
	using namespace restinio::http_field_parsers;

	using uuid_image_t = std::array< char, 36 >;

	const auto make_from_string = []( restinio::string_view_t what ) {
		uuid_image_t result;
		std::copy( what.begin(), what.end(), &result[0] );
		return result;
	};

	const auto try_parse = []( restinio::string_view_t what ) {
		const auto parser = produce<uuid_image_t>(
				repeat(8u, 8u, hexdigit_p() >> to_container()),
				symbol_p('-') >> to_container(),
				repeat(3u, 3u,
					repeat(4u, 4u, hexdigit_p() >> to_container()),
					symbol_p('-') >> to_container()),
				repeat(12u, 12u, hexdigit_p() >> to_container())
			) >> to_lower();
		return restinio::easy_parser::try_parse( what, parser );
	};

	{
		const auto result = try_parse(
				"12345678-0000-1111-2222-123456789abc" );

		REQUIRE( result );
		REQUIRE( make_from_string( "12345678-0000-1111-2222-123456789abc" )
				== *result );
	}

	{
		const auto result = try_parse(
				"12AB56cd-0000-1111-2222-123456789AbC" );

		REQUIRE( result );
		REQUIRE( make_from_string( "12ab56cd-0000-1111-2222-123456789abc" )
				== *result );
	}
}

TEST_CASE( "custom_consumer and std::array", "[custom_consumer][std::array]" )
{
	using namespace restinio::http_field_parsers;

	using array_t = std::array< char, 4 >;
	struct parse_result_t {
		std::string m_data;
	};

	const auto try_parse = []( restinio::string_view_t what ) {
		const auto parser = produce<parse_result_t>(
			produce<array_t>(
				repeat(4u, 4u, hexdigit_p() >> to_container())
			) >> custom_consumer( []( parse_result_t & to, array_t && value ) {
					to.m_data.assign( &value[0], value.size() );
				} )
		);
		return restinio::easy_parser::try_parse( what, parser );
	};

	{
		const auto result = try_parse( "" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "12345678:0000-1111-2222-123456789abc" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "1234" );

		REQUIRE( result );
		REQUIRE( "1234" == result->m_data );
	}

	{
		const auto result = try_parse( "aBcD" );

		REQUIRE( result );
		REQUIRE( "aBcD" == result->m_data );
	}
}

TEST_CASE( "convert and std::array", "[convert][std::array]" )
{
	using namespace restinio::http_field_parsers;

	using array_t = std::array< char, 4 >;

	const auto try_parse = []( restinio::string_view_t what ) {
		const auto parser = produce<std::string>(
			produce<array_t>(
				repeat(4u, 4u, hexdigit_p() >> to_container())
			) >> convert( []( array_t && value ) {
					return std::string( &value[0], value.size() );
				} )
			>> as_result()
		);
		return restinio::easy_parser::try_parse( what, parser );
	};

	{
		const auto result = try_parse( "" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "12345678:0000-1111-2222-123456789abc" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "1234" );

		REQUIRE( result );
		REQUIRE( "1234" == *result );
	}

	{
		const auto result = try_parse( "aBcD" );

		REQUIRE( result );
		REQUIRE( "aBcD" == *result );
	}
}

TEST_CASE( "alternatives, as_result and variant",
		"[alternatives][as_result][token_p][variant]" )
{
	using namespace restinio::http_field_parsers;

	using book_identity = restinio::variant_t<int, std::string>;

	const auto try_parse = []( restinio::string_view_t what ) {
		const auto parser = produce<book_identity>(
				alternatives(
					non_negative_decimal_number_p<int>() >> as_result(),
					token_p() >> as_result()
				)
			);

		return restinio::easy_parser::try_parse( what, parser );
	};

	{
		const auto result = try_parse( "" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "1234" );

		REQUIRE( result );
		const auto & identity = *result;
		const auto * n = restinio::get_if<int>(&identity);
		REQUIRE( n );
		REQUIRE( 1234 == *n );
	}

	{
		const auto result = try_parse( "SomeName" );

		REQUIRE( result );
		const auto & identity = *result;
		const auto * n = restinio::get_if<std::string>(&identity);
		REQUIRE( n );
		REQUIRE( "SomeName" == *n );
	}
}

TEST_CASE( "force_only_this_alternative",
		"[alternatives][force_only_this_alternative]" )
{
	using namespace restinio::http_field_parsers;

	struct increment_t { int m_v; };
	struct decrement_t { int m_v; };

	using inc_or_dec_t = restinio::variant_t<increment_t, decrement_t>;

	const auto try_parse = []( restinio::string_view_t what ) {
		return restinio::easy_parser::try_parse( what,
				produce<inc_or_dec_t>(
					alternatives(
						produce<increment_t>(
							exact("increment"),
							force_only_this_alternative(
								symbol('='),
								decimal_number_p<int>() >> &increment_t::m_v
							)
						) >> as_result(),
						produce<decrement_t>(
							token_p() >> skip(),
							force_only_this_alternative(
								symbol(' '),
								decimal_number_p<int>() >> &decrement_t::m_v
							)
						) >> as_result()
					)
				) );
	};

	{
		const auto result = try_parse( "increment=234" );

		REQUIRE( result );
		REQUIRE( 0 == result->index() );
		REQUIRE( 234 == restinio::get<increment_t>(*result).m_v );
	}

	{
		const auto result = try_parse( "increment 234" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "increment=2a4" );

		REQUIRE( !result );
	}

	{
		const auto result = try_parse( "decrement 234" );

		REQUIRE( result );
		REQUIRE( 1 == result->index() );
		REQUIRE( 234 == restinio::get<decrement_t>(*result).m_v );
	}
}

TEST_CASE( "token", "[token]" )
{
	using namespace restinio::http_field_parsers;

	const auto try_parse = []( restinio::string_view_t what ) {
		return restinio::easy_parser::try_parse( what, token_p() );
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
						token_p() >> to_lower() >> as_result() )
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
					token_p() >> &result_t::first,
					maybe(
						symbol('/'),
						token_p() >> &result_t::second
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
						token_p() >> &result_t::first,
						symbol('/'),
						token_p() >> &result_t::second
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
					token_p() >> &result_t::first,
					symbol('/'),
					token_p() >> &result_t::second,
					not_clause(
						symbol(';'),
						symbol('q')
					),
					maybe(
						symbol(';'),
						token_p() >> &result_t::third
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
					token_p() >> &result_t::first,
					symbol('/'),
					token_p() >> &result_t::second,
					and_clause(
						symbol(';'),
						symbol('q')
					),
					symbol(';'),
					token_p() >> &result_t::third
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
				token_p() >> &media_type_t::m_type,
				alternatives(
					symbol('/'),
					symbol('='),
					symbol('[')
				),
				token_p() >> &media_type_t::m_subtype )
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
			token_p() >> to_lower() >> to_container(),
			repeat( 0, N,
				alternatives(symbol(','), symbol(';')),
				token_p() >> to_lower() >> to_container()
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
						token_p() >> &media_type_t::m_type,
						symbol('/'),
						token_p() >> &media_type_t::m_subtype
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
							token_p() >> &pairs_holder_t::value_t::first,
							symbol('='),
							token_p() >> &pairs_holder_t::value_t::second
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
							token_p() >> &pairs_holder_t::value_t::first,
							symbol('='),
							token_p() >> &pairs_holder_t::value_t::second
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
						symbol_p('*') >> to_container()
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
					token_p() >> to_lower() >> &media_type_t::m_type,
					symbol('/'),
					token_p() >> to_lower() >> &media_type_t::m_subtype
				) >> &content_type_t::m_media_type,

				produce< std::map<std::string, std::string> >(
					repeat( 0, N,
						produce< std::pair<std::string, std::string> >(
							symbol(';'),
							ows(),

							token_p() >> to_lower() >>
									&std::pair<std::string, std::string>::first,

							symbol('='),

							produce< std::string >(
								alternatives(
									token_p()
											>> to_lower()
											>> as_result(),
									quoted_string_p()
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
				token_p() >> to_lower() >>
						&value_with_opt_params_t::m_value,

				produce< value_with_opt_params_t::param_storage_t >(
					repeat( 0, N,
						produce< value_with_opt_params_t::param_t >(
							symbol(';'),
							ows(),

							token_p() >> to_lower() >>
									&value_with_opt_params_t::param_t::first,

							produce< restinio::optional_t<std::string> >(
								maybe(
									symbol('='),

									alternatives(
										token_p() >> to_lower() >> as_result(),
										quoted_string_p() >> as_result()
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
						token_p() >> &accumulator_t::m_one,
						symbol(';') ),
					sequence(
						symbol('1'), symbol('='),
						token_p() >> &accumulator_t::m_one,
						symbol(','), symbol('2'), symbol('='),
						token_p() >> &accumulator_t::m_two,
						symbol(';') ),
					sequence(
						symbol('1'), symbol('='),
						token_p() >> &accumulator_t::m_one,
						symbol(','), symbol('2'), symbol('='),
						token_p() >> &accumulator_t::m_two,
						symbol(','), symbol('3'), symbol('='),
						token_p() >> &accumulator_t::m_three,
						symbol(';') ),
					sequence(
						symbol('1'), symbol('='),
						token_p() >> skip(),
						symbol(','), symbol('2'), symbol('='),
						token_p() >> skip(),
						symbol(','), symbol('3'), symbol('='),
						token_p() >> &accumulator_t::m_three,
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
		return restinio::easy_parser::try_parse( what, qvalue_p() );
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
		return restinio::easy_parser::try_parse( what, weight_p() );
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
				token_p() >> to_lower() >> &media_type_t::m_type,
				symbol('/'),
				token_p() >> to_lower() >> &media_type_t::m_subtype );

		return restinio::easy_parser::try_parse(
				what,
				non_empty_comma_separated_list_p<
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
				token_p() >> to_lower() >> &media_type_t::m_type,
				symbol('/'),
				token_p() >> to_lower() >> &media_type_t::m_subtype );

		return restinio::easy_parser::try_parse(
				what,
				maybe_empty_comma_separated_list_p<
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
				comment_p() 
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
		REQUIRE( "abc(def)ghk" == *result );
	}

	{
		const auto result = try_parse("(a(b(c)d(e)((f))))");

		REQUIRE( result );
		REQUIRE( "abcdef" == *result );
	}
}

TEST_CASE( "IPv4 parser", "[non_negative_decimal_number_p]" )
{
	namespace ep = restinio::easy_parser;

	const auto one_group = ep::non_negative_decimal_number_p<std::uint8_t>();

	const auto rule = ep::produce<std::uint32_t>(
			ep::produce< std::array<std::uint8_t, 4> >(
				ep::repeat(3u, 3u,
					one_group >> ep::to_container(),
					ep::symbol('.')),
				one_group >> ep::to_container()
			)
			>> ep::convert( [](const auto & arr) {
					std::uint32_t result{};
					for(const auto o : arr) {
						result <<= 8;
						result |= o;
					}
					return result;
				} )
			>> ep::as_result()
		);

	{
		const auto result = ep::try_parse( "", rule );
		REQUIRE( !result );
	}

	{
		const auto result = ep::try_parse( "127.0.0.1", rule );
		REQUIRE( result );
		REQUIRE( 0x7F000001u == *result );
	}

	{
		const auto result = ep::try_parse( "192.168.1.1", rule );

		REQUIRE( result );
		REQUIRE( 0xC0A80101u == *result );
	}

	{
		const auto result = ep::try_parse( "192 168.1.1", rule );

		REQUIRE( !result );
	}


	{
		const auto result = ep::try_parse( "192.168.1.1.", rule );

		REQUIRE( !result );
	}

}
