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
#include <restinio/impl/overflow_controlled_integer_accumulator.hpp>

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
	pattern_not_found,
	//! There are some unconsumed non-whitespace characters in the input
	//! after the completion of parsing.
	unconsumed_input,
	//! Illegal value was found in the input.
	/*!
	 * @since v.0.6.2
	 */
	illegal_value_found,
	//! A failure of parsing an alternative marked as "force only this
	//! alternative".
	/*!
	 * This error code is intended for internal use for the implementation
	 * of alternatives() and force_only_this_alternative() stuff.
	 *
	 * This error tells the parser that other alternatives should not be
	 * checked and the parsing of the whole alternatives clause should
	 * failed too.
	 *
	 * @since v.0.6.7
	 */
	force_only_this_alternative_failed
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
 * @since v.0.6.1
 */
struct nothing_t {};

//
// result_value_wrapper
//
/*!
 * @brief A template with specializations for different kind
 * of result values and for type `nothing`.
 *
 * Every specialization for a case when T is not a container should have the
 * following content:
 * @code
 * struct result_value_wrapper<...>
 * {
 * 	using result_type = ... // type of the result value.
 * 	using wrapped_type = ... // type to be created inside a producer
 * 		// to hold a temporary value during the parsing.
 *
 * 	static void
 * 	as_result( wrapped_type & to, result_type && what );
 *
 * 	RESTINIO_NODISCARD static result_type &&
 * 	unwrap_value( wrapped_type & from );
 * };
 * @endcode
 *
 * Every specialization for a case when T is a container should have
 * the following content:
 * @code
 * struct result_value_wrapper<...>
 * {
 * 	using result_type = ... // type of the result value.
 * 	using value_type = ... // type of object to be placed into a container
 * 		// if result_type is a container.
 * 	using wrapped_type = ... // type to be created inside a producer
 * 		// to hold a temporary value during the parsing.
 *
 * 	static void
 * 	as_result( wrapped_type & to, result_type && what );
 *
 * 	static void
 * 	to_container( wrapped_type & to, value_type && item );
 *
 * 	RESTINIO_NODISCARD static result_type &&
 * 	unwrap_value( wrapped_type & from );
 * };
 * @endcode
 *
 * @since v.0.6.6
 */
template< typename T >
struct result_value_wrapper
{
	using result_type = T;
	using wrapped_type = result_type;

	static void
	as_result( wrapped_type & to, result_type && what )
	{
		to = std::move(what);
	}

	RESTINIO_NODISCARD
	static result_type &&
	unwrap_value( wrapped_type & v )
	{
		return std::move(v);
	}
};

//
// result_wrapper_for
//
/*!
 * @brief A metafunction for detection of actual result_value_wrapper
 * type for T
 *
 * If a specialization of result_value_wrapper defines wrapped_type
 * as a different type from result_type then transform() and consume()
 * methods will receive a reference to wrapped_type. And there will be
 * a task to detect actual result_type from a wrapped_type.
 *
 * To solve this task it is necessary to have a way to get
 * result_value_wrapper<result_type> from wrapped_type.
 *
 * This metafunction is that way.
 *
 * @note
 * For each specialization of result_value_wrapper<T> that introduces
 * wrapped_type different from result_type a specialization of
 * result_wrapper_for should also be provided. For example:
 * @code
 * class my_type {...};
 * class my_type_wrapper { ... };
 *
 * namespace restinio {
 * namespace easy_parser {
 *
 * template<>
 * struct result_value_wrapper<my_type> {
 * 	using result_type = my_type;
 * 	using wrapped_type = my_type_wrapper;
 * 	...
 * };
 * template<>
 * struct result_wrapper_for<my_type_wrapper> {
 * 	using type = result_value_wrapper<my_type>;
 * };
 *
 * } // namespace easy_parser
 * } // namespace restinio
 * @endcode
 *
 * @since v.0.6.6
 */
template< typename T >
struct result_wrapper_for
{
	using type = result_value_wrapper< T >;
};

template< typename T >
using result_wrapper_for_t = typename result_wrapper_for<T>::type;

template< typename T, typename... Args >
struct result_value_wrapper< std::vector< T, Args... > >
{
	using result_type = std::vector< T, Args... >;
	using value_type = typename result_type::value_type;
	using wrapped_type = result_type;

	static void
	as_result( wrapped_type & to, result_type && what )
	{
		to = std::move(what);
	}

	static void
	to_container( wrapped_type & to, value_type && what )
	{
		to.push_back( std::move(what) );
	}

	RESTINIO_NODISCARD
	static result_type &&
	unwrap_value( wrapped_type & v )
	{
		return std::move(v);
	}
};

namespace impl
{

//
// std_array_wrapper
//
/*!
 * @brief A special wrapper for std::array type to be used inside
 * a producer during the parsing.
 *
 * This type is intended to be used inside a specialization of
 * result_value_wrapper for std::array.
 *
 * This type holds the current index that can be used by
 * to_container method for addition of a new item to the result array.
 *
 * @since v.0.6.6
 */
template< typename T, std::size_t S >
struct std_array_wrapper
{
	std::array< T, S > m_array;
	std::size_t m_index{ 0u };
};

} /* namespace impl */

template< typename T, std::size_t S >
struct result_value_wrapper< std::array< T, S > >
{
	using result_type = std::array< T, S >;
	using value_type = typename result_type::value_type;
	using wrapped_type = impl::std_array_wrapper< T, S >;

	static void
	as_result( wrapped_type & to, result_type && what )
	{
		to.m_array = std::move(what);
		to.m_index = 0u;
	}

	static void
	to_container( wrapped_type & to, value_type && what )
	{
		if( to.m_index >= S )
			throw exception_t(
					"index in the result std::array is out of range, "
					"index=" + std::to_string(to.m_index) +
					", size={}" + std::to_string(S) );

		to.m_array[ to.m_index ] = std::move(what);
		++to.m_index;
	}

	RESTINIO_NODISCARD
	static result_type &&
	unwrap_value( wrapped_type & v )
	{
		return std::move(v.m_array);
	}
};

/*!
 * @brief A specialization of result_wrapper_for metafunction for
 * the case of std::array wrapper.
 *
 * @since v.0.6.6
 */
template< typename T, std::size_t S >
struct result_wrapper_for< impl::std_array_wrapper<T, S> >
{
	using type = result_value_wrapper< std::array< T, S > >;
};

template< typename Char, typename... Args >
struct result_value_wrapper< std::basic_string< Char, Args... > >
{
	using result_type = std::basic_string< Char, Args... >;
	using value_type = Char;
	using wrapped_type = result_type;

	static void
	as_result( wrapped_type & to, result_type && what )
	{
		to = std::move(what);
	}

	static void
	to_container( wrapped_type & to, value_type && what )
	{
		to.push_back( what );
	}

	RESTINIO_NODISCARD
	static result_type &&
	unwrap_value( wrapped_type & v )
	{
		return std::move(v);
	}
};

template< typename K, typename V, typename... Args >
struct result_value_wrapper< std::map< K, V, Args... > >
{
	using result_type = std::map< K, V, Args... >;
	// NOTE: we can't use container_type::value_type here
	// because value_type for std::map is std::pair<const K, V>,
	// not just std::pair<K, V>,
	using value_type = std::pair<K, V>;
	using wrapped_type = result_type;

	static void
	as_result( wrapped_type & to, result_type && what )
	{
		to = std::move(what);
	}

	static void
	to_container( wrapped_type & to, value_type && what )
	{
		to.emplace( std::move(what) );
	}

	RESTINIO_NODISCARD
	static result_type &&
	unwrap_value( wrapped_type & v )
	{
		return std::move(v);
	}
};

template<>
struct result_value_wrapper< nothing_t >
{
	using result_type = nothing_t;
	using value_type = nothing_t;
	using wrapped_type = result_type;

	static void
	as_result( wrapped_type &, result_type && ) noexcept {}

	static void
	to_container( wrapped_type &, value_type && ) noexcept {}

	RESTINIO_NODISCARD
	static result_type &&
	unwrap_value( wrapped_type & v )
	{
		return std::move(v);
	}
};

/*!
 * @brief A special marker that means infinite repetitions.
 *
 * @since v.0.6.1
 */
constexpr std::size_t N = std::numeric_limits<std::size_t>::max();

//
// digits_to_consume_t
//
/*!
 * @brief Limits for number of digits to be extracted during
 * parsing of decimal numbers.
 *
 * @since v.0.6.6
 */
class digits_to_consume_t
{
public:
	using underlying_int_t = std::int_fast8_t;

	//! Minimal number of digits to consume.
	/*!
	 * @note
	 * Can't be 0, but this value is not checked for
	 * performance reasons.
	 */
	underlying_int_t m_min;
	//! Maximal number of digits to consume.
	underlying_int_t m_max;

public:
	/*!
	 * A constructor for the case when min = max and both are
	 * equal to @a total.
	 */
	constexpr
	digits_to_consume_t( underlying_int_t total ) noexcept
		:	m_min{ total }
		,	m_max{ total }
	{}

	/*!
	 * A constructor for the case when min and max are specified
	 * separately.
	 */
	constexpr
	digits_to_consume_t(
		underlying_int_t min,
		underlying_int_t max ) noexcept
		:	m_min{ min }
		,	m_max{ max }
	{}

	//! Get the minimal value.
	RESTINIO_NODISCARD
	constexpr auto
	min() const noexcept { return m_min; }

	//! Get the maximum value.
	RESTINIO_NODISCARD
	constexpr auto
	max() const noexcept { return m_max; }

	//! Get the value that means that maximum is not limited.
	RESTINIO_NODISCARD
	constexpr static auto
	unlimited_max() noexcept
	{
		return std::numeric_limits<underlying_int_t>::max();
	}

	/*!
	 * Returns `digits_to_consume_t{1, unlimited_max()}`.
	 */
	RESTINIO_NODISCARD
	constexpr static auto
	from_one_to_max() noexcept
	{
		return digits_to_consume_t{ 1, unlimited_max() };
	}
};

/*!
 * @brief Create a limit for number of digits to be extracted.
 *
 * Makes a limit where min==max and both are equal to @a total.
 *
 * Usage example:
 * @code
 * using namespace restinio::easy_parser;
 *
 * auto x_uint32_p = hexadecimal_number_p<std::uint32_t>(expected_digits(8));
 * @endcode
 *
 * @since v.0.6.6
 */
RESTINIO_NODISCARD
inline constexpr digits_to_consume_t
expected_digits( digits_to_consume_t::underlying_int_t total ) noexcept
{
	return { total };
}

/*!
 * @brief Create a limit for number of digits to be extracted.
 *
 * Makes a limit where min and max are specified separately.
 *
 * Usage example:
 * @code
 * using namespace restinio::easy_parser;
 *
 * auto ten_digits_int32_p = decimal_number_p<std::int32_t>(expected_digits(1, 10));
 * @endcode
 *
 * @since v.0.6.6
 */
RESTINIO_NODISCARD
inline constexpr digits_to_consume_t
expected_digits(
	digits_to_consume_t::underlying_int_t min,
	digits_to_consume_t::underlying_int_t max ) noexcept
{
	return { min, max };
}

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
// is_space_predicate_t
//
/*!
 * @brief A preducate for symbol_producer_template that checks that
 * a symbol is a space.
 *
 * @since v.0.6.4
 */
struct is_space_predicate_t
{
	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		return is_space(actual);
	}
};

//
// is_digit
//
/*!
 * @brief Is a character a decimal digit?
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
// is_digit_predicate_t
//
/*!
 * @brief A predicate for cases where char to be expected to be a decimal digit.
 *
 * @since v.0.6.6
 */
struct is_digit_predicate_t
{
	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		return is_digit( actual );
	}
};

//
// is_hexdigit
//
/*!
 * @brief Is a character a hexadecimal digit?
 *
 * @since v.0.6.6
 */
