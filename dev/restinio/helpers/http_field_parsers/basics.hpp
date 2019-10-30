/*
 * RESTinio
 */

/*!
 * @file
 * @brief Utilities for parsing values of http-fields
 *
 * @since v.0.6.1
 */

#pragma once

#include <restinio/impl/string_caseless_compare.hpp>

#include <restinio/helpers/easy_parser.hpp>

#include <restinio/expected.hpp>

#include <algorithm>

namespace restinio
{

namespace http_field_parsers
{

using namespace restinio::easy_parser;

namespace meta = restinio::utils::metaprogramming;

namespace qvalue_details
{

using underlying_uint_t = std::uint_least16_t;

constexpr underlying_uint_t maximum = 1000u;
constexpr underlying_uint_t zero = 0u;

class trusted
{
	const underlying_uint_t m_value;

public :
	explicit constexpr
	trusted( underlying_uint_t value ) noexcept : m_value{ value } {}

	constexpr auto get() const noexcept { return m_value; }
};

class untrusted
{
	underlying_uint_t m_value;

public :
	explicit
	untrusted( underlying_uint_t value ) : m_value{ value }
	{
		if( m_value > maximum )
			throw exception_t( "invalid value for "
					"http_field_parser::rfc::qvalue_t" );
	}

	auto get() const noexcept { return m_value; }
};

} /* namespace qvalue_details */

//
// qvalue_t
//
class qvalue_t
{
public :
	using underlying_uint_t = qvalue_details::underlying_uint_t;
	using trusted = qvalue_details::trusted;
	using untrusted = qvalue_details::untrusted;

	static constexpr trusted maximum{ qvalue_details::maximum };
	static constexpr trusted zero{ qvalue_details::zero };

private :
	// Note: with the terminal 0-symbol.
	using underlying_char_array_t = std::array<char, 6>;

	underlying_uint_t m_value{};

	underlying_char_array_t
	make_char_array() const noexcept
	{
		underlying_char_array_t result;
		if( maximum.get() == m_value )
		{
			std::strcpy( &result[0], "1.000" );
		}
		else
		{
			result[0] = '0';
			result[1] = '.';

			result[2] = '0' + static_cast<char>(m_value / 100u);
			const auto d2 = m_value % 100u;
			result[3] = '0' + static_cast<char>(d2 / 10u);
			const auto d3 = d2 % 10u;
			result[4] = '0' + static_cast<char>(d3);
			result[5] = 0;
		}

		return result;
	}

public :

	qvalue_t() = default;

	qvalue_t( untrusted val ) noexcept
		:	m_value{ val.get() }
	{}

	qvalue_t( trusted val ) noexcept
		:	m_value{ val.get() }
	{}

	auto as_uint() const noexcept { return m_value; }

	auto as_string() const
	{
		return std::string{ &make_char_array().front() };
	}

	friend std::ostream &
	operator<<( std::ostream & to, const qvalue_t & what )
	{
		return (to << &what.make_char_array().front());
	}
};

RESTINIO_NODISCARD
bool
operator==( const qvalue_t & a, const qvalue_t & b ) noexcept
{
	return a.as_uint() == b.as_uint();
}

RESTINIO_NODISCARD
bool
operator!=( const qvalue_t & a, const qvalue_t & b ) noexcept
{
	return a.as_uint() != b.as_uint();
}

RESTINIO_NODISCARD
bool
operator<( const qvalue_t & a, const qvalue_t & b ) noexcept
{
	return a.as_uint() < b.as_uint();
}

RESTINIO_NODISCARD
bool
operator<=( const qvalue_t & a, const qvalue_t & b ) noexcept
{
	return a.as_uint() <= b.as_uint();
}

namespace impl
{

using namespace restinio::easy_parser::impl;

//
// ows_producer_t
//
class ows_t : public producer_tag< restinio::optional_t<char> >
{
public :
	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse(
		source_t & from ) const noexcept
	{
		std::size_t extracted_spaces{};
		character_t ch;
		for( ch = from.getch();
			!ch.m_eof && is_space(ch.m_ch);
			ch = from.getch() )
		{
			++extracted_spaces;
		}

		if( !ch.m_eof )
			// The first non-space char should be returned back.
			from.putback();

		if( extracted_spaces > 0u )
			return result_type{ ' ' };

		return result_type{ nullopt };
	}
};

//
// token_t
//
class token_producer_t : public producer_tag< std::string >
{
	RESTINIO_NODISCARD
	static optional_t< parse_error_t >
	try_parse_value( source_t & from, std::string & accumulator )
	{
		error_reason_t reason = error_reason_t::pattern_not_found;

		do
		{
			const auto ch = from.getch();
			if( ch.m_eof )
			{
				reason = error_reason_t::unexpected_eof;
				break;
			}

			if( !is_token_char(ch.m_ch) )
			{
				from.putback();
				reason = error_reason_t::unexpected_character;
				break;
			}

			accumulator += ch.m_ch;
		}
		while( true );

		if( accumulator.empty() )
		{
			return parse_error_t{ from.current_position(), reason };
		}

		return nullopt;
	}

