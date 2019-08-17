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
#include <iterator>

#include <restinio/impl/include_fmtlib.hpp>

#include <restinio/exception.hpp>
#include <restinio/string_view.hpp>

namespace restinio
{

namespace path2regex
{

namespace impl
{

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

} /* namespace impl */

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

		options_t &
		delimiters( std::string p ) &
		{
			m_delimiters = std::move( p );
			return *this;
		}

		options_t &&
		delimiters( std::string p ) &&
		{
			return std::move( this->delimiters( std::move( p ) ) );
		}

		const std::string &
		delimiters() const
		{
			return m_delimiters;
		}

		options_t &
		ends_with( std::vector< std::string > p ) &
		{
			m_ends_with = std::move( p );
			return *this;
		}

		options_t &&
		ends_with( std::vector< std::string > p ) &&
		{
			return std::move( this->ends_with( std::move( p ) ) );
		}

		const std::vector< std::string > &
		ends_with() const
		{
			return m_ends_with;
		}

		std::string
		make_ends_with() const
		{
			std::string result;

			for( const auto & e : m_ends_with )
			{
				if( !e.empty() )
				{
					result += impl::escape_string( e ) + "|";
				}
			}

			result += "$";

			return result;
		}

	private:
		//! When true the route will be case sensitive.
		bool m_sensitive{ false };

		//! When false the trailing slash is optional.
		bool m_strict{ false };

		//! When false the path will match at the beginning.
		bool m_ending{ true };

		//! Path delimiter.
		std::string m_delimiter{ "/" };

		//! Path delimiters.
		std::string m_delimiters{ "./" };

		//! Path delimiter.
		std::vector< std::string > m_ends_with;
};

//
// param_appender_t
//

//! Appends sub-match as a request parameter to specified container.
template < typename Route_Param_Appender >
using param_appender_t =
	std::function< void ( Route_Param_Appender &, string_view_t ) >;

//
// param_appender_sequence_t
//

//! A sequence of appenders for submatches.
template < typename Route_Param_Appender >
using param_appender_sequence_t = std::vector< param_appender_t< Route_Param_Appender > >;

//
// make_param_setter
//

//! Create default appender for named parameter.
template < typename Route_Param_Appender >
inline param_appender_t< Route_Param_Appender >
make_param_setter( string_view_t key )
{
	return
		[ key ](
			Route_Param_Appender & parameters,
			string_view_t value ){
			parameters.add_named_param( key, value );
		};
}

//! Create default appender indexed parameter.
template < typename Route_Param_Appender >
inline param_appender_t< Route_Param_Appender >
make_param_setter( std::size_t )
{
	return
		[]( Route_Param_Appender & parameters, string_view_t value ){
			parameters.add_indexed_param( value );
	};
}

namespace impl
{

//
// string_view_buffer_storage_appender_t
//

//! Appender for names to a given buffered string.
template < typename Container >
class string_view_buffer_storage_appender_t final
{
	public:
		string_view_buffer_storage_appender_t( std::size_t reserve_size, Container & buffer )
			:	m_buffer{ buffer }
		{
			m_buffer.reserve( reserve_size );
			assert( m_buffer.capacity() >= reserve_size );
		}

		//! Appends a given name to buffer,
		//! and returns a string view object within the context of a buffer.
		string_view_t
		append_name( const std::string & name )
		{
			const auto n = name.size();
			if( m_buffer.capacity() - m_buffer.size() < n )
			{
				// This actually should never happen,
				// because buffer is set to the size
				// of a whole route-path that itself contains all the names.
				throw exception_t{ "unable to insert data into names buffer" };
			}

			// Remember where previous names finishes.
			const auto prev_size = m_buffer.size();

			std::copy( name.data(), name.data() + n, std::back_inserter( m_buffer ) );
			return string_view_t{ m_buffer.data() + prev_size, n };
		}

		//! A stub for indexed paramaters.
		std::size_t
		append_name( std::size_t i ) const
		{
			return i;
		}

