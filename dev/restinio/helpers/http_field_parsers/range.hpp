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

/*!
 * @brief Value of range for the case where both ends of the range
 * are defined.
 *
 * This type will be used if a range is defined such way:
@verbatim
bytes=1000-5000,6000-7000
@endverbatim
 *
 * @since v.0.6.2
 */
template< typename T >
struct double_ended_range_t
{
	T first;
	T last;
};

/*!
 * @brief Value of range for the case where only left border of the
 * range is defined.
 *
 * This type will be used if a range is defined such way:
@verbatim
bytes=1000-
@endverbatim
 *
 * @since v.0.6.2
 */
template< typename T >
struct open_ended_range_t
{
	T first;
};

/*!
 * @brief Value of range for the case where only length of range's
 * suffix is defined.
 *
 * This type will be used if a range is defined such way:
@verbatim
bytes=-450
@endverbatim
 *
 * @since v.0.6.2
 */
template< typename T >
struct suffix_length_t
{
	T length;
};

/*!
 * @brief Variant type for all possible cases of specification for one range.
 *
 * @since v.0.6.2
 */
template< typename T >
using byte_range_spec_t = variant_t<
		double_ended_range_t<T>,
		open_ended_range_t<T>,
		suffix_length_t<T> >;

/*!
 * @brief A struct that holds a container of byte_range_specs.
 *
 * @since v.0.6.2
 */
template< typename T >
struct byte_ranges_specifier_t
{
	std::vector< byte_range_spec_t<T> > ranges;
};

/*!
 * @brief A description of a range value of units those are not "bytes".
 *
 * This type will be used for values like:
@verbatim
x-megabytes=1-45,450-1300
@endverbatim
 *
 * Please note that other_ranges_specifier_t::range_set contains the raw
 * value. E.g. for the example above range_set will hold "1-45,450-1300".
 *
 * @since v.0.6.2
 */
struct other_ranges_specifier_t
{
	std::string range_unit;
	std::string range_set;
};

/*!
 * @brief Variant type for holding parsed value of Range HTTP-field.
 *
 * @since v.0.6.2
 */
template< typename T >
using value_t = variant_t<
		byte_ranges_specifier_t<T>,
		other_ranges_specifier_t >;

/*!
 * @brief Factory for creation of a parser for byte_range_spec values.
 *
 * Creates a parser for the following rule:
@verbatim
byte-range-spec = byte-range / suffix-byte-range-spec

byte-range      = first-byte-pos "-" [ last-byte-pos ]
first-byte-pos  = 1*DIGIT
last-byte-pos   = 1*DIGIT

suffix-byte-range-spec = "-" suffix-length
suffix-length = 1*DIGIT
@endverbatim
 *
 * The parser returned produces value of byte_range_spec_t<T>.
 *
 * @since v.0.6.2
 */
template< typename T >
RESTINIO_NODISCARD
auto
make_byte_range_spec_parser()
{
	return produce< byte_range_spec_t<T> >(
			alternatives(
				produce< double_ended_range_t<T> >(
						non_negative_decimal_number_p<T>()
								>> &double_ended_range_t<T>::first,
						symbol('-'),
						non_negative_decimal_number_p<T>()
								>> &double_ended_range_t<T>::last
				) >> as_result(),
				produce< open_ended_range_t<T> >(
						non_negative_decimal_number_p<T>()
								>> &open_ended_range_t<T>::first,
						symbol('-')
				) >> as_result(),
				produce< suffix_length_t<T> >(
						symbol('-'),
						non_negative_decimal_number_p<T>()
								>> &suffix_length_t<T>::length
				) >> as_result()
			)
	);
}

/*!
 * @brief Factory for a parser of 'bytes=' prefix.
 *
 * @since v.0.6.2
 */
RESTINIO_NODISCARD
inline auto
make_bytes_prefix_parser()
{
	return sequence( exact( "bytes" ), symbol('=') );
}

/*!
 * @brief Factory for creation of a parser for byte_ranges_specifier values.
 *
 * Creates a parser for the following rule:
@verbatim
byte-ranges-specifier = bytes-unit "=" byte-range-set
byte-range-set  = 1#( byte-range-spec / suffix-byte-range-spec )
byte-range-spec = first-byte-pos "-" [ last-byte-pos ]
first-byte-pos  = 1*DIGIT
last-byte-pos   = 1*DIGIT

suffix-byte-range-spec = "-" suffix-length
suffix-length = 1*DIGIT
@endverbatim
 *
 * The parser returned produces value of byte_ranges_specifier_t<T>.
 *
 * @since v.0.6.2
 */
template< typename T >
RESTINIO_NODISCARD
auto
make_byte_ranges_specifier_parser()
{
	return produce< byte_ranges_specifier_t<T> >(
			make_bytes_prefix_parser(),
			force_only_this_alternative(
				non_empty_comma_separated_list_p<
								std::vector< byte_range_spec_t<T> > >(
						make_byte_range_spec_parser<T>()
				) >> &byte_ranges_specifier_t<T>::ranges
			)
	);
}

/*!
 * @brief Factory for creation of a parser for other_ranges_specifier values.
 *
 * Creates a parser for the following rule:
@verbatim
other-ranges-specifier = other-range-unit "=" other-range-set
other-range-set = 1*VCHAR
@endverbatim
 *
 * The parser returned produces value of other_ranges_specifier_t.
 *
 * @since v.0.6.2
 */
