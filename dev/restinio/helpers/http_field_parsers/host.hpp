/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of Host HTTP-field.
 *
 * @since v.0.6.9
 */

#pragma once

#include <restinio/helpers/http_field_parsers/basics.hpp>

#include <restinio/helpers/http_field_parsers/details/pct_encoded_symbols.hpp>

#include <restinio/variant.hpp>

namespace restinio
{

namespace http_field_parsers
{

namespace host_details
{

namespace ep_impl = restinio::easy_parser::impl;
namespace hfp_impl = restinio::http_field_parsers::impl;
namespace hfp_details = restinio::http_field_parsers::details;

//
// unreserved_predicate_t
//
/*!
 * @brief A preducate for symbol_producer_template that checks that
 * a symbol is unreserved symbol from RCF3986.
 *
 * See: https://tools.ietf.org/html/rfc3986#appendix-A
 *
 * @since v.0.6.9
 */
struct unreserved_predicate_t
{
	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		return hfp_impl::is_alpha(actual)
				|| hfp_impl::is_digit(actual)
				|| '-' == actual
				|| '.' == actual
				|| '_' == actual
				|| '~' == actual
				;
	}
};

//
// unreserved_symbol_producer
//
/*!
 * @brief A factory for producer that extracts unreserved symbols.
 *
 * See: https://tools.ietf.org/html/rfc3986#appendix-A
 *
 * @since v.0.6.9
 */
RESTINIO_NODISCARD
inline auto
unreserved_symbol_p()
{
	return ep_impl::symbol_producer_template_t< unreserved_predicate_t >{};
}

//
// sub_delims_predicate_t
//
/*!
 * @brief A preducate for symbol_producer_template that checks that
 * a symbol is sub-delims symbol from RCF3986.
 *
 * See: https://tools.ietf.org/html/rfc3986#appendix-A
 *
 * @since v.0.6.9
 */
struct sub_delims_predicate_t
{
	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		return '!' == actual
				|| '$' == actual
				|| '&' == actual
				|| '\'' == actual
				|| '(' == actual
				|| ')' == actual
				|| '*' == actual
				|| '+' == actual
				|| ',' == actual
				|| ';' == actual
				|| '=' == actual
				;
	}
};

//
// sub_delims_symbol_producer
//
/*!
 * @brief A factory for producer that extracts sub-delims symbols.
 *
 * See: https://tools.ietf.org/html/rfc3986#appendix-A
 *
 * @since v.0.6.9
 */
RESTINIO_NODISCARD
inline auto
sub_delims_symbol_p()
{
	return ep_impl::symbol_producer_template_t< sub_delims_predicate_t >{};
}

//
// ipv4_address_producer
//
/*!
 * @brief A factory for producer of IPv4address value.
 *
 * Produces `std::string`.
 *
 * Uses the following grammar (see https://tools.ietf.org/html/rfc3986#appendix-A):
@verbatim
IPv4address   = dec-octet "." dec-octet "." dec-octet "." dec-octet

dec-octet     = DIGIT                 ; 0-9
              / %x31-39 DIGIT         ; 10-99
              / "1" 2DIGIT            ; 100-199
              / "2" %x30-34 DIGIT     ; 200-249
              / "25" %x30-35          ; 250-255
@endverbatim
 *
 * @since v.0.6.9
 */
RESTINIO_NODISCARD
inline auto
ipv4_address_p()
{
	const auto dec_octet = produce< std::string >(
			alternatives(
				sequence(
					symbol_p('2') >> to_container(),
					symbol_p('5') >> to_container(),
					symbol_from_range_p('0', '5') >> to_container()
				),
				sequence(
					symbol_p('2') >> to_container(),
					symbol_from_range_p('0', '4') >> to_container(),
					digit_p() >> to_container()
				),
				sequence(
					symbol_p('1') >> to_container(),
					digit_p() >> to_container(),
					digit_p() >> to_container()
				),
				sequence(
					symbol_from_range_p('1', '9') >> to_container(),
					digit_p() >> to_container()
				),
				digit_p() >> to_container()
			)
		);

	return produce< std::string >(
			dec_octet >> to_container(),
			symbol_p('.') >> to_container(),
			dec_octet >> to_container(),
			symbol_p('.') >> to_container(),
			dec_octet >> to_container(),
			symbol_p('.') >> to_container(),
			dec_octet >> to_container()
		);
}

//FIXME: maybe this should be a part of easy_parser?
#if 0
struct debug_printer : public ep_impl::clause_tag
{
	std::string m_tag;

	debug_printer( std::string v ) noexcept : m_tag{ std::move(v) } {}