	private:
		Container & m_buffer;
};

using names_buffer_appender_t = string_view_buffer_storage_appender_t< std::string >;

//! The main path matching expression.
constexpr auto path_regex_str =
	R"((\\.)|(?:\:(\w+)(?:\(((?:\\.|[^\\()])+)\))?|\(((?:\\.|[^\\()])+)\))([+*?])?)";

enum class token_type_t : std::uint8_t
{
	plain_string,
	capturing_token
};

//
// token_t
//

//! Base class for token variants.
template < typename Route_Param_Appender >
class token_t
{
	public:
		token_t() = default;
		token_t( const token_t & ) = delete;
		token_t( token_t && ) = delete;
		virtual ~token_t() = default;

		virtual token_type_t
		append_self_to(
			std::string & route,
			param_appender_sequence_t< Route_Param_Appender > & param_appender_sequence,
			names_buffer_appender_t & names_buffer_appender ) const = 0;

		virtual bool
		is_end_delimited( const std::string & ) const noexcept
		{
			return false;
		}
};

template < typename Route_Param_Appender >
using token_unique_ptr_t = std::unique_ptr< token_t< Route_Param_Appender > >;

template < typename Route_Param_Appender >
using token_list_t = std::vector< token_unique_ptr_t< Route_Param_Appender > >;

//
// plain_string_token_t
//

//! Plain str token.
template < typename Route_Param_Appender >
class plain_string_token_t final : public token_t< Route_Param_Appender >
{
	public:
		plain_string_token_t( const std::string & path )
			:	m_escaped_path{ escape_string( path ) }
			,	m_last_char{ path.back() }
		{}

		virtual token_type_t
		append_self_to(
			std::string & route,
			param_appender_sequence_t< Route_Param_Appender > &,
			names_buffer_appender_t & ) const override
		{
			route += m_escaped_path;

			return token_type_t::plain_string;
		}

		virtual bool
		is_end_delimited( const std::string & delimiters ) const noexcept override
		{
			return std::string::npos != delimiters.find( m_last_char );
		}

	private:
		//! Already escaped piece of the route.
		const std::string m_escaped_path;
		const char m_last_char;
};

template < typename Route_Param_Appender >
token_unique_ptr_t< Route_Param_Appender >
create_token( std::string path )
{
	using token_t = plain_string_token_t< Route_Param_Appender >;
	return std::make_unique< token_t >( std::move( path ) );
}

//
// parameter_token_t
//

//! Token for paramater (named/indexed).
template < typename Route_Param_Appender, typename Name >
class parameter_token_t final : public token_t< Route_Param_Appender >
{
	public:
		parameter_token_t( const parameter_token_t & ) = delete;
		parameter_token_t( parameter_token_t && ) = delete;

		parameter_token_t(
			Name name,
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

		virtual token_type_t
		append_self_to(
			std::string & route,
			param_appender_sequence_t< Route_Param_Appender > & param_appender_sequence,
			names_buffer_appender_t & names_buffer_appender ) const override
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

			param_appender_sequence.push_back(
				make_param_setter< Route_Param_Appender >(
					names_buffer_appender.append_name( m_name ) ) );

			return token_type_t::capturing_token;
		}

