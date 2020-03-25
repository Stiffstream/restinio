/*
	restinio
*/

/*!
	escape functions.
*/

#pragma once

#include <string>
#include <unordered_map>

#include <restinio/impl/include_fmtlib.hpp>

#include <restinio/exception.hpp>
#include <restinio/utils/percent_encoding.hpp>
#include <restinio/optional.hpp>

namespace restinio
{

namespace impl
{

inline const char *
modified_memchr( int chr , const char * from, const char * to )
{
	const char * result = static_cast< const char * >(
			std::memchr( from, chr, static_cast<std::size_t>(to - from) ) );

	return result ? result : to;
}

} /* namespace impl */

//
// query_string_params_t
//

//! Parameters container for query strings parameters.
class query_string_params_t final
{
	public:
		using parameters_container_t = std::vector< std::pair< string_view_t, string_view_t > >;

		//! Constructor for the case when query string empty of
		//! contains a set of key-value pairs.
		query_string_params_t(
			std::unique_ptr< char[] > data_buffer,
			parameters_container_t parameters )
			:	m_data_buffer{ std::move( data_buffer ) }
			,	m_parameters{ std::move( parameters ) }
		{}

		//! Constructor for the case when query string contains only tag
		//! (web beacon).
		query_string_params_t(
			std::unique_ptr< char[] > data_buffer,
			optional_t< string_view_t > tag )
			:	m_data_buffer{ std::move( data_buffer ) }
			,	m_tag{ tag }
		{}

		query_string_params_t( query_string_params_t && ) = default;
		query_string_params_t & operator = ( query_string_params_t && ) = default;

		query_string_params_t( const query_string_params_t & ) = delete;
		query_string_params_t & operator = ( const query_string_params_t & ) = delete;

		//! Get parameter.
		string_view_t
		operator [] ( string_view_t key ) const
		{
			return find_parameter_with_check( key ).second;
		}

		//! Check parameter.
		bool
		has( string_view_t key ) const noexcept
		{
			return m_parameters.end() != find_parameter( key );
		}

		//! Get the value of a parameter if it exists.
		//! @since v.0.4.4
		optional_t< string_view_t >
		get_param( string_view_t key ) const noexcept
		{
			const auto it = find_parameter( key );

			return m_parameters.end() != it ?
				optional_t< string_view_t >{ it->second } :
				optional_t< string_view_t >{ nullopt };
		}

		//! Get the size of parameters.
		auto size() const noexcept { return m_parameters.size(); }

		//! Is there any parameters?
		//! @since v.0.4.8
		bool empty() const noexcept { return m_parameters.empty(); }

		//! @name Iterate parameters.
		//! @{
		parameters_container_t::const_iterator
		begin() const noexcept
		{
			return m_parameters.begin();
		}

		parameters_container_t::const_iterator
		end() const noexcept
		{
			return m_parameters.end();
		}
		//! @}

		//! Get the tag (web beacon) part.
		/*!
			A value of "tag" (also known as web beacon) is available only
			if URI looks like that:
			\verbatim
			http://example.com/resource?value
			\endverbatim
			In that case tag will contain `value`. For URI with different
			formats tag() will return empty optional.

			@since v.0.4.9
		*/
		auto tag() const noexcept { return m_tag; }

	private:
		parameters_container_t::const_iterator
		find_parameter( string_view_t key ) const noexcept
		{
			return
				std::find_if(
					m_parameters.begin(),
					m_parameters.end(),
					[&]( const auto p ){
						return key == p.first;
					} );
		}

		parameters_container_t::const_reference
		find_parameter_with_check( string_view_t key ) const
		{
			auto it = find_parameter( key );

			if( m_parameters.end() == it )
			{
				throw exception_t{
					fmt::format(
						"unable to find parameter \"{}\"",
						std::string{ key.data(), key.size() } ) };
			}

			return *it;
		}

		//! Shared buffer for string_view of named parameterts names.
		std::unique_ptr< char[] > m_data_buffer;
		parameters_container_t m_parameters;

