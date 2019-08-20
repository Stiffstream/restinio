/*
	restinio
*/

/*!
	Convert strings to numeric types.
*/

#pragma once

#include <cctype>
#include <string>
#include <limits>
#include <stdexcept>
#include <algorithm>

#include <restinio/impl/include_fmtlib.hpp>

#include <restinio/string_view.hpp>
#include <restinio/exception.hpp>

#include "from_string_details.ipp"

namespace restinio
{

namespace utils
{

//! Read int values.
//! \{

inline void
read_value( std::int64_t & v, const char * data, std::size_t size )
{
	v = details::parse_integer< details::int64_parse_traits_t >( data, data + size );
}

inline void
read_value( std::uint64_t & v, const char * data, std::size_t size )
{
	v = details::parse_integer< details::uint64_parse_traits_t >( data, data + size );
}

inline void
read_value( std::int32_t & v, const char * data, std::size_t size )
{
	v = details::parse_integer< details::int32_parse_traits_t >( data, data + size );
}

inline void
read_value( std::uint32_t & v, const char * data, std::size_t size )
{
	v = details::parse_integer< details::uint32_parse_traits_t >( data, data + size );
}

inline void
read_value( std::int16_t & v, const char * data, std::size_t size )
{
	v = details::parse_integer< details::int16_parse_traits_t >( data, data + size );
}

inline void
read_value( std::uint16_t & v, const char * data, std::size_t size )
{
	v = details::parse_integer< details::uint16_parse_traits_t >( data, data + size );
}

inline void
read_value( std::int8_t & v, const char * data, std::size_t size )
{
	v = details::parse_integer< details::int8_parse_traits_t >( data, data + size );
}

inline void
read_value( std::uint8_t & v, const char * data, std::size_t size )
{
	v = details::parse_integer< details::uint8_parse_traits_t >( data, data + size );
}
//! \}


//! Read float values.
//! \{
inline void
read_value( float & v, const char * data, std::size_t size )
{
	std::string buf{ data, size };

	v = std::stof( buf );
}

inline void
read_value( double & v, const char * data, std::size_t size )
{
	std::string buf{ data, size };

	v = std::stod( buf );
}
//! \}

//! Get a value from string.
template < typename Value_Type >
Value_Type
from_string( string_view_t s )
{
	Value_Type result;

	read_value( result, s.data(), s.length() );

	return result;
}

//! Get a value from string.
template <>
inline std::string
from_string< std::string >( string_view_t s )
{
	return std::string{ s.data(), s.size() };
}

//! Get a value from string_view.
template <>
inline string_view_t
from_string< string_view_t >( string_view_t s )
{
	return s;
}

// //! Get a value from string.
// template < typename Value_Type >
// Value_Type
// from_string( const std::string & s )
// {
// 	return from_string< Value_Type >( string_view_t{ s.data(), s.size() } );
// }

} /* namespace utils */

} /* namespace restinio */
