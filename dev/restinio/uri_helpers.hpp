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

#include <restinio/exception.hpp>
#include <restinio/utils/percent_encoding.hpp>

namespace restinio
{

namespace impl
{

inline const char *
modified_memchr( int chr , const char * from, const char * to )
{
	const char * result =
		static_cast< const char * >( std::memchr( from, chr, to - from ) );

	return result ? result : to;
}

} /* namespace impl */


template < typename Table = std::unordered_map< std::string, std::string > >
Table
parse_query_string( const std::string & query_string )
{
	const char * const very_first_pos = query_string.data();
	const char * query_remainder = very_first_pos;
	const char * query_end = query_remainder + query_string.size();

	Table result;

	query_remainder = impl::modified_memchr( '?', query_remainder, query_end );

	if( query_end > query_remainder )
	{
		// Skip '?'.
		++query_remainder;

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
						std::distance(very_first_pos, query_remainder) ) };
			}

			const char * const amp_symbol_or_end =
				impl::modified_memchr( '&', eq_symbol + 1, query_end );

			// Handle next pair of parameters found.
			std::string key = utils::unescape_percent_encoding(
					query_string.substr(
							std::distance(very_first_pos, query_remainder ),
							std::distance( query_remainder, eq_symbol ) ) );
			std::string value = utils::unescape_percent_encoding(
					query_string.substr(
							std::distance(very_first_pos, eq_symbol + 1),
							std::distance(eq_symbol + 1, amp_symbol_or_end ) ) );

			result.emplace( std::move(key), std::move(value) );

			query_remainder = amp_symbol_or_end + 1;
		}
	}

	return result;
}

} /* namespace restinio */