		//! Tag (or web beacon) part.
		/*! @since v.0.4.9 */
		optional_t< string_view_t > m_tag;
};

//! Cast query string parameter to a given type.
template < typename Value_Type >
Value_Type
get( const query_string_params_t & params, string_view_t key )
{
	return get< Value_Type >( params[ key ] );
}

namespace parse_query_traits
{

namespace details
{

/*!
 * @brief Helper class to be reused in implementation of query-string
 * parsing traits.
 *
 * Implements `find_next_separator` method that recongnizes `&` and
 * `;` as `name=value` separators.
 *
 * @since v.0.6.5
 */
struct ampersand_and_semicolon_as_separators
{
	static string_view_t::size_type
	find_next_separator(
		string_view_t where,
		string_view_t::size_type start_from ) noexcept
	{
		return where.find_first_of( "&;", start_from );
	}
};

/*!
 * @brief Helper class to be reused in implementation of query-string
 * parsing traits.
 *
 * Implements `find_next_separator` method that recongnizes only `&`
 * `name=value` separator.
 *
 * @since v.0.6.5
 */
struct ampersand_only_as_separators
{
	static string_view_t::size_type
	find_next_separator(
		string_view_t where,
		string_view_t::size_type start_from ) noexcept
	{
		return where.find_first_of( '&', start_from );
	}
};

} /* namespace details */

/*!
 * @brief Traits for the default RESTinio parser for query string.
 *
 * The default RESTinio parser prohibit usage of unexcaped asterisk.
 *
 * @note
 * This traits type is used by default. It means that a call:
 * @code
 * auto result = restinio::parse_query<restinio::parse_query_traits::restinio_defaults>("name=value");
 * @endcode
 * is equivalent to:
 * @code
 * auto result = restinio::parse_query("name=value");
 * @endcode
 *
 * @since v.0.4.9.1
 */
struct restinio_defaults
	:	public restinio::utils::restinio_default_unescape_traits
	,	public details::ampersand_and_semicolon_as_separators
{};

/*!
 * @brief Traits for parsing a query string in JavaScript-compatible mode.
 *
 * In that mode several non-percent-encoded characters are allowed:
 * `-`, `.`, `~`, `_`, `*`, `!`, `'`, `(`, `)`
 *
 * Usage example:
 * @code
 * auto result = restinio::parse_query<restinio::parse_query_traits::javascript_compatible>("name=A*");
 * @endcode
 *
 * @since v.0.4.9.1
 */
struct javascript_compatible
	:	public restinio::utils::javascript_compatible_unescape_traits
	,	public details::ampersand_and_semicolon_as_separators
{};

/*!
 * @brief Traits for parsing a query string in
 * application/x-www-form-urlencoded mode.
 *
 * In that mode:
 *
 * - `name=value` pairs can be concatenated only by `&`;
 * - the following characters can only be used unescaped: `*` (0x2A), `-`
 *   (0x2D), `.` (0x2E), `_` (0x5F), `0`..`9` (0x30..0x39), `A`..`Z`
 *   (0x41..0x5A), `a`..`z` (0x61..0x7A);
 * - space character (0x20) should be replaced by + (0x2B);
 * - *all other characters should be represented as percent-encoded*.
 *
 * Reference for more details: https://url.spec.whatwg.org/#concept-urlencoded-byte-serializer
 *
 * Usage example:
 * @code
 * auto result = restinio::parse_query<restinio::parse_query_traits::x_www_form_urlencoded>("name=A*");
 * @endcode
 *
 * @since v.0.6.5
 */
struct x_www_form_urlencoded
	:	public restinio::utils::x_www_form_urlencoded_unescape_traits
	,	public details::ampersand_only_as_separators
{};

/*!
 * @brief Traits for parsing a query string in a very relaxed mode.
 *
 * In that mode all characters described in that rule from
 * [RCF3986](https://tools.ietf.org/html/rfc3986) can be used as unexceped:
@verbatim
query         = *( pchar / "/" / "?" )
pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
reserved      = gen-delims / sub-delims
gen-delims    = ":" / "/" / "?" / "#" / "[" / "]" / "@"
sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
                 / "*" / "+" / "," / ";" / "="
@endverbatim
 *
 * Additionaly this traits allows to use unexcaped space character.
 *
 * Note that despite that fact that symbols like `#`, `+`, `=` and `&` can be
 * used in non-percent-encoded form they play special role and are interpreted
 * special way. So such symbols should be percent-encoded if they are used as
 * part of name or value in query string.
 *
 * Only ampersand (`&`) can be used as `name=value` pairs separator.
 *
 * Usage example:
 * @code
 * auto result = restinio::parse_query<restinio::parse_query_traits::relaxed>("a=(&b=)&c=[&d=]&e=!&f=,&g=;");
 * @endcode
 *
 * @since v.0.6.5
 */
struct relaxed
	:	public restinio::utils::relaxed_unescape_traits
	,	public details::ampersand_only_as_separators
{};

} /* namespace parse_query_traits */

/*!
 * @brief Type that indicates a failure of an attempt of query-string parsing.
 *
 * @since v.0.6.5
 */
class parse_query_failure_t
{
	//! Description of a failure.
	std::string m_description;

public:
	parse_query_failure_t( std::string description )
		:	m_description{ std::move(description) }
	{}
	parse_query_failure_t(
		utils::unescape_percent_encoding_failure_t && failure )
		:	m_description{ failure.giveout_description() }
	{}

	//! Get a reference to the description of the failure.
	RESTINIO_NODISCARD
	const std::string &
	description() const noexcept { return m_description; }

