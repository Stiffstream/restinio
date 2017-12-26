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
	digits_representation_max_size()
	{
		return 3;
	}

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
	}
};

struct uint8_parse_traits_t
{
	using type_t = std::uint8_t;

	static constexpr std::size_t
	digits_representation_max_size()
	{
		return 3;
	}

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
	}
};

struct int16_parse_traits_t
{
	using type_t = std::int16_t;

	static constexpr std::size_t
	digits_representation_max_size()
	{
		return 5;
	}

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
	}
};

struct uint16_parse_traits_t
{
	using type_t = std::uint16_t;

	static constexpr std::size_t
	digits_representation_max_size()
	{
		return 5;
	}

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
	}
};

struct int32_parse_traits_t
{
	using type_t = std::int32_t;

	static constexpr std::size_t
	digits_representation_max_size()
	{
		return 10;
	}

	static const char *
	min_representation()
	{
		static constexpr char r[] = "2147483648";
		return r;
	}

	static const char *
	max_representation()
	{
		static constexpr char r[] = "2147483647";
		return r;
	}

	static const char * type_name()
	{
		static constexpr char r[] = "int32_t";
		return r;
	}
};

struct uint32_parse_traits_t
{
	using type_t = std::uint32_t;

	static constexpr std::size_t
	digits_representation_max_size()
	{
		return 10;
	}

	static const char *
	min_representation()
	{
		static constexpr char r[] = "0";
		return r;
	}

	static const char *
	max_representation()
	{
		static constexpr char r[] = "4294967295";
		return r;
	}

	static const char * type_name()
	{
		static constexpr char r[] = "uint32_t";
		return r;
	}
};

struct int64_parse_traits_t
{
	using type_t = std::int64_t;

	static constexpr std::size_t
	digits_representation_max_size()
	{
		return 19;
	}

	static const char *
	min_representation()
	{
		static constexpr char r[] = "9223372036854775808";
		return r;
	}

	static const char *
	max_representation()
	{
		static constexpr char r[] = "9223372036854775807";
		return r;
	}

	static const char * type_name()
	{
		static constexpr char r[] = "int64_t";
		return r;
	}
};

struct uint64_parse_traits_t
{
	using type_t = std::uint64_t;

	static constexpr std::size_t
	digits_representation_max_size()
	{
		return 20;
	}

	static const char *
	min_representation()
	{
		static constexpr char r[] = "0";
		return r;
	}

	static const char *
	max_representation()
	{
		static constexpr char r[] = "18446744073709551615";
		return r;
	}

	static const char * type_name()
	{
		static constexpr char r[] = "uint64_t";
		return r;
	}
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
			result = result*10 - mapping_table[ static_cast< std::size_t >( *data_begin++ ) ];
		}
	else
		while( data_begin != data_end )
		{
			result = result*10 + mapping_table[ static_cast< std::size_t >( *data_begin++ ) ];
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
		result = result * 10 + mapping_table[ static_cast< std::size_t >( *data_begin++ ) ];
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

	const auto representation_size = static_cast< std::size_t >( data_end - data_begin );

	if( 0 == representation_size )
		throw exception_t{ fmt::format( "invalid {} value: empty string", Traits::type_name() ) };

	if( Traits::digits_representation_max_size() < representation_size )
		throw exception_t{
			fmt::format(
				"invalid {} value: max digits for type is {}",
				Traits::type_name(),
				Traits::digits_representation_max_size() ) };

	const std::uint8_t * const mapping_table = digits_mapping< std::uint8_t >();

	if( std::any_of(
			data_begin,
			data_end,
			[&]( auto d ){ return 0xFF == mapping_table[ static_cast< std::size_t >( d ) ]; } ) )
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

} /* namespace details */

} /* namespace utils */

} /* namespace restinio */
