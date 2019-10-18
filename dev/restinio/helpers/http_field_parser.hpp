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

#include <restinio/impl/to_lower_lut.hpp>

#include <restinio/utils/tuple_algorithms.hpp>
#include <restinio/utils/metaprogramming.hpp>

#include <restinio/string_view.hpp>
#include <restinio/compiler_features.hpp>

#include <restinio/exception.hpp>
#include <restinio/optional.hpp>

#include <iostream>
#include <limits>
#include <map>
#include <array>
#include <cstring>

namespace restinio
{

namespace http_field_parser
{

namespace meta = restinio::utils::metaprogramming;

//FIXME: document this!
struct nothing_t {};

//FIXME: document this!
template< typename T >
struct default_container_adaptor;

template< typename T, typename... Args >
struct default_container_adaptor< std::vector< T, Args... > >
{
	using container_type = std::vector< T, Args... >;
	using value_type = typename container_type::value_type;

	static void
	store( container_type & to, value_type && what )
	{
		to.push_back( std::move(what) );
	}
};

template< typename Char, typename... Args >
struct default_container_adaptor< std::basic_string< Char, Args... > >
{
	using container_type = std::basic_string< Char, Args... >;
	using value_type = Char;

	static void
	store( container_type & to, value_type && what )
	{
		to.push_back( what );
	}
};

template< typename K, typename V, typename... Args >
struct default_container_adaptor< std::map< K, V, Args... > >
{
	using container_type = std::map< K, V, Args... >;
	// NOTE: we can't use container_type::value_type here
	// because value_type for std::map is std::pair<const K, V>,
	// not just std::pair<K, V>,
	using value_type = std::pair<K, V>;

	static void
	store( container_type & to, value_type && what )
	{
		to.emplace( std::move(what) );
	}
};

template<>
struct default_container_adaptor< nothing_t >
{
	using container_type = nothing_t;
	using value_type = nothing_t;

	static void
	store( container_type &, value_type && ) noexcept {}
};

constexpr std::size_t N = std::numeric_limits<std::size_t>::max();

namespace rfc
{

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

} /* namespace rfc */

namespace impl
{

//
// character_t
//
struct character_t
{
	bool m_eof;
	char m_ch;
};

RESTINIO_NODISCARD
inline bool
operator==( const character_t & a, const character_t & b ) noexcept
{
	return (a.m_eof == b.m_eof && a.m_ch == b.m_ch);
}

RESTINIO_NODISCARD
inline bool
operator!=( const character_t & a, const character_t & b ) noexcept
{
	return (a.m_eof != b.m_eof || a.m_ch != b.m_ch);
}

constexpr char SP = ' ';
constexpr char	HTAB = '\x09';

//
// is_space
//
RESTINIO_NODISCARD
inline constexpr bool
is_space( const char ch ) noexcept
{
	return ch == SP || ch == HTAB;
}

//
// is_vchar
//
RESTINIO_NODISCARD
inline constexpr bool
is_vchar( const char ch ) noexcept
{
	return (ch >= '\x41' && ch <= '\x5A') ||
			(ch >= '\x61' && ch <= '\x7A');
}

//
// is_obs_text
//
RESTINIO_NODISCARD
inline constexpr bool
is_obs_text( const char ch ) noexcept
{
	constexpr unsigned short left = 0x80u;
	constexpr unsigned short right = 0xFFu;

	const unsigned short t = static_cast<unsigned short>(
			static_cast<unsigned char>(ch));

	return (t >= left && t <= right);
}

//
// is_qdtext
//
RESTINIO_NODISCARD
inline constexpr bool
is_qdtext( const char ch ) noexcept
{
	return ch == SP ||
			ch == HTAB ||
			ch == '!' ||
			(ch >= '\x23' && ch <= '\x5B') ||
			(ch >= '\x5D' && ch <= '\x7E') ||
			is_obs_text( ch );
}

//
// is_digit
//
RESTINIO_NODISCARD
inline constexpr bool
is_digit( const char ch ) noexcept
{
	return (ch >= '0' && ch <= '9');
}

//
// source_t
//
class source_t
{
	const string_view_t m_data;
	string_view_t::size_type m_index{};

public:
	using position_t = string_view_t::size_type;

