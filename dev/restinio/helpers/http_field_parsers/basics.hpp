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

#include <restinio/helpers/easy_parser.hpp>

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
	auto
	try_parse(
		source_t & from ) const noexcept
	{
		std::pair< bool, optional_t<char> > result;

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

		result.first = true;
		if( extracted_spaces > 0u )
		{
			result.second = ' ';
		}

		return result;
	}
};

//
// token_t
//
class token_producer_t : public producer_tag< std::string >
{
	RESTINIO_NODISCARD
	static bool
	try_parse_value( source_t & from, std::string & accumulator )
	{
		do
		{
			const auto ch = from.getch();
			if( ch.m_eof )
				break;

			if( !is_token_char(ch.m_ch) )
			{
				from.putback();
				break;
			}

			accumulator += ch.m_ch;
		}
		while( true );

		if( accumulator.empty() )
			return false;

		return true;
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
	auto
	try_parse( source_t & from ) const
	{
		std::pair< bool, std::string > result;
		result.first = try_parse_value( from, result.second );
		return result;
	}
};

//
// quoted_string_producer_t
//
class quoted_string_producer_t : public producer_tag< std::string >
{
	RESTINIO_NODISCARD
	static bool
	try_parse_value( source_t & from, std::string & accumulator )
	{
		bool second_quote_extracted{ false };
		do
		{
			const auto ch = from.getch();
			if( ch.m_eof )
				break;

			if( '"' == ch.m_ch )
				second_quote_extracted = true;
			else if( '\\' == ch.m_ch )
			{
				const auto next = from.getch();
				if( next.m_eof )
					break;
				else if( SP == next.m_ch || HTAB == next.m_ch ||
						is_vchar( next.m_ch ) ||
						is_obs_text( next.m_ch ) )
				{
					accumulator += next.m_ch;
				}
				else
					break;
			}
			else if( is_qdtext( ch.m_ch ) )
				accumulator += ch.m_ch;
			else
				break;
		}
		while( !second_quote_extracted );

		return second_quote_extracted;
	}

public :
	RESTINIO_NODISCARD
	auto
	try_parse( source_t & from ) const
	{
		source_t::content_consumer_t consumer{ from };

		std::pair< bool, std::string > result;
		result.first = false;

		const auto ch = from.getch();
		if( !ch.m_eof )
		{
			if( '"' == ch.m_ch )
				result.first = try_parse_value( from, result.second );
		}

		if( result.first )
			consumer.acquire_content();

		return result;
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
	std::pair< bool, qvalue_t >
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

		if( parse_result.first )
			return std::make_pair( true,
					qvalue_t{ qvalue_t::trusted{ parse_result.second.m_value } } );
		else
			return std::make_pair( false, qvalue_t{} );
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
	non_empty_comma_separated_list_producer_t(
		Element_Producer && element )
		:	m_element{ std::move(element) }
	{}

	RESTINIO_NODISCARD
	std::pair< bool, Container >
	try_parse( source_t & from )
	{
		std::pair< bool, Container > result;
		result.first = false;

		const auto appender = to_container<Container_Adaptor>();

		result.first = sequence(
				repeat( 0, N, symbol(','), ows() ),
				m_element >> appender,  
				repeat( 0, N,
					ows(), symbol(','),
					maybe( ows(), m_element >> appender )
				)
			).try_process( from, result.second );

		return result;
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
	maybe_empty_comma_separated_list_producer_t(
		Element_Producer && element )
		:	m_element{ std::move(element) }
	{}

	RESTINIO_NODISCARD
	std::pair< bool, Container >
	try_parse( source_t & from )
	{
		std::pair< bool, Container > result;
		result.first = false;

		const auto appender = to_container<Container_Adaptor>();

		result.first = maybe(
				alternatives( symbol(','), m_element >> appender ),
				repeat( 0, N,
					ows(), symbol(','),
					maybe( ows(), m_element >> appender )
				)
			).try_process( from, result.second );

		return result;
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

