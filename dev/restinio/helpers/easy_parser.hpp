/*
 * RESTinio
 */

/*!
 * @file
 * @brief An very small, simple and somewhat limited implementation of
 * recursive-descent parser.
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
#include <restinio/expected.hpp>

#include <iostream>
#include <limits>
#include <map>
#include <array>
#include <vector>
#include <cstring>

namespace restinio
{

namespace easy_parser
{

namespace meta = restinio::utils::metaprogramming;

//
// error_reason_t
//
/*!
 * @brief Reason of parsing error.
 *
 * @since v.0.6.1
 */
enum class error_reason_t
{
	//! Unexpected character is found in the input.
	unexpected_character,
	//! Unexpected end of input is encontered when some character expected. 
	unexpected_eof,
	//! None of alternatives was found in the input.
	no_appropriate_alternative,
	//! Required pattern is not found in the input.
	pattern_not_found
};

//
// parse_error_t
//
/*!
 * @brief Information about parsing error.
 *
 * @since v.0.6.1
 */
class parse_error_t
{
	//! Position in the input stream.
	std::size_t m_position;
	//! The reason of the error.
	error_reason_t m_reason;

public:
	//! Initializing constructor.
	parse_error_t(
		std::size_t position,
		error_reason_t reason ) noexcept
		:	m_position{ position }
		,	m_reason{ reason }
	{}

	//! Get the position in the input stream where error was detected.
	RESTINIO_NODISCARD
	std::size_t
	position() const noexcept { return m_position; }

	//! Get the reason of the error.
	RESTINIO_NODISCARD
	error_reason_t
	reason() const noexcept { return m_reason; }
};

//
// nothing_t
//
/*!
 * @brief A special type to be used in the case where there is no
 * need to store produced value.
 *
 * @since v.0.6.1.
 */
struct nothing_t {};

//
// default_container_adaptor
//
/*!
 * @brief A template with specializations for different kind
 * of containers and for type `nothing`.
 *
 * Every specialization will have the following content:
 * @code
 * struct default_container_adaptor<...>
 * {
 * 	using container_type = ... // some type of the container.
 * 	using value_type = ... // type of object to be placed into a container.
 *
 * 	static void store( container_type & to, value_type && what )
 * 	{ ... }
 * };
 * @endcode
 *
 * @since v.0.6.1.
 */
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

/*!
 * @brief A special marker that means infinite repetitions.
 *
 * @since v.0.6.1.
 */
constexpr std::size_t N = std::numeric_limits<std::size_t>::max();

namespace impl
{

//
// character_t
//
/*!
 * @brief One character extracted from the input stream.
 *
 * If the characted extracted successfuly then m_eof will be `false`.
 * If the end of input reached then m_eof is `true` and the value
 * of m_ch is undefined.
 *
 * @since v.0.6.1
 */
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

/*!
 * @brief A constant for SPACE value.
 *
 * @since v.0.6.1
 */
constexpr char SP = ' ';
/*!
 * @brief A constant for Horizontal Tab value.
 *
 * @since v.0.6.1
 */
constexpr char	HTAB = '\x09';

//
// is_space
//
/*!
 * @brief If a character a space character?
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline constexpr bool
is_space( const char ch ) noexcept
{
	return ch == SP || ch == HTAB;
}

//
// is_digit
//
/*!
 * @brief Is a character a digit?
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline constexpr bool
is_digit( const char ch ) noexcept
{
	return (ch >= '0' && ch <= '9');
}

//
// source_t
//
/*!
 * @brief The class that implements "input stream".
 *
 * It is expected that string_view passed to the constructor of
 * source_t will outlive the instance of source_t.
 *
 * @since v.0.6.1
 */
class source_t
{
	//! The content to be used as "input stream".
	const string_view_t m_data;
	//! The current position in the input stream.
	/*!
	 * \note
	 * m_index can have value of m_data.size(). In that case
	 * EOF will be returned.
	 */
	string_view_t::size_type m_index{};

public:
	//! Type to be used as the index inside the input stream.
	using position_t = string_view_t::size_type;

