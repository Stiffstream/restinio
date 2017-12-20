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

//
// query_string_params_t
//

//! Parameters container for query strings parameters.
class query_string_params_t final
{
	public:
		using parameters_container_t = std::vector< std::pair< string_view_t, string_view_t > >;

		query_string_params_t(
			std::unique_ptr< char[] > data_buffer,
			parameters_container_t parameters )
			:	m_data_buffer{ std::move( data_buffer ) }
			,	m_parameters{ std::move( parameters ) }
		{}

		query_string_params_t( query_string_params_t && ) = default;
		query_string_params_t & operator = ( query_string_params_t && ) = default;

		query_string_params_t( const query_string_params_t & ) = delete;
		const query_string_params_t & operator = ( const query_string_params_t & ) = delete;

		//! Get parameter.
		string_view_t
		operator [] ( string_view_t key ) const
		{
			return find_parameter_with_check( key ).second;
		}

		//! Check parameter.
		bool
		has( string_view_t key ) const
		{
			return m_parameters.end() != find_parameter( key );
		}

		//! Get the size of parameters.
		auto size() const { return m_parameters.size(); }

		//! Iterate parameters.
		//! //{
		parameters_container_t::const_iterator
		begin() const
		{
			return m_parameters.begin();
		}

		parameters_container_t::const_iterator
		end() const
		{
			return m_parameters.end();
		}
		//! //}

	private:
		parameters_container_t::const_iterator
		find_parameter( string_view_t key ) const
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
				throw exception_t{
					fmt::format(
						"invalid parameter name: {}",
						std::string{ key.data(), key.size() } ) };

			return *it;
		}

		//! Shared buffer for string_view of named parameterts names.
		std::unique_ptr< char[] > m_data_buffer;
		parameters_container_t m_parameters;
};

//! Cast query string parameter to a given type.
template < typename Value_Type >
Value_Type
get( const query_string_params_t & params, string_view_t key )
{
	return get< Value_Type >( params[ key ] );
}

inline query_string_params_t
parse_query_string( const std::string & query_string )
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

		const std::size_t params_offset = std::distance( very_first_pos, query_remainder );

		{
			const auto data_size = query_end - query_remainder;
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
						params_offset +
							std::distance(
								static_cast<const char *>( data_buffer.get() ),
								query_remainder) ) };
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

} /* namespace restinio */
