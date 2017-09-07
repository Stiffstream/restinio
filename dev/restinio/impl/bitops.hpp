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
struct mask_bits< 6, T >
{
	static constexpr T mask() { return static_cast<T>(0x3Fu); }
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

/*!
 * \brief Extract N bits from a bigger integer value.
 *
 * Usage example:
 * \code
 * // Extract 8 bits as unsigned char from bits 24..31 in uint32_t.
 * const std::uint32_t v1 = some_uint_value();
 * const auto u8 = n_bits_from<std::uint8_t, 24>(v1);
 *
 * // Extract 6 bits as char from bits 12..17 in uint32_t.
 * const auto ch = n_bits_from<char, 12, 6>(v1);
 *
 * // Extract 4 bits as unsigned int from bits 32..35 in uint64_t.
 * const std::uint64_t v2 = some_uint64_value();
 * const auto ui = n_bits_from<unsigned int, 32, 4>(v2);
 * \endcode
 * 
 */
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

