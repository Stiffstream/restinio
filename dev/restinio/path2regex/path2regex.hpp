/*
	restinio
*/

/*!
	Utility for converting express.js style routes to regexp.

	Code adopted from https://github.com/pillarjs/path-to-regexp.
*/
#pragma once

#include <cassert>
#include <regex>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace restinio
{

namespace path2regex
{

//
// options_t
//

//! Options for matching routes.
class options_t
{
	public:
		options_t &
		sensitive( bool s ) &
		{
			m_sensitive = s;
			return *this;
		}

		options_t &&
		sensitive( bool s ) &&
		{
			return std::move( this->sensitive( s ) );
		}

		bool
		sensitive() const
		{
			return m_sensitive;
		}

		options_t &
		strict( bool p ) &
		{
			m_strict = p;
			return *this;
		}

		options_t &&
		strict( bool p ) &&
		{
			return std::move( this->strict( p ) );
		}

		bool
		strict() const
		{
			return m_strict;
		}

		options_t &
		ending( bool p ) &
		{
			m_ending = p;
			return *this;
		}

		options_t &&
		ending( bool p ) &&
		{
			return std::move( this->ending( p ) );
		}

		bool
		ending() const
		{
			return m_ending;
		}

		options_t &
		delimiter( std::string p ) &
		{
			m_delimiter = std::move( p );
			return *this;
		}

		options_t &&
		delimiter( std::string p ) &&
		{
			return std::move( this->delimiter( std::move( p ) ) );
		}

		const std::string &
		delimiter() const
		{
			return m_delimiter;
		}

		std::string
		make_delimiter( std::string d ) const
		{
			std::string result{ std::move( d ) };

			if( result.empty() )
				result = delimiter();

			return result;
		}

	private:
		//! When true the route will be case sensitive.
		bool m_sensitive{ false };

		//! When false the trailing slash is optional.
		bool m_strict{ false };

		//! When false the path will match at the beginning.
		bool m_ending{ true };

		//! Path delimeter.
		std::string m_delimiter{ "/" };
};

//
// param_appender_t
//

//! Appends sub-match as a request parameter to specified container.
template < typename PARAM_CONTAINER >
using param_appender_t =
	std::function< void ( PARAM_CONTAINER &, std::string ) >;

//
// param_appender_sequence_t
//

//! A sequence of appenders for submatches.
template < typename PARAM_CONTAINER >
using param_appender_sequence_t = std::vector< param_appender_t< PARAM_CONTAINER > >;

//
// make_param_setter
//

//! Create default appender for named parameter.
template < typename PARAM_CONTAINER >
inline param_appender_t< PARAM_CONTAINER >
make_param_setter( std::string key )
{
	return
		[ key ]( PARAM_CONTAINER & parameters, std::string value ){
			parameters.add_named_param( key, std::move( value ) );
	};
}

//! Create default appender indexed parameter.
template < typename PARAM_CONTAINER >
inline param_appender_t< PARAM_CONTAINER >
make_param_setter( std::size_t /* index */)
{
	return
		[ ]( PARAM_CONTAINER & parameters, std::string value ){
			parameters.add_indexed_param( std::move( value ) );
	};
}

namespace impl
{

//! The main path matching expression.
constexpr auto PATH_REGEX_STR =
	R"((\\.)|([\/.])?(?:(?:\:(\w+)(?:\(((?:\\.|[^\\()])+)\))?|\(((?:\\.|[^\\()])+)\))([+*?])?|(\*)))";

//
// escape_group()
//

//! Escapes not allowed symbols in a sub-match group assigned to a parameter.
inline auto
escape_group( const std::string & group )
{
	std::string result;
	result.reserve( group.size() + group.size() / 2 );

	for( const char c : group )
	{
		if( '=' == c || '!' == c || ':' == c ||
			'$' == c || '/' == c || '(' == c || ')' == c )
			result+= '\\';

		result+= c;
	}

	return result;
}

//
// escape_string()
//

//! Excape regex control chars.
inline auto
escape_string( const std::string & group )
{
	std::string result;
	result.reserve( group.size() + group.size() / 2 );

	for( const char c : group )
	{
		if( '.' == c || '+' == c || '*' == c ||
			'?' == c || '=' == c || '^' == c ||
			':' == c || '$' == c || '{' == c ||
			'}' == c || '(' == c || ')' == c ||
			'[' == c || ']' == c || '|' == c ||
			'\\' == c || '/' == c )
			result+= '\\';

		result+= c;
	}

	return result;
}

//
// token_t
//

//! Base class for token variants.
template < typename PARAM_CONTAINER >
class token_t
{
	public:
		token_t() = default;
		token_t( const token_t & ) = delete;
		token_t( token_t && ) = delete;
		virtual ~token_t() = default;

		virtual void
		append_self_to(
			std::string & route,
			param_appender_sequence_t< PARAM_CONTAINER > & param_appender_sequence ) const = 0;
};

template < typename PARAM_CONTAINER >
using token_unique_ptr_t = std::unique_ptr< token_t< PARAM_CONTAINER > >;

template < typename PARAM_CONTAINER >
using token_list_t = std::vector< token_unique_ptr_t< PARAM_CONTAINER > >;

//
// plain_string_token_t
//

//! Plain str token.
template < typename PARAM_CONTAINER >
class plain_string_token_t final : public token_t< PARAM_CONTAINER >
{
	public:
		plain_string_token_t( const std::string & path )
			:	m_escaped_path{ escape_string( path ) }
		{}

		virtual void
		append_self_to(
			std::string & route,
			param_appender_sequence_t< PARAM_CONTAINER > & ) const override
		{
			route += m_escaped_path;
		}

	private:
		//! Already escaped piece of the route.
		const std::string m_escaped_path;
};

template < typename PARAM_CONTAINER >
token_unique_ptr_t< PARAM_CONTAINER >
create_token( std::string path )
{
	using token_t = plain_string_token_t< PARAM_CONTAINER >;
	return std::make_unique< token_t >( std::move( path ) );
}

//
// parameter_token_t
//

//! Token for paramater (named/indexed).
template < typename PARAM_CONTAINER, typename NAME_TYPE >
class parameter_token_t final : public token_t< PARAM_CONTAINER >
{
	public:
		parameter_token_t( const parameter_token_t & ) = delete;
		parameter_token_t( parameter_token_t && ) = delete;

		parameter_token_t(
			NAME_TYPE name,
			const std::string & prefix,
			std::string delimiter,
			bool optional,
			bool repeat,
			bool partial,
			std::string pattern )
			:	m_name{ std::move( name ) }
			,	m_escaped_prefix{ escape_string( prefix ) }
			,	m_delimiter{ std::move( delimiter ) }
			,	m_optional{ optional }
			,	m_repeat{ repeat }
			,	m_partial{ partial }
			,	m_pattern{ std::move( pattern ) }
		{}

		virtual void
		append_self_to(
			std::string & route,
			param_appender_sequence_t< PARAM_CONTAINER > & param_appender_sequence ) const override
		{
			// Basic capturing pattern.
			auto capture = "(?:" + m_pattern + ")";

			if( m_repeat )
			{
				// Add * as the parameter can be repeeated.
				capture += "(?:" + m_escaped_prefix + capture + ")*";
			}

			if( m_optional )
			{
				// Optional param goes in ()?.
				if( !m_partial )
				{
					capture = "(?:" + m_escaped_prefix + "(" + capture + "))?";
				}
				else
				{
					capture = m_escaped_prefix + "(" + capture + ")?";
				}
			}
			else
			{
				// Mandatory param goes in ().
				capture = m_escaped_prefix + "(" + capture + ")";
			}

			route += capture;

			param_appender_sequence.push_back( make_param_setter< PARAM_CONTAINER >( m_name ) );
		}

	private:
		const NAME_TYPE m_name;
		const std::string m_escaped_prefix;
		const std::string m_delimiter;
		const bool m_optional;
		const bool m_repeat;
		const bool m_partial;
		const std::string m_pattern;
};

//
// create_token()
//

//! Creates tokent for specific parameter.
template < typename PARAM_CONTAINER, typename NAME_TYPE >
inline token_unique_ptr_t< PARAM_CONTAINER >
create_token(
	NAME_TYPE name,
	std::string prefix,
	std::string delimiter,
	bool optional,
	bool repeat,
	bool partial,
	std::string pattern )
{
	return std::make_unique< parameter_token_t< PARAM_CONTAINER, NAME_TYPE > >(
		std::move( name ),
		std::move( prefix ),
		std::move( delimiter ),
		optional,
		repeat,
		partial,
		std::move( pattern ) );
}

//! Indexes for different groups in matched result
//! (used when extructing tokens from initial route).
//! \{
constexpr std::size_t group_escaped_idx = 1;
constexpr std::size_t group_prefix_idx = 2;
constexpr std::size_t group_name_idx = 3;
constexpr std::size_t group_capture_idx = 4;
constexpr std::size_t group_group_idx = 5;
constexpr std::size_t group_modifier_idx = 6;
constexpr std::size_t group_asterisk_idx = 7;
//! \}

//
// handle_param_token()
//

//! Handling of a parameterized token.
template < typename PARAM_CONTAINER, typename MATCH >
inline void
handle_param_token(
	const options_t & options,
	const MATCH & match,
	std::string & path,
	token_list_t< PARAM_CONTAINER > & result )
{
	// Add preceding path as a plain string token.
	if( !path.empty() )
		result.push_back( create_token< PARAM_CONTAINER >( std::move( path ) ) );

	std::string name{ match[ group_name_idx ].str() };
	std::string prefix{ match[ group_prefix_idx ].str() };
	std::string delimiter = options.make_delimiter( prefix );
	const std::string modifier{ match[ group_modifier_idx ].str() };
	const bool optional = modifier == "?" || modifier == "*";
	const bool repeat = modifier == "+" || modifier == "*";

	const auto next = match.suffix().str().substr( 0, 1 );
	const bool partial = !prefix.empty() && !next.empty() && prefix != next;
	const bool asterisk = !match[ group_asterisk_idx ].str().empty();

	auto create_pattern = [ asterisk, delimiter ]( auto pattern ){
		if( !pattern.empty() )
		{
			pattern = escape_group( pattern );
		}
		else if( asterisk )
		{
			pattern = ".*";
		}
		else
		{
			pattern = "[^" + escape_string( delimiter ) + "]+?";
		}
		return pattern;
	};

	if( !name.empty() )
	{
		// Named parameter.
		result.push_back(
			create_token< PARAM_CONTAINER >(
				name,
				std::move( prefix ),
				std::move( delimiter ),
				optional,
				repeat,
				partial,
				create_pattern( match[ group_capture_idx ].str() ) ) );
	}
	else
	{
		// Indexed parameter.
		result.push_back(
			create_token< PARAM_CONTAINER >(
				std::size_t{ 0 }, // just to have a variable of this type.
				std::move( prefix ),
				std::move( delimiter ),
				optional,
				repeat,
				partial,
				create_pattern( match[ group_group_idx ].str() ) ) );
	}
}

//
// parse()
//

//! Parse a string for the raw tokens.
template < typename PARAM_CONTAINER >
token_list_t< PARAM_CONTAINER >
parse( const std::string & route_str, const options_t & options )
{
	token_list_t< PARAM_CONTAINER > result;

	std::string path{};
	std::regex main_path_regex{ PATH_REGEX_STR };

	auto token_it =
		std::sregex_iterator( route_str.begin(), route_str.end(), main_path_regex );
	auto token_end = std::sregex_iterator{};

	if( token_it == token_end )
	{
		// Path is a single token.
		path = route_str;
	}

	while( token_it != token_end )
	{
		const auto & match = *token_it;

		assert( 8 == match.size() );

		path += match.prefix();

		const auto escaped = match[ group_escaped_idx ].str();
		if( !escaped.empty() )
		{
			assert( 2 == escaped.size() );

			path += escaped[ 1 ];
		}
		else
		{
			handle_param_token( options, match, path, result );
		}

		auto next_it = token_it;
		std::advance( next_it, 1 );

		if( next_it == token_end )
		{
			path += match.suffix();
		}

		token_it = next_it;
	}

	if( !path.empty() )
		result.push_back( create_token< PARAM_CONTAINER >( std::move( path ) ) );

	return result;
}

//
// route_regex_matcher_data_t
//

//! Resulting regex and param extraction for a specific route.
template < typename PARAM_CONTAINER >
struct route_regex_matcher_data_t
{
	route_regex_matcher_data_t() = default;
	route_regex_matcher_data_t( route_regex_matcher_data_t && ) = default;

	std::regex m_regex;
	param_appender_sequence_t< PARAM_CONTAINER > m_param_appender_sequence;
};

//
// tokens2regexp()
//

//! Makes route regex matcher out of path tokens.
template < typename PARAM_CONTAINER >
auto
tokens2regexp( const token_list_t< PARAM_CONTAINER > & tokens, const options_t & options )
{
	route_regex_matcher_data_t< PARAM_CONTAINER > result;
	std::string route;
	auto & param_appender_sequence = result.m_param_appender_sequence;

	for( const auto & t : tokens )
	{
		t->append_self_to( route, param_appender_sequence );
	}

	const auto & delimiter = escape_string( options.delimiter() );
	const bool ends_with_delimiter =
		route.size() >= delimiter.size() &&
		route.substr( route.size() - delimiter.size() ) == delimiter;

	// In non-strict mode we allow a slash at the end of match. If the path to
	// match already ends with a slash, we remove it for consistency. The slash
	// is valid at the end of a path match, not in the middle. This is important
	// in non-ending mode, where "/test/" shouldn't match "/test//route".
	if( !options.strict() )
	{
		if( ends_with_delimiter )
			route.resize( route.size() - delimiter.size() );

		route += "(?:" + delimiter + "(?=$))?";
	}

	if( options.ending() )
	{
		route += '$';
	}
	else if( !( options.strict() && ends_with_delimiter ) )
	{
		// In non-ending mode, we need the capturing groups to match as much as
		// possible by using a positive lookahead to the end or next path segment.
		route += "(?=" + delimiter + "|$)";
	}

	auto regex_flags = std::regex::ECMAScript;
	if( !options.sensitive() )
	{
		regex_flags |= std::regex::icase;
	}

	result.m_regex.assign( "^" + route, regex_flags );
	return result;
}

} /* namespace impl */

//
// path2regex()
//

//! The main path matching regexp.
template < typename PARAM_CONTAINER >
inline auto
path2regex(
	const std::string & path,
	const options_t & options )
{
	return impl::tokens2regexp< PARAM_CONTAINER >(
			impl::parse< PARAM_CONTAINER >( path, options ),
			options );
}

} /* namespace path2regex */

} /* namespace restinio */