	explicit source_t( string_view_t data ) noexcept : m_data{ data } {}

	RESTINIO_NODISCARD
	character_t
	getch() noexcept
	{
		if( m_index < m_data.size() )
		{
			return {false, m_data[ m_index++ ]};
		}
		else
			return {true, 0};
	}

	void
	putback() noexcept
	{
		if( m_index )
			--m_index;
	}

	RESTINIO_NODISCARD
	position_t
	current_position() const noexcept
	{
		return m_index;
	}

	void
	backto( position_t pos ) noexcept
	{
		if( pos <= m_data.size() )
			m_index = pos;
	}

	RESTINIO_NODISCARD
	bool
	eof() const noexcept
	{
		return m_index >= m_data.size();
	}

	class content_consumer_t
	{
		source_t & m_from;
		const position_t m_started_at;
		bool m_consumed{ false };

	public :
		content_consumer_t() = delete;
		content_consumer_t( const content_consumer_t & ) = delete;
		content_consumer_t( content_consumer_t && ) = delete;

		content_consumer_t( source_t & from ) noexcept
			:	m_from{ from }
			,	m_started_at{ from.current_position() }
		{}

		~content_consumer_t() noexcept
		{
			if( !m_consumed )
				m_from.backto( m_started_at );
		}

		void
		consume() noexcept
		{
			m_consumed = true;
		}
	};

//FIXME: this is debug method!
string_view_t
current_content() const noexcept
{
	if( m_index < m_data.size() )
		return m_data.substr( m_index );
	else
		return {"--EOF--"};
}

};

enum class entity_type_t
{
	producer,
	transformer,
	consumer,
	clause
};

template< typename Result_Type >
struct producer_tag
{
	using result_type = Result_Type;
	static constexpr entity_type_t entity_type = entity_type_t::producer;
};

template< typename T, typename = meta::void_t<> >
struct is_producer : public std::false_type {};

template< typename T >
struct is_producer< T, meta::void_t< decltype(T::entity_type) > >
{
	static constexpr bool value = entity_type_t::producer == T::entity_type;
};

template< typename T >
constexpr bool is_producer_v = is_producer<T>::value;

template< typename Result_Type >
struct transformer_tag
{
	using result_type = Result_Type;
	static constexpr entity_type_t entity_type = entity_type_t::transformer;
};

template< typename T, typename = meta::void_t<> >
struct is_transformer : public std::false_type {};

template< typename T >
struct is_transformer< T, meta::void_t< decltype(T::entity_type) > >
{
	static constexpr bool value = entity_type_t::transformer == T::entity_type;
};

template< typename T >
constexpr bool is_transformer_v = is_transformer<T>::value;

template< typename T >
class value_transformer_t
{
	static_assert( is_transformer_v<T>, "T should be a transformer type" );

	T m_transformer;

public :
	using result_type = typename T::result_type;

	value_transformer_t(
		T && transformer )
		:	m_transformer{ std::move(transformer) }
	{}

	T giveaway() noexcept(noexcept(T{std::move(std::declval<T>())}))
	{
		return std::move(m_transformer);
	}
};

template< typename Producer, typename Transformer >
class transformed_value_producer_t
	:	public producer_tag< typename Transformer::result_type >
{
	static_assert( is_producer_v<Producer>,
			"Transformer should be a transformer type" );
	static_assert( is_transformer_v<Transformer>,
			"Transformer should be a transformer type" );

	Producer m_producer;
	Transformer m_transformer;

public :
	using result_type = typename Transformer::result_type;

	transformed_value_producer_t(
		Producer && producer,
		Transformer && transformer )
		:	m_producer{ std::move(producer) }
		,	m_transformer{ std::move(transformer) }
	{}

	RESTINIO_NODISCARD
	auto
	try_parse( source_t & source )
	{
		std::pair< bool, result_type > result;
		
		auto producer_result = m_producer.try_parse( source );
		result.first = producer_result.first;
		if( producer_result.first )
		{
			result.second = m_transformer.transform(
					std::move(producer_result.second) );
		}

		return result;
	}
};

template< typename P, typename T >
RESTINIO_NODISCARD
std::enable_if_t<
	is_producer_v<P>,
	transformed_value_producer_t< P, T > >
operator>>(
	P producer,
	value_transformer_t<T> transformer )
{
	using transformator_type = transformed_value_producer_t< P, T >;

	return transformator_type{ std::move(producer), transformer.giveaway() };
};

struct consumer_tag
{
	static constexpr entity_type_t entity_type = entity_type_t::consumer;
};

template< typename T, typename = meta::void_t<> >
struct is_consumer : public std::false_type {};

template< typename T >
struct is_consumer< T, meta::void_t< decltype(T::entity_type) > >
{
	static constexpr bool value = entity_type_t::consumer == T::entity_type;
};

template< typename T >
constexpr bool is_consumer_v = is_consumer<T>::value;

//
// value_consumer_t
//
template< typename C >
class value_consumer_t 
{
	static_assert( is_consumer_v<C>, "C should be a consumer type" );