	template< typename Target_Type >
	RESTINIO_NODISCARD
	optional_t< parse_error_t >
	try_process( ep_impl::source_t & from, Target_Type & /*target*/ )
	{
		std::cout << "*** debug_print: " << m_tag << std::endl;

		return nullopt;
	}
};
#endif

//
// ipv6_address_producer
//
/*!
 * @brief A factory for producer of ipv6_address value.
 *
 * Produces `std::string`.
 *
 * Uses the following grammar (see https://tools.ietf.org/html/rfc3986#appendix-A):
@verbatim
   IPv6address   =                            6( h16 ":" ) ls32
                 /                       "::" 5( h16 ":" ) ls32
                 / [               h16 ] "::" 4( h16 ":" ) ls32
                 / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
                 / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
                 / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
                 / [ *4( h16 ":" ) h16 ] "::"              ls32
                 / [ *5( h16 ":" ) h16 ] "::"              h16
                 / [ *6( h16 ":" ) h16 ] "::"

   h16           = 1*4HEXDIG
   ls32          = ( h16 ":" h16 ) / IPv4address
@endverbatim
 *
 * @since v.0.6.9
 */
RESTINIO_NODISCARD
inline auto
ipv6_address_p()
{
	const auto h16 = produce< std::string >(
			repeat( 1u, 4u, hexdigit_p() >> to_container() )
		);
	const auto h16_with_colon = sequence(
			h16 >> to_container(),
			symbol_p(':') >> to_container(),
			not_clause( symbol(':') )
		);
	const auto ls32 = produce< std::string >(
			alternatives(
				sequence(
					h16 >> to_container(),
					symbol_p(':') >> to_container(),
					h16 >> to_container()
				),
				ipv4_address_p() >> as_result()
			)
		);
	const auto double_colon =
		exact_p( "::" ) >> just( std::string{ "::" } ) >> to_container()
		;

	return produce< std::string >(
			alternatives(
				sequence(
					repeat( 6u, 6u, h16_with_colon ),
					ls32 >> to_container()
				),
				sequence(
					double_colon,
					repeat( 5u, 5u, h16_with_colon ),
					ls32 >> to_container()
				),
				sequence(
					maybe( h16 >> to_container() ),
					double_colon,
					repeat( 4u, 4u, h16_with_colon ),
					ls32 >> to_container()
				),
				sequence(
					maybe(
						repeat( 0u, 1u, h16_with_colon ),
						h16 >> to_container()
					),
					double_colon,
					repeat( 3u, 3u, h16_with_colon ),
					ls32 >> to_container()
				),
				sequence(
					maybe(
						repeat( 0u, 2u, h16_with_colon ),
						h16 >> to_container()
					),
					double_colon,
					repeat( 2u, 2u, h16_with_colon ),
					ls32 >> to_container()
				),
				sequence(
					maybe(
						repeat( 0u, 3u, h16_with_colon ),
						h16 >> to_container()
					),
					double_colon,
					h16_with_colon,
					ls32 >> to_container()
				),
				sequence(
					maybe(
						repeat( 0u, 4u, h16_with_colon ),
						h16 >> to_container()
					),
					double_colon,
					ls32 >> to_container()
				),
				sequence(
					maybe(
						repeat( 0u, 5u, h16_with_colon ),
						h16 >> to_container()
					),
					double_colon,
					h16 >> to_container()
				),
				sequence(
					maybe(
						repeat( 0u, 6u, h16_with_colon ),
						h16 >> to_container()
					),
					double_colon
				)
			)
		);
}

//
// reg_name_producer
//
/*!
 * @brief A factory for producer of reg-name value.
 *
 * Produces `std::string`.
 *
 * @note
 * reg-name is defined in RFC3986 as:
@verbatim
reg-name      = *( unreserved / pct-encoded / sub-delims )
@endverbatim
 * but this producer uses more strict grammar (because empty reg-name
 * in Host HTTP-field has no sense):
@verbatim
reg-name      = 1*( unreserved / pct-encoded / sub-delims )
@endverbatim
 *
 * @since v.0.6.9
 */
RESTINIO_NODISCARD
inline auto
reg_name_p()
{
	return produce< std::string >(
			repeat( 1, N,
				alternatives(
					unreserved_symbol_p() >> to_container(),
					hfp_details::pct_encoded_symbols_p()
							>> hfp_details::pct_encoded_symbols_consumer_t{},
					sub_delims_symbol_p() >> to_container()
				)
			)
		);
}

} /* namespace host_details */

//
// raw_host_value_t
//
/*!
 * @brief Tools for working with the raw value of Host HTTP-field.
 *
 * This struct represents parsed value of HTTP-field Host with out
 * advanced processing of parsed value (like decoding percent-encoded
 * symbols into UTF-8 byte sequences and transforming string representation
 * of IP addresses into internal form).
 *
 * See https://tools.ietf.org/html/rfc3986#appendix-A.
 *
 * @note
 * Value of 'host' is converted to lower case.
 *
 * @since v.0.6.9
 */
