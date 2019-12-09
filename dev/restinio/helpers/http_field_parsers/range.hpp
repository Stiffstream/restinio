/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of Range HTTP-field.
 *
 * @since v.0.6.2
 */

#pragma once

#include <restinio/helpers/http_field_parsers/basics.hpp>

#include <restinio/variant.hpp>

namespace restinio
{

namespace http_field_parsers
{

namespace range_details
{

template< typename T >
struct double_ended_range_t
{
	T first;
	T last;
};

template< typename T >
struct open_ended_range_t
{
	T first;
};

template< typename T >
struct suffix_length_t
{
	T length;
};

template< typename T >
using byte_range_spec_t = variant_t<
		double_ended_range_t<T>,
		open_ended_range_t<T>,
		suffix_length_t<T> >;

template< typename T >
struct byte_ranges_specifier_t
{
	std::vector< byte_range_spec_t<T> > ranges;
};

struct other_ranges_specifier_t
{
	std::string range_unit;
	std::string range_set;
};

template< typename T >
using value_t = variant_t<
		byte_ranges_specifier_t<T>,
		other_ranges_specifier_t >;

template< typename T >
RESTINIO_NODISCARD
auto
make_byte_range_spec_parser()
{
	return produce< byte_range_spec_t<T> >(
			alternatives(
				produce< double_ended_range_t<T> >(
						positive_decimal_number_producer<T>()
								>> &double_ended_range_t<T>::first,
						symbol('-'),
						positive_decimal_number_producer<T>()
								>> &double_ended_range_t<T>::last
				) >> as_result(),
				produce< open_ended_range_t<T> >(
						positive_decimal_number_producer<T>()
								>> &open_ended_range_t<T>::first,
						symbol('-')
				) >> as_result(),
				produce< suffix_length_t<T> >(
						symbol('-'),
						positive_decimal_number_producer<T>()
								>> &suffix_length_t<T>::length
				) >> as_result()
			)
	);
}

template< typename T >
RESTINIO_NODISCARD
auto
make_byte_ranges_specifier_parser()
{
	return produce< byte_ranges_specifier_t<T> >(
			symbol('b'), symbol('y'), symbol('t'), symbol('e'), symbol('s'),
			symbol('='),
			non_empty_comma_separated_list_producer<
							std::vector< byte_range_spec_t<T> > >(
					make_byte_range_spec_parser<T>()
			) >> &byte_ranges_specifier_t<T>::ranges
	);
}

RESTINIO_NODISCARD
inline auto
make_other_ranges_specifier_parser()
{
	return produce< other_ranges_specifier_t >(
			token_producer() >> &other_ranges_specifier_t::range_unit,
			symbol('='),
			produce< std::string >(
				repeat( 1u, N, vchar_symbol_producer() >> to_container() )
			) >> &other_ranges_specifier_t::range_set
		);
}

} /* namespace range_details */

//
// range_value_t
//
/*!
 * @brief Tools for working with the value of Range HTTP-field.
 *
 * This struct represents parsed value of HTTP-field Range
 * (see https://tools.ietf.org/html/rfc7233#section-3.1 and
 * https://tools.ietf.org/html/rfc7233#section-2):
@verbatim
Range = byte-ranges-specifier / other-ranges-specifier

byte-ranges-specifier = bytes-unit "=" byte-range-set
byte-range-set  = 1#( byte-range-spec / suffix-byte-range-spec )
byte-range-spec = first-byte-pos "-" [ last-byte-pos ]
first-byte-pos  = 1*DIGIT
last-byte-pos   = 1*DIGIT

suffix-byte-range-spec = "-" suffix-length
suffix-length = 1*DIGIT

other-ranges-specifier = other-range-unit "=" other-range-set
other-range-set = 1*VCHAR
@endverbatim
 *
 * @since v.0.6.2
 */
template< typename T >
struct range_value_t
{
	using double_ended_range_t = range_details::double_ended_range_t<T>;
	using open_ended_range_t = range_details::open_ended_range_t<T>;
	using suffix_length_t = range_details::suffix_length_t<T>;
	using byte_range_spec_t = range_details::byte_range_spec_t<T>;
	using byte_ranges_specifier_t = range_details::byte_ranges_specifier_t<T>;
	using other_ranges_specifier_t = range_details::other_ranges_specifier_t;
	using value_t = range_details::value_t<T>;

	value_t value;

	/*!
	 * @brief A factory function for a parser of Range value.
	 *
	 * @since v.0.6.2
	 */
	RESTINIO_NODISCARD
	static auto
	make_parser()
	{
		using namespace range_details;

		return produce< range_value_t >(
				alternatives(
						make_byte_ranges_specifier_parser<T>() >>
								&range_value_t::value,
						make_other_ranges_specifier_parser() >>
								&range_value_t::value
				)
		);
	}

	/*!
	 * @brief An attempt to parse Range HTTP-field.
	 *
	 * @since v.0.6.2
	 */
	RESTINIO_NODISCARD
	static expected_t< range_value_t, restinio::easy_parser::parse_error_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

} /* namespace http_field_parsers */

} /* namespace restinio */