	C m_consumer;

public :
	value_consumer_t( C && consumer ) : m_consumer{ std::move(consumer) } {}

	C giveaway() noexcept(noexcept(C{std::move(std::declval<C>())}))
	{
		return std::move(m_consumer);
	}
};

struct clause_tag
{
	static constexpr entity_type_t entity_type = entity_type_t::clause;
};

template< typename T, typename = meta::void_t<> >
struct is_clause : public std::false_type {};

template< typename T >
struct is_clause< T, meta::void_t< decltype(T::entity_type) > >
{
	static constexpr bool value = entity_type_t::clause == T::entity_type;
};

template< typename T >
constexpr bool is_clause_v = is_clause<T>::value;

template< typename P, typename C >
class consume_value_clause_t : public clause_tag
{
	static_assert( is_producer_v<P>, "P should be a producer type" );
	static_assert( is_consumer_v<C>, "C should be a consumer type" );

	P m_producer;
	C m_consumer;

public :
	consume_value_clause_t( P && producer, C && consumer )
		:	m_producer{ std::move(producer) }
		,	m_consumer{ std::move(consumer) }
	{}

	template< typename Target_Type >
	RESTINIO_NODISCARD
	bool
	try_process( source_t & from, Target_Type & target )
	{
		auto parse_result = m_producer.try_parse( from );
		if( parse_result.first )
		{
			m_consumer.consume( target, std::move(parse_result.second) );
			return true;
		}
		else
			return false;
	}
};

template< typename P, typename C >
RESTINIO_NODISCARD
std::enable_if_t<
	is_producer_v<P>,
	consume_value_clause_t< P, C > >
operator>>( P producer, value_consumer_t<C> consumer )
{
	return { std::move(producer), consumer.giveaway() };
}

template< typename Producer >
class top_level_clause_t
{
	Producer m_producer;

public :
	top_level_clause_t( Producer && producer )
		:	m_producer{ std::move(producer) }
	{}

	RESTINIO_NODISCARD
	auto
	try_process( source_t & from )
	{
		return m_producer.try_parse( from );
	}
};

RESTINIO_NODISCARD
bool
ensure_no_remaining_content(
	source_t & from )
{
	while( !from.eof() )
	{
		if( !is_space( from.getch().m_ch ) )
			return false;
	}

	return true;
}

//
// alternatives_clause_t
//
template<
	typename Subitems_Tuple >
class alternatives_clause_t : public clause_tag
{
	Subitems_Tuple m_subitems;

public :
	alternatives_clause_t(
		Subitems_Tuple && subitems )
		:	m_subitems{ std::move(subitems) }
	{}

	template< typename Target_Type >
	RESTINIO_NODISCARD
	bool
	try_process( source_t & from, Target_Type & target )
	{
		return restinio::utils::tuple_algorithms::any_of(
				m_subitems,
				[&from, &target]( auto && one_producer ) {
					source_t::content_consumer_t consumer{ from };
					Target_Type tmp_value{ target };

					bool success = one_producer.try_process( from, tmp_value );
					if( success )
					{
						target = std::move(tmp_value);
						consumer.consume();
					}

					return success;
				} );
	}
};

//
// maybe_clause_t
//
template<
	typename Subitems_Tuple >
class maybe_clause_t : public clause_tag
{
	Subitems_Tuple m_subitems;

public :
	maybe_clause_t(
		Subitems_Tuple && subitems )
		:	m_subitems{ std::move(subitems) }
	{}