	//! Get out the value of the description of the failure.
	/*!
	 * This method is intended for cases when this description should be move
	 * elsewhere (to another object like parse_query_failure_t or to some
	 * exception-like object).
	 */
	RESTINIO_NODISCARD
	std::string
	giveout_description() noexcept { return m_description; }
};

/*!
 * @brief Helper function for parsing query string.
 *
 * Unlike parse_query() function the try_parse_query() doesn't throw if
 * some unsupported character sequence is found.
 *
 * @note
 * Parsing traits should be specified explicitly.
 *
 * Usage example:
 * @code
 * auto result = restinio::try_parse_query<
 * 		restinio::parse_query_traits::javascript_compatible>("name=A*&flags=!");
 * if(!result) {
 * 	std::cerr << "Unable to parse query-string: " << result.error().description() << std::endl;
 * }
 * else {
 * 	const restinio::query_string_params_t & params = *result;
 * 	...
 * }
 * @endcode
 *
 * @attention
 * This function is not noexcept and can throw on other types of
 * failures (like unability to allocate a memory).
 *
 * @since v.0.6.5
 */
template< typename Parse_Traits >
RESTINIO_NODISCARD
expected_t< query_string_params_t, parse_query_failure_t >
try_parse_query(
	//! Query part of the request target.
	string_view_t original_query_string )
{
	std::unique_ptr< char[] > data_buffer;
	query_string_params_t::parameters_container_t parameters;

	if( !original_query_string.empty() )
	{
		// Because query string is not empty a new buffer should be
		// allocated and query string should be copied to it.
		data_buffer.reset( new char[ original_query_string.size() ] );
		std::memcpy(
				data_buffer.get(),
				original_query_string.data(),
				original_query_string.size() );

		// Work with created buffer:
		string_view_t work_query_string{
				data_buffer.get(), 
				original_query_string.size()
		};
		string_view_t::size_type pos{ 0 };
		const string_view_t::size_type end_pos = work_query_string.size();

		while( pos < end_pos )
		{
			const auto eq_pos = work_query_string.find_first_of( '=', pos );

			if( string_view_t::npos == eq_pos )
			{
				// Since v.0.4.9 we should check the presence of tag (web beacon)
				// in query string.
				// Tag can be the only item in query string.
				if( pos != 0u )
					// The query string has illegal format.
					return make_unexpected( parse_query_failure_t{
							fmt::format(
								"invalid format of key-value pairs in query_string, "
								"no '=' symbol starting from position {}",
								pos )
						} );
				else
				{
					// Query string contains only tag (web beacon).
					auto tag_unescape_result =
							utils::try_inplace_unescape_percent_encoding< Parse_Traits >(
									&data_buffer[ pos ],
									end_pos - pos );
					if( !tag_unescape_result )
						return make_unexpected( parse_query_failure_t{
								std::move(tag_unescape_result.error())
							} );

					const string_view_t tag = work_query_string.substr(
							pos, *tag_unescape_result );

					return query_string_params_t{ std::move( data_buffer ), tag };
				}
			}

			const auto eq_pos_next = eq_pos + 1u;
			auto separator_pos = Parse_Traits::find_next_separator(
					work_query_string, eq_pos_next );
			if( string_view_t::npos == separator_pos )
				separator_pos = work_query_string.size();

			// Handle next pair of parameters found.
			auto key_unescape_result =
					utils::try_inplace_unescape_percent_encoding< Parse_Traits >(
							&data_buffer[ pos ],
							eq_pos - pos );
			if( !key_unescape_result )
				return make_unexpected( parse_query_failure_t{
						std::move(key_unescape_result.error())
					} );

			auto value_unescape_result =
					utils::try_inplace_unescape_percent_encoding< Parse_Traits >(
							&data_buffer[ eq_pos_next ],
							separator_pos - eq_pos_next );
			if( !value_unescape_result )
				return make_unexpected( parse_query_failure_t{
						std::move(value_unescape_result.error())
					} );

			parameters.emplace_back(
					string_view_t{ &data_buffer[ pos ], *key_unescape_result },
					string_view_t{ &data_buffer[ eq_pos_next ], *value_unescape_result } );

			pos = separator_pos + 1u;
		}
	}

	return query_string_params_t{
			std::move( data_buffer ),
			std::move( parameters )
	};
}

//! Parse query key-value parts.
/*!
	Since v.0.4.9 this function correctly handles the following cases:

	- presence of tag (web beacon) in URI. For example, when URI looks like
	`http://example.com/resource?tag`. In that case value of the tag (web
	beacon) can be obtained via query_string_params_t::tag() method.
   References: [web beacon](https://en.wikipedia.org/wiki/Web_beacon) and
	[query-string-tracking](https://en.wikipedia.org/wiki/Query_string#Tracking);
	- usage of `;` instead of `&` as parameter separator.

	Since v.0.4.9.1 this function can be parametrized by parser traits. For
	example:
	@code
	auto result = restinio::parse_query<restinio::parse_query_traits::javascript_compatible>("name=A*");
	@endcode
*/
template< typename Parse_Traits = parse_query_traits::restinio_defaults >
RESTINIO_NODISCARD
query_string_params_t
parse_query(
	//! Query part of the request target.
	string_view_t original_query_string )
{
	auto r = try_parse_query< Parse_Traits >( original_query_string );
	if( !r )
		throw exception_t{ std::move(r.error().giveout_description()) };

	return std::move(*r);
}

} /* namespace restinio */
