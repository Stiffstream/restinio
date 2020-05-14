/*
 * restinio
 */

/*!
 * Helpers for extraction group of bits from numbers.
 */

#pragma once

#include <cstdint>

namespace restinio {

namespace utils{

namespace impl {

namespace bitops {

namespace details {

template< typename T >
constexpr T mask( unsigned bits_to_extract )
{
	return bits_to_extract <= 1u ? T{1} :
		static_cast<T>((mask<T>(bits_to_extract-1) << 1) | T{1});
}

template< typename T >
struct bits_count;

template<>
struct bits_count<std::uint8_t> { static constexpr unsigned count = 8u; };

template<>
struct bits_count<std::int8_t> { static constexpr unsigned count = 8u; };

template<>
struct bits_count<char> { static constexpr unsigned count = 8u; };

template<>
struct bits_count<std::uint16_t> { static constexpr unsigned count = 16u; };

template<>
struct bits_count<std::int16_t> { static constexpr unsigned count = 16u; };

template<>
struct bits_count<std::uint32_t> { static constexpr unsigned count = 32u; };

template<>
struct bits_count<std::int32_t> { static constexpr unsigned count = 32u; };

template<>
struct bits_count<std::uint64_t> { static constexpr unsigned count = 64u; };

template<>
struct bits_count<std::int64_t> { static constexpr unsigned count = 64u; };

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
	unsigned Shift,
	unsigned Bits_To_Extract = details::bits_count<T>::count,
	typename F = unsigned int >
T
n_bits_from( F value )
{
	return static_cast<T>(value >> Shift) & details::mask<T>(Bits_To_Extract);
}

} /* namespace bitops */

} /* namespace impl */

} /* namespace utils */

} /* namespace restinio */