struct raw_host_value_t
{
	struct reg_name_t
	{
		std::string v;

		reg_name_t() = default;
		explicit reg_name_t( std::string val ) noexcept : v{ std::move(val) } {}

		friend bool
		operator==( const reg_name_t & a, const reg_name_t & b ) noexcept
		{
			return a.v == b.v;
		}

		friend bool
		operator!=( const reg_name_t & a, const reg_name_t & b ) noexcept
		{
			return a.v != b.v;
		}

		friend bool
		operator<( const reg_name_t & a, const reg_name_t & b ) noexcept
		{
			return a.v < b.v;
		}

		RESTINIO_NODISCARD
		static reg_name_t
		from_string( std::string v ) noexcept
		{
			return reg_name_t{ std::move(v) };
		}
	};

	struct ipv4_address_t
	{
		std::string v;

		ipv4_address_t() = default;
		explicit ipv4_address_t( std::string val ) noexcept : v{ std::move(val) } {}

		friend bool
		operator==( const ipv4_address_t & a, const ipv4_address_t & b ) noexcept
		{
			return a.v == b.v;
		}

		friend bool
		operator!=( const ipv4_address_t & a, const ipv4_address_t & b ) noexcept
		{
			return a.v != b.v;
		}

		friend bool
		operator<( const ipv4_address_t & a, const ipv4_address_t & b ) noexcept
		{
			return a.v < b.v;
		}

		RESTINIO_NODISCARD
		static ipv4_address_t
		from_string( std::string v ) noexcept
		{
			return ipv4_address_t{ std::move(v) };
		}
	};

	struct ipv6_address_t
	{
		std::string v;

		ipv6_address_t() = default;
		explicit ipv6_address_t( std::string val ) noexcept : v{ std::move(val) } {}

		friend bool
		operator==( const ipv6_address_t & a, const ipv6_address_t & b ) noexcept
		{
			return a.v == b.v;
		}

		friend bool
		operator!=( const ipv6_address_t & a, const ipv6_address_t & b ) noexcept
		{
			return a.v != b.v;
		}

		friend bool
		operator<( const ipv6_address_t & a, const ipv6_address_t & b ) noexcept
		{
			return a.v < b.v;
		}

		RESTINIO_NODISCARD
		static ipv6_address_t
		from_string( std::string v ) noexcept
		{
			return ipv6_address_t{ std::move(v) };
		}
	};

	using host_value_t = variant_t< reg_name_t, ipv4_address_t, ipv6_address_t >;

	host_value_t host;

	//! Optional port value.
	/*!
	 * Will be empty if there is no 'port' in the value of Host HTTP-field.
	 */
	optional_t<std::uint16_t> port;

	/*!
	 * @brief A factory function for a parser of Host value.
	 *
	 * @since v.0.6.9
	 */
	RESTINIO_NODISCARD
	static auto
	make_parser()
	{
		using namespace host_details;

		return produce< raw_host_value_t >(
			produce< host_value_t >(
				alternatives(

					produce< ipv6_address_t >(
						symbol('['),
						ipv6_address_p()
								>> to_lower()
								>> convert( ipv6_address_t::from_string )
								>> as_result(),
						symbol(']')
					) >> as_result(),

					produce< ipv4_address_t >(
						ipv4_address_p()
								>> convert( ipv4_address_t::from_string )
								>> as_result()
					) >> as_result(),

					produce< reg_name_t >(
						reg_name_p() >> to_lower()
								>> convert( reg_name_t::from_string )
								>> as_result()
					) >> as_result()
				)
			) >> &raw_host_value_t::host,
			maybe(
				symbol(':'),
				non_negative_decimal_number_p< std::uint16_t >()
					>> &raw_host_value_t::port
			)
		);
	}

	/*!
	 * @brief An attempt to parse Host HTTP-field.
	 *
	 * @since v.0.6.9
	 */
	RESTINIO_NODISCARD
	static expected_t< raw_host_value_t, restinio::easy_parser::parse_error_t >
	try_parse( string_view_t what )
	{
		return restinio::easy_parser::try_parse( what, make_parser() );
	}
};

inline std::ostream &
operator<<( std::ostream & to, const raw_host_value_t & rhv )
{
	struct host_dumper_t
	{
		std::ostream & m_to;

		void operator()( const raw_host_value_t::reg_name_t & n ) const
		{
			m_to << n.v;
		}

		void operator()( const raw_host_value_t::ipv4_address_t & n ) const
		{
			m_to << n.v;
		}

		void operator()( const raw_host_value_t::ipv6_address_t & n ) const
		{
			m_to << '[' << n.v << ']';
		}
	};

	visit( host_dumper_t{ to }, rhv.host );

	if( rhv.port )
		to << ':' << *(rhv.port) << std::endl;

	return to;
}

} /* namespace http_field_parsers */

} /* namespace restinio */

