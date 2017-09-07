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

} /* namespace details */

template< typename T, unsigned BITS_TO_EXTRACT, unsigned SHIFT, typename F >
T
n_bits_from( F value )
{
	return static_cast<T>(value >> SHIFT)
			& details::mask_bits<BITS_TO_EXTRACT, T>::mask();
}

} /* namespace bitops */

} /* namespace impl */

} /* namespace restinio */

