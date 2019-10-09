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

#include <restinio/string_view.hpp>
#include <restinio/compiler_features.hpp>

#include <iostream>

namespace restinio
{

namespace http_field_parser
{

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

//
// is_space
//
RESTINIO_NODISCARD
inline constexpr bool
is_space( const char ch ) noexcept { return ch == ' ' || ch == '\x09'; }

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

//
// try_parse_impl
//
template< typename Final_Value >
RESTINIO_NODISCARD
bool
try_parse_impl( source_t & /*from*/, Final_Value & /*final_value*/ )
{
	return true;
}

template< typename Final_Value, typename H, typename ...Tail >
RESTINIO_NODISCARD
bool
try_parse_impl(
	source_t & from,
	Final_Value & final_value,
	H && what,
	Tail && ...tail )
{
	if( what.try_parse( from, final_value ) )
	{
		return try_parse_impl( from, final_value, std::forward<Tail>(tail)... );
	}
	return false;
}

namespace rfc
{

class ows_t
{
public :
	template< typename Final_Value >
	RESTINIO_NODISCARD
	bool
	try_parse(
		source_t & from, Final_Value & /*to*/ ) const noexcept
	{
		for( auto ch = from.getch();
			!ch.m_eof && is_space(ch.m_ch);
			ch = from.getch() )
		{}

		from.putback();

		return true;
	}
};

class delimiter_t
{
	char m_delimiter;

public :
	delimiter_t( char delimiter ) : m_delimiter{delimiter} {}

	template< typename Final_Value >
	RESTINIO_NODISCARD
	bool
	try_parse(
		source_t & from, Final_Value & /*to*/ ) const noexcept
	{
		const auto ch = from.getch();
		if( !ch.m_eof && m_delimiter == ch.m_ch )
			return true;

		return false;
	}
};

class token_non_template_base_t
{
protected :
	//FIXME: is it really should be a class member?
	std::string m_value;

	void
	reset()
	{
		m_value.clear();
	}

	RESTINIO_NODISCARD
	bool
	try_parse_value( source_t & from )
	{
		reset();

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

			m_value += ch.m_ch;
		}
		while( true );

		if( m_value.empty() )
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
};

template< typename Setter >
class token_t : public token_non_template_base_t
{
	Setter m_setter;

public:
	template< typename Arg >
	token_t( Arg && setter ) : m_setter{ std::forward<Arg>(setter) }
	{}

	template< typename Final_Value >
	RESTINIO_NODISCARD
	bool
	try_parse(
		source_t & from, Final_Value & to )
	{
		if( try_parse_value( from ) )
		{
			m_setter( to, std::move(m_value) );
			return true;
		}
		else
			return false;
	}
};

} /* namespace rfc */

template< bool valid_index, std::size_t I >
struct try_parse_item_impl
{
	template< typename Final_Value, typename... Parsers >
	RESTINIO_NODISCARD
	static bool apply(
		source_t & source,
		Final_Value & receiver,
		std::tuple<Parsers...> & parsers )
	{
std::cout << "*** content: " << source.current_content() << std::endl;
		if( std::get<I>(parsers).try_parse(source, receiver) )
			return try_parse_item_impl< (I+1 < sizeof...(Parsers)), I+1 >::apply(
					source, receiver, parsers );
		else
			return false;
	}
};

template< std::size_t I >
struct try_parse_item_impl< false, I >
{
	template< typename Final_Value, typename... Parsers >
	RESTINIO_NODISCARD
	static bool apply( source_t &, Final_Value &, std::tuple<Parsers...> & )
	{
		return true;
	}
};

template< typename Final_Value, typename... Parsers >
RESTINIO_NODISCARD
bool
try_parse_item(
	source_t & source,
	Final_Value & receiver,
	std::tuple< Parsers... > & parsers )
{
	return try_parse_item_impl< (0 < sizeof...(Parsers)), 0 >::apply(
			source, receiver, parsers );
}

template<
	typename Container_Adaptor,
	typename Setter,
	typename Subitems_Tuple >
class repeat_t
{
	using container_type = typename Container_Adaptor::container_type;
	using value_type = typename Container_Adaptor::value_type;