	private:
		const Name m_name;
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
template < typename Route_Param_Appender, typename Name >
inline token_unique_ptr_t< Route_Param_Appender >
create_token(
	Name name,
	std::string prefix,
	std::string delimiter,
	bool optional,
	bool repeat,
	bool partial,
	std::string pattern )
{
	return std::make_unique< parameter_token_t< Route_Param_Appender, Name > >(
		std::move( name ),
		std::move( prefix ),
		std::move( delimiter ),
		optional,
		repeat,
		partial,
		std::move( pattern ) );
}

//! Indexes for different groups in matched result
//! (used when extracting tokens from initial route).
//! \{
constexpr std::size_t group_escaped_idx = 1;
constexpr std::size_t group_name_idx = 2;
constexpr std::size_t group_capture_idx = 3;
constexpr std::size_t group_group_idx = 4;
constexpr std::size_t group_modifier_idx = 5;
//! \}

//! Checks that string doesn't contain non-excaped brackets
inline std::string
check_no_unescaped_brackets( string_view_t strv, std::size_t base_pos )
{
	auto pos = strv.find( '(' );
	if( std::string::npos != pos )
	{
		throw exception_t{
			fmt::format(
				"non-escaped bracket '(' at pos {}: may be unmatched group start",
				base_pos + pos ) };
	}

	pos = strv.find( ')' );
	if( std::string::npos != pos )
	{
		throw exception_t{
			fmt::format(
				"non-escaped bracket ')' at pos {}: may be unmatched group finish",
				base_pos + pos ) };
	}

	return std::string{ strv.data(), strv.size() };
}

//
// handle_param_token()
//

//! Handling of a parameterized token.
template < typename Route_Param_Appender, typename MATCH >
inline void
handle_param_token(
	const options_t & options,
	const MATCH & match,
	std::string & path,
	bool & path_escaped,
	token_list_t< Route_Param_Appender > & result )
{
	std::string prefix{ "" }; // prev in js code.
	if( !path_escaped && !path.empty() )
	{
		const auto k = path.size() - 1;

		if( std::string::npos != options.delimiters().find( path[k] ) )
		{
			prefix = path.substr( k, 1 );
			path = path.substr( 0, k );
		}
	}

	// Push the current path onto the tokens.
	if( !path.empty() )
	{
		result.push_back( create_token< Route_Param_Appender >( std::move( path ) ) );
		path_escaped = false;
	}

	const auto next = match.suffix().str().substr( 0, 1 );

	std::string name{ match[ group_name_idx ].str() };
	const std::string modifier{ match[ group_modifier_idx ].str() };

	const bool partial = !prefix.empty() && !next.empty() && prefix != next;

	const bool optional = modifier == "?" || modifier == "*";
	const bool repeat = modifier == "+" || modifier == "*";
	std::string delimiter = options.make_delimiter( prefix );

	auto create_pattern = [ delimiter ]( auto pattern ){
		if( !pattern.empty() )
		{
			pattern = escape_group( pattern );
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
			create_token< Route_Param_Appender >(
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
			create_token< Route_Param_Appender >(
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
template < typename Route_Param_Appender >
token_list_t< Route_Param_Appender >
parse( string_view_t route_sv, const options_t & options )
{
	token_list_t< Route_Param_Appender > result;

	std::string path{};
	const std::regex main_path_regex{ path_regex_str };
	bool path_escaped = false;

	std::cregex_iterator token_it{
			route_sv.data(),
			route_sv.data() + route_sv.size(),
			main_path_regex
	};
	std::cregex_iterator token_end{};

	if( token_it == token_end )
	{
		// Path is a single token.
		path = check_no_unescaped_brackets( route_sv, 0 );
	}

	while( token_it != token_end )
	{
		const auto & match = *token_it;

		assert( 6 == match.size() );

		const string_view_t prefix{
				match.prefix().first,
				static_cast<std::size_t>( match.prefix().length() ) };

		path += check_no_unescaped_brackets( prefix,
				static_cast<std::size_t>(match.position()) - prefix.size() );

		const auto escaped = match[ group_escaped_idx ].str();
		if( !escaped.empty() )
		{
			assert( 2 == escaped.size() );
			path += escaped[ 1 ];
			path_escaped = true;
		}
		else
		{
			handle_param_token( options, match, path, path_escaped, result );
		}

		auto next_it = token_it;
		std::advance( next_it, 1 );

		if( next_it == token_end )
		{
			const std::string suffix{ match.suffix() };
			path +=
				check_no_unescaped_brackets(
					suffix,
					static_cast<std::size_t>(match.position() + match.length()) );
		}

		token_it = next_it;
	}

	if( !path.empty() )
		result.push_back( create_token< Route_Param_Appender >( std::move( path ) ) );

	return result;
}

//
// route_regex_matcher_data_t
//

//! Resulting regex and param extraction for a specific route.
template < typename Route_Param_Appender, typename Regex_Engine >
struct route_regex_matcher_data_t
{
	route_regex_matcher_data_t() = default;
	route_regex_matcher_data_t( const route_regex_matcher_data_t & ) = delete;
	route_regex_matcher_data_t & operator = ( const route_regex_matcher_data_t & ) = delete;

	route_regex_matcher_data_t( route_regex_matcher_data_t && ) = default;
	route_regex_matcher_data_t & operator = ( route_regex_matcher_data_t && ) = delete;

	using regex_t = typename Regex_Engine::compiled_regex_t;

	regex_t m_regex;

	//! Char buffer for holding named paramaters.
	/*!
		In order to store named parameters 'names' in a continous block of memory
		and use them in param_appender_sequence items as string_view.
	*/
	std::shared_ptr< std::string > m_named_params_buffer;

	//! Appenders for captured values (names/indexed groups).
	param_appender_sequence_t< Route_Param_Appender > m_param_appender_sequence;
};

//
// tokens2regexp()
//

//! Makes route regex matcher out of path tokens.
template < typename Route_Param_Appender, typename Regex_Engine >
auto
tokens2regexp(
	string_view_t path,
	const token_list_t< Route_Param_Appender > & tokens,
	const options_t & options )
{
	route_regex_matcher_data_t< Route_Param_Appender, Regex_Engine > result;
	try
	{
		result.m_named_params_buffer = std::make_shared< std::string >();
		names_buffer_appender_t
			names_buffer_appender{ path.size(), *result.m_named_params_buffer };

		std::string route;
		auto & param_appender_sequence = result.m_param_appender_sequence;

		// The number of capture groups in resultin regex
		// 1 is for match of a route itself.
		std::size_t captured_groups_count = 1 ;

		for( const auto & t : tokens )
		{
			const auto appended_token_type =
				t->append_self_to( route, param_appender_sequence, names_buffer_appender );

			if( token_type_t::capturing_token == appended_token_type )
				++captured_groups_count;
		}

		if( Regex_Engine::max_capture_groups() < captured_groups_count )
		{
			// This number of captures is not possible with this engine.
			throw exception_t{
				fmt::format(
					"too many parameter to capture from route: {}, while {} is the maximum",
					captured_groups_count,
					Regex_Engine::max_capture_groups() ) };
		}

		const auto & delimiter = escape_string( options.delimiter() );
		const auto & ends_with = options.make_ends_with();

		if( options.ending() )
		{
			if( !options.strict() )
			{
				route += "(?:" + delimiter + ")?";
			}

			if( ends_with == "$" )
				route += '$';
			else
				route += "(?=" + ends_with + ")";
		}
		else
		{
			if( !options.strict() )
				route += "(?:" + delimiter + "(?=" + ends_with + "))?";

			if( !tokens.empty() &&
				!tokens.back()->is_end_delimited( options.delimiters() ) )
				route += "(?=" + delimiter + "|" + ends_with + ")";
		}

		result.m_regex = Regex_Engine::compile_regex( "^" + route, options.sensitive() );
	}
	catch( const std::exception & ex )
	{
		throw exception_t{
			fmt::format( "unable to process route \"{}\": {}", path, ex.what() ) };
	}

	return result;
}

} /* namespace impl */

//
// path2regex()
//

//! The main path matching regexp.
template < typename Route_Param_Appender, typename Regex_Engine >
inline auto
path2regex(
	string_view_t path,
	const options_t & options )
{
	return impl::tokens2regexp< Route_Param_Appender, Regex_Engine >(
			path,
			impl::parse< Route_Param_Appender >( path, options ),
			options );
}

} /* namespace path2regex */

} /* namespace restinio */