RESTINIO_NODISCARD
inline auto
make_other_ranges_specifier_parser()
{
	return produce< other_ranges_specifier_t >(
			token_p() >> &other_ranges_specifier_t::range_unit,
			symbol('='),
			force_only_this_alternative(
				produce< std::string >(
					repeat( 1u, N, vchar_symbol_p() >> to_container() )
				) >> &other_ranges_specifier_t::range_set
			)
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
 * \tparam T integer type for holding parsed values for byte-ranges-specifier.
 * It is expected to be type like int, unsigned int, long, unsigned long,
 * std::uint64_t, std::uint_least64_t and so on.
 *
 * @since v.0.6.2
 */
template< typename T >
struct range_value_t
{
	/*!
	 * @brief Value of range for the case where both ends of the range
	 * are defined.
	 *
	 * This type will be used if a range is defined such way:
	@verbatim
	bytes=1000-5000,6000-7000
	@endverbatim
	 *
	 * Usage example:
	 * @code
		using range_type = restinio::http_field_parsers::range_value_t<std::uint_least64_t>;
		const auto parse_result = range_type::try_parse(range_field_value);
		if(parse_result) {
			if(const auto * byte_ranges =
					restinio::get_if<range_type::byte_ranges_specifier_t>(parse_result->value)) {
				for(const auto & r : byte_ranges->ranges) {
					if(const auto * full_range =
							restinio::get_if<range_type::double_ended_range_t>(&r)) {
						... // access to full_range->first and full_range->last
					}
					else
						...
				}
			}
		}
	 * @endcode
	 *
	 * @since v.0.6.2
	 */
	using double_ended_range_t = range_details::double_ended_range_t<T>;

	/*!
	 * @brief Value of range for the case where only left border of the
	 * range is defined.
	 *
	 * This type will be used if a range is defined such way:
	@verbatim
	bytes=1000-
	@endverbatim
	 *
	 * Usage example:
	 * @code
		using range_type = restinio::http_field_parsers::range_value_t<std::uint_least64_t>;
		const auto parse_result = range_type::try_parse(range_field_value);
		if(parse_result) {
			if(const auto * byte_ranges =
					restinio::get_if<range_type::byte_ranges_specifier_t>(parse_result->value)) {
				for(const auto & r : byte_ranges->ranges) {
					if(const auto * open_range =
							restinio::get_if<range_type::open_ended_range_t>(&r)) {
						... // access to open_range->first.
					}
					else
						...
				}
			}
		}
	 * @endcode
	 * @since v.0.6.2
	 */
	using open_ended_range_t = range_details::open_ended_range_t<T>;

	/*!
	 * @brief Value of range for the case where only length of range's
	 * suffix is defined.
	 *
	 * This type will be used if a range is defined such way:
	@verbatim
	bytes=-450
	@endverbatim
	 *
	 * Usage example:
	 * @code
		using range_type = restinio::http_field_parsers::range_value_t<std::uint_least64_t>;
		const auto parse_result = range_type::try_parse(range_field_value);
		if(parse_result) {
			if(const auto * byte_ranges =
					restinio::get_if<range_type::byte_ranges_specifier_t>(parse_result->value)) {
				for(const auto & r : byte_ranges->ranges) {
					if(const auto * suffix =
							restinio::get_if<range_type::suffix_length_t>(&r)) {
						... // access to suffix->first.
					}
					else
						...
				}
			}
		}
	 * @endcode
	 *
	 * @since v.0.6.2
	 */
	using suffix_length_t = range_details::suffix_length_t<T>;

	/*!
	 * @brief Variant type for all possible cases of specification for one range.
	 *
	 * @since v.0.6.2
	 */
	using byte_range_spec_t = range_details::byte_range_spec_t<T>;

	/*!
	 * @brief A struct that holds a container of byte_range_specs.
	 *
	 * Usage example:
	 * @code
		using range_type = restinio::http_field_parsers::range_value_t<std::uint_least64_t>;
		const auto parse_result = range_type::try_parse(range_field_value);
		if(parse_result) {
			if(const auto * byte_ranges =
					restinio::get_if<range_type::byte_ranges_specifier_t>(parse_result->value)) {
				for(const auto & r : byte_ranges->ranges) {
					if(const auto * full_range =
							restinio::get_if<range_type::double_ended_range_t>(&r)) {
						... // access to full_range->first and full_range->last
					}
					else if(const auto * open_range =
							restinio::get_if<range_type::open_ended_range_t>(&r)) {
						... // access to open_range->first.
					}
					else if(const auto * suffix =
							restinio::get_if<range_type::suffix_length_t>(&r)) {
						... // access to suffix->first.
					}
				}
			}
		}
	 * @endcode
	 *
	 * @since v.0.6.2
	 */
	using byte_ranges_specifier_t = range_details::byte_ranges_specifier_t<T>;

	/*!
	 * @brief A description of a range value of units those are not "bytes".
	 *
	 * This type will be used for values like:
	@verbatim
	x-megabytes=1-45,450-1300
	@endverbatim
	 *
	 * Please note that other_ranges_specifier_t::range_set contains the raw
	 * value. E.g. for the example above range_set will hold "1-45,450-1300".
	 *
	 * @since v.0.6.2
	 */
	using other_ranges_specifier_t = range_details::other_ranges_specifier_t;

	/*!
	 * @brief Variant type for holding parsed value of Range HTTP-field.
	 *
	 * @since v.0.6.2
	 */
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