RESTINIO_NODISCARD
inline constexpr bool
is_hexdigit( const char ch ) noexcept
{
	return (ch >= '0' && ch <= '9') ||
			(ch >= 'A' && ch <= 'F') ||
			(ch >= 'a' && ch <= 'f');
}

//
// is_hexdigit_predicate_t
//
/*!
 * @brief A predicate for cases where char to be expected
 * to be a hexadecimal digit.
 *
 * @since v.0.6.6
 */
struct is_hexdigit_predicate_t
{
	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		return is_hexdigit( actual );
	}
};

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

	/*!
	 * @brief A helper class to automatically return acquired
	 * content back to the input stream.
	 *
	 * Usage example:
	 * @code
	 * expected_t<result_type, parse_error_t> try_parse(source_t & from) {
	 * 	source_t::content_consumer_t consumer{from};
	 * 	for(auto ch = from.getch(); some_condition(ch); ch = from.getch())
	 * 	{
	 * 		... // do something with ch.
	 * 	}
	 * 	if(no_errors_detected())
	 * 		// All acquired content should be consumed.
	 * 		consumer.commit();
	 *
	 * 	// Otherwise all acquired content will be returned back to the input stream.
	 * 	...
	 * }
	 * @endcode
	 *
	 * @since v.0.6.1
	 */
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

		//! Consume all acquired content.
		/*!
		 * @note
		 * If that method is not called then all acquired content
		 * will be returned back.
		 */
		void
		commit() noexcept
		{
			m_consumed = true;
		}
	};
};

//
// entity_type_t
//
/*!
 * @brief A marker for distinguish different kind of entities in parser.
 *
 * @since v.0.6.1
 */
enum class entity_type_t
{
	//! Entity is a producer of values.
	producer,
	//! Entity is a transformer of a value from one type to another.
	transformer,
	//! Entity is a consumer of values. It requires a value on the input
	//! and doesn't produces anything.
	consumer,
	//! Entity is a clause. It doesn't produces anything.
	clause,
	//! Entity is a transformer-proxy. It can't be used directly, only
	//! for binding a producer and transformer together.
	/*!
	 * @since v.0.6.6.
	 */
	transformer_proxy
};

//
// producer_tag
//
/*!
 * @brief A special base class to be used with producers.
 *
 * Every producer class should have the following content:
 *
 * @code
 * class some_producer_type
 * {
 * public:
 * 	using result_type = ... // some producer-specific type.
 * 	static constexpr entity_type_t entity_type = entity_type_t::producer;
 *
 * 	expected_t<result_type, parse_error_t>
 * 	try_parse(source_t & from);
 *
 * 	...
 * };
 * @endcode
 *
 * @since v.0.6.1
 */
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

/*!
 * @brief A meta-value to check whether T is a producer type.
 *
 * @note
 * The current implementation checks only the presence of T::entity_type of
 * type entity_type_t and the value of T::entity_type. Presence of
 * T::result_type and T::try_parse is not checked.
 *
 * @since v.0.6.1
 */
template< typename T >
constexpr bool is_producer_v = is_producer<T>::value;

//
// transformer_tag
//
/*!
 * @brief A special base class to be used with transformers.
 *
 * Every transformer class should have the following content:
 *
 * @code
 * class some_transformer_type
 * {
 * public:
 * 	using result_type = ... // some transformer-specific type.
 * 	static constexpr entity_type_t entity_type = entity_type_t::transformer;
 *
 * 	result_type
 * 	transform(Input_Type && from);
 *
 * 	...
 * };
 * @endcode
 * where `Input_Type` is transformer's specific types.
 *
 * @since v.0.6.1
 */
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

/*!
 * @brief A meta-value to check whether T is a transformer type.
 *
 * @note
 * The current implementation checks only the presence of T::entity_type of
 * type entity_type_t and the value of T::entity_type. Presence of
 * T::result_type and T::transform is not checked.
 *
 * @since v.0.6.1
 */
template< typename T >
constexpr bool is_transformer_v = is_transformer<T>::value;

//
// transformed_value_producer_t
//
/*!
 * @brief A template of producer that gets a value from another
 * producer, transforms it and produces transformed value.
 *
 * @tparam Producer the type of producer of source value.
 * @tparam Transformer the type of transformer from source to the target value.
 *
 * @since v.0.6.1
 */
template< typename Producer, typename Transformer >
class transformed_value_producer_t
	:	public producer_tag< typename Transformer::result_type >
{
	static_assert( is_producer_v<Producer>,
			"Producer should be a producer type" );
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

/*!
 * @brief A special operator to connect a value producer with value transformer.
 *
 * @since v.0.6.1
 */
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
}

//
// transformer_proxy_tag
//
/*!
 * @brief A special base class to be used with transformer-proxies.
 *
 * Every transformer-proxy class should have the following content:
 *
 * @code
 * class some_transformer_proxy_type
 * {
 * public:
 * 	static constexpr entity_type_t entity_type = entity_type_t::transformer;
 *
 * 	template< typename Input_Type >
 * 	auto
 * 	make_transformer();
 * 	...
 * };
 * @endcode
 * where `Input_Type` is will be specified by a producer.
 *
 * @since v.0.6.6
 */
struct transformer_proxy_tag
{
	static constexpr entity_type_t entity_type = entity_type_t::transformer_proxy;
};

template< typename T, typename = meta::void_t<> >
struct is_transformer_proxy : public std::false_type {};

template< typename T >
struct is_transformer_proxy< T, meta::void_t< decltype(T::entity_type) > >
{
	static constexpr bool value = entity_type_t::transformer_proxy == T::entity_type;
};

/*!
 * @brief A meta-value to check whether T is a transformer-proxy type.
 *
 * @note
 * The current implementation checks only the presence of T::entity_type of
 * type entity_type_t and the value of T::entity_type.
 *
 * @since v.0.6.6
 */
template< typename T >
constexpr bool is_transformer_proxy_v = is_transformer_proxy<T>::value;

/*!
 * @brief A special operator to connect a value producer with value transformer
 * via transformer-proxy.
 *
 * @since v.0.6.6
 */
template<
	typename P,
	typename T,
	typename S = std::enable_if_t<
			is_producer_v<P> & is_transformer_proxy_v<T>,
			void > >
RESTINIO_NODISCARD
auto
operator>>(
	P producer,
	T transformer_proxy )
{
	auto real_transformer = transformer_proxy.template make_transformer< 
			typename P::result_type >();

	using transformator_type = std::decay_t< decltype(real_transformer) >;

	using producer_type = transformed_value_producer_t< P, transformator_type >;

	return producer_type{ std::move(producer), std::move(real_transformer) };
}

//
// consumer_tag
//
/*!
 * @brief A special base class to be used with consumers.
 *
 * Every consumer class should have the following content:
 *
 * @code
 * class some_consumer_type
 * {
 * public :
 * 	static constexpr entity_type_t entity_type = entity_type_t::consumer;
 *
 * 	void consume( Target_Type & dest, Value && current_value );
 * 	...
 * };
 * @endcode
 * where `Target_Type` and `Value` are consumer's specific types.
 *
 * @since v.0.6.1
 */
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

/*!
 * @brief A meta-value to check whether T is a consumer type.
 *
 * @note
 * The current implementation checks only the presence of T::entity_type of
 * type entity_type_t and the value of T::entity_type. Presence of
 * T::consume is not checked.
 *
 * @since v.0.6.1
 */
template< typename T >
constexpr bool is_consumer_v = is_consumer<T>::value;

//
// clause_tag
//
/*!
 * @brief A special base class to be used with clauses.
 *
 * Every clause class should have the following content:
 *
 * @code
 * class some_consumer_type
 * {
 * public :
 * 	static constexpr entity_type_t entity_type = entity_type_t::clause;
 *
 * 	optional_t<parse_error_t>
 * 	try_process(source_t & from, Target_Type & dest);
 * 	...
 * };
 * @endcode
 * where `Target_Type` is clause's specific types.
 *
 * @since v.0.6.1
 */
struct clause_tag
{
	static constexpr entity_type_t entity_type = entity_type_t::clause;
};

template< typename T, typename = meta::void_t<> >
struct is_clause : public std::false_type {};

template< typename T >
struct is_clause< T, meta::void_t<
	decltype(std::decay_t<T>::entity_type) > >
{
	using real_type = std::decay_t<T>;

	static constexpr bool value = entity_type_t::clause == real_type::entity_type;
};

/*!
 * @brief A meta-value to check whether T is a consumer type.
 *
 * @note
 * The current implementation checks only the presence of T::entity_type of
 * type entity_type_t and the value of T::entity_type. Presence of
 * T::try_process is not checked.
 *
 * @since v.0.6.1
 */
template< typename T >
constexpr bool is_clause_v = is_clause<T>::value;

//
// tuple_of_entities_t
//
/*!
 * @brief A helper meta-function to create an actual type of tuple
 * with clauses/producers.
 *
 * Usage example:
 * @code
 * template< typename... Clauses >
 * auto
 * some_clause( Clauses && ...clauses ) {
 * 	using clause_type = impl::some_clause_t<
 * 			impl::tuple_of_entities_t<Clauses...> >;
 * 	return clause_type{ std::forward<Clauses>(clauses)... };
 * }
 * @endcode
 *
 * The tuple_of_entities_t takes care about such cases as references and
 * constness of parameters. For example:
 * @code
 * auto c = symbol('c');
 * const auto b = symbol('b');
 * auto clause = some_clause(c, b);
 * @endcode
 * In that case `Clauses...` will be `symbol_clause_t&, const
 * symbol_clause_t&`. And an attempt to make type `std::tuple<Clauses...>` will
 * produce type `std::tuple<symbol_clause_t&, const symbol_clause_t&>`. But we
 * need `std::tuple<symbol_clause_t, symbol_clause_t>`. This result will be
 * obtained if `tuple_of_entities_t` is used instead of `std::tuple`.
 *
 * @since v.0.6.6
 */
template< typename... Entities >
using tuple_of_entities_t = meta::rename_t<
		meta::transform_t< std::decay, meta::type_list<Entities...> >,
		std::tuple >;

//
// consume_value_clause_t
//
/*!
 * @brief A template for a clause that binds a value producer with value
 * consumer.
 *
 * @tparam P the type of value producer.
 * @tparam C the type of value consumer.
 *
 * @since v.0.6.1
 */
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

/*!
 * @brief A special operator to connect a value producer with a value consumer.
 *
 * @since v.0.6.1
 */
template< typename P, typename C >
RESTINIO_NODISCARD
std::enable_if_t<
	is_producer_v<P> && is_consumer_v<C>,
	consume_value_clause_t< P, C > >
operator>>( P producer, C consumer )
{
	return { std::move(producer), std::move(consumer) };
}

//
// top_level_clause_t
//
/*!
 * @brief A special class to be used as the top level clause in parser.
 *
 * @note
 * That class doesn't look like an ordinal clause and can't be connected
 * with other clauses. Method try_process has the different format and
 * returns the value of Producer::try_parse.
 *
 * @since v.0.6.1
 */
