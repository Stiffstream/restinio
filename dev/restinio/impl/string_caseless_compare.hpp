/*
 * RESTinio
 */

/*!
 * @file
 * @brief Helpers for caseless comparison of strings.
 *
 * @since v.0.6.1
 */

#pragma once

#include <restinio/impl/to_lower_lut.hpp>
#include <restinio/string_view.hpp>

namespace restinio
{

namespace impl
{

namespace string_caseless_compare_details
{

constexpr auto
uchar_at( const char * const from, const std::size_t at ) noexcept
{
	return static_cast< unsigned char >( from[ at ] );
}

} /* namespace string_caseless_compare_details */

//
// is_equal_caseless()
//

//! Comparator for fields names.
inline bool
is_equal_caseless(
	const char * a,
	const char * b,
	std::size_t size ) noexcept
{
	using namespace string_caseless_compare_details;

	const unsigned char * const table = to_lower_lut< unsigned char >();

	for( std::size_t i = 0; i < size; ++i )
		if( table[ uchar_at( a, i ) ] != table[ uchar_at( b, i ) ] )
			return false;

	return true;
}

//
// is_equal_caseless()
//

//! Comparator for fields names.
inline bool
is_equal_caseless(
	const char * a,
	std::size_t a_size,
	const char * b,
	std::size_t b_size ) noexcept
{
	if( a_size == b_size )
	{
		return is_equal_caseless( a, b, a_size );
	}

	return false;
}

//
// is_equal_caseless()
//

//! Comparator for fields names.
inline bool
is_equal_caseless( string_view_t a, string_view_t b ) noexcept
{
	return is_equal_caseless( a.data(), a.size(), b.data(), b.size() );
}

} /* namespace impl */

} /* namespace restinio */

