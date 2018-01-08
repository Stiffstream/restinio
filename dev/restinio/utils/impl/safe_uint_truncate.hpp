/*
 * restinio
 */

/*!
 * \file
 * \brief Helpers for safe truncation of unsigned integers.
 */

#pragma once

#include <stdexcept>
#include <type_traits>
#include <limits>
#include <cstddef>
#include <cstdint>

namespace restinio {

namespace utils {

namespace impl {

template<bool Is_Uint64_Longer>
struct safe_uint64_to_size_t {};

template<>
struct safe_uint64_to_size_t<true> {
	static constexpr std::size_t
	truncate(std::uint64_t v)
	{
		if( v > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max()) )
			throw std::runtime_error( "64-bit value can't be safely truncated "
					"into std::size_t type" );
		return static_cast<std::size_t>(v);
	}
};

template<>
struct safe_uint64_to_size_t<false> {
	static constexpr std::size_t
	truncate(std::uint64_t v) { return static_cast<std::size_t>(v); }
};

inline constexpr std::size_t
uint64_to_size_t(std::uint64_t v)
{
	return safe_uint64_to_size_t<(sizeof(std::uint64_t) > sizeof(std::size_t))>::truncate(v);
}

} /* namespace impl */

} /* namespace utils */

} /* namespace restinio */