	RESTINIO_NODISCARD
	static constexpr bool
	is_token_char( const char ch ) noexcept
	{
		return is_vchar(ch) || is_digit(ch) ||
				ch == '!' ||
				ch == '#' ||
				ch == '$' ||
				ch == '%' ||
				ch == '&' ||
				ch == '\'' ||
				ch == '*' ||
				ch == '+' ||
				ch == '-' ||
				ch == '.' ||
				ch == '^' ||
				ch == '_' ||
				ch == '`' ||
				ch == '|' ||
				ch == '~';
	}

public :
	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse( source_t & from ) const
	{
		std::string value;
		const auto try_result = try_parse_value( from, value );
		if( !try_result )
			return value;
		else
			return make_unexpected( *try_result );
	}
};

//
// quoted_string_producer_t
//
class quoted_string_producer_t : public producer_tag< std::string >
{
	RESTINIO_NODISCARD
	static optional_t< parse_error_t >
	try_parse_value( source_t & from, std::string & accumulator )
	{
		error_reason_t reason = error_reason_t::pattern_not_found;

		bool second_quote_extracted{ false };
		do
		{
			const auto ch = from.getch();
			if( ch.m_eof )
			{
				reason = error_reason_t::unexpected_eof;
				break;
			}

			if( '"' == ch.m_ch )
				second_quote_extracted = true;
			else if( '\\' == ch.m_ch )
			{
				const auto next = from.getch();
				if( next.m_eof )
				{
					reason = error_reason_t::unexpected_eof;
					break;
				}
				else if( SP == next.m_ch || HTAB == next.m_ch ||
						is_vchar( next.m_ch ) ||
						is_obs_text( next.m_ch ) )
				{
					accumulator += next.m_ch;
				}
				else
				{
					reason = error_reason_t::unexpected_character;
					from.putback();
					break;
				}
			}
			else if( is_qdtext( ch.m_ch ) )
				accumulator += ch.m_ch;
			else
			{
				reason = error_reason_t::unexpected_character;
				from.putback();
				break;
			}
		}
		while( !second_quote_extracted );

		if( !second_quote_extracted )
			return parse_error_t{ from.current_position(), reason };
		else
			return nullopt;
	}

public :
	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse( source_t & from ) const
	{
		source_t::content_consumer_t consumer{ from };

		const auto ch = from.getch();
		if( !ch.m_eof )
		{
			if( '"' == ch.m_ch )
			{
				std::string value;
				const auto try_result = try_parse_value( from, value );
				if( !try_result )
				{
					consumer.acquire_content();
					return std::move(value);
				}
				else
					return make_unexpected( *try_result );
			}
			else
			{
				return make_unexpected( parse_error_t{
						consumer.started_at(),
						error_reason_t::unexpected_character
				} );
			}
		}
		else
			return make_unexpected( parse_error_t{
					consumer.started_at(),
					error_reason_t::unexpected_eof
			} );
	}
};

} /* namespace impl */

//
// ows_producer
//
RESTINIO_NODISCARD
auto
ows_producer() noexcept { return impl::ows_t{}; }

//
// ows
//
RESTINIO_NODISCARD
auto
ows() noexcept { return ows_producer() >> skip(); }

//
// token_producer
//
RESTINIO_NODISCARD
auto
token_producer() noexcept { return impl::token_producer_t{}; }

//
// quoted_string_producer
//
RESTINIO_NODISCARD
auto
quoted_string_producer() noexcept
{
	return impl::quoted_string_producer_t{};
}

namespace impl
{

//
// qvalue_producer_t
//
class qvalue_producer_t
	:	public producer_tag< qvalue_t >
{
	// This type has to be used as type parameter for produce().
	struct zero_initialized_unit_t
	{
		qvalue_t::underlying_uint_t m_value{0u};
	};

	class digit_consumer_t : public consumer_tag
	{
		const qvalue_t::underlying_uint_t m_multiplier;
	
	public :
		digit_consumer_t( qvalue_t::underlying_uint_t m )
			:	m_multiplier{ m }
		{}
	
		void
		consume( zero_initialized_unit_t & dest, char && digit )
		{
			dest.m_value += m_multiplier *
					static_cast< qvalue_t::underlying_uint_t >(digit - '0');
		}
	};

public :
	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse( source_t & from ) const noexcept
	{
		const auto parse_result = produce< zero_initialized_unit_t >(
				alternatives(
					sequence(
						symbol('0'),
						maybe(
							symbol('.'),
							maybe( digit_producer() >> digit_consumer_t{100},
								maybe( digit_producer() >> digit_consumer_t{10},
									maybe( digit_producer() >> digit_consumer_t{1} )
								)
							)
						)
					),
					sequence(
						symbol_producer('1') >> digit_consumer_t{1000},
						maybe(
							symbol('.'),
							maybe( symbol('0'),
								maybe( symbol('0'),
									maybe( symbol('0') )
								)
							)
						)
					)
				)
			).try_parse( from );

		if( parse_result )
			return qvalue_t{ qvalue_t::trusted{ parse_result->m_value } };
		else
			return make_unexpected( parse_result.error() );
	}
};

} /* namespace impl */

//
// qvalue
//
RESTINIO_NODISCARD
auto
qvalue_producer() noexcept
{
	return impl::qvalue_producer_t{};
}

//
// weight_producer
//
RESTINIO_NODISCARD
auto
weight_producer() noexcept
{
	return produce< qvalue_t >(
			ows(),
			symbol(';'),
			ows(),
			alternatives( symbol('q'), symbol('Q') ),
			symbol('='),
			qvalue_producer() >> as_result()
		);
}

namespace impl
{

//
// non_empty_comma_separated_list_producer_t
//
template<
	typename Container,
	template<class> class Container_Adaptor,
	typename Element_Producer >
class non_empty_comma_separated_list_producer_t
	: public producer_tag< Container >
{
	static_assert( impl::is_producer_v<Element_Producer>,
			"Element_Producer should be a value producer type" );

	Element_Producer m_element;

public :
	using typename producer_tag< Container >::result_type;

	non_empty_comma_separated_list_producer_t(
		Element_Producer && element )
		:	m_element{ std::move(element) }
	{}

	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse( source_t & from )
	{
		Container tmp_value;

		const auto appender = to_container<Container_Adaptor>();

		const auto process_result = sequence(
				repeat( 0, N, symbol(','), ows() ),
				m_element >> appender,  
				repeat( 0, N,
					ows(), symbol(','),
					maybe( ows(), m_element >> appender )
				)
			).try_process( from, tmp_value );

		if( !process_result )
			return std::move(tmp_value);
		else
			return make_unexpected( *process_result );
	}
};

//
// maybe_empty_comma_separated_list_producer_t
//
template<
	typename Container,
	template<class> class Container_Adaptor,
	typename Element_Producer >
class maybe_empty_comma_separated_list_producer_t
	:	public producer_tag< Container >
{
	static_assert( impl::is_producer_v<Element_Producer>,
			"Element_Producer should be a value producer type" );

	Element_Producer m_element;

public :
	using typename producer_tag< Container >::result_type;

	maybe_empty_comma_separated_list_producer_t(
		Element_Producer && element )
		:	m_element{ std::move(element) }
	{}

	RESTINIO_NODISCARD
	expected_t< result_type, parse_error_t >
	try_parse( source_t & from )
	{
		Container tmp_value;

		const auto appender = to_container<Container_Adaptor>();

		const auto process_result = maybe(
				alternatives( symbol(','), m_element >> appender ),
				repeat( 0, N,
					ows(), symbol(','),
					maybe( ows(), m_element >> appender )
				)
			).try_process( from, tmp_value );

		if( !process_result )
			return std::move(tmp_value);
		else
			return make_unexpected( *process_result );
	}
};

} /* namespace impl */

//
// non_empty_comma_separated_list_producer
//
template<
	typename Container,
	template<class> class Container_Adaptor = default_container_adaptor,
	typename Element_Producer >
RESTINIO_NODISCARD
auto
non_empty_comma_separated_list_producer( Element_Producer element )
{
	static_assert( impl::is_producer_v<Element_Producer>,
			"Element_Producer should be a value producer type" );

	return impl::non_empty_comma_separated_list_producer_t<
			Container,
			Container_Adaptor,
			Element_Producer >{ std::move(element) };
}

//
// maybe_empty_comma_separated_list_producer
//
template<
	typename Container,
	template<class> class Container_Adaptor = default_container_adaptor,
	typename Element_Producer >
RESTINIO_NODISCARD
auto
maybe_empty_comma_separated_list_producer( Element_Producer element )
{
	static_assert( impl::is_producer_v<Element_Producer>,
			"Element_Producer should be a value producer type" );

	return impl::maybe_empty_comma_separated_list_producer_t<
			Container,
			Container_Adaptor,
			Element_Producer >{ std::move(element) };
}

//
// parameter_with_mandatory_value_t
//
using parameter_with_mandatory_value_t = std::pair< std::string, std::string >;

//
// parameter_with_mandatory_value_container_t
//
using parameter_with_mandatory_value_container_t =
		std::vector< parameter_with_mandatory_value_t >;

//
// not_found_t
//
//FIXME: document this!
struct not_found_t {};

//
// find_first
//
//FIXME: document this!
RESTINIO_NODISCARD
inline expected_t< string_view_t, not_found_t >
find_first(
	const parameter_with_mandatory_value_container_t & where,
	string_view_t what )
{
	const auto it = std::find_if( where.begin(), where.end(),
			[&what]( const auto & pair ) {
				return restinio::impl::is_equal_caseless( pair.first, what );
			} );
	if( it != where.end() )
		return string_view_t{ it->second };
	else
		return make_unexpected( not_found_t{} );
}

namespace impl
{

namespace params_with_value_producer_details
{

RESTINIO_NODISCARD
auto
make_parser()
{
	return produce< parameter_with_mandatory_value_container_t >(
			repeat( 0, N,
				produce< parameter_with_mandatory_value_t >(
					ows(),
					symbol(';'),
					ows(),
					token_producer() >> to_lower()
							>> &parameter_with_mandatory_value_t::first,
					symbol('='),
					alternatives(
						token_producer()
								>> &parameter_with_mandatory_value_t::second,
						quoted_string_producer()
								>> &parameter_with_mandatory_value_t::second
					)
				) >> to_container()
			)
		);
}

} /* namespace params_with_value_producer_details */

//
// params_with_value_producer_t
//
class params_with_value_producer_t
	:	public producer_tag< parameter_with_mandatory_value_container_t >
{
	using actual_producer_t = std::decay_t<
			decltype(params_with_value_producer_details::make_parser()) >;

	actual_producer_t m_producer{
			params_with_value_producer_details::make_parser() };

public :
	params_with_value_producer_t() = default;

	RESTINIO_NODISCARD
	auto
	try_parse( source_t & from )
	{
		return m_producer.try_parse( from );
	}
};

} /* namespace impl */

//
// params_with_value_producer
//
RESTINIO_NODISCARD
impl::params_with_value_producer_t
params_with_value_producer() { return {}; }

//
// parameter_with_optional_value_t
//
using parameter_with_optional_value_t =
		std::pair< std::string, restinio::optional_t<std::string> >;

//
// parameter_with_optional_value_container_t
//
using parameter_with_optional_value_container_t =
		std::vector< parameter_with_optional_value_t >;

namespace impl
{

namespace params_with_opt_value_producer_details
{

RESTINIO_NODISCARD
auto
make_parser()
{
	return produce< parameter_with_optional_value_container_t >(
			repeat( 0, N,
				produce< parameter_with_optional_value_t >(
					ows(),
					symbol(';'),
					ows(),
					token_producer() >> to_lower()
							>> &parameter_with_optional_value_t::first,
					maybe(
						symbol('='),
						alternatives(
							token_producer()
									>> &parameter_with_optional_value_t::second,
							quoted_string_producer()
									>> &parameter_with_optional_value_t::second
						)
					)
				) >> to_container()
			)
		);
}

} /* namespace params_with_opt_value_producer_details */

//
// params_with_opt_value_producer_t
//
class params_with_opt_value_producer_t
	:	public producer_tag< parameter_with_optional_value_container_t >
{
	using actual_producer_t = std::decay_t<
			decltype(params_with_opt_value_producer_details::make_parser()) >;

	actual_producer_t m_producer{
			params_with_opt_value_producer_details::make_parser() };

public :
	params_with_opt_value_producer_t() = default;

	RESTINIO_NODISCARD
	auto
	try_parse( source_t & from )
	{
		return m_producer.try_parse( from );
	}
};

} /* namespace impl */

//
// params_with_opt_value_producer
//
RESTINIO_NODISCARD
impl::params_with_opt_value_producer_t
params_with_opt_value_producer() { return {}; }

} /* namespace http_field_parser */

} /* namespace restinio */

