/*
	restinio
*/

/*!
	escape functions.
*/

#pragma once

#include <string>
#include <unordered_map>

#include <fmt/format.h>
#include <fmt/ostream.h>

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

//! Parse query key-value parts.
/*!
	\deprecated Obsolete in v.0.4.1. Use restinio::parse_query() instead.

	\attention Because this function is obsolete it doesn't receive fixes and
	new features of restinio::parse_query().
*/
[[deprecated("use restinio::parse_query() instead")]]
inline query_string_params_t
parse_query_string( string_view_t query_string )
{
	const char * const very_first_pos = query_string.data();
	const char * query_remainder = very_first_pos;
	const char * query_end = query_remainder + query_string.size();

	std::unique_ptr< char[] > data_buffer;
	query_string_params_t::parameters_container_t parameters;

	query_remainder = impl::modified_memchr( '?', query_remainder, query_end );

	if( query_end > query_remainder )
	{
		// Skip '?'.
		++query_remainder;

		const auto params_offset = static_cast<std::size_t>(
				std::distance( very_first_pos, query_remainder ));

		{
			const auto data_size = static_cast<std::size_t>(
					query_end - query_remainder);
			data_buffer.reset( new char[ data_size] );
			std::memcpy( data_buffer.get(), query_remainder, data_size );

			// Work with created buffer:
			query_remainder = data_buffer.get();
			query_end = query_remainder + data_size;
		}

		query_end = impl::modified_memchr( '#', query_remainder, query_end );

		while( query_end > query_remainder )
		{
			const char * const eq_symbol =
				impl::modified_memchr( '=', query_remainder, query_end );

			if( query_end == eq_symbol )
			{
				throw exception_t{
					fmt::format(
						"invalid format of key-value pairs in query_string: {}, "
						"no '=' symbol starting from position {}",
						query_string,
						params_offset + static_cast<std::size_t>(
							std::distance(
								static_cast<const char *>( data_buffer.get() ),
								query_remainder) )) };
			}

			const char * const amp_symbol_or_end =
				impl::modified_memchr( '&', eq_symbol + 1, query_end );

			// Handle next pair of parameters found.
			string_view_t key{
							query_remainder,
							utils::inplace_unescape_percent_encoding(
								const_cast< char * >( query_remainder ), // Legal: we are refering buffer.
								static_cast< std::size_t >(
									std::distance( query_remainder, eq_symbol ) ) ) };

			string_view_t value{
							eq_symbol + 1,
							utils::inplace_unescape_percent_encoding(
								const_cast< char * >( eq_symbol + 1 ), // Legal: we are refering buffer.
								static_cast< std::size_t >(
									std::distance( eq_symbol + 1, amp_symbol_or_end ) ) ) };

			parameters.emplace_back( std::move( key ), std::move( value ) );

			query_remainder = amp_symbol_or_end + 1;
		}
	}

	return query_string_params_t{ std::move( data_buffer ), std::move( parameters ) };
}

namespace parse_query_traits
{

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
using restinio_defaults = restinio::utils::restinio_default_unescape_traits;

/*!
 * @brief Traits for parsing a query string in JavaScript-compatible mode.
 *
 * In that mode unexcaped asterisk is alowed.
 *
 * Usage example:
 * @code
 * auto result = restinio::parse_query<restinio::parse_query_traits::javascript_compatible>("name=A*");
 * @endcode
 *
 * @since v.0.4.9.1
 */
using javascript_compatible = restinio::utils::javascript_compatible_unescape_traits;

} /* namespace parse_query_traits */

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
inline query_string_params_t
parse_query(
	//! Query part of the request target.
	string_view_t original_query_string )
{
	constexpr const string_view_t separators{ "&;", 2u };

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
					throw exception_t{
						fmt::format(
							"invalid format of key-value pairs in query_string: {}, "
							"no '=' symbol starting from position {}",
							original_query_string,
							pos ) };
				else
				{
					// Query string contains only tag (web beacon).
					const auto tag_size =
							utils::inplace_unescape_percent_encoding< Parse_Traits >(
									&data_buffer[ pos ],
									end_pos - pos );

					const string_view_t tag = work_query_string.substr(
							pos, tag_size );

					return query_string_params_t{ std::move( data_buffer ), tag };
				}
			}

			const auto eq_pos_next = eq_pos + 1u;
			auto separator_pos = work_query_string.find_first_of(
					separators, eq_pos_next );
			if( string_view_t::npos == separator_pos )
				separator_pos = work_query_string.size();

			// Handle next pair of parameters found.
			string_view_t key{
					&data_buffer[ pos ],
					utils::inplace_unescape_percent_encoding< Parse_Traits >(
							&data_buffer[ pos ],
							eq_pos - pos )
			};

			string_view_t value{
					&data_buffer[ eq_pos_next ],
					utils::inplace_unescape_percent_encoding< Parse_Traits >(
							&data_buffer[ eq_pos_next ],
							separator_pos - eq_pos_next )
			};

			parameters.emplace_back( std::move( key ), std::move( value ) );

			pos = separator_pos + 1u;
		}
	}

	return query_string_params_t{
			std::move( data_buffer ),
			std::move( parameters )
	};
}

} /* namespace restinio */