	//! Initializing constructor.
	explicit source_t( string_view_t data ) noexcept : m_data{ data } {}

	//! Get the next character from the input stream.
	/*!
	 * EOF can be returned in the case if there is no more data in
	 * the input stream.
	 */
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

	//! Return one character back to the input stream.
	void
	putback() noexcept
	{
		if( m_index )
			--m_index;
	}

	//! Get the current position in the stream.
	RESTINIO_NODISCARD
	position_t
	current_position() const noexcept
	{
		return m_index;
	}

	//! Return the current position in the input stream
	//! at the specified position.
	void
	backto( position_t pos ) noexcept
	{
		if( pos <= m_data.size() )
			m_index = pos;
	}

	//! Is EOF has been reached?
	RESTINIO_NODISCARD
	bool
	eof() const noexcept
	{
		return m_index >= m_data.size();
	}

	//! Return a fragment from the input stream.
	/*!
	 * \attention
	 * The value of \a from should be lesser than the size of the
	 * input stream.
	 */
	RESTINIO_NODISCARD
	string_view_t
	fragment(
		//! Starting position for the fragment required.
		string_view_t::size_type from,
		//! Length of the fragment required.
		//! Value string_view_t::npos means the whole remaining content
		//! of the input stream starting from position \a from.
		string_view_t::size_type length = string_view_t::npos ) const noexcept
	{
		return m_data.substr( from, length );
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

		position_t
		started_at() const noexcept
		{
			return m_started_at;
		}

		void
		acquire_content() noexcept
		{
			m_consumed = true;
		}
	};
};

enum class entity_type_t
{
	producer,
	transformer,
	consumer,
	clause
};

//FIXME: a requirement for producer type should be described.

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

//FIXME: a requirement for transformer type should be described.

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
	expected_t< result_type, parse_error_t >
	try_parse( source_t & source )
	{
		auto producer_result = m_producer.try_parse( source );
		if( producer_result )
		{
			return m_transformer.transform( std::move(*producer_result) );
		}
		else
			return make_unexpected( producer_result.error() );
	}
};

template< typename P, typename T >
RESTINIO_NODISCARD
std::enable_if_t<
	is_producer_v<P> & is_transformer_v<T>,
	transformed_value_producer_t< P, T > >
operator>>(
	P producer,
	T transformer )
{
	using transformator_type = transformed_value_producer_t< P, T >;

	return transformator_type{ std::move(producer), std::move(transformer) };
};

//FIXME: a requirement for consumer type should be described.

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

//FIXME: a requirement for clause type should be described.

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
	optional_t< parse_error_t >
	try_process( source_t & from, Target_Type & target )
	{
		auto parse_result = m_producer.try_parse( from );
		if( parse_result )
		{
			m_consumer.consume( target, std::move(*parse_result) );
			return nullopt;
		}
		else
			return parse_result.error();
	}
};

template< typename P, typename C >
RESTINIO_NODISCARD
std::enable_if_t<
	is_producer_v<P> && is_consumer_v<C>,
	consume_value_clause_t< P, C > >