template< typename Producer >
class top_level_clause_t
{
	static_assert( is_producer_v<Producer>,
			"Producer should be a producer type" );

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

//
// ensure_no_remaining_content
//
/*!
 * @brief A special function to check that there is no more actual
 * data in the input stream except whitespaces.
 *
 * @return parse_error_t if some non-whitespace character is found
 * in the input stream.
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline optional_t< parse_error_t >
ensure_no_remaining_content(
	source_t & from )
{
	while( !from.eof() )
	{
		if( !is_space( from.getch().m_ch ) )
		{
			from.putback(); // Otherwise current_position() will be wrong.
			return parse_error_t{
					from.current_position(),
					error_reason_t::unconsumed_input
			};
		}
	}

	return nullopt;
}

//
// remove_trailing_spaces
//
/*!
 * @brief Helper function for removal of trailing spaces from a string-view.
 *
 * @since v.0.6.7
 */
RESTINIO_NODISCARD
inline string_view_t
remove_trailing_spaces( string_view_t from ) noexcept
{
	auto s = from.size();
	for(; s && is_space( from[ (s-1u) ] ); --s) {}

	return from.substr( 0u, s );
}

//
// alternatives_clause_t
//
/*!
 * @brief A template for implementation of clause that selects one of
 * alternative clauses.
 *
 * This template implements rules like:
   @verbatim
   T := A | B | C
   @endverbatim
 *
 * It works very simple way:
 *
 * - `try_process` for the first alternative is called. If it fails then...
 * - `try_process` for the second alternative is called. If it fails then...
 * - `try_process` for the third alternative is called...
 * - and so on.
 *
 * If no one of alternatives is selected then the current position in
 * the input stream is restored.
 *
 * @note
 * The copy of Target_Type object passed to `try_process` method is
 * created before checking each alternative.
 *
 * @tparam Subitems_Tuple the type of std::tuple with items for every
 * alternative clauses.
 *
 * @since v.0.6.1
 */
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

		optional_t< parse_error_t > actual_parse_error;
		const bool success = restinio::utils::tuple_algorithms::any_of(
				m_subitems,
				[&from, &target, &actual_parse_error]( auto && one_producer ) {
					source_t::content_consumer_t consumer{ from };
					Target_Type tmp_value{ target };

					actual_parse_error = one_producer.try_process( from, tmp_value );
					if( !actual_parse_error )
					{
						target = std::move(tmp_value);
						consumer.commit();

						return true;
					}
					else {
						// Since v.0.6.7 we should check for
						// force_only_this_alternative_failed error.
						// In the case of that error enumeration of alternatives
						// should be stopped.
						return error_reason_t::force_only_this_alternative_failed ==
								actual_parse_error->reason();
					}
				} );

		if( !success || actual_parse_error )
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
/*!
 * @brief A template for implementation of clause that checks and
 * handles presence of optional entity in the input stream.
 *
 * This template implements rules like:
   @verbatim
   T := [ A B C ]
   @endverbatim
 *
 * @note
 * The copy of Target_Type object passed to `try_process` method is
 * created before checking the presence of subitems. If all subitems
 * are found then the value of that temporary object moved back to
 * \a target parameter of `try_process` method.
 *
 * @note
 * This clause always returns success even if nothing has been
 * consumed from the input stream.
 *
 * @tparam Subitems_Tuple the type of std::tuple with items for every
 * clause to be checked.
 *
 * @since v.0.6.1
 */
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
			consumer.commit();
		}

		// maybe_clause always returns success even if nothing consumed.
		return nullopt;
	}
};

//
// not_clause_t
//
/*!
 * @brief A template for implementation of clause that checks absence of
 * some entity in the input stream.
 *
 * This template implements rules like:
   @verbatim
	T := !A B 
   @endverbatim
 * where not_clause_t is related to the part `!A` only.
 *
 * @note
 * The empty temporary object of Target_Type passed to call of `try_process` of
 * subitems.
 *
 * @note
 * This clause always returns the current position in the input stream
 * back at the position where this clause was called.
 *
 * @tparam Subitems_Tuple the type of std::tuple with items for every
 * clause to be checked.
 *
 * @since v.0.6.1
 */
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
/*!
 * @brief A template for implementation of clause that checks the presence of
 * some entity in the input stream.
 *
 * This template implements rules like:
   @verbatim
	T := A &B 
   @endverbatim
 * where and_clause_t is related to the part `&B` only.
 *
 * @note
 * The empty temporary object of Target_Type passed to call of `try_process` of
 * subitems.
 *
 * @note
 * This clause always returns the current position in the input stream
 * back at the position where this clause was called.
 *
 * @tparam Subitems_Tuple the type of std::tuple with items for every
 * clause to be checked.
 *
 * @since v.0.6.1
 */
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
/*!
 * @brief A template for implementation of clause that checks and
 * handles presence of sequence of entities in the input stream.
 *
 * This template implements rules like:
   @verbatim
   T := A B C
   @endverbatim
 *
 * @note
 * The copy of Target_Type object passed to `try_process` method is
 * created before checking the presence of subitems. If all subitems
 * are found then the value of that temporary object moved back to
 * @a target parameter of `try_process` method.
 *
 * @tparam Subitems_Tuple the type of std::tuple with items for every
 * clause to be checked.
 *
 * @since v.0.6.1
 */
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
			consumer.commit();
		}

		return result;
	}
};

//
// forced_alternative_clause_t
//
/*!
 * @brief An alternative that should be parsed correctly or the parsing
 * of the whole alternatives clause should fail.
 *
 * This special clause is intended to be used in the implementation
 * of restinio::easy_parser::force_only_this_alternative(). See the
 * description of that function for more details.
 *
 * @since v.0.6.7
 */
template<
	typename Subitems_Tuple >
class forced_alternative_clause_t : public sequence_clause_t< Subitems_Tuple >
{
	using base_type_t = sequence_clause_t< Subitems_Tuple >;

public :
	using base_type_t::base_type_t;

	template< typename Target_Type >
	RESTINIO_NODISCARD
	optional_t< parse_error_t >
	try_process( source_t & from, Target_Type & target )
	{
		const auto starting_pos = from.current_position();

		if( base_type_t::try_process( from, target ) )
		{
			// The forced clause is not parsed correctly.
			// So the special error code should be returned in that case.
			return parse_error_t{
					starting_pos,
					error_reason_t::force_only_this_alternative_failed
			};
		}
		else
			return nullopt;
	}
};

//
// produce_t
//
/*!
 * @brief A template for producing a value of specific type of
 * a sequence of entities from the input stream.
 *
 * Creates a new empty object of type Target_Type in `try_parse` and
 * then call `try_process` methods for every subitems. A reference to
 * that new object is passed to every `try_process` call.
 *
 * @tparam Target_Type the type of value to be produced.
 * @tparam Subitems_Tuple the type of std::tuple with items for every
 * clause to be checked.
 *
 * @since v.0.6.1
 */
template<
	typename Target_Type,
	typename Subitems_Tuple >
class produce_t : public producer_tag< Target_Type >
{
	using value_wrapper_t = result_value_wrapper< Target_Type >;

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
		typename value_wrapper_t::wrapped_type tmp_value{};
		optional_t< parse_error_t > error;

		const bool success = restinio::utils::tuple_algorithms::all_of(
				m_subitems,
				[&from, &tmp_value, &error]( auto && one_clause ) {
					error = one_clause.try_process( from, tmp_value );
					return !error;
				} );

		if( success )
			return value_wrapper_t::unwrap_value( tmp_value );
		else
			return make_unexpected( *error );
	}
};

//
// repeat_clause_t
//
/*!
 * @brief A template for handling repetition of clauses.
 *
 * Calls `try_process` for all subitems until some of them returns
 * error or max_occurences will be passed.
 *
 * Returns failure if min_occurences wasn't passed.
 *
 * @tparam Subitems_Tuple the type of std::tuple with items for every
 * clause to be checked.
 *
 * @since v.0.6.1
 */
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
				item_consumer.commit();
				++count;
			}
		}

		if( count >= m_min_occurences )
		{
			whole_consumer.commit();
			return nullopt;
		}

		return parse_error_t{
				from.current_position(),
				error_reason_t::pattern_not_found
		};
	}
};

//
// symbol_producer_template_t
//
/*!
 * @brief A template for producer of charachers that satisfy some predicate.
 *
 * In the case of success returns the expected character.
 *
 * @tparam Predicate the type of predicate to check extracted symbol.
 *
 * @since v.0.6.1
 */
