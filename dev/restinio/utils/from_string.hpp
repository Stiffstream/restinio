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

#include <restinio/string_view.hpp>
#include <restinio/exception.hpp>

namespace restinio
{

namespace utils
{

//! Read int values.
//! \{

inline void
read_value( std::int32_t v, const char * data, std::size_t size )
{
	// SSO must be enough for all integer types.
	std::string buf{ data, size };

	v = std::stol( buf );
}

inline void
read_value( std::uint32_t v, const char * data, std::size_t size )
{
	// SSO must be enough for all integer types.
	std::string buf{ data, size };

	v = std::stoul( buf );
}

inline void
read_value( std::int64_t v, const char * data, std::size_t size )
{
	// SSO must be enough for all integer types.
	std::string buf{ data, size };

	v = std::stoll( buf );
}

inline void
read_value( std::uint64_t v, const char * data, std::size_t size )
{
	// SSO must be enough for all integer types.
	std::string buf{ data, size };

	v = std::stoull( buf );
}

inline void
read_value( std::int16_t v, const char * data, std::size_t size )
{
	std::int32_t res;
	read_value( res, data, size );

	if( std::numeric_limits< std::int16_t >::max() < res ||
		std::numeric_limits< std::int16_t >::min() < res )
	{
		throw std::out_of_range{ fmt::format( "invalid int16_t value: {}", res ) };
	}

	v = static_cast< std::int16_t >( res );
}

inline void
read_value( std::uint16_t v, const char * data, std::size_t size )
{
	std::uint32_t res;
	read_value( res, data, size );

	if( std::numeric_limits< std::uint16_t >::max() < res ||
		std::numeric_limits< std::uint16_t >::min() < res )
	{
		throw std::out_of_range{ fmt::format( "invalid uint16_t value: {}", res ) };
	}

	v = static_cast< std::uint16_t >( res );
}

inline void
read_value( std::int8_t v, const char * data, std::size_t size )
{
	std::int32_t res;
	read_value( res, data, size );

	if( std::numeric_limits< std::int8_t >::max() < res ||
		std::numeric_limits< std::int8_t >::min() < res )
	{
		throw std::out_of_range{ fmt::format( "invalid int8_t value: {}", res ) };
	}

	v = static_cast< std::int8_t >( res );
}

inline void
read_value( std::uint8_t v, const char * data, std::size_t size )
{
	std::uint32_t res;
	read_value( res, data, size );

	if( std::numeric_limits< std::uint8_t >::max() < res ||
		std::numeric_limits< std::uint8_t >::min() < res )
	{
		throw std::out_of_range{ fmt::format( "invalid uint8_t value: {}", res ) };
	}

	v = static_cast< std::uint8_t >( res );
}
//! \}


//! Read float values.
//! \{
inline void
read_value( float v, const char * data, std::size_t size )
{
	std::string buf{ data, size };

	v = std::stof( buf );
}

inline void
read_value( double v, const char * data, std::size_t size )
{
	std::string buf{ data, size };

	v = std::stod( buf );
}
//! \}


//! Get a value from string.
template < typename Value_Type >
Value_Type
from_string( const string_view_t & s )
{
	Value_Type result;

	read_value( result, s.data(), s.length() );

	return result;
}

//! Get a value from string.
template <>
inline std::string
from_string< std::string >( const string_view_t & s )
{
	return std::string{ s.data(), s.size() };
}

} /* namespace utils */

} /* namespace restinio */