	template< typename Target_Type >
	RESTINIO_NODISCARD
	bool
	try_process( source_t & from, Target_Type & target )
	{
		source_t::content_consumer_t consumer{ from };
		Target_Type tmp_value{ target };

		const bool success = restinio::utils::tuple_algorithms::all_of(
				m_subitems,
				[&from, &tmp_value]( auto && one_producer ) {
					return one_producer.try_process( from, tmp_value );
				} );

		if( success )
		{
			target = std::move(tmp_value);
			consumer.consume();
		}

		// maybe_producer always returns true even if nothing consumed.
		return true;
	}
};

//
// sequence_clause_t
//
template<
	typename Subitems_Tuple >
class sequence_clause_t : public clause_tag
{
	Subitems_Tuple m_subitems;

public :
	sequence_clause_t(
		Subitems_Tuple && subitems )
		:	m_subitems{ std::move(subitems) }
	{}

	template< typename Target_Type >
	RESTINIO_NODISCARD
	bool
	try_process( source_t & from, Target_Type & target )
	{
		source_t::content_consumer_t consumer{ from };
		Target_Type tmp_value{ target };

		const bool success = restinio::utils::tuple_algorithms::all_of(
				m_subitems,
				[&from, &tmp_value]( auto && one_producer ) {
					return one_producer.try_process( from, tmp_value );
				} );

		if( success )
		{
			target = std::move(tmp_value);
			consumer.consume();
		}

		return success;
	}
};

//
// produce_t
//
template<
	typename Target_Type,
	typename Subitems_Tuple >
class produce_t : public producer_tag< Target_Type >
{
	Subitems_Tuple m_subitems;

public :
	produce_t(
		Subitems_Tuple && subitems )
		:	m_subitems{ std::move(subitems) }
	{}

	RESTINIO_NODISCARD
	auto
	try_parse( source_t & from )
	{
		std::pair< bool, Target_Type > result;

		result.first = restinio::utils::tuple_algorithms::all_of(
				m_subitems,
				[&from, &result]( auto && one_clause ) {
					return one_clause.try_process( from, result.second );
				} );

		return result;
	}
};

//
// repeat_clause_t
//
template<
	template<class> class Container_Adaptor,
	typename Subitems_Tuple >
class repeat_clause_t : public clause_tag
{
	std::size_t m_min_occurences;
	std::size_t m_max_occurences;

	Subitems_Tuple m_subitems;

public :
	repeat_clause_t(
		std::size_t min_occurences,
		std::size_t max_occurences,
		Subitems_Tuple && subitems )
		:	m_min_occurences{ min_occurences }
		,	m_max_occurences{ max_occurences }
		,	m_subitems{ std::move(subitems) }
	{}

	template< typename Target_Type >
	RESTINIO_NODISCARD
	bool
	try_process( source_t & from, Target_Type & dest )
	{
		using adaptor_type = Container_Adaptor< Target_Type >;
		using value_type = typename adaptor_type::value_type;

		source_t::content_consumer_t whole_consumer{ from };

		std::size_t count{};
		bool failure_detected{ false };
		for(; !failure_detected && count != m_max_occurences; )
		{
			value_type item;

			source_t::content_consumer_t item_consumer{ from };

			failure_detected = !restinio::utils::tuple_algorithms::all_of(
					m_subitems,
					[&from, &item]( auto && one_clause ) {
						return one_clause.try_process( from, item );
					} );

			if( !failure_detected )
			{
				// Another item successfully parsed and should be stored.
				adaptor_type::store( dest, std::move(item) );
				item_consumer.consume();
				++count;
			}
		}

		const bool success = count >= m_min_occurences;
		if( success )
			whole_consumer.consume();

		return success;
	}
};

#if 0
//
// one_or_more_of_t
//
template<
	typename Container_Adaptor,
	typename Producer >
class one_or_more_of_t
{
	using container_type = typename Container_Adaptor::container_type;
	using value_type = typename Container_Adaptor::value_type;