template< typename Predicate >
class symbol_producer_template_t
	:	public producer_tag< char >
	,	protected Predicate
{
public:
	template< typename... Args >
	symbol_producer_template_t( Args &&... args )
		:	 Predicate{ std::forward<Args>(args)... }
	{}

	RESTINIO_NODISCARD
	expected_t< char, parse_error_t >
	try_parse( source_t & from ) const noexcept
	{
		const auto ch = from.getch();
		if( !ch.m_eof )
		{
			// A call to predicate.
			if( (*this)(ch.m_ch) )
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
// any_symbol_predicate_t
//
/*!
 * @brief A predicate that allows extraction of any symbol.
 *
 * This predicate is necessary for implementation of any_symbol_p()
 * producer.
 *
 * @since v.0.6.6
 */
struct any_symbol_predicate_t
{
	RESTINIO_NODISCARD
	constexpr bool
	operator()( const char ) const noexcept
	{
		return true;
	}
};

//
// particular_symbol_predicate_t
//
/*!
 * @brief A predicate for cases where exact match of expected and
 * actual symbols is required.
 *
 * @since v.0.6.1
 */
struct particular_symbol_predicate_t
{
	char m_expected;

	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		return m_expected == actual;
	}
};

//
// not_particular_symbol_predicate_t
//
/*!
 * @brief A predicate for cases where mismatch with a particular
 * symbol is required.
 *
 * @since v.0.6.6
 */
struct not_particular_symbol_predicate_t
{
	char m_sentinel;

	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		return m_sentinel != actual;
	}
};

//
// caseless_particular_symbol_predicate_t
//
/*!
 * @brief A predicate for cases where the case-insensitive match of expected
 * and actual symbols is required.
 *
 * @since v.0.6.6
 */
struct caseless_particular_symbol_predicate_t
{
	char m_expected;

	caseless_particular_symbol_predicate_t( char v ) noexcept
		:	m_expected{ restinio::impl::to_lower_case( v ) }
	{}

	RESTINIO_NODISCARD
	bool
	operator()( const char actual ) const noexcept
	{
		return m_expected == restinio::impl::to_lower_case(actual);
	}
};

//
// symbol_producer_t
//
/*!
 * @brief A producer for the case when a particual character is expected
 * in the input stream.
 *
 * In the case of success returns the expected character.
 *
 * @since v.0.6.1
 */
class symbol_producer_t
	: public symbol_producer_template_t< particular_symbol_predicate_t >
{
	using base_type_t =
		symbol_producer_template_t< particular_symbol_predicate_t >;

public:
	symbol_producer_t( char expected )
		:	base_type_t{ particular_symbol_predicate_t{expected} }
	{}
};

//
// any_symbol_if_not_producer_t
//
/*!
 * @brief A producer for the case when any character except the specific
 * sentinel character is expected in the input stream.
 *
 * In the case of success returns a character from the input stream.
 *
 * @since v.0.6.6
 */
class any_symbol_if_not_producer_t
	: public symbol_producer_template_t< not_particular_symbol_predicate_t >
{
	using base_type_t =
		symbol_producer_template_t< not_particular_symbol_predicate_t >;

public:
	any_symbol_if_not_producer_t( char sentinel )
		:	base_type_t{ not_particular_symbol_predicate_t{sentinel} }
	{}
};

//
// caseless_symbol_producer_t
//
/*!
 * @brief A producer for the case when a particual character is expected
 * in the input stream.
 *
 * Performs caseless comparison of symbols.
 *
 * In the case of success returns the character from the input stream
 * (e.g. without transformation to lower or upper case).
 *
 * @since v.0.6.6
 */
class caseless_symbol_producer_t
	: public symbol_producer_template_t< caseless_particular_symbol_predicate_t >
{
	using base_type_t =
		symbol_producer_template_t< caseless_particular_symbol_predicate_t >;

public:
	caseless_symbol_producer_t( char expected )
		:	base_type_t{ caseless_particular_symbol_predicate_t{expected} }
	{}
};

//
// digit_producer_t
//
/*!
 * @brief A producer for the case when a decimal digit is expected
 * in the input stream.
 *
 * In the case of success returns the extracted character.
 *
 * @since v.0.6.1
 */
class digit_producer_t
	: public symbol_producer_template_t< is_digit_predicate_t >
{
public:
	digit_producer_t() {}
};

//
// hexdigit_producer_t
//
/*!
 * @brief A producer for the case when a hexadecimal digit is expected
 * in the input stream.
 *
 * In the case of success returns the extracted character.
 *
 * @since v.0.6.6
 */
class hexdigit_producer_t
	: public symbol_producer_template_t< is_hexdigit_predicate_t >
{
public:
	hexdigit_producer_t() {}
};

//
// try_parse_digits_with_digits_limit
//
/*!
 * @brief Helper function for parsing integers with respect to
 * the number of digits to be consumed.
 *
 * Usage example:
 * @code
 * // For the case of unsigned or positive signed integer:
 * auto r = try_parse_digits_with_digits_limit<unsigned int>(from,
 * 		expected_digits(4),
 * 		restinio::impl::overflow_controlled_integer_accumulator_t<unsigned int, 10>{});
 * // For the case of negative signed integer.
 * auto r = try_parse_digits_with_digits_limit<short>(from,
 * 		expected_digits(4),
 * 		restinio::impl::overflow_controlled_integer_accumulator_t<
 * 				short,
 * 				10,
 * 				restinio::impl::check_negative_extremum>{});
 * @endcode
 *
 * @since v.0.6.6
 */
template< typename T, typename Value_Accumulator >
RESTINIO_NODISCARD
expected_t< T, parse_error_t >
try_parse_digits_with_digits_limit(
	source_t & from,
	digits_to_consume_t digits_limit,
	Value_Accumulator acc ) noexcept
{
	source_t::content_consumer_t consumer{ from };

	digits_to_consume_t::underlying_int_t symbols_processed{};

	for( auto ch = from.getch(); !ch.m_eof; ch = from.getch() )
	{
		if( is_digit(ch.m_ch) )
		{
			acc.next_digit( static_cast<T>(ch.m_ch - '0') );

			if( acc.overflow_detected() )
				return make_unexpected( parse_error_t{
						consumer.started_at(),
						error_reason_t::illegal_value_found
				} );

			++symbols_processed;
			if( symbols_processed == digits_limit.max() )
				break;
		}
		else
		{
			from.putback();
			break;
		}
	}

	if( symbols_processed < digits_limit.min() )
		// Not all required digits are extracted.
		return make_unexpected( parse_error_t{
				from.current_position(),
				error_reason_t::pattern_not_found
		} );
	else
	{
		consumer.commit();
		return acc.value();
	}
}

//
// try_parse_hexdigits_with_digits_limit
//
/*!
 * @brief Helper function for parsing integers in hexadecimal form.
 *
 * Usage example:
 * @code
 * // For the case of unsigned or positive signed integer:
 * auto r = try_parse_hexdigits_with_digits_limit<unsigned int>(from,
 * 		expected_digits(4, 8),
 * 		restinio::impl::overflow_controlled_integer_accumulator_t<unsigned int, 16>{});
 * @endcode
 *
 * @since v.0.6.6
 */
template< typename T, typename Value_Accumulator >
RESTINIO_NODISCARD
expected_t< T, parse_error_t >
try_parse_hexdigits_with_digits_limit(
	source_t & from,
	digits_to_consume_t digits_limit,
	Value_Accumulator acc ) noexcept
{
	const auto ch_to_digit = []( char ch ) -> std::pair<bool, T> {
		if( ch >= '0' && ch <= '9' )
			return std::make_pair( true, static_cast<T>(ch - '0') );
		else if( ch >= 'A' && ch <= 'F' )
			return std::make_pair( true, static_cast<T>(10 + (ch - 'A')) );
		else if( ch >= 'a' && ch <= 'f' )
			return std::make_pair( true, static_cast<T>(10 + (ch - 'a')) );
		else
			return std::make_pair( false, static_cast<T>(0) );
	};

	source_t::content_consumer_t consumer{ from };

	digits_to_consume_t::underlying_int_t symbols_processed{};

	for( auto ch = from.getch(); !ch.m_eof; ch = from.getch() )
	{
		const auto d = ch_to_digit( ch.m_ch );
		if( d.first )
		{
			acc.next_digit( d.second );

			if( acc.overflow_detected() )
				return make_unexpected( parse_error_t{
						consumer.started_at(),
						error_reason_t::illegal_value_found
				} );

			++symbols_processed;
			if( symbols_processed == digits_limit.max() )
				break;
		}
		else
		{
			from.putback();
			break;
		}
	}

	if( symbols_processed < digits_limit.min() )
		// Not all required digits are extracted.
		return make_unexpected( parse_error_t{
				from.current_position(),
				error_reason_t::pattern_not_found
		} );
	else
	{
		consumer.commit();
		return acc.value();
	}
}

//
// non_negative_decimal_number_producer_t
//
/*!
 * @brief A producer for the case when a non-negative decimal number is
 * expected in the input stream.
 *
 * In the case of success returns the extracted number.
 *
 * @since v.0.6.2
 */
template< typename T >
class non_negative_decimal_number_producer_t : public producer_tag< T >
{
public:
	RESTINIO_NODISCARD
	expected_t< T, parse_error_t >
	try_parse( source_t & from ) const noexcept
	{
		return try_parse_digits_with_digits_limit< T >(
				from,
				digits_to_consume_t::from_one_to_max(),
				restinio::impl::overflow_controlled_integer_accumulator_t<T, 10>{} );
	}
};

//
// non_negative_decimal_number_producer_with_digits_limit_t
//
/*!
 * @brief A producer for the case when a non-negative decimal number is
 * expected in the input stream.
 *
 * This class takes into account a number of digits to be consumed.
 *
 * In the case of success returns the extracted number.
 *
 * @since v.0.6.6
 */
template< typename T >
class non_negative_decimal_number_producer_with_digits_limit_t
	:	public non_negative_decimal_number_producer_t<T>
{
	digits_to_consume_t m_digits_limit;

public:
	non_negative_decimal_number_producer_with_digits_limit_t(
		digits_to_consume_t digits_limit )
		:	m_digits_limit{ digits_limit }
	{}

	RESTINIO_NODISCARD
	expected_t< T, parse_error_t >
	try_parse( source_t & from ) const noexcept
	{
		return try_parse_digits_with_digits_limit< T >(
				from,
				m_digits_limit,
				restinio::impl::overflow_controlled_integer_accumulator_t<T, 10>{} );
	}
};

//
// hexadecimal_number_producer_t
//
/*!
 * @brief A producer for the case when a number in hexadecimal form is expected
 * in the input stream.
 *
 * In the case of success returns the extracted number.
 *
 * @since v.0.6.6
 */
template< typename T >
class hexadecimal_number_producer_t : public producer_tag< T >
{
	static_assert( std::is_unsigned<T>::value,
			"T is expected to be unsigned type" );

public:
	RESTINIO_NODISCARD
	expected_t< T, parse_error_t >
	try_parse( source_t & from ) const noexcept
	{
		return try_parse_hexdigits_with_digits_limit< T >(
				from,
				digits_to_consume_t::from_one_to_max(),
				restinio::impl::overflow_controlled_integer_accumulator_t<T, 16>{} );
	}
};

//
// hexadecimal_number_producer_with_digits_limit_t
//
/*!
 * @brief A producer for the case when a number in hexadecimal form is expected
 * in the input stream.
 *
 * This class takes into account a number of digits to be consumed.
 *
 * In the case of success returns the extracted number.
 *
 * @since v.0.6.6
 */
template< typename T >
class hexadecimal_number_producer_with_digits_limit_t
	:	public hexadecimal_number_producer_t< T >
{
	digits_to_consume_t m_digits_limit;

public:
	hexadecimal_number_producer_with_digits_limit_t(
		digits_to_consume_t digits_limit )
		:	m_digits_limit{ digits_limit }
	{}

	RESTINIO_NODISCARD
	expected_t< T, parse_error_t >
	try_parse( source_t & from ) const noexcept
	{
		return try_parse_hexdigits_with_digits_limit< T >(
				from,
				m_digits_limit,
				restinio::impl::overflow_controlled_integer_accumulator_t<T, 16>{} );
	}
};

//
// decimal_number_producer_t
//
/*!
 * @brief A producer for the case when a signed decimal number is
 * expected in the input stream.
 *
 * In the case of success returns the extracted number.
 *
 * @since v.0.6.6
 */
template< typename T >
class decimal_number_producer_t : public producer_tag<T>
{
	static_assert( std::is_signed<T>::value,
			"decimal_number_producer_t can be used only for signed types" );

public:
	using try_parse_result_type = expected_t< T, parse_error_t >;

	RESTINIO_NODISCARD
	try_parse_result_type
	try_parse( source_t & from ) const noexcept
	{
		return try_parse_impl( from,
				[]() noexcept {
					return digits_to_consume_t::from_one_to_max();
				} );
	}

protected:
	template< typename Digits_Limit_Maker >
	RESTINIO_NODISCARD
	try_parse_result_type
	try_parse_impl(
		source_t & from,
		Digits_Limit_Maker && digits_limit_maker ) const noexcept
	{
		source_t::content_consumer_t consumer{ from };

		auto sign_ch = from.getch();
		if( !sign_ch.m_eof )
		{
			const auto r = try_parse_with_this_first_symbol(
					from,
					sign_ch.m_ch,
					std::forward<Digits_Limit_Maker>(digits_limit_maker) );

			if( r )
				consumer.commit();

			return r;
		}
		else
			return make_unexpected( parse_error_t{
					from.current_position(),
					error_reason_t::pattern_not_found
			} );
	}

private:
	template< typename Digits_Limit_Maker >
	RESTINIO_NODISCARD
	static try_parse_result_type
	try_parse_with_this_first_symbol(
		source_t & from,
		char first_symbol,
		Digits_Limit_Maker && digits_limit_maker ) noexcept
	{
		using restinio::impl::overflow_controlled_integer_accumulator_t;
		using restinio::impl::check_negative_extremum;

		if( '-' == first_symbol )
		{
			const auto r = try_parse_digits_with_digits_limit< T >(
					from,
					digits_limit_maker(),
					overflow_controlled_integer_accumulator_t<
							T,
							10,
							check_negative_extremum >{} );
			if( r )
				return static_cast< T >( -(*r) ); // This static_cast is required
					// for clang compiler that warns that if type of *r is `short`,
					// then -(*r) will have type `int`.
			else
				return r;
		}
		else if( '+' == first_symbol )
		{
			return try_parse_digits_with_digits_limit< T >(
					from,
					digits_limit_maker(),
					overflow_controlled_integer_accumulator_t< T, 10 >{} );
		}
		else if( is_digit(first_symbol) )
		{
			from.putback();
			return try_parse_digits_with_digits_limit< T >(
					from,
					digits_limit_maker(),
					overflow_controlled_integer_accumulator_t< T, 10 >{} );
		}

		return make_unexpected( parse_error_t{
				from.current_position(),
				error_reason_t::pattern_not_found
		} );
	}
};

//
// decimal_number_producer_with_digits_limit_t
//
/*!
 * @brief A producer for the case when a signed decimal number is
 * expected in the input stream.
 *
 * This class takes into account a number of digits to be consumed.
 *
 * In the case of success returns the extracted number.
 *
 * @since v.0.6.6
 */
template< typename T >
class decimal_number_producer_with_digits_limit_t
	:	public decimal_number_producer_t< T >
{
	digits_to_consume_t m_digits_limit;

public:
	decimal_number_producer_with_digits_limit_t(
		digits_to_consume_t digits_limit )
		:	m_digits_limit{ digits_limit }
	{}

	RESTINIO_NODISCARD
	auto
	try_parse( source_t & from ) const noexcept
	{
		return this->try_parse_impl(
				from,
				[this]() noexcept { return m_digits_limit; } );
	}
};

//
// any_value_skipper_t
//
/*!
 * @brief A special consumer that simply throws any value away.
 *
 * This consumer is intended to be used in the case when the presence
 * of some value should be checked but the value itself isn't needed.
 *
 * @since v.0.6.1
 */
struct any_value_skipper_t : public consumer_tag
{
	template< typename Target_Type, typename Value >
	void
	consume( Target_Type &, Value && ) const noexcept {}
};

//
// as_result_consumer_t
//
/*!
 * @brief A consumer for the case when the current value should
 * be returned as the result for the producer at one level up.
 *
 * For example that consumer can be necessary for rules like that:
   @verbatim
	T := 'v' '=' token
	@endverbatim
 * such rule will be implemented by a such sequence of clauses:
 * @code
 * produce<std::string>(symbol('v'), symbol('='), token_p() >> as_result());
 * @endcode
 * The result of `token_p()` producer in a subclause should be returned
 * as the result of top-level producer.
 *
 * @since v.0.6.1
 */
struct as_result_consumer_t : public consumer_tag
{
	template< typename Target_Type, typename Value >
	void
	consume( Target_Type & dest, Value && src ) const
	{
		result_wrapper_for_t<Target_Type>::as_result(
				dest, std::forward<Value>(src) );
	}
};

//
// just_result_consumer_t
//
/*!
 * @brief A consumer for the case when a specific value should
 * be used as the result instead of the value produced on
 * the previous step.
 *
 * @since v.0.6.6
 */
template< typename Result_Type >
class just_result_consumer_t : public consumer_tag
{
	Result_Type m_result;

	// NOTE: this helper method is necessary for MSVC++ compiler.
	// It's because MSVC++ can't compile expression:
	//
	// as_result(dest, Result_Type{m_result})
	//
	// in consume() method for trivial types like size_t.
	Result_Type
	make_copy_of_result() const
		noexcept(noexcept(Result_Type{m_result}))
	{
		return m_result;
	}

public :
	template< typename Result_Arg >
	just_result_consumer_t( Result_Arg && result )
		noexcept(noexcept(Result_Type{std::forward<Result_Arg>(result)}))
		:	m_result{ std::forward<Result_Arg>(result) }
	{}

	template< typename Target_Type, typename Value >
	void
	consume( Target_Type & dest, Value && ) const
	{
		result_wrapper_for_t<Target_Type>::as_result(
				dest,
				// NOTE: use a copy of m_result.
				make_copy_of_result() );
	}
};

//
// custom_consumer_t
//
/*!
 * @brief A template for consumers that are released by lambda/functional
 * objects.
 *
 * @tparam C the type of lambda/functional object/function pointer to
 * be used as the actual consumer.
 *
 * @since v.0.6.1
 */
template< typename C >
class custom_consumer_t : public consumer_tag
{
	C m_consumer;

public :
	custom_consumer_t( C && consumer ) : m_consumer{std::move(consumer)} {}

	template< typename Target_Type, typename Value >
	void
	consume( Target_Type & dest, Value && src ) const
		noexcept(noexcept(m_consumer(dest, std::forward<Value>(src))))
	{
		m_consumer( dest, std::forward<Value>(src) );
	}
};

//
// field_setter_consumer_t
//
/*!
 * @brief A template for consumers that store a value to the specified
 * field of a target object.
 *
 * @tparam F type of the target field
 * @tparam C type of the target object.
 *
 * @since v.0.6.1
 */
template< typename F, typename C >
class field_setter_consumer_t : public consumer_tag
{
	using pointer_t = F C::*;

	pointer_t m_ptr;

public :
	field_setter_consumer_t( pointer_t ptr ) noexcept : m_ptr{ptr} {}

	// NOTE: it seems that this method won't be compiled if
	// result_value_wrapper::result_type differs from
	// result_value_wrapper::wrapped_type.
	//
	// This is not a problem for the current version.
	// But this moment would require more attention in the future.
	void
	consume( C & to, F && value ) const
		noexcept(noexcept(to.*m_ptr = std::move(value)))
	{
		to.*m_ptr = std::move(value);
	}
};

/*!
 * @brief A special operator to connect a value producer with
 * field_setter_consumer.
 *
 * @since v.0.6.1
 */
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
// tuple_item_consumer_t
//
/*!
 * @brief A consumer that stores a result value at the specified
 * index in the result tuple.
 *
 * @since v.0.6.6
 */
template< std::size_t Index >
struct tuple_item_consumer_t : public consumer_tag
{
	// NOTE: it seems that this method won't be compiled if
	// result_value_wrapper::result_type differs from
	// result_value_wrapper::wrapped_type.
	//
	// This is not a problem for the current version.
	// But this moment would require more attention in the future.
	template< typename Target_Type, typename Value >
	void
	consume( Target_Type && to, Value && value )
	{
		std::get<Index>(std::forward<Target_Type>(to)) =
				std::forward<Value>(value);
	}
};

//
// to_lower_transformer_t
//
template< typename Input_Type >
struct to_lower_transformer_t;

/*!
 * @brief An implementation of transformer that converts the content
 * of the input std::string to lower case.
 *
 * @since v.0.6.1
 */
template<>
struct to_lower_transformer_t< std::string >
	: public transformer_tag< std::string >
{
	using input_type = std::string;

	RESTINIO_NODISCARD
	result_type
	transform( input_type && input ) const noexcept
	{
		result_type result{ std::move(input) };
		std::transform( result.begin(), result.end(), result.begin(),
			[]( unsigned char ch ) -> char {
				return restinio::impl::to_lower_case(ch);
			} );

		return result;
	}
};

/*!
 * @brief An implementation of transformer that converts the content
 * of the input character to lower case.
 *
 * @since v.0.6.6
 */
template<>
struct to_lower_transformer_t< char >
	: public transformer_tag< char >
{
	using input_type = char;

	RESTINIO_NODISCARD
	result_type
	transform( input_type && input ) const noexcept
	{
		return restinio::impl::to_lower_case(input);
	}
};

/*!
 * @brief An implementation of transformer that converts the content
 * of the input std::array to lower case.
 *
 * @since v.0.6.6
 */
template< std::size_t S >
struct to_lower_transformer_t< std::array< char, S > >
	: public transformer_tag< std::array< char, S > >
{
	using input_type = std::array< char, S >;
	using base_type = transformer_tag< input_type >;

	RESTINIO_NODISCARD
	typename base_type::result_type
	transform( input_type && input ) const noexcept
	{
		typename base_type::result_type result;
		std::transform( input.begin(), input.end(), result.begin(),
			[]( unsigned char ch ) -> char {
				return restinio::impl::to_lower_case(ch);
			} );

		return result;
	}
};

//
// to_lower_transformer_proxy_t
//
/*!
 * @brief A proxy for the creation of an appropriate to_lower_transformer.
 *
 * @since v.0.6.6
 */
struct to_lower_transformer_proxy_t : public transformer_proxy_tag
{
	template< typename Input_Type >
	RESTINIO_NODISCARD
	auto
	make_transformer() const noexcept
	{
		return to_lower_transformer_t< Input_Type >{};
	}
};

//
// just_value_transformer_t
//
/*!
 * @brief A transformer that skips incoming value and returns
 * a value specified by a user.
 *
 * @since v.0.6.6
 */
template< typename T >
class just_value_transformer_t : public transformer_tag< T >
{
	T m_value;

public :
	just_value_transformer_t( T v ) noexcept(noexcept(T{std::move(v)}))
		: m_value{ std::move(v) }
	{}

	template< typename Input >
	RESTINIO_NODISCARD
	T
	transform( Input && ) const noexcept(noexcept(T{m_value}))
	{
		return m_value;
	}
};

//
// convert_transformer_t
//
/*!
 * @brief A transformator that uses a user supplied function/functor
 * for conversion a value from one type to another.
 *
 * @since v.0.6.6
 */
template< typename Output_Type, typename Converter >
class convert_transformer_t : public transformer_tag< Output_Type >
{
	Converter m_converter;

public :
	template< typename Convert_Arg >
	convert_transformer_t( Convert_Arg && converter )
		noexcept(noexcept(Converter{std::forward<Convert_Arg>(converter)}))
		: m_converter{ std::forward<Convert_Arg>(converter) }
	{}

	template< typename Input >
	RESTINIO_NODISCARD
	Output_Type
	transform( Input && input ) const
		noexcept(noexcept(m_converter(std::forward<Input>(input))))
	{
		return m_converter(std::forward<Input>(input));
	}
};

//
// convert_transformer_proxy_t
//
/*!
 * @brief A proxy for the creation of convert_transformer instances
 * for a specific value producers.
 *
 * @note
 * This class is intended to be used in implementation of operator>>
 * for cases like that:
 * @code
 * symbol_p('k') >> convert([](auto ch) { return 1024u; })
 * @endcode
 *
 * @since v.0.6.6
 */
template< typename Converter >
class convert_transformer_proxy_t : public transformer_proxy_tag
{
	Converter m_converter;

public :
	template< typename Convert_Arg >
	convert_transformer_proxy_t( Convert_Arg && converter )
		noexcept(noexcept(Converter{std::forward<Convert_Arg>(converter)}))
		: m_converter{ std::forward<Convert_Arg>(converter) }
	{}

	template< typename Input_Type >
	RESTINIO_NODISCARD
	auto
	make_transformer() const &
		noexcept(noexcept(Converter{m_converter}))
	{
		using output_type = std::decay_t<
				decltype(m_converter(std::declval<Input_Type&&>())) >;

		return convert_transformer_t< output_type, Converter >{ m_converter };
	}

	template< typename Input_Type >
	RESTINIO_NODISCARD
	auto
	make_transformer() &&
		noexcept(noexcept(Converter{std::move(m_converter)}))
	{
		using output_type = std::decay_t<
				decltype(m_converter(std::declval<Input_Type&&>())) >;

		return convert_transformer_t< output_type, Converter >{
				std::move(m_converter)
		};
	}
};

//
// try_parse_exact_fragment
//

// Requires that begin is not equal to end.
template< typename It >
RESTINIO_NODISCARD
expected_t< bool, parse_error_t >
try_parse_exact_fragment( source_t & from, It begin, It end )
{
	assert( begin != end );

	source_t::content_consumer_t consumer{ from };

	for( auto ch = from.getch(); !ch.m_eof; ch = from.getch() )
	{
		if( ch.m_ch != *begin )
			return make_unexpected( parse_error_t{
					consumer.started_at(),
					error_reason_t::pattern_not_found
				} );
		if( ++begin == end )
			break;
	}

	if( begin != end )
		return make_unexpected( parse_error_t{
				consumer.started_at(),
				error_reason_t::unexpected_eof
			} );

	consumer.commit();

	return true;
}

//
// exact_fixed_size_fragment_producer_t
//
/*!
 * @brief A producer that expects a fragment in the input and
 * produces boolean value if that fragment is found.
 *
 * This class is indended for working with fixed-size string literals
 * with terminating null-symbol.
 *
 * @since v.0.6.6
 */
template< std::size_t Size >
class exact_fixed_size_fragment_producer_t
	:	public producer_tag< bool >
{
	static_assert( 1u < Size, "Size is expected to greater that 1" );

	// NOTE: there is no space for last zero-byte.
	std::array< char, Size-1u > m_fragment;

public:
	exact_fixed_size_fragment_producer_t( const char (&f)[Size] )
	{
		// NOTE: last zero-byte is discarded.
		std::copy( &f[ 0 ], &f[ m_fragment.size() ], m_fragment.data() );
	}

	RESTINIO_NODISCARD
	expected_t< bool, parse_error_t >
	try_parse( source_t & from )
	{
		return try_parse_exact_fragment( from,
				m_fragment.begin(), m_fragment.end() );
	}
};

//
// exact_fragment_producer_t
//
/*!
 * @brief A producer that expects a fragment in the input and
 * produces boolean value if that fragment is found.
 *
 * @since v.0.6.6
 */
class exact_fragment_producer_t
	:	public producer_tag< bool >
{
	std::string m_fragment;

public:
	exact_fragment_producer_t( std::string fragment )
		:	m_fragment{ std::move(fragment) }
	{
		if( m_fragment.empty() )
			throw exception_t( "'fragment' value for exact_fragment_producer_t "
					"can't be empty!" );
	}

	RESTINIO_NODISCARD
	expected_t< bool, parse_error_t >
	try_parse( source_t & from )
	{
		return try_parse_exact_fragment( from,
				m_fragment.begin(), m_fragment.end() );
	}
};

} /* namespace impl */