operator>>( P producer, C consumer )
{
	return { std::move(producer), std::move(consumer) };
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
inline optional_t< parse_error_t >
ensure_no_remaining_content(
	source_t & from )
{
	while( !from.eof() )
	{
		if( !is_space( from.getch().m_ch ) )
		{
			return parse_error_t{
					from.current_position(),
					error_reason_t::unexpected_character
			};
		}
	}

	return nullopt;
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
	optional_t< parse_error_t >
	try_process( source_t & from, Target_Type & target )
	{
		const auto starting_pos = from.current_position();

		const bool success = restinio::utils::tuple_algorithms::any_of(
				m_subitems,
				[&from, &target]( auto && one_producer ) {
					source_t::content_consumer_t consumer{ from };
					Target_Type tmp_value{ target };

					auto result = one_producer.try_process( from, tmp_value );
					if( !result )
					{
						target = std::move(tmp_value);
						consumer.acquire_content();

						return true;
					}

					return false;
				} );

		if( !success )
			return parse_error_t{
					starting_pos,
					error_reason_t::no_appropriate_alternative
			};
		else
			return nullopt;
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
	optional_t< parse_error_t >
	try_process( source_t & from, Target_Type & target )
	{
		source_t::content_consumer_t consumer{ from };
		Target_Type tmp_value{ target };

		const bool success = restinio::utils::tuple_algorithms::all_of(
				m_subitems,
				[&from, &tmp_value]( auto && one_producer ) {
					return !one_producer.try_process( from, tmp_value );
				} );

		if( success )
		{
			target = std::move(tmp_value);
			consumer.acquire_content();
		}

		// maybe_producer always returns success even if nothing consumed.
		return nullopt;
	}
};

//
// not_clause_t
//
template<
	typename Subitems_Tuple >
class not_clause_t : public clause_tag
{
	Subitems_Tuple m_subitems;

public :
	not_clause_t(
		Subitems_Tuple && subitems )
		:	m_subitems{ std::move(subitems) }
	{}

	template< typename Target_Type >
	RESTINIO_NODISCARD
	optional_t< parse_error_t >
	try_process( source_t & from, Target_Type & )
	{
		// NOTE: will always return the current position back.
		source_t::content_consumer_t consumer{ from };

		Target_Type dummy_value;

		const auto success = !restinio::utils::tuple_algorithms::all_of(
				m_subitems,
				[&from, &dummy_value]( auto && one_producer ) {
					return !one_producer.try_process( from, dummy_value );
				} );

		// This is contra-intuitive but: we return pattern_not_found in
		// the case when pattern is actually found in the input.
		if( !success )
			return parse_error_t{
					consumer.started_at(),
					//FIXME: maybe a more appropriate error_reason can
					//be used here?
					error_reason_t::pattern_not_found
			};
		else
			return nullopt;
	}
};

//
// and_clause_t
//
template<
	typename Subitems_Tuple >
class and_clause_t : public clause_tag
{
	Subitems_Tuple m_subitems;

public :
	and_clause_t(
		Subitems_Tuple && subitems )
		:	m_subitems{ std::move(subitems) }
	{}

	template< typename Target_Type >
	RESTINIO_NODISCARD
	optional_t< parse_error_t >
	try_process( source_t & from, Target_Type & )
	{
		// NOTE: will always return the current position back.
		source_t::content_consumer_t consumer{ from };

		Target_Type dummy_value;

		const bool success = restinio::utils::tuple_algorithms::all_of(
				m_subitems,
				[&from, &dummy_value]( auto && one_producer ) {
					return !one_producer.try_process( from, dummy_value );
				} );

		if( !success )
			return parse_error_t{
					consumer.started_at(),
					error_reason_t::pattern_not_found
			};
		else
			return nullopt;
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
	optional_t< parse_error_t >
	try_process( source_t & from, Target_Type & target )
	{
		source_t::content_consumer_t consumer{ from };
		Target_Type tmp_value{ target };

		// We should store actual parse error from subitems to return it.
		optional_t< parse_error_t > result;

		const bool success = restinio::utils::tuple_algorithms::all_of(
				m_subitems,
				[&from, &tmp_value, &result]( auto && one_producer ) {
					result = one_producer.try_process( from, tmp_value );
					return !result;
				} );

		if( success )
		{
			target = std::move(tmp_value);
			consumer.acquire_content();
		}

		return result;
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
	expected_t< Target_Type, parse_error_t >
	try_parse( source_t & from )
	{
		Target_Type tmp_value;
		optional_t< parse_error_t > error;

		const bool success = restinio::utils::tuple_algorithms::all_of(
				m_subitems,
				[&from, &tmp_value, &error]( auto && one_clause ) {
					error = one_clause.try_process( from, tmp_value );
					return !error;
				} );

		if( success )
			return std::move(tmp_value);
		else
			return make_unexpected( *error );
	}
};

//
// repeat_clause_t
//
template<
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
	optional_t< parse_error_t >
	try_process( source_t & from, Target_Type & dest )
	{
		source_t::content_consumer_t whole_consumer{ from };

		std::size_t count{};
		bool failure_detected{ false };
		for(; !failure_detected && count != m_max_occurences; )
		{
			source_t::content_consumer_t item_consumer{ from };

			failure_detected = !restinio::utils::tuple_algorithms::all_of(
					m_subitems,
					[&from, &dest]( auto && one_clause ) {
						return !one_clause.try_process( from, dest );
					} );

			if( !failure_detected )
			{
				// Another item successfully parsed and should be stored.
				item_consumer.acquire_content();
				++count;
			}
		}

		if( count >= m_min_occurences )
		{
			whole_consumer.acquire_content();
			return nullopt;
		}

		return parse_error_t{
				from.current_position(),
				error_reason_t::pattern_not_found
		};
	}
};

//
// symbol_producer_t
//
class symbol_producer_t : public producer_tag< char >
{
	char m_expected;

public:
	symbol_producer_t( char expected ) : m_expected{ expected } {}

	RESTINIO_NODISCARD
	expected_t< char, parse_error_t >
	try_parse( source_t & from ) const noexcept
	{
		const auto ch = from.getch();
		if( !ch.m_eof )
		{
			if( ch.m_ch == m_expected )
				return ch.m_ch;
			else
			{
				from.putback();
				return make_unexpected( parse_error_t{
						from.current_position(),
						error_reason_t::unexpected_character
				} );
			}
		}
		else
			return make_unexpected( parse_error_t{
					from.current_position(),
					error_reason_t::unexpected_eof
			} );
	}
};

//
// digit_producer_t
//
class digit_producer_t : public producer_tag< char >
{
public:
	RESTINIO_NODISCARD
	expected_t< char, parse_error_t >
	try_parse( source_t & from ) const noexcept
	{
		const auto ch = from.getch();
		if( !ch.m_eof )
		{
			if( is_digit(ch.m_ch) )
				return ch.m_ch;
			else
			{
				from.putback();
				return make_unexpected( parse_error_t{
						from.current_position(),
						error_reason_t::unexpected_character
				} );
			}
		}
		else
			return make_unexpected( parse_error_t{
					from.current_position(),
					error_reason_t::unexpected_eof
			} );
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
// field_setter_consumer_t
//
template< typename F, typename C >
class field_setter_consumer_t : public consumer_tag
{
	using pointer_t = F C::*;

	pointer_t m_ptr;

public :
	field_setter_consumer_t( pointer_t ptr ) noexcept : m_ptr{ptr} {}

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
	consume_value_clause_t< P, field_setter_consumer_t<F,C> > >
operator>>( P producer, F C::*member_ptr )
{
	return {
			std::move(producer),
			field_setter_consumer_t<F,C>{ member_ptr }
	};
}

//
// to_lower_transformer_t
//
struct to_lower_transformer_t : public transformer_tag< std::string >
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
// not_clause
//
template< typename... Clauses >
RESTINIO_NODISCARD
auto
not_clause( Clauses &&... clauses )
{
	static_assert( meta::all_of_v< impl::is_clause, Clauses... >,
			"all arguments for sequence() should be clauses" );

	using clause_type_t = impl::not_clause_t< std::tuple<Clauses...> >;

	return clause_type_t{
			std::make_tuple(std::forward<Clauses>(clauses)...)
	};
}

//
// and_clause
//
template< typename... Clauses >
RESTINIO_NODISCARD
auto
and_clause( Clauses &&... clauses )
{
	static_assert( meta::all_of_v< impl::is_clause, Clauses... >,
			"all arguments for sequence() should be clauses" );

	using clause_type_t = impl::and_clause_t< std::tuple<Clauses...> >;

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

	using producer_type_t = impl::repeat_clause_t< std::tuple<Clauses...> >;

	return producer_type_t{
			min_occurences,
			max_occurences,
			std::make_tuple(std::forward<Clauses>(clauses)...)
	};
}

//
// skip
//
RESTINIO_NODISCARD
auto
skip() noexcept { return impl::any_value_skipper_t{}; }

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
auto
as_result() noexcept { return impl::as_result_consumer_t{}; }

//
// custom_consumer
//
template< typename F >
RESTINIO_NODISCARD
auto
custom_consumer( F consumer )
{
	using actual_consumer_t = impl::custom_consumer_t< F >;

	return actual_consumer_t{ std::move(consumer) };
}

namespace impl
{

//
// to_container_consumer_t
//
template<
	template<class> class Container_Adaptor >
struct to_container_consumer_t : public consumer_tag
{
	template< typename Container, typename Item >
	void
	consume( Container & to, Item && item )
	{
		Container_Adaptor<Container>::store( to, std::move(item) );
	}
};

} /* namespace impl */

//
// to_container
//
//FIXME: document this!
template<
	template<class> class Container_Adaptor = default_container_adaptor >
RESTINIO_NODISCARD
auto
to_container()
{
	return impl::to_container_consumer_t<Container_Adaptor>();
}

//
// to_lower
//
RESTINIO_NODISCARD
auto
to_lower() noexcept { return impl::to_lower_transformer_t{}; }

//
// try_parse
//
template< typename Producer >
RESTINIO_NODISCARD
expected_t< typename Producer::result_type, parse_error_t >
try_parse(
	string_view_t from,
	Producer producer )
{
	static_assert( impl::is_producer_v<Producer>,
			"Producer should be a value producer type" );

	impl::source_t source{ from };

	auto result = impl::top_level_clause_t< Producer >{ std::move(producer) }
			.try_process( source );

	if( result )
	{
		// We should ensure that all content has been consumed.
		const auto all_content_check =
				impl::ensure_no_remaining_content( source );
		if( all_content_check )
			return make_unexpected( *all_content_check );
	}

	return result;
}

//
// make_error_description
//
//FIXME: document this!
RESTINIO_NODISCARD
inline std::string
make_error_description(
	const parse_error_t & error,
	string_view_t from )
{
	const auto append_quote = [&]( std::string & dest ) {
		constexpr std::size_t max_quote_size = 16u;
		if( error.position() > 0u )
		{
			// How many chars we can get from right of error position?
			const auto prefix_size = error.position() > max_quote_size ?
					max_quote_size : error.position();

			dest.append( 1u, '"' );
			dest.append(
					&from[ error.position() ] - prefix_size,
					prefix_size );
			dest.append( "\" >>> " );
		}

		const char problematic_symbol = error.position() < from.size() ?
				from[ error.position() ] : '?';
		dest.append( 1u, '\'' );
		if( problematic_symbol >= '\x00' && problematic_symbol < ' ' )
		{
			constexpr char hex_digits[] = "0123456789abcdef";

			dest.append( "\\x" );
			dest.append( 1u, hex_digits[
					static_cast<unsigned char>(problematic_symbol) >> 4 ] );
			dest.append( 1u, hex_digits[
					static_cast<unsigned char>(problematic_symbol) & 0xfu ] );
		}
		else
			dest.append( 1u, problematic_symbol );

		dest.append( 1u, '\'' );

		if( error.position() + 1u < from.size() )
		{
			// How many chars we can get from the right of error position?
			const auto suffix_size =
					error.position() + 1u + max_quote_size < from.size() ?
					max_quote_size : from.size() - error.position() - 1u;

			dest.append( " <<< \"" );
			dest.append( &from[ error.position() + 1u ], suffix_size );
			dest.append( 1u, '"' );
		}
	};

	std::string result;

	switch( error.reason() )
	{
		case error_reason_t::unexpected_character:
			result += "unexpected character at ";
			result += std::to_string( error.position() );
			result += ": ";
			append_quote( result );
		break;

		case error_reason_t::unexpected_eof:
			result += "unexpected EOF at ";
			result += std::to_string( error.position() );
		break;

		case error_reason_t::no_appropriate_alternative:
			result += "appropriate alternative can't found at ";
			result += std::to_string( error.position() );
			result += ": ";
			append_quote( result );
		break;

		case error_reason_t::pattern_not_found:
			result += "expected pattern is not found at ";
			result += std::to_string( error.position() );
			result += ": ";
			append_quote( result );
		break;
	}

	return result;
}

} /* namespace easy_parser */

} /* namespace restinio */