	value_producer_t<Producer> m_producer;

public :
	one_or_more_of_t( Producer && producer )
		:	m_producer{ std::move(producer) }
	{}

	RESTINIO_NODISCARD
	std::pair< bool, container_type >
	try_parse( source_t & from );
};

//
// any_number_of_t
//
template<
	typename Container_Adaptor,
	typename Producer >
class any_number_of_t
{
	using container_type = typename Container_Adaptor::container_type;
	using value_type = typename Container_Adaptor::value_type;

	value_producer_t<Producer> m_producer;

public :
	any_number_of_t( Producer && producer )
		:	m_producer{ std::move(producer) }
	{}

	RESTINIO_NODISCARD
	std::pair< bool, container_type >
	try_parse( source_t & from );
};

#endif

namespace rfc
{

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
			consumer.consume();

		return result;
	}
};

} /* namespace rfc */

//
// symbol_producer_t
//
class symbol_producer_t : public producer_tag< char >
{
	char m_expected;

public:
	symbol_producer_t( char expected ) : m_expected{ expected } {}

	RESTINIO_NODISCARD
	std::pair< bool, char >
	try_parse( source_t & from ) const noexcept
	{
		const auto ch = from.getch();
		if( !ch.m_eof )
		{
			if( ch.m_ch == m_expected )
				return std::make_pair( true, ch.m_ch );
			else
				from.putback();
		}

		return std::make_pair( false, '\x00' );
	}
};

//
// digit_producer_t
//
class digit_producer_t : public producer_tag< char >
{
public:
	RESTINIO_NODISCARD
	std::pair< bool, char >
	try_parse( source_t & from ) const noexcept
	{
		const auto ch = from.getch();
		if( !ch.m_eof )
		{
			if( is_digit(ch.m_ch) )
				return std::make_pair( true, ch.m_ch );
			else
				from.putback();
		}

		return std::make_pair( false, '\x00' );
	}
};

//
// any_value_skipper_t
//
struct any_value_skipper_t : public consumer_tag
{
	template< typename Target_Type, typename Value >
	void
	consume( Target_Type &, Value && ) const noexcept {}
};

//
// as_result_consumer_t
//
struct as_result_consumer_t : public consumer_tag
{
	template< typename Target_Type, typename Value >
	void
	consume( Target_Type & dest, Value && src ) const
		noexcept(noexcept(dest=std::forward<Value>(src)))
	{
		dest = std::forward<Value>(src);
	}
};

//
// custom_consumer_t
//
template< typename C >
class custom_consumer_t : public consumer_tag
{
	C m_consumer;

public :
	custom_consumer_t( C && consumer ) : m_consumer{std::move(consumer)} {}

	template< typename Target_Type, typename Value >
	void
	consume( Target_Type & dest, Value && src ) const
	{
		m_consumer( dest, std::forward<Value>(src) );
	}
};

//
// field_setter_t
//
template< typename F, typename C >
class field_setter_t : public consumer_tag
{
	using pointer_t = F C::*;

	pointer_t m_ptr;

public :
	field_setter_t( pointer_t ptr ) noexcept : m_ptr{ptr} {}

	void
	consume( C & to, F && value ) const
		noexcept(noexcept(to.*m_ptr = std::move(value)))
	{
		to.*m_ptr = std::move(value);
	}
};

template< typename P, typename F, typename C >
RESTINIO_NODISCARD
std::enable_if_t<
	is_producer_v<P>,
	consume_value_clause_t< P, field_setter_t<F,C> > >
operator>>( P producer, F C::*member_ptr )
{
	return {
			std::move(producer),
			field_setter_t<F,C>{ member_ptr }
	};
}

//
// to_lower_t
//
struct to_lower_t : public transformer_tag< std::string >
{
	using input_type = std::string;