//
// produce
//
/*!
 * @brief A factory function to create a producer that creates an
 * instance of the target type by using specified clauses.
 *
 * Usage example:
 * @code
 * produce<std::string>(symbol('v'), symbol('='), token_p() >> as_result());
 * @endcode
 *
 * @tparam Target_Type the type of value to be produced.
 * @tparam Clauses the list of clauses to be used for a new value.
 *
 * @since v.0.6.1
 */
template<
	typename Target_Type,
	typename... Clauses >
RESTINIO_NODISCARD
auto
produce( Clauses &&... clauses )
{
	static_assert( 0 != sizeof...(clauses),
			"list of clauses can't be empty" );
	static_assert( meta::all_of_v< impl::is_clause, Clauses... >,
			"all arguments for produce() should be clauses" );

	using producer_type_t = impl::produce_t<
			Target_Type,
			impl::tuple_of_entities_t<Clauses...> >;

	return producer_type_t{
			std::make_tuple(std::forward<Clauses>(clauses)...)
	};
}

//
// alternatives
//
/*!
 * @brief A factory function to create an alternatives clause.
 *
 * Usage example:
 * @code
 * produce<std::string>(
 * 	alternatives(
 * 		sequence(symbol('v'), symbol('='), token_p() >> as_result()),
 * 		sequence(symbol('T'), symbol('/'), token_p() >> as_result())
 * 	)
 * );
 * @endcode
 * Please note the usage of sequence() inside the call to
 * alternatives().
 *
 * @tparam Clauses the list of clauses to be used as alternatives.
 *
 * @since v.0.6.1
 */