	std::size_t m_min_occurences;
	std::size_t m_max_occurences;

	Setter m_setter;

	Subitems_Tuple m_subitems;

public :
	template< typename Setter_Arg >
	repeat_t(
		std::size_t min_occurences,
		std::size_t max_occurences,
		Setter_Arg && setter,
		Subitems_Tuple && subitems )
		:	m_min_occurences{ min_occurences }
		,	m_max_occurences{ max_occurences }
		,	m_setter{ std::forward<Setter_Arg>(setter) }
		,	m_subitems{ std::move(subitems) }
	{}

	template< typename Final_Value >
	RESTINIO_NODISCARD
	bool
	try_parse(
		source_t & from, Final_Value & to )
	{
		container_type aggregate;

		std::size_t count{};
		for( ; count < m_max_occurences; ++count )
		{
			value_type item;
			if( !try_parse_item( from, item, m_subitems ) )
				return false;

			Container_Adaptor::store( aggregate, std::move(item) );
		}

		if( count < m_min_occurences )
			return false;

		m_setter( to, std::move(aggregate) );
		return true;
	}
};

} /* namespace impl */

//FIXME: document this!
template<
	typename Container,
	template<class> class Container_Adaptor = default_container_adaptor,
	typename Setter,
	typename... Parsers >
RESTINIO_NODISCARD
auto
repeat(
	std::size_t min_occurences,
	std::size_t max_occurences,
	Setter && setter,
	Parsers &&... parsers )
{
	return impl::repeat_t<
				Container_Adaptor<Container>,
				std::decay_t<Setter>,
				std::tuple<Parsers...>
			>{
				min_occurences,
				max_occurences,
				std::forward<Setter>(setter),
				std::make_tuple( std::forward<Parsers>(parsers)... )
			};
}

namespace rfc
{

RESTINIO_NODISCARD
auto
ows() noexcept
{
	return restinio::http_field_parser::impl::rfc::ows_t{};
}

RESTINIO_NODISCARD
auto
comma() noexcept
{
	return restinio::http_field_parser::impl::rfc::delimiter_t{ ',' };
}

RESTINIO_NODISCARD
auto
semicolon() noexcept
{
	return restinio::http_field_parser::impl::rfc::delimiter_t{ ';' };
}

RESTINIO_NODISCARD
auto
delimiter( char v ) noexcept
{
	return restinio::http_field_parser::impl::rfc::delimiter_t{ v };
}

template< typename Setter >
RESTINIO_NODISCARD
auto
token( Setter && setter ) noexcept
{
	return restinio::http_field_parser::impl::rfc::token_t<Setter>{
			std::forward<Setter>(setter)
	};
}

} /* namespace rfc */

//
// try_parse_field_value
//
template< typename Final_Value, typename ...Fragments >
RESTINIO_NODISCARD
auto
try_parse_field_value(
	string_view_t from,
	Fragments && ...fragments )
{
	using result_pair_t = std::pair< bool, Final_Value >;

	impl::source_t source{ from };

	result_pair_t result;

	Final_Value tmp_final_value;

	if( impl::try_parse_impl(
			source,
			tmp_final_value,
			std::forward<Fragments>(fragments)... ) )
	{
		result.second = std::move(tmp_final_value);
		result.first = true;
	}
	else
		result.first = false;

	return result;
}

#if 0
//
// try_parse_whole_field
//
template< typename ...Fragments >
RESTINIO_NODISCARD
auto
try_parse_whole_field(
	string_view_t from,
	string_view_t field_name,
	Fragments && ...fragments )
{
	impl::source_t source{ from };

	using result_tuple_t = impl::meta::result_type_detector_t<Fragments...>;
	result_tuple_t result;

	std::get<0>(result) = false;

	// Use index 1 because index 0 is used by boolean flag.
	impl::try_parse_impl<1>(
			result,
			source,
			impl::field_name_t{ field_name },
			std::forward<Fragments>(fragments)... );

	return result;
}

#endif

} /* namespace http_field_parser */

} /* namespace restinio */