	RESTINIO_NODISCARD
	result_type
	transform( input_type && input ) const noexcept
	{
		result_type result{ std::move(input) };
		std::transform( result.begin(), result.end(), result.begin(),
			[]( unsigned char ch ) -> char {
				return static_cast<char>(
						restinio::impl::to_lower_lut<unsigned char>()[ch]
				);
			} );

		return result;
	}
};

#if 0
//
// just_transformer_t
//
template< typename V >
class just_transformer_t : public transformer_tag<V>
{
	V m_value;

public :
	using result_type = typename transformer_tag<V>::result_type;

	just_transformer_t( V && v ) : m_value{ std::move(v) } {}

	template< typename Input_Type >
	RESTINIO_NODISCARD
	result_type
	transform( Input_Type && ) const
	{
		return m_value;
	}
};
#endif

} /* namespace impl */

//
// produce
//
template< typename Target_Type, typename... Clauses >
RESTINIO_NODISCARD
auto
produce( Clauses &&... clauses )
{
	static_assert( meta::all_of_v< impl::is_clause, Clauses... >,
			"all arguments for produce() should be clauses" );

	using producer_type_t = impl::produce_t<
			Target_Type,
			std::tuple<Clauses...> >;

	return producer_type_t{
			std::make_tuple(std::forward<Clauses>(clauses)...)
	};
}

//
// alternatives
//
template< typename... Clauses >
RESTINIO_NODISCARD
auto
alternatives( Clauses &&... clauses )
{
	static_assert( meta::all_of_v< impl::is_clause, Clauses... >,
			"all arguments for alternatives() should be clauses" );

	using clause_type_t = impl::alternatives_clause_t< std::tuple<Clauses...> >;

	return clause_type_t{
			std::make_tuple(std::forward<Clauses>(clauses)...)
	};
}

//
// maybe
//
template< typename... Clauses >
RESTINIO_NODISCARD
auto
maybe( Clauses &&... clauses )
{
	static_assert( meta::all_of_v< impl::is_clause, Clauses... >,
			"all arguments for maybe() should be clauses" );

	using clause_type_t = impl::maybe_clause_t< std::tuple<Clauses...> >;

	return clause_type_t{
			std::make_tuple(std::forward<Clauses>(clauses)...)
	};
}

//
// sequence
//
template< typename... Clauses >
RESTINIO_NODISCARD
auto
sequence( Clauses &&... clauses )
{
	static_assert( meta::all_of_v< impl::is_clause, Clauses... >,
			"all arguments for sequence() should be clauses" );

	using clause_type_t = impl::sequence_clause_t< std::tuple<Clauses...> >;

	return clause_type_t{
			std::make_tuple(std::forward<Clauses>(clauses)...)
	};
}

//
// repeat
//
template<
	template<class C> class Container_Adaptor = default_container_adaptor,
	typename... Clauses >
RESTINIO_NODISCARD
auto
repeat(
	std::size_t min_occurences,
	std::size_t max_occurences,
	Clauses &&... clauses )
{
	static_assert( meta::all_of_v< impl::is_clause, Clauses... >,
			"all arguments for repeat() should be clauses" );

	using producer_type_t = impl::repeat_clause_t<
			Container_Adaptor,
			std::tuple<Clauses...> >;

	return producer_type_t{
			min_occurences,
			max_occurences,
			std::make_tuple(std::forward<Clauses>(clauses)...)
	};
}

#if 0
//
// one_or_more_of
//
template<
	typename Container,
	template<class C> class Container_Adaptor = default_container_adaptor,
	typename Producer >
RESTINIO_NODISCARD
auto
one_or_more_of(
	impl::value_producer_t<Producer> producer )
{
	using producer_type_t = impl::one_or_more_of_t<
			Container_Adaptor<Container>,
			Producer >;

	using result_type_t = impl::value_producer_t< producer_type_t >;

	return result_type_t{
			producer_type_t{ producer.giveaway() }
	};
}

//
// any_number_of
//
template<
	typename Container,
	template<class C> class Container_Adaptor = default_container_adaptor,
	typename Producer >
RESTINIO_NODISCARD
auto
any_number_of(
	impl::value_producer_t<Producer> producer )
{
	using producer_type_t = impl::any_number_of_t<
			Container_Adaptor<Container>,
			Producer >;

	using result_type_t = impl::value_producer_t< producer_type_t >;

	return result_type_t{
			producer_type_t{ producer.giveaway() }
	};
}
#endif