template< typename... Clauses >
RESTINIO_NODISCARD
auto
alternatives( Clauses &&... clauses )
{
	static_assert( 0 != sizeof...(clauses),
			"list of clauses can't be empty" );
	static_assert( meta::all_of_v< impl::is_clause, Clauses... >,
			"all arguments for alternatives() should be clauses" );

	using clause_type_t = impl::alternatives_clause_t<
			impl::tuple_of_entities_t< Clauses... > >;

	return clause_type_t{
			std::make_tuple(std::forward<Clauses>(clauses)...)
	};
}

//
// maybe
//
/*!
 * @brief A factory function to create an optional clause.
 *
 * Usage example:
 * @code
 * produce<std::pair<std::string, std::string>>(
 * 	token_p() >> &std::pair<std::string, std::string>::first,
 * 	maybe(
 * 		symbol('='),
 * 		token_p() >> &std::pair<std::string, std::string>::second
 * 	)
 * );
 * @endcode
 *
 * @tparam Clauses the list of clauses to be used as optional sequence.
 *
 * @since v.0.6.1
 */
template< typename... Clauses >
RESTINIO_NODISCARD
auto
maybe( Clauses &&... clauses )
{
	static_assert( 0 != sizeof...(clauses),
			"list of clauses can't be empty" );
	static_assert( meta::all_of_v< impl::is_clause, Clauses... >,
			"all arguments for maybe() should be clauses" );

	using clause_type_t = impl::maybe_clause_t<
			impl::tuple_of_entities_t<Clauses...> >;

	return clause_type_t{
			std::make_tuple(std::forward<Clauses>(clauses)...)
	};
}

//
// not_clause
//
/*!
 * @brief A factory function to create a not_clause.
 *
 * Usage example:
 * @code
 * produce<std::pair<std::string, std::string>>(
 * 	token_p() >> &std::pair<std::string, std::string>::first,
 * 	symbol(' '),
 * 	token_p() >> &std::pair<std::string, std::string>::second
 * 	not_clause(symbol('.'))
 * );
 * @endcode
 * this expression corresponds the following rule:
   @verbatim
   T := token SP token !'.'
	@endverbatim
 *
 * @tparam Clauses the list of clauses to be used as sequence to be checked.
 *
 * @since v.0.6.1
 */
template< typename... Clauses >
RESTINIO_NODISCARD
auto
not_clause( Clauses &&... clauses )
{
	static_assert( 0 != sizeof...(clauses),
			"list of clauses can't be empty" );
	static_assert( meta::all_of_v< impl::is_clause, Clauses... >,
			"all arguments for not_clause() should be clauses" );

	using clause_type_t = impl::not_clause_t<
			impl::tuple_of_entities_t<Clauses...> >;

	return clause_type_t{
			std::make_tuple(std::forward<Clauses>(clauses)...)
	};
}

//
// and_clause
//
/*!
 * @brief A factory function to create an and_clause.
 *
 * Usage example:
 * @code
 * produce<std::pair<std::string, std::string>>(
 * 	token_p() >> &std::pair<std::string, std::string>::first,
 * 	symbol(' '),
 * 	token_p() >> &std::pair<std::string, std::string>::second
 * 	and_clause(symbol(','), maybe(symbol(' ')), token_p() >> skip())
 * );
 * @endcode
 * this expression corresponds the following rule:
   @verbatim
   T := token SP token &(',' [' '] token)
	@endverbatim
 *
 * @tparam Clauses the list of clauses to be used as sequence to be checked.
 *
 * @since v.0.6.1
 */
template< typename... Clauses >
RESTINIO_NODISCARD
auto
and_clause( Clauses &&... clauses )
{
	static_assert( 0 != sizeof...(clauses),
			"list of clauses can't be empty" );
	static_assert( meta::all_of_v< impl::is_clause, Clauses... >,
			"all arguments for sequence() should be clauses" );

	using clause_type_t = impl::and_clause_t<
			impl::tuple_of_entities_t<Clauses...> >;

	return clause_type_t{
			std::make_tuple(std::forward<Clauses>(clauses)...)
	};
}

//
// sequence
//
/*!
 * @brief A factory function to create a sequence of subclauses
 *
 * Usage example:
 * @code
 * produce<std::string>(
 * 	alternatives(
 * 		sequence(symbol('v'), symbol('='), token_p() >> as_result()),
 * 		sequence(symbol('T'), symbol('/'), token_p() >> as_result())
 * 	)
 * );
 * @endcode
 * Please note the usage of sequence() inside the call to
 * alternatives().
 *
 * @tparam Clauses the list of clauses to be used as the sequence.
 *
 * @since v.0.6.1
 */
template< typename... Clauses >
RESTINIO_NODISCARD
auto
sequence( Clauses &&... clauses )
{
	static_assert( 0 != sizeof...(clauses),
			"list of clauses can't be empty" );
	static_assert( meta::all_of_v< impl::is_clause, Clauses... >,
			"all arguments for sequence() should be clauses" );

	using clause_type_t = impl::sequence_clause_t<
			impl::tuple_of_entities_t< Clauses... > >;

	return clause_type_t{
			std::make_tuple(std::forward<Clauses>(clauses)...)
	};
}

//
// force_only_this_alternative
//
/*!
 * @brief An alternative that should be parsed correctly or the parsing
 * of the whole alternatives clause should fail.
 *
 * This special clause is intended to be used to avoid mistakes in
 * grammars like that:
@verbatim
v = "key" '=' token
  | token '=' 1*VCHAR
@endverbatim
 * If that grammar will be used for parsing a sentence like "key=123" then
 * the second alternative will be selected. It's because the parsing
 * of rule <tt>"key" '=' token</tt> fails at `123` and the second alternative
 * will be tried. And "key" will be recognized as a token.
 *
 * Before v.0.6.7 this mistake can be avoided by using rules like those:
@verbatim
v = "key" '=' token
  | !"key" token '=' 1*VCHAR
@endverbatim
 *
 * Since v.0.6.7 this mistake can be avoided by using
 * force_only_this_alternative() function:
 * @code
 * alternatives(
 * 	sequence(
 * 		exact("key"),
 * 		force_only_this_alternative(
 * 			symbol('='),
 * 			token() >> skip()
 * 		)
 * 	),
 * 	sequence(
 * 		token() >> skip(),
 * 		symbol('='),
 * 		repeat(1, N, vchar_symbol_p() >> skip())
 * 	)
 * );
 * @endcode
 *
 * @since v.0.6.7
 */
template< typename... Clauses >
RESTINIO_NODISCARD
auto
force_only_this_alternative( Clauses &&... clauses )
{
	static_assert( 0 != sizeof...(clauses),
			"list of clauses can't be empty" );
	static_assert( meta::all_of_v< impl::is_clause, Clauses... >,
			"all arguments for force_only_this_alternative() should "
			"be clauses" );

	using clause_type_t = impl::forced_alternative_clause_t<
			impl::tuple_of_entities_t< Clauses... > >;

	return clause_type_t{
			std::make_tuple(std::forward<Clauses>(clauses)...)
	};
}

//
// repeat
//
/*!
 * @brief A factory function to create repetitor of subclauses.
 *
 * Usage example:
 * @code
 * using str_pair = std::pair<std::string, std::string>;
 * produce<std::vector<str_pair>>(
 * 	produce<str_pair>(
 * 		token_p() >> &str_pair::first,
 * 		symbol('='),
 * 		token_p() >> &str_pair::second
 * 	) >> to_container(),
 * 	repeat(0, N,
 * 		symbol(','),
 * 		produce<str_pair>(
 * 			token_p() >> &str_pair::first,
 * 			symbol('='),
 * 			token_p() >> &str_pair::second
 * 		) >> to_container()
 * 	)
 * );
 * @endcode
 * this expression corresponds to the following rule:
   @verbatim
	T := token '=' token *(',' token '=' token)
   @endverbatim
 *
 * @tparam Clauses the list of clauses to be used as the sequence
 * to be repeated.
 *
 * @since v.0.6.1
 */
template<
	typename... Clauses >
RESTINIO_NODISCARD
auto
repeat(
	//! Minimal occurences of the sequences in the repetition.
	std::size_t min_occurences,
	//! Maximal occurences of the sequences in the repetition.
	/*!
	 * @note
	 * The repetition will be stopped when that numer of repetitions
	 * will be reached.
	 */
	std::size_t max_occurences,
	//! The sequence of clauses to be repeated.
	Clauses &&... clauses )
{
	static_assert( 0 != sizeof...(clauses),
			"list of clauses can't be empty" );
	static_assert( meta::all_of_v< impl::is_clause, Clauses... >,
			"all arguments for repeat() should be clauses" );

	using producer_type_t = impl::repeat_clause_t<
			impl::tuple_of_entities_t<Clauses...> >;

	return producer_type_t{
			min_occurences,
			max_occurences,
			std::make_tuple(std::forward<Clauses>(clauses)...)
	};
}

//
// skip
//
/*!
 * @brief A factory function to create a skip_consumer.
 *
 * Usage example:
 * @code
 * produce<std::string>(
 * 	token_p() >> as_result(),
 * 	not_clause(symbol('='), token_p() >> skip())
 * );
 * @endcode
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline auto
skip() noexcept { return impl::any_value_skipper_t{}; }

//
// any_symbol_p
//
/*!
 * @brief A factory function to create an any_symbol_producer.
 *
 * @return a producer that expects any symbol in the input stream
 * and returns it.
 * 
 * @since v.0.6.6
 */
RESTINIO_NODISCARD
inline auto
any_symbol_p() noexcept
{
	return impl::symbol_producer_template_t<impl::any_symbol_predicate_t>{};
}

