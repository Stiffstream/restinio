/*
 * restinio
 */

/*!
 * Helpers for extraction group of bits from numbers.
 */

#pragma once

namespace restinio {

namespace impl {

namespace bitops {

namespace details {

template< unsigned BITS_TO_EXTRACT, typename T >
struct mask_bits;

template< typename T >
struct mask_bits< 4, T >
{
	static constexpr T mask() { return static_cast<T>(0xFu); }
};

template< typename T >
struct mask_bits< 8, T >
{
	static constexpr T mask() { return static_cast<T>(0xFFu); }
};

template< typename T >
struct bits_count;

template<>
struct bits_count<unsigned char> { static constexpr unsigned count = 8u; };

template<>
struct bits_count<char> { static constexpr unsigned count = 8u; };

} /* namespace details */

template<
	typename T,
	unsigned SHIFT,
	unsigned BITS_TO_EXTRACT = details::bits_count<T>::count,
	typename F = unsigned int >
T
n_bits_from( F value )
{
	return static_cast<T>(value >> SHIFT)
			& details::mask_bits<BITS_TO_EXTRACT, T>::mask();
}

} /* namespace bitops */

} /* namespace impl */

} /* namespace restinio */