//
// skip
//
RESTINIO_NODISCARD
impl::value_consumer_t< impl::any_value_skipper_t >
skip() noexcept { return { impl::any_value_skipper_t{} }; }

//
// symbol_producer
//
RESTINIO_NODISCARD
auto
symbol_producer( char expected ) noexcept
{
	return impl::symbol_producer_t{expected};
}

//
// symbol
//
RESTINIO_NODISCARD
auto
symbol( char expected ) noexcept
{
	return symbol_producer(expected) >> skip();
}

//
// digit_producer
//
RESTINIO_NODISCARD
auto
digit_producer() noexcept
{
	return impl::digit_producer_t{};
}

//
// digit
//
RESTINIO_NODISCARD
auto
digit() noexcept
{
	return digit_producer() >> skip();
}

//
// as_result
//
RESTINIO_NODISCARD
impl::value_consumer_t< impl::as_result_consumer_t >
as_result() noexcept { return { impl::as_result_consumer_t{} }; }

//
// custom_consumer
//
template< typename F >
RESTINIO_NODISCARD
auto
custom_consumer( F consumer )
{
	using actual_consumer_t = impl::custom_consumer_t< F >;

	return impl::value_consumer_t< actual_consumer_t >{
			actual_consumer_t{ std::move(consumer) }
	};
}

//
// to_lower
//
RESTINIO_NODISCARD
impl::value_transformer_t< impl::to_lower_t >
to_lower() noexcept { return { impl::to_lower_t{} }; }

#if 0
//
// just
//
template< typename V >
RESTINIO_NODISCARD
auto
just( V && v )
{
	using value_type = std::decay_t<V>;
	using transformer_type = impl::just_transformer_t<value_type>;

	return impl::value_transformer_t< transformer_type >{
			transformer_type{ std::forward<V>(v) }
	};
}
#endif

namespace rfc
{

//
// ows_producer
//
RESTINIO_NODISCARD
auto
ows_producer() noexcept { return impl::rfc::ows_t{}; }

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
token_producer() noexcept { return impl::rfc::token_producer_t{}; }

//
// quoted_string_producer
//
RESTINIO_NODISCARD
auto
quoted_string_producer() noexcept
{
	return impl::rfc::quoted_string_producer_t{};
}

} /* namespace rfc */

