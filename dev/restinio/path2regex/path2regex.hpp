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
			'!' == c || ':' == c || '$' == c ||
			'{' == c || '}' == c || '(' == c ||
			')' == c || '[' == c || ']' == c ||
			'|' == c || '\\' == c || '/' == c )
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
		starting( bool p ) &
		{
			m_starting = p;
			return *this;
		}

		options_t &&
		starting( bool p ) &&
		{
			return std::move( this->starting( p ) );
		}

		bool
		starting() const
		{
			return m_starting;
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
		prefixes( std::string p ) &
		{
			m_prefixes = std::move( p );
			return *this;
		}

		options_t &&
		prefixes( std::string p ) &&
		{
			return std::move( this->prefixes( std::move( p ) ) );
		}

		const std::string &
		prefixes() const
		{
			return m_prefixes;
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

		//! When true the regexp will match to the end of the string.
		bool m_ending{ true };

	//! When false the path will match at the beginning.
		bool m_starting{ true };

		//! End delimiter.
		std::string m_delimiter{ "/#?" };

		//! List of characters to automatically consider prefixes when parsing.
		std::string m_prefixes{ "./" };

		//! End delimiter.
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
		is_end_delimited( const std::string & prefixes ) const noexcept override
		{
			return std::string::npos != prefixes.find( m_last_char );
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
			const std::string & suffix,
			const std::string & pattern,
			const std::string & modifier
			 )
			:	m_name{ std::move( name ) }
			,	m_escaped_prefix{ escape_string( prefix ) }
			,	m_escaped_suffix{ escape_string( suffix ) }
			,	m_pattern{ std::move( pattern ) }
			,	m_modifier{ std::move( modifier ) }
		{}
	// optional() is apparently not used anymore in tokens2regexp; but I left it, in case that changes at some point.
		bool optional() const {
			return (m_modifier == "*" || m_modifier == "?");
		}
		bool repeat() const {
			return (m_modifier == "*" || m_modifier == "+");
		}
		virtual token_type_t
		append_self_to(
			std::string & route,
			param_appender_sequence_t< Route_Param_Appender > & param_appender_sequence,
			names_buffer_appender_t & names_buffer_appender ) const override
		{
			std::string capture{};
			if( !m_pattern.empty() )
			{
				if ( !m_escaped_prefix.empty() || !m_escaped_suffix.empty() ) {

					if( repeat() )
					{
						const std::string mod{m_modifier == "*" ? "?" : ""};
						// Add * as the parameter can be repeeated.
						capture += fmt::format(
							RESTINIO_FMT_FORMAT_STRING(
								"(?:{}((?:{})(?:{}{}(?:{}))*){}){})"
							 ),
							m_escaped_prefix, m_pattern, m_escaped_suffix, m_escaped_prefix, m_pattern, m_escaped_suffix, mod
						);
					} else {
						capture += fmt::format(
							RESTINIO_FMT_FORMAT_STRING(
								"(?:{}({}){}){}"
							 ),
							m_escaped_prefix, m_pattern, m_escaped_suffix, m_modifier
						);
					}
				} else {
					if ( repeat() ) {
						capture += fmt::format(
							RESTINIO_FMT_FORMAT_STRING(
								"((?:{}){})"
							 ),
							m_pattern, m_modifier
						);
					} else {
						capture += fmt::format(
							RESTINIO_FMT_FORMAT_STRING(
								"({}){}"
							 ),
							m_pattern, m_modifier
						);
					}
				}
			} else {
				capture += fmt::format(
					RESTINIO_FMT_FORMAT_STRING(
						"(?:{}{}){}"
					 ),
					m_escaped_prefix, m_escaped_suffix, m_modifier
				);
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
		const std::string m_escaped_suffix;
		const std::string m_delimiter;
		const std::string m_pattern;
		const std::string m_modifier;
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
	std::string suffix,
	std::string pattern,
	std::string modifier )
{
	return std::make_unique< parameter_token_t< Route_Param_Appender, Name > >(
		std::move( name ),
		std::move( prefix ),
		std::move( suffix ),
		std::move( pattern ),
		std::move( modifier ) );
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

enum class lextoken_type_t : std::uint8_t {
	open,
	close,
	pattern,
	name,
	char_t,
	escaped_char,
	modifier,
	end
};

struct lextoken_t {
	lextoken_t(std::string& _val, std::size_t _index, lextoken_type_t _type):
		value(std::move(_val)), index(_index), type(_type) {};
	lextoken_t(const char* _val, std::size_t _index, lextoken_type_t _type):
		value(std::string(_val, 1)), index(_index), type(_type) {};

	std::string value;
	size_t index;
	lextoken_type_t type;
};

inline const std::string print_type(const lextoken_type_t& _type) {
	std::string result;
	switch (_type) {
		case lextoken_type_t::open:
			result = "open";
			break;
		case lextoken_type_t::close:
			result = "close";
			break;
		case lextoken_type_t::pattern:
			result = "pattern";
			break;
		case lextoken_type_t::name:
			result = "name";
			break;
		case lextoken_type_t::char_t:
			result = "char_t";
			break;
		case lextoken_type_t::escaped_char:
			result = "escaped_char";
			break;
		case lextoken_type_t::modifier:
			result = "modifier";
			break;
		case lextoken_type_t::end:
			result = "end";
			break;
		default:
			throw exception_t( "Invalid lextoken_type" );
	}
	return result;
};

using lextokens_t = std::vector<lextoken_t>;

inline lextokens_t lexer (string_view_t str) {
	lextokens_t tokens;
	for (string_view_t::const_iterator c = str.begin(); c != str.end();) {
		assert ( c >= str.data() );
		std::size_t _index = static_cast<size_t>(c - str.data());
		if (*c == '*' || *c == '+' || *c == '?') {
			tokens.emplace_back(lextoken_t(c, _index, lextoken_type_t::modifier));
			std::advance(c,1);
			continue;
		}
		if (*c == '\\') {
			std::advance(c,1);
			tokens.emplace_back(lextoken_t(c, _index + 1, lextoken_type_t::escaped_char));
			std::advance(c,1);
			continue;
		}
		if (*c == '{') {
			tokens.emplace_back(lextoken_t(c, _index, lextoken_type_t::open));
			std::advance(c,1);
			continue;
		}
		if (*c == '}') {
			tokens.emplace_back(lextoken_t(c, _index, lextoken_type_t::close));
			std::advance(c,1);
			continue;
		}
		if (*c == ':') {
			std::string groupName;
			std::size_t j = _index + 1;
			while (j < str.size()) {
				unsigned code = static_cast<unsigned>(str.at(j));
				if ((code >= 48 && code <= 57) || ///< '0-9'
					(code >= 65 && code <= 90) || ///< 'A-Z'
					(code >= 97 && code <= 122) || ///< 'a-z'
					(code == 95)) { ///< '_'
						groupName += str.at(j);
						j++;
						continue;
					}
				break;
			}
			if (groupName.empty())
				throw exception_t{"Missing parameter name at: " + std::to_string(_index)};
	
			tokens.emplace_back(lextoken_t(groupName, _index, lextoken_type_t::name));
			std::advance(c, j - _index);
			continue;
		}
		if (*c == '(') {
			std::string pattern;
			std::size_t count = 1;
			std::size_t j = _index + 1;
			if (str.at(j) == '?')
				throw exception_t{"Pattern cannot start with '?' at: " + std::to_string(j)};
			while (j < str.size()) {
				if (str.at(j) == '\\') {
					pattern += {str.at(j), str.at(j + 1)}; // string_view_lite doesn't let us use substr() to do this.
					j+=2;
					continue;
				}
				if (str.at(j) == ')') {
					count--;
					if (count == 0) {
						j++;
						break;
					}
				}
				else if (str.at(j) == '(') {
					count++;
					if (str.at(j + 1) != '?')
						throw exception_t{"Capturing groups are not allowed at: " + std::to_string(j)};
				}
				pattern += str.at(j++);
			}
			if (count != 0)
				throw exception_t{"Unbalanced pattern at: " + std::to_string(_index)};
			if (pattern.empty())
				throw exception_t{"Missing pattern at: " + std::to_string(_index)};
			
			tokens.emplace_back(lextoken_t(pattern, _index, lextoken_type_t::pattern));
			std::advance(c, j - _index);
			continue;
		}
		tokens.emplace_back(lextoken_t(c, _index, lextoken_type_t::char_t));
		std::advance(c, 1);
		continue;
	}
	tokens.emplace_back(lextoken_t("", str.size() - 1, lextoken_type_t::end));
	return tokens;
}


inline optional_t<std::string> try_consume (lextoken_type_t type, lextokens_t::const_iterator& token) {
	if (token->type == type) {
		return (token++)->value;
	}
	return nonstd::nullopt;
}

inline void must_consume (lextoken_type_t type, lextokens_t::const_iterator& token) {
	auto ret = try_consume(type, token);
	if (ret) {
		return;
	}
	throw std::runtime_error("Unexpected type: " + print_type(token->type) + ", expected: " + print_type(type));
}

inline optional_t<std::string> consume_text (lextokens_t::const_iterator& token) {
	std::string result{};
	optional_t<std::string> value{"_"};
	while (value) {
		value = try_consume(lextoken_type_t::char_t, token);
		if (!value)
			value = try_consume(lextoken_type_t::escaped_char, token);
		if (!value) break;
		result += value.value();
	}
	if (!result.empty())
		return result;
	return nonstd::nullopt;
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

	std::string defaultPattern{R"-([^)-"};
	std::string delimiter{options.delimiter()};
	defaultPattern += escape_string(options.delimiter()) + R"-(]+?)-";
	std::string path{};
	std::size_t key{0};

	auto create_pattern = [ options ]( auto pattern ) -> std::string {
		if( !pattern.empty() )
		{
			pattern = escape_group( pattern );
		}
		else
		{
			pattern = R"-([^)-" + escape_string( options.delimiter() ) + R"-(]+?)-";
		}

		return pattern;
	};
	lextokens_t tokens = lexer(route_sv);
	for (lextokens_t::const_iterator token = tokens.begin(); token != tokens.end();) {
		std::string prefix{};
		auto char_t = try_consume(lextoken_type_t::char_t, token);
		auto name = try_consume(lextoken_type_t::name, token);
		auto pattern = try_consume(lextoken_type_t::pattern, token);

		if (name || pattern) {
			prefix = char_t.value_or("");
			if (options.prefixes().find(prefix) == std::string::npos) {
				path += prefix;
				prefix.clear();
			}
			
			if (!path.empty()) {
				result.push_back( create_token< Route_Param_Appender >( std::move( path ) ) );
				path.clear();
			}
			optional_t<std::string> modifier = try_consume(lextoken_type_t::modifier, token);
			if (name) {
				result.push_back( create_token< Route_Param_Appender >(
					name.value(),
					prefix,
					"",
					create_pattern( pattern.value_or(defaultPattern) ),
					modifier.value_or("") ) );
			}
			else {
				result.push_back( create_token< Route_Param_Appender >(
					key++,
					prefix,
					"",
					create_pattern( pattern.value_or(defaultPattern) ),
					modifier.value_or("") ) );
			}
			continue;
		}
		optional_t<std::string> value;
		if (char_t) {
			value = char_t;
		}
		else {
			value = try_consume(lextoken_type_t::escaped_char, token);
		}
		if (value) {
			path += value.value();
			continue;
		}
		
		if (!path.empty()) {
			result.push_back ( create_token< Route_Param_Appender >(
				std::move(path ) ) );
			path.clear();
		}
		auto open = try_consume(lextoken_type_t::open, token);
		if (open) {
			auto prefix = consume_text(token);
			auto name = try_consume(lextoken_type_t::name, token);
			auto pattern = try_consume(lextoken_type_t::pattern, token);
			auto suffix = consume_text(token);

			must_consume(lextoken_type_t::close, token);
			optional_t<std::string> modifier = try_consume(lextoken_type_t::modifier, token);

			if (name) {
				result.push_back( create_token< Route_Param_Appender >(
					name.value(),
					prefix.value_or(""),
					suffix.value_or(""),
					create_pattern( pattern.value_or(defaultPattern) ),
					modifier.value_or("") ) );
			}
			else {
				if (pattern) {
					result.push_back( create_token< Route_Param_Appender >(
						key++,
						prefix.value_or(""),
						suffix.value_or(""),
						create_pattern( pattern.value() ),
						modifier.value_or("") ) );
				}
				else {
					result.push_back( create_token< Route_Param_Appender >(
						"",
						prefix.value_or(""),
						suffix.value_or(""),
						create_pattern( defaultPattern ),
						modifier.value_or("") ) );
				}
			}
			continue;
		}
		must_consume(lextoken_type_t::end, token);
	}
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

		std::string route{options.starting() ? "^" : ""};
		auto & param_appender_sequence = result.m_param_appender_sequence;

		// The number of capture groups in resultin regex
		// 1 is for match of a route itself.
		std::size_t captured_groups_count = 1 ;
		const auto & delimiter = "[" + escape_string( options.delimiter() ) + "]";
		const auto & ends_with = "[" + options.make_ends_with() + "]";

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
					RESTINIO_FMT_FORMAT_STRING(
						"too many parameter to capture from route: {}, while {} "
						"is the maximum" ),
					captured_groups_count,
					Regex_Engine::max_capture_groups() ) };
		}


		if( options.ending() )
		{
			if( !options.strict() )
			{
				route += delimiter + "?";
			}

			if( options.make_ends_with() == "$" )
				route += '$';
			else
				route += "(?=" + ends_with + ")";
		}
		else
		{
			if( !options.strict() )
				route += "(?:" + delimiter + "(?=" + ends_with + "))?";

			if( !tokens.empty() &&
				!tokens.back()->is_end_delimited( options.prefixes() ) )
				route += "(?=" + delimiter + "|" + ends_with + ")";
		}

		result.m_regex = Regex_Engine::compile_regex(route, options.sensitive() );
	}
	catch( const std::exception & ex )
	{
		throw exception_t{
			fmt::format(
					RESTINIO_FMT_FORMAT_STRING( "unable to process route \"{}\": {}" ),
					fmtlib_tools::streamed( path ), ex.what() ) };
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
