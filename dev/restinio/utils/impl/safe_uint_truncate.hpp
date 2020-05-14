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
	static std::size_t
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
	static std::size_t
	truncate(std::uint64_t v) { return static_cast<std::size_t>(v); }
};

/*!
 * \brief Helper function for truncating uint64 to std::size_t with
 * exception if that truncation will lead to data loss.
 *
 * A check of \a v is performed only if std::size_t has less capacity
 * than std::uint64_t (for example on 32-bit systems).
 *
 * \throw std::runtime_error if the value of \a v can't truncated to
 * std::size_t without loss of data.
 *
 * \since
 * v.0.4.1
 */
inline std::size_t
uint64_to_size_t(std::uint64_t v)
{
	return safe_uint64_to_size_t<(sizeof(std::uint64_t) > sizeof(std::size_t))>::truncate(v);
}

} /* namespace impl */

} /* namespace utils */

} /* namespace restinio */