//
// symbol_p
//
/*!
 * @brief A factory function to create a symbol_producer.
 *
 * @return a producer that expects @a expected in the input stream
 * and returns it if that character is found.
 * 
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline auto
symbol_p( char expected ) noexcept
{
	return impl::symbol_producer_t{expected};
}

//
// any_symbol_if_not_p
//
/*!
 * @brief A factory function to create a any_symbol_if_not_producer.
 *
 * @return a producer that expects any character except @a sentinel in the
 * input stream and returns it if that character is found.
 * 
 * @since v.0.6.6
 */
RESTINIO_NODISCARD
inline auto
any_symbol_if_not_p( char sentinel ) noexcept
{
	return impl::any_symbol_if_not_producer_t{sentinel};
}

//
// caseless_symbol_p
//
/*!
 * @brief A factory function to create a caseless_symbol_producer.
 *
 * This producer performs caseless comparison of characters.
 *
 * @return a producer that expects @a expected in the input stream
 * and returns it if that character is found.
 * 
 * @since v.0.6.6
 */
RESTINIO_NODISCARD
inline auto
caseless_symbol_p( char expected ) noexcept
{
	return impl::caseless_symbol_producer_t{expected};
}

//
// symbol
//
/*!
 * @brief A factory function to create a clause that expects the
 * speficied symbol, extracts it and then skips it.
 *
 * The call to `symbol('a')` function is an equivalent of:
 * @code
 * symbol_p('a') >> skip()
 * @endcode
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline auto
symbol( char expected ) noexcept
{
	return symbol_p(expected) >> skip();
}

//
// caseless_symbol
//
/*!
 * @brief A factory function to create a clause that expects the
 * speficied symbol, extracts it and then skips it.
 *
 * This clause performs caseless comparison of characters.
 *
 * The call to `caseless_symbol('a')` function is an equivalent of:
 * @code
 * caseless_symbol_p('a') >> skip()
 * @endcode
 *
 * @since v.0.6.6
 */
RESTINIO_NODISCARD
inline auto
caseless_symbol( char expected ) noexcept
{
	return caseless_symbol_p(expected) >> skip();
}

//
// space_p
//
/*!
 * @brief A factory function to create a space_producer.
 *
 * @return a producer that expects space character in the input stream
 * and returns it if that character is found.
 * 
 * @since v.0.6.4
 */
RESTINIO_NODISCARD
inline auto
space_p() noexcept
{
	return impl::symbol_producer_template_t< impl::is_space_predicate_t >{};
}

//
// space
//
/*!
 * @brief A factory function to create a clause that expects a space,
 * extracts it and then skips it.
 *
 * The call to `space()` function is an equivalent of:
 * @code
 * space_p() >> skip()
 * @endcode
 *
 * @since v.0.6.4
 */
RESTINIO_NODISCARD
inline auto
space() noexcept
{
	return space_p() >> skip();
}

//
// digit_p
//
/*!
 * @brief A factory function to create a digit_producer.
 *
 * @return a producer that expects a decimal digit in the input stream
 * and returns it if a decimal digit is found.
 * 
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline auto
digit_p() noexcept
{
	return impl::digit_producer_t{};
}

//
// digit
//
/*!
 * @brief A factory function to create a clause that expects a decimal digit,
 * extracts it and then skips it.
 *
 * The call to `digit()` function is an equivalent of:
 * @code
 * digit_p() >> skip()
 * @endcode
 *
 * @since v.0.6.6
 */
RESTINIO_NODISCARD
inline auto
digit() noexcept
{
	return digit_p() >> skip();
}

//
// hexdigit_p
//
/*!
 * @brief A factory function to create a hexdigit_producer.
 *
 * @return a producer that expects a hexadecimal digit in the input stream
 * and returns it if a hexadecimal digit is found.
 * 
 * @since v.0.6.6
 */
RESTINIO_NODISCARD
inline auto
hexdigit_p() noexcept
{
	return impl::hexdigit_producer_t{};
}

//
// hexdigit
//
/*!
 * @brief A factory function to create a clause that expects a hexadecimal
 * digit, extracts it and then skips it.
 *
 * The call to `hexdigit()` function is an equivalent of:
 * @code
 * hexdigit_p() >> skip()
 * @endcode
 *
 * @since v.0.6.6
 */
RESTINIO_NODISCARD
inline auto
hexdigit() noexcept
{
	return hexdigit_p() >> skip();
}

//
// non_negative_decimal_number_p
//
/*!
 * @brief A factory function to create a non_negative_decimal_number_producer.
 *
 * @note
 * This parser consumes all digits until the first non-digit symbol will
 * be found in the input. It means that in the case of `1111someword` the
 * first four digits (e.g. `1111`) will be extracted from the input and
 * the remaining part (e.g. `someword`) won't be consumed by this parser.
 *
 * @return a producer that expects a positive decimal number in the input stream
 * and returns it if a number is found.
 * 
 * @since v.0.6.2
 */
template< typename T >
RESTINIO_NODISCARD
inline auto
non_negative_decimal_number_p() noexcept
{
	return impl::non_negative_decimal_number_producer_t<T>{};
}

//
// non_negative_decimal_number_p
//
/*!
 * @brief A factory function to create a non_negative_decimal_number_producer.
 *
 * @note
 * This parser consumes a number of digits with respect to @a digits_limit.
 *
 * Usage example:
 * @code
 * using namespace restinio::easy_parser;
 *
 * struct compound_number {
 * 	short prefix_;
 * 	int suffix_;
 * };
 *
 * auto parse = produce<compound_number>(
 * 	non_negative_decimal_number_p<short>(expected_digits(2, 5))
 * 		>> &compound_number::prefix_,
 * 	non_negative_decimal_number_p<int>(expected_digits(7, 12))
 * 		>> &compound_number::suffix_
 * );
 * @endcode
 *
 * @return a producer that expects a positive decimal number in the input stream
 * and returns it if a number is found.
 * 
 * @since v.0.6.2
 */
template< typename T >
RESTINIO_NODISCARD
inline auto
non_negative_decimal_number_p( digits_to_consume_t digits_limit ) noexcept
{
	return impl::non_negative_decimal_number_producer_with_digits_limit_t<T>{
			digits_limit
	};
}

//FIXME: remove in v.0.7.0!
//
// positive_decimal_number_p
//
/*!
 * @brief A factory function to create a producer for non-negative
 * decimal numbers.
 *
 * @deprecated Use non_negative_decimal_number_p.
 *
 * @since v.0.6.2
 */
template< typename T >
[[deprecated]] RESTINIO_NODISCARD
inline auto
positive_decimal_number_producer() noexcept
{
	return non_negative_decimal_number_p<T>();
}

//
// hexadecimal_number_p
//
/*!
 * @brief A factory function to create a hexadecimal_number_producer.
 *
 * @note
 * This parser consumes all digits until the first non-digit symbol will
 * be found in the input. It means that in the case of `1111someword` the
 * first four digits (e.g. `1111`) will be extracted from the input and
 * the remaining part (e.g. `someword`) won't be consumed by this parser.
 *
 * @attention
 * T should be an unsigned type.
 *
 * @return a producer that expects a positive hexadecimal number in the input
 * stream and returns it if a number is found.
 * 
 * @since v.0.6.6
 */
template< typename T >
RESTINIO_NODISCARD
inline auto
hexadecimal_number_p() noexcept
{
	return impl::hexadecimal_number_producer_t<T>{};
}

//
// hexadecimal_number_p
//
/*!
 * @brief A factory function to create a hexadecimal_number_producer.
 *
 * @note
 * This parser consumes a number of digits with respect to @a digits_limit.
 *
 * Usage example:
 * @code
 * using namespace restinio::easy_parser;
 *
 * struct compound_number {
 * 	short prefix_;
 * 	int suffix_;
 * };
 *
 * auto parse = produce<compound_number>(
 * 	hexadecimal_number_p<short>(expected_digits(4))
 * 		>> &compound_number::prefix_,
 * 	hexadecimal_number_p<int>(expected_digits(7, 12))
 * 		>> &compound_number::suffix_
 * );
 * @endcode
 *
 * @attention
 * T should be an unsigned type.
 *
 * @return a producer that expects a positive hexadecimal number in the input
 * stream and returns it if a number is found.
 * 
 * @since v.0.6.6
 */
template< typename T >
RESTINIO_NODISCARD
inline auto
hexadecimal_number_p( digits_to_consume_t digits_limit ) noexcept
{
	return impl::hexadecimal_number_producer_with_digits_limit_t<T>{
			digits_limit
	};
}

//
// decimal_number_p
//
/*!
 * @brief A factory function to create a decimal_number_producer.
 *
 * Parses numbers in the form:
@verbatim
number = [sign] DIGIT+
sign = '-' | '+'
@endverbatim
 *
 * @note
 * This parser consumes all digits until the first non-digit symbol will be
 * found in the input. It means that in the case of `-1111someword` the leading
 * minus sign and thefirst four digits (e.g. `-1111`) will be extracted from
 * the input and the remaining part (e.g. `someword`) won't be consumed by this
 * parser.
 *
 * @attention
 * Can be used only for singed number types (e.g. short, int, long,
 * std::int32_t and so on).
 *
 * @return a producer that expects a decimal number in the input stream
 * and returns it if a number is found.
 * 
 * @since v.0.6.6
 */
template< typename T >
RESTINIO_NODISCARD
inline auto
decimal_number_p() noexcept
{
	static_assert( std::is_signed<T>::value,
			"decimal_number_p() can be used only for signed numeric types" );

	return impl::decimal_number_producer_t<T>{};
}

//
// decimal_number_p
//
/*!
 * @brief A factory function to create a decimal_number_producer.
 *
 * Parses numbers in the form:
@verbatim
number = [sign] DIGIT+
sign = '-' | '+'
@endverbatim
 *
 * @note
 * This parser consumes a number of digits with respect to @a digits_limit.
 * The leading sign (if present) is not added to a number of extracted digits.
 *
 * Usage example:
 * @code
 * using namespace restinio::easy_parser;
 *
 * struct compound_number {
 * 	short prefix_;
 * 	int suffix_;
 * };
 *
 * auto parse = produce<compound_number>(
 * 	decimal_number_p<short>(expected_digits(4))
 * 		>> &compound_number::prefix_,
 * 	decimal_number_p<int>(expected_digits(7, 12))
 * 		>> &compound_number::suffix_
 * );
 * @endcode
 *
 * @attention
 * Can be used only for singed number types (e.g. short, int, long,
 * std::int32_t and so on).
 *
 * @return a producer that expects a decimal number in the input stream
 * and returns it if a number is found.
 * 
 * @since v.0.6.6
 */
template< typename T >
RESTINIO_NODISCARD
inline auto
decimal_number_p( digits_to_consume_t digits_limit ) noexcept
{
	static_assert( std::is_signed<T>::value,
			"decimal_number_p() can be used only for signed numeric types" );

	return impl::decimal_number_producer_with_digits_limit_t<T>{
			digits_limit
	};
}

//
// as_result
//
/*!
 * @brief A factory function to create a as_result_consumer.
 *
 * Usage example:
 * @code
 * produce<std::string>(
 * 	symbol('v'),
 * 	symbol('='),
 * 	token_p() >> as_result(),
 * 	symbol('.')
 * );
 * @endcode
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline auto
as_result() noexcept { return impl::as_result_consumer_t{}; }

//
// custom_consumer
//
/*!
 * @brief A factory function to create a custom_consumer.
 *
 * Usage example:
 * @code
 * class composed_value {
 * 	std::string name_;
 * 	std::string value_;
 * public:
 * 	composed_value() = default;
 *
 * 	void set_name(std::string name) { name_ = std::move(name); }
 * 	void set_value(std::string value) { value_ = std::move(value); }
 * 	...
 * };
 * produce<composed_value>(
 * 	token_p() >> custom_consumer(
 * 		[](composed_value & to, std::string && what) {
 * 			to.set_name(std::move(what));
 * 		} ),
 * 	symbol('='),
 * 	token_p() >> custom_consumer(
 * 		[](composed_value & to, std::string && what) {
 * 			to.set_value(std::move(what));
 * 		} ),
 * 	symbol('.')
 * );
 * @endcode
 *
 * @note
 * A custom consumer should be a function/lambda/function objects with
 * the following prototype:
 * @code
 * void(Target_Type & destination, Value && value);
 * @endcode
 *
 * @since v.0.6.1
 */
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
/*!
 * @brief A template for a consumer that stories values into a container.
 *
 * Instances of such consumer will be used inside repeat_clause_t.
 *
 * @tparam Container_Adaptor a class that knows how to store a value
 * into the target container. See result_value_wrapper for the
 * requirement for such type.
 *
 * @since v.0.6.1
 */
