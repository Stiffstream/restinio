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

#include <fmt/format.h>

#include <restinio/string_view.hpp>
#include <restinio/exception.hpp>

namespace restinio
{

namespace utils
{

namespace details
{

template< typename C >
const C * digits_mapping()
{
	static constexpr C table[] = {
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

		0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
			return table;
}

struct int8_parse_traits_t
{
	using type_t = std::int8_t;

	static constexpr std::size_t
	digits_representation_max_size(){
		return 3;
	};

	static const char *
	min_representation()
	{
		static constexpr char r[] = "128";
		return r;
	}

	static const char *
	max_representation()
	{
		static constexpr char r[] = "127";
		return r;
	}

	static const char * type_name()
	{
		static constexpr char r[] = "int8_t";
		return r;
	};
};

struct uint8_parse_traits_t
{
	using type_t = std::uint8_t;

	static constexpr std::size_t
	digits_representation_max_size(){
		return 3;
	};

	static const char *
	min_representation()
	{
		static constexpr char r[] = "0";
		return r;
	}

	static const char *
	max_representation()
	{
		static constexpr char r[] = "255";
		return r;
	}

	static const char * type_name()
	{
		static constexpr char r[] = "uint8_t";
		return r;
	};
};

struct int16_parse_traits_t
{
	using type_t = std::int16_t;

	static constexpr std::size_t
	digits_representation_max_size(){
		return 5;
	};

	static const char *
	min_representation()
	{
		static constexpr char r[] = "32768";
		return r;
	}

	static const char *
	max_representation()
	{
		static constexpr char r[] = "32767";
		return r;
	}

	static const char * type_name()
	{
		static constexpr char r[] = "int16_t";
		return r;
	};
};

struct uint16_parse_traits_t
{
	using type_t = std::uint16_t;

	static constexpr std::size_t
	digits_representation_max_size(){
		return 5;
	};

	static const char *
	min_representation()
	{
		static constexpr char r[] = "0";
		return r;
	}

	static const char *
	max_representation()
	{
		static constexpr char r[] = "65535";
		return r;
	}

	static const char * type_name()
	{
		static constexpr char r[] = "uint16_t";
		return r;
	};
};


template < typename Integer >
Integer
parse_integer_no_checks(
	const std::uint8_t * const mapping_table,
	const char * data_begin,
	const char * data_end,
	bool apply_minus_sign,
	std::true_type /* is signed */)
{
	Integer result = 0;

	if( apply_minus_sign )
		while( data_begin != data_end )
		{
			result = result*10 - mapping_table[ *data_begin++ ];
		}
	else
		while( data_begin != data_end )
		{
			result = result*10 + mapping_table[ *data_begin++ ];
		}

	return result;
}

template < typename Integer >
Integer
parse_integer_no_checks(
	const std::uint8_t * const mapping_table,
	const char * data_begin,
	const char * data_end,
	bool ,
	std::false_type /* is signed */ )
{
	Integer result = 0;

	while( data_begin != data_end )
	{
		result = result * 10 + mapping_table[ *data_begin++ ];
	}

	return result;
}

template < typename Traits >
typename Traits::type_t
parse_integer( const char * data_begin, const char * data_end )
{
	bool apply_minus_sign = false;
	if( '-' == *data_begin )
	{
		if( !std::is_signed< typename Traits::type_t >::value )
		{
			throw exception_t{
				fmt::format(
					"invalid {} value: unsigned starts with minus",
					Traits::type_name() ) };
		}

		// else:
		apply_minus_sign = true;
		++data_begin;
	}
	else if( '+' == *data_begin )
	{
		++data_begin;
	}

	const auto representation_size = data_end - data_begin;

	if( 0 == representation_size )
		throw exception_t{ fmt::format( "invalid {} value: empty string", Traits::type_name() ) };

	if( Traits::digits_representation_max_size() < representation_size )
		throw exception_t{
			fmt::format(
				"invalid {} value: max digits for type is {}",
				Traits::type_name(),
				Traits::digits_representation_max_size() ) };

	const std::uint8_t * const mapping_table = digits_mapping< std::uint8_t >();

	if( std::any_of( data_begin, data_end, [&]( auto d ){ return 0xFF == mapping_table[ d ]; } ) )
	{
		throw exception_t{
			fmt::format( "invalid {} value: invalid digit", Traits::type_name() ) };
	}

	if( Traits::digits_representation_max_size() == representation_size )
	{
		const char * const posssible_max = apply_minus_sign ?
			Traits::min_representation() : Traits::max_representation();

		if( 0 < std::memcmp( data_begin, posssible_max, representation_size ) )
			throw std::out_of_range{
				fmt::format( "invalid {} value: out of range", Traits::type_name() ) };
	}

	using is_signed_t = typename std::is_signed< typename Traits::type_t >::type;

	return
		parse_integer_no_checks< typename Traits::type_t >(
			mapping_table,
			data_begin,
			data_end,
			apply_minus_sign,
			is_signed_t{} );
}

// 18446744073709551616
// -9223372036854775808
constexpr std::size_t max_decimal_digits_in_supported_int_types = 20;

} /* namespace details */

inline void
check_int_representation_size( const char * data, std::size_t size )
{
	if( details::max_decimal_digits_in_supported_int_types < size )
	{
		throw exception_t{
			fmt::format(
				"invalid value: {}",
				std::string{ data, size } ) };
	}
}

//! Read int values.
//! \{

inline void
read_value( std::int64_t & v, const char * data, std::size_t size )
{
	check_int_representation_size( data, size );

	// SSO must be enough for all integer types.
	std::string buf{ data, size };

	v = std::stoll( buf );
}

inline void
read_value( std::uint64_t & v, const char * data, std::size_t size )
{
	check_int_representation_size( data, size );


	// SSO must be enough for all integer types.
	std::string buf{ data, size };

	v = std::stoull( buf );
}

inline void
read_value( std::int32_t & v, const char * data, std::size_t size )
{
	std::int64_t res;
	read_value( res, data, size );

	if( std::numeric_limits< std::int32_t >::max() < res ||
		std::numeric_limits< std::int32_t >::min() > res )
	{
		throw std::out_of_range{ fmt::format( "invalid int32_t value: {}", res ) };
	}

	v = static_cast< std::int32_t >( res );
}

inline void
read_value( std::uint32_t & v, const char * data, std::size_t size )
{
	std::uint64_t res;
	read_value( res, data, size );

	if( std::numeric_limits< std::uint32_t >::max() < res ||
		std::numeric_limits< std::uint32_t >::min() > res )
	{
		throw std::out_of_range{ fmt::format( "invalid uint32_t value: {}", res ) };
	}

	v = static_cast< std::uint32_t >( res );
}

inline void
read_value( std::int16_t & v, const char * data, std::size_t size )
{
	// std::int32_t res;
	// read_value( res, data, size );

	// if( std::numeric_limits< std::int16_t >::max() < res ||
	// 	std::numeric_limits< std::int16_t >::min() > res )
	// {
	// 	throw std::out_of_range{ fmt::format( "invalid int16_t value: {}", res ) };
	// }

	// v = static_cast< std::int16_t >( res );
	v = details::parse_integer< details::int16_parse_traits_t >( data, data + size );
}

inline void
read_value( std::uint16_t & v, const char * data, std::size_t size )
{
	// std::uint32_t res;
	// read_value( res, data, size );

	// if( std::numeric_limits< std::uint16_t >::max() < res ||
	// 	std::numeric_limits< std::uint16_t >::min() > res )
	// {
	// 	throw std::out_of_range{ fmt::format( "invalid uint16_t value: {}", res ) };
	// }

	// v = static_cast< std::uint16_t >( res );
	v = details::parse_integer< details::uint16_parse_traits_t >( data, data + size );
}

inline void
read_value( std::int8_t & v, const char * data, std::size_t size )
{
	// std::int32_t res;
	// read_value( res, data, size );

	// if( std::numeric_limits< std::int8_t >::max() < res ||
	// 	std::numeric_limits< std::int8_t >::min() > res )
	// {
	// 	throw std::out_of_range{ fmt::format( "invalid int8_t value: {}", res ) };
	// }

	// v = static_cast< std::int8_t >( res );
	v = details::parse_integer< details::int8_parse_traits_t >( data, data + size );
}

inline void
read_value( std::uint8_t & v, const char * data, std::size_t size )
{
	// std::uint32_t res;
	// read_value( res, data, size );

	// if( std::numeric_limits< std::uint8_t >::max() < res ||
	// 	std::numeric_limits< std::uint8_t >::min() > res )
	// {
	// 	throw std::out_of_range{ fmt::format( "invalid uint8_t value: {}", res ) };
	// }

	// v = static_cast< std::uint8_t >( res );

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

//! Get a value from string.
template < typename Value_Type >
Value_Type
from_string( const std::string & s )
{
	return from_string< Value_Type >( string_view_t{ s } );
}

} /* namespace utils */

} /* namespace restinio */
