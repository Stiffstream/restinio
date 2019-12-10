
/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/range.hpp>

namespace restinio
{

namespace http_field_parsers
{

#if 0
bool
operator==( const accept_language_value_t::item_t & a,
	const accept_language_value_t::item_t & b ) noexcept
{
	return std::tie( a.language_range, a.weight )
			== std::tie( b.language_range, b.weight );
}

std::ostream &
operator<<( std::ostream & to, const accept_language_value_t::item_t & a )
{
	return (to << a.language_range << ";" << a.weight.as_string());
}
#endif

} /* namespace http_field_parsers */

} /* namespace restinio */

TEST_CASE( "Range", "[range]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		using range_type = range_value_t<int>;

		const auto result = range_type::try_parse(
				"" );

		REQUIRE( !result );
	}

	{
		using range_type = range_value_t<int>;

		const auto result = range_type::try_parse(
				"myunits=some,units,set" );

		REQUIRE( result );

		const auto * v = restinio::get_if<range_type::other_ranges_specifier_t>(
				&(result->value) );
		REQUIRE( v );
		REQUIRE( "myunits"s == v->range_unit );
		REQUIRE( "some,units,set"s == v->range_set );
	}

	{
		using range_type = range_value_t<int>;

		const auto result = range_type::try_parse(
				"bytes=-1" );

		REQUIRE( result );

		const auto * v = restinio::get_if<range_type::byte_ranges_specifier_t>(
				&(result->value) );
		REQUIRE( v );
		REQUIRE( 1u == v->ranges.size() );

		const auto * f = restinio::get_if<range_type::suffix_length_t>(
				&( v->ranges[0] ) );
		REQUIRE( f );
		REQUIRE( 1 == f->length );
	}

	{
		using range_type = range_value_t<int>;

		const auto result = range_type::try_parse(
				"bytes=100-5000" );

		REQUIRE( result );

		const auto * v = restinio::get_if<range_type::byte_ranges_specifier_t>(
				&(result->value) );
		REQUIRE( v );
		REQUIRE( 1u == v->ranges.size() );

		const auto * f = restinio::get_if<range_type::double_ended_range_t>(
				&( v->ranges[0] ) );
		REQUIRE( f );
		REQUIRE( 100 == f->first );
		REQUIRE( 5000 == f->last );
	}

	{
		using range_type = range_value_t<int>;

		const auto result = range_type::try_parse(
				"bytes=4500-" );

		REQUIRE( result );

		const auto * v = restinio::get_if<range_type::byte_ranges_specifier_t>(
				&(result->value) );
		REQUIRE( v );
		REQUIRE( 1u == v->ranges.size() );

		const auto * f = restinio::get_if<range_type::open_ended_range_t>(
				&( v->ranges[0] ) );
		REQUIRE( f );
		REQUIRE( 4500 == f->first );
	}

	{
		using range_type = range_value_t<int>;

		const auto result = range_type::try_parse(
				"bytes=100-5000,6000-9000,-450" );

		REQUIRE( result );

		const auto * v = restinio::get_if<range_type::byte_ranges_specifier_t>(
				&(result->value) );
		REQUIRE( v );
		REQUIRE( 3u == v->ranges.size() );

		{
			const auto * f = restinio::get_if<range_type::double_ended_range_t>(
					&( v->ranges[0] ) );
			REQUIRE( f );
			REQUIRE( 100 == f->first );
			REQUIRE( 5000 == f->last );
		}
		{
			const auto * f = restinio::get_if<range_type::double_ended_range_t>(
					&( v->ranges[1] ) );
			REQUIRE( f );
			REQUIRE( 6000 == f->first );
			REQUIRE( 9000 == f->last );
		}
		{
			const auto * f = restinio::get_if<range_type::suffix_length_t>(
					&( v->ranges[2] ) );
			REQUIRE( f );
			REQUIRE( 450 == f->length );
		}
	}

	{
		using range_type = range_value_t<int>;

		const auto result = range_type::try_parse(
				"bytes=100-5000,15000-,6000-9000" );

		REQUIRE( result );

		const auto * v = restinio::get_if<range_type::byte_ranges_specifier_t>(
				&(result->value) );
		REQUIRE( v );
		REQUIRE( 3u == v->ranges.size() );

		{
			const auto * f = restinio::get_if<range_type::double_ended_range_t>(
					&( v->ranges[0] ) );
			REQUIRE( f );
			REQUIRE( 100 == f->first );
			REQUIRE( 5000 == f->last );
		}
		{
			const auto * f = restinio::get_if<range_type::open_ended_range_t>(
					&( v->ranges[1] ) );
			REQUIRE( f );
			REQUIRE( 15000 == f->first );
		}
		{
			const auto * f = restinio::get_if<range_type::double_ended_range_t>(
					&( v->ranges[2] ) );
			REQUIRE( f );
			REQUIRE( 6000 == f->first );
			REQUIRE( 9000 == f->last );
		}
	}

	{
		using range_type = range_value_t<int>;

		const auto result = range_type::try_parse(
				"bytes=-450,100-5000,6000-9000,15000-" );

		REQUIRE( result );

		const auto * v = restinio::get_if<range_type::byte_ranges_specifier_t>(
				&(result->value) );
		REQUIRE( v );
		REQUIRE( 4u == v->ranges.size() );

		{
			const auto * f = restinio::get_if<range_type::suffix_length_t>(
					&( v->ranges[0] ) );
			REQUIRE( f );
			REQUIRE( 450 == f->length );
		}
		{
			const auto * f = restinio::get_if<range_type::double_ended_range_t>(
					&( v->ranges[1] ) );
			REQUIRE( f );
			REQUIRE( 100 == f->first );
			REQUIRE( 5000 == f->last );
		}
		{
			const auto * f = restinio::get_if<range_type::double_ended_range_t>(
					&( v->ranges[2] ) );
			REQUIRE( f );
			REQUIRE( 6000 == f->first );
			REQUIRE( 9000 == f->last );
		}
		{
			const auto * f = restinio::get_if<range_type::open_ended_range_t>(
					&( v->ranges[3] ) );
			REQUIRE( f );
			REQUIRE( 15000 == f->first );
		}
	}

	{
		using range_type = range_value_t<int>;

		const auto result = range_type::try_parse(
				"Bytes=-450,100-5000,6000-9000,15000-" );

		REQUIRE( result );

		const auto * v = restinio::get_if<range_type::other_ranges_specifier_t>(
				&(result->value) );
		REQUIRE( v );
		REQUIRE( "Bytes"s == v->range_unit );
		REQUIRE( "-450,100-5000,6000-9000,15000-"s == v->range_set );
	}

	{
		using range_type = range_value_t<std::int16_t>;

		{
			const auto result = range_type::try_parse(
					"bytes=-450" );

			REQUIRE( result );

			const auto * v = restinio::get_if<range_type::byte_ranges_specifier_t>(
					&(result->value) );
			REQUIRE( v );
			REQUIRE( 1u == v->ranges.size() );

			const auto * f = restinio::get_if<range_type::suffix_length_t>(
					&( v->ranges[0] ) );
			REQUIRE( f );
			REQUIRE( 450 == f->length );
		}

		{
			const auto result = range_type::try_parse(
					"bytes=-32767" );

			REQUIRE( result );

			const auto * v = restinio::get_if<range_type::byte_ranges_specifier_t>(
					&(result->value) );
			REQUIRE( v );
			REQUIRE( 1u == v->ranges.size() );

			const auto * f = restinio::get_if<range_type::suffix_length_t>(
					&( v->ranges[0] ) );
			REQUIRE( f );
			REQUIRE( 32767 == f->length );
		}

		{
			const auto result = range_type::try_parse(
					"bytes=-32768" );

			REQUIRE( !result );
		}
	}

	{
		using range_type = range_value_t<std::uint16_t>;

		{
			const auto result = range_type::try_parse(
					"bytes=-450" );

			REQUIRE( result );

			const auto * v = restinio::get_if<range_type::byte_ranges_specifier_t>(
					&(result->value) );
			REQUIRE( v );
			REQUIRE( 1u == v->ranges.size() );

			const auto * f = restinio::get_if<range_type::suffix_length_t>(
					&( v->ranges[0] ) );
			REQUIRE( f );
			REQUIRE( 450u == f->length );
		}

		{
			const auto result = range_type::try_parse(
					"bytes=-65535" );

			REQUIRE( result );

			const auto * v = restinio::get_if<range_type::byte_ranges_specifier_t>(
					&(result->value) );
			REQUIRE( v );
			REQUIRE( 1u == v->ranges.size() );

			const auto * f = restinio::get_if<range_type::suffix_length_t>(
					&( v->ranges[0] ) );
			REQUIRE( f );
			REQUIRE( 65535u == f->length );
		}

		{
			const auto result = range_type::try_parse(
					"bytes=-65536" );

			REQUIRE( !result );
		}
	}
}