namespace impl
{

namespace rfc
{

//
// qvalue_producer_t
//
class qvalue_producer_t
	:	public producer_tag< restinio::http_field_parser::rfc::qvalue_t >
{
	using qvalue_t = restinio::http_field_parser::rfc::qvalue_t;

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

	RESTINIO_NODISCARD
	static auto
	digit_consumer( qvalue_t::underlying_uint_t m )
	{
		return value_consumer_t< digit_consumer_t >{ digit_consumer_t{m} };
	}

public :
	RESTINIO_NODISCARD
	std::pair< bool, restinio::http_field_parser::rfc::qvalue_t >
	try_parse( source_t & from ) const noexcept
	{
		const auto parse_result = produce< zero_initialized_unit_t >(
				alternatives( symbol('q'), symbol('Q') ),
				symbol('='),
				alternatives(
					sequence(
						symbol('0'),
						maybe(
							symbol('.'),
							maybe( digit_producer() >> digit_consumer(100),
								maybe( digit_producer() >> digit_consumer(10),
									maybe( digit_producer() >> digit_consumer(1) )
								)
							)
						)
					),
					sequence(
						symbol_producer('1') >> digit_consumer(1000),
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

} /* namespace rfc */

} /* namespace impl */

namespace rfc
{

//
// qvalue
//
RESTINIO_NODISCARD
auto
qvalue_producer() noexcept
{
	using restinio::http_field_parser::impl::rfc::qvalue_producer_t;

	return qvalue_producer_t{};
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
			qvalue_producer() >> as_result()
		);
}

} /* namespace rfc */

#if 0
namespace impl
{

//
// one_or_more_of_t implementation
//
template< typename Container_Adaptor, typename Producer >
RESTINIO_NODISCARD
std::pair<
	bool,
	typename one_or_more_of_t<Container_Adaptor, Producer>::container_type >
one_or_more_of_t<Container_Adaptor, Producer>::try_parse( source_t & from )
{
	namespace hfp = restinio::http_field_parser;

	std::pair< bool, container_type > result;
	result.first = false;

	auto opt_intro_clause = repeat< nothing_t >( 0, N,
		symbol(',') >> skip(),
		hfp::rfc::ows() >> skip() ) >> skip();
	if( opt_intro_clause.try_process( from, result.second ) )
	{
		auto the_first_item_clause = m_producer >> custom_consumer(
				[]( auto & dest, value_type && item ) {
					Container_Adaptor::store( dest, std::move(item) );
				} );
		if( the_first_item_clause.try_process( from, result.second ) )
		{
			auto remaining_item_clause =
				produce< restinio::optional_t<value_type> >(
					hfp::rfc::ows() >> skip(),
					symbol(',') >> skip(),
					optional< value_type >(
						hfp::rfc::ows() >> skip(),
						m_producer >> as_result()
					) >> as_result()
				) >> custom_consumer(
					[]( auto & dest, restinio::optional_t<value_type> && item )
					{
						if( item )
							Container_Adaptor::store( dest, std::move(*item) );
					} );

			bool process_result{ true };
			do
			{
				const auto pos = from.current_position();
				process_result = remaining_item_clause.try_process(
						from, result.second );
				if( !process_result )
					from.backto( pos );
			}
			while( process_result );

			result.first = true;
		}
	}

	return result;
}

//FIXME: it's just a workaround.
template< typename T >
struct value_to_optional : public transformer_tag< restinio::optional_t<T> >
{
	RESTINIO_NODISCARD
	auto
	transform( T && value ) const
	{
		return restinio::optional_t<T>{ std::move(value) };
	}
};

//
// any_number_of_t implementation
//
template< typename Container_Adaptor, typename Producer >
RESTINIO_NODISCARD
std::pair<
	bool,
	typename any_number_of_t<Container_Adaptor, Producer>::container_type >
any_number_of_t<Container_Adaptor, Producer>::try_parse( source_t & from )
{
	namespace hfp = restinio::http_field_parser;

	std::pair< bool, container_type > result;
	// It seems that this method will always return 'success' even
	// if nothing has been parsed. It is because all values are optional.
	result.first = true;

	using optional_value_t = restinio::optional_t< value_type >;
	const auto append_opt_val = []( auto & dest, optional_value_t && item ) {
		if( item )
			Container_Adaptor::store( dest, std::move(*item) );
	};

	using val_to_optional_transformer_t = value_to_optional< value_type >;

	auto opt_intro_clause =
		alternatives< optional_value_t >(
			symbol(',') >> just( optional_value_t{} ),
			m_producer >> impl::value_transformer_t< val_to_optional_transformer_t >{
					val_to_optional_transformer_t{} }
		) >> custom_consumer( append_opt_val );

	if( opt_intro_clause.try_process( from, result.second ) )
	{
		auto remaining_item_clause =
			produce< optional_value_t >(
				hfp::rfc::ows() >> skip(),
				symbol(',') >> skip(),
				optional< value_type >(
					hfp::rfc::ows() >> skip(),
					m_producer >> as_result()
				) >> as_result()
			) >> custom_consumer( append_opt_val );

		bool process_result{ true };
		do
		{
			const auto pos = from.current_position();
			process_result = remaining_item_clause.try_process(
					from, result.second );
			if( !process_result )
				from.backto( pos );
		}
		while( process_result );
	}

	return result;
}

} /* namespace impl */

#endif

//
// try_parse_field_value
//
template< typename Producer >
RESTINIO_NODISCARD
auto
try_parse_field_value(
	string_view_t from,
	Producer producer )
{
	static_assert( impl::is_producer_v<Producer>,
			"Producer should be a value producer type" );

	impl::source_t source{ from };

	auto result = impl::top_level_clause_t< Producer >{ std::move(producer) }
			.try_process( source );

	if( !result.first ||
			!impl::ensure_no_remaining_content( source ) )
	{
		result.first = false;
	}

	return result;
}

} /* namespace http_field_parser */

} /* namespace restinio */