struct to_container_consumer_t : public consumer_tag
{
	template< typename Container, typename Item >
	void
	consume( Container & to, Item && item )
	{
		using container_adaptor_type = result_wrapper_for_t<Container>;
		container_adaptor_type::to_container( to, std::move(item) );
	}
};

} /* namespace impl */

//
// to_container
//
/*!
 * @brief A factory function to create a to_container_consumer.
 *
 * Usage example:
 * @code
 * using str_pair = std::pair<std::string, std::string>;
 * produce<std::vector<str_pair>>(
 * 	produce<str_pair>(
 * 		token_p() >> &str_pair::first,
 * 		symbol('='),
 * 		token_p() >> &str_pair::second
 * 	) >> to_container(),
 * 	repeat(0, N,
 * 		symbol(','),
 * 		produce<str_pair>(
 * 			token_p() >> &str_pair::first,
 * 			symbol('='),
 * 			token_p() >> &str_pair::second
 * 		) >> to_container()
 * 	)
 * );
 * @endcode
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline auto
to_container()
{
	return impl::to_container_consumer_t();
}

//
// to_lower
//
/*!
 * @brief A factory function to create a to_lower_transformer.
 *
 * Usage example:
 * @code
 * produce<std::string>(
 * 	symbol('T'), symbol(':'),
 * 	token_p() >> to_lower() >> as_result()
 * );
 * ...
 * // Since v.0.6.6 to_lower can also be used for a single character.
 * produce<char>(
 * 	exact("I="), any_symbol_p() >> to_lower() >> as_result() );
 * @endcode
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline auto
to_lower() noexcept { return impl::to_lower_transformer_proxy_t{}; }

//
// just
//
/*!
 * @brief A special transformer that replaces the produced value by
 * a value specified by a user.
 *
 * Usage example:
 * @code
 * produce<unsigned int>(
 * 	alternatives(
 * 		symbol('b') >> just(1u) >> as_result(),
 * 		symbol('k') >> just(1024u) >> as_result(),
 * 		symbol('m') >> just(1024u*1024u) >> as_result()
 * 	)
 * );
 * @endcode
 *
 * @since v.0.6.6
 */
template< typename T >
RESTINIO_NODISCARD
auto
just( T value ) noexcept(noexcept(impl::just_value_transformer_t<T>{value}))
{
	return impl::just_value_transformer_t<T>{value};
}

//
// just_result
//
/*!
 * @brief A special consumer that replaces the produced value by
 * a value specified by a user and sets that user-specified value
 * as the result.
 *
 * Usage example:
 * @code
 * produce<unsigned int>(
 * 	alternatives(
 * 		symbol('b') >> just_result(1u),
 * 		symbol('k') >> just_result(1024u),
 * 		symbol('m') >> just_result(1024u*1024u)
 * 	)
 * );
 * @endcode
 *
 * @since v.0.6.6
 */
template< typename T >
RESTINIO_NODISCARD
auto
just_result( T value )
	noexcept(noexcept(impl::just_result_consumer_t<T>{value}))
{
	return impl::just_result_consumer_t<T>{value};
}

//
// convert
//
/*!
 * @brief A factory function to create convert_transformer.
 *
 * Usage example:
 * @code
 * struct tmp_size { std::uint32_t c_{1u}; std::uint32_t m_{1u}; };
 * auto size_producer = produce<std::uint64_t>(
 * 	produce<tmp_size>(
 * 		non_negative_decimal_number_p<std::uint32_t>()
 * 				>> &tmp_size::c_,
 * 		maybe(
 * 			produce<std::uint32_t>(
 * 				alternatives(
 * 					caseless_symbol_p('b') >> just_result(1u),
 * 					caseless_symbol_p('k') >> just_result(1024u),
 * 					caseless_symbol_p('m') >> just_result(1024u*1024u)
 * 				)
 * 			) >> &tmp_size::m_
 * 		)
 * 	) >> convert( [](const tmp_size & ts) {
 * 				return std::uint64_t{ts.c_} * ts.m_;
 * 			} )
 * 		>> as_result()
 * );
 * @endcode
 *
 * @since v.0.6.6
 */
template< typename Converter >
RESTINIO_NODISCARD
auto
convert( Converter && converter )
{
	using converter_type = std::decay_t<Converter>;

	using transformer_proxy_type = impl::convert_transformer_proxy_t<
			converter_type >;

	return transformer_proxy_type{ std::forward<Converter>(converter) };
}

//
// exact_p
//
/*!
 * @brief A factory function that creates an instance of
 * exact_fragment_producer.
 *
 * Usage example:
 * @code
 * produce<std::string>(
 * 	alternatives(
 * 		exact_p("pro") >> just_result("Professional"),
 * 		exact_p("con") >> just_result("Consumer")
 * 	)
 * );
 * @endcode
 *
 * @since v.0.6.6
 */
RESTINIO_NODISCARD
inline auto
exact_p( string_view_t fragment )
{
	return impl::exact_fragment_producer_t{
			std::string{ fragment.data(), fragment.size() }
		};
}

/*!
 * @brief A factory function that creates an instance of
 * exact_fragment_producer.
 *
 * Usage example:
 * @code
 * produce<std::string>(
 * 	alternatives(
 * 		exact_p("pro") >> just_result("Professional"),
 * 		exact_p("con") >> just_result("Consumer")
 * 	)
 * );
 * @endcode
 *
 * @attention
 * This version is dedicated to be used with string literals.
 * Because of that the last byte from a literal will be ignored (it's
 * assumed that this byte contains zero).
 * But this behavior would lead to unexpected results in such cases:
 * @code
 * const char prefix[]{ 'h', 'e', 'l', 'l', 'o' };
 *
 * produce<std::string>(exact_p(prefix) >> just_result("Hi!"));
 * @endcode
 * because the last byte with value 'o' will be ignored by
 * exact_producer. To avoid such behavior string_view_t should be
 * used explicitely:
 * @code
 * produce<std::string>(exact_p(string_view_t{prefix})
 * 		>> just_result("Hi!"));
 * @endcode
 *
 * @since v.0.6.6
 */
template< std::size_t Size >
RESTINIO_NODISCARD
auto
exact_p( const char (&fragment)[Size] )
{
	return impl::exact_fixed_size_fragment_producer_t<Size>{ fragment };
}

//
// exact
//
/*!
 * @brief A factory function that creates an instance of
 * exact_fragment clause.
 *
 * Usage example:
 * @code
 * produce<std::string>(exact("version="), token() >> as_result());
 * @endcode
 *
 * @since v.0.6.6
 */
RESTINIO_NODISCARD
inline auto
exact( string_view_t fragment )
{
	return impl::exact_fragment_producer_t{
			std::string{ fragment.data(), fragment.size() }
		} >> skip();
}

/*!
 * @brief A factory function that creates an instance of
 * exact_fragment clause.
 *
 * Usage example:
 * @code
 * produce<std::string>(exact("version="), token() >> as_result());
 * @endcode
 *
 * @attention
 * This version is dedicated to be used with string literals.
 * Because of that the last byte from a literal will be ignored (it's
 * assumed that this byte contains zero).
 * But this behavior would lead to unexpected results in such cases:
 * @code
 * const char prefix[]{ 'v', 'e', 'r', 's', 'i', 'o', 'n', '=' };
 *
 * produce<std::string>(exact(prefix), token() >> as_result());
 * @endcode
 * because the last byte with value '=' will be ignored by
 * exact_producer. To avoid such behavior string_view_t should be
 * used explicitely:
 * @code
 * produce<std::string>(exact(string_view_t{prefix}),	token() >> as_result());
 * @endcode
 * 
 * @since v.0.6.6
 */
template< std::size_t Size >
RESTINIO_NODISCARD
auto
exact( const char (&fragment)[Size] )
{
	return impl::exact_fixed_size_fragment_producer_t<Size>{ fragment } >> skip();
}

//
// try_parse
//
/*!
 * @brief Perform the parsing of the specified content by using
 * specified value producer.
 *
 * @note
 * The whole content of @a from should be consumed. There can be
 * whitespace remaining in @from after the return from
 * Producer::try_parser. But if there will be some non-whitespace
 * symbol the failure will be reported.
 * (As a side note: since v.0.6.7 all trailing spaces are removed
 * from @from before the parsing starts.)
 *
 * Usage example
 * @code
 * const auto tokens = try_parse(
 * 	"first,Second;Third;Four",
 * 	produce<std::vector<std::string>>(
 * 		token_p() >> to_lower() >> to_container(),
 * 		repeat( 0, N,
 * 			alternatives(symbol(','), symbol(';')),
 * 			token_p() >> to_lower() >> to_container()
 * 		)
 * 	)
 * );
 * if(tokens)
 * 	for(const auto & v: *tokens)
 * 		std::cout << v << std::endl;
 * @endcode
 *
 * @since v.0.6.1
 */
template< typename Producer >
RESTINIO_NODISCARD
expected_t< typename Producer::result_type, parse_error_t >
try_parse(
	string_view_t from,
	Producer producer )
{
	static_assert( impl::is_producer_v<Producer>,
			"Producer should be a value producer type" );

	from = impl::remove_trailing_spaces( from );
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
/*!
 * @brief Make textual description of error returned by try_parse function.
 *
 * @note
 * The format of textual description is not specified and can be changed
 * in some future versions without notice.
 *
 * Usage example:
 * @code
 * const char * content = "first,Second;Third;Four";
 * const auto tokens = try_parse(
 * 	content,
 * 	produce<std::vector<std::string>>(
 * 		token_p() >> to_lower() >> to_container(),
 * 		repeat( 0, N,
 * 			alternatives(symbol(','), symbol(';')),
 * 			token_p() >> to_lower() >> to_container()
 * 		)
 * 	)
 * );
 * if(tokens)
 * {
 * 	for(const auto & v: *tokens)
 * 		std::cout << v << std::endl;
 * }
 * else
 * 	std::cerr << make_error_description(tokens.error(), content) << std::endl;
 * @endcode
 *
 * @since v.0.6.1
 */
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

	const auto basic_reaction = [&](const char * msg) {
		result += msg;
		result += " at ";
		result += std::to_string( error.position() );
		result += ": ";
		append_quote( result );
	};

	switch( error.reason() )
	{
		case error_reason_t::unexpected_character:
			basic_reaction( "unexpected character" );
		break;

		case error_reason_t::unexpected_eof:
			result += "unexpected EOF at ";
			result += std::to_string( error.position() );
		break;

		case error_reason_t::no_appropriate_alternative:
			basic_reaction( "appropriate alternative can't found" );
		break;

		case error_reason_t::pattern_not_found:
			basic_reaction( "expected pattern is not found" );
		break;

		case error_reason_t::unconsumed_input:
			basic_reaction( "unconsumed input found" );
		break;

		case error_reason_t::illegal_value_found:
			basic_reaction( "some illegal value found" );
		break;

		case error_reason_t::force_only_this_alternative_failed:
			basic_reaction( "forced selection alternative failed" );
		break;
	}

	return result;
}

} /* namespace easy_parser */

} /* namespace restinio */

