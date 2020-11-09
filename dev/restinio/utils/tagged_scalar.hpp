/*
 * RESTinio
 */

/*!
 * @file
 * @brief Helper template for defining tagged scalar types.
 *
 * @since v.0.6.12
 */

#pragma once

#include <restinio/compiler_features.hpp>

#include <type_traits>

namespace restinio
{

namespace utils
{

//
// tagged_scalar_t
//
/*!
 * @brief Helper template for defining tagged scalar types.
 *
 * Usage example:
 * @code
 * struct max_parallel_connections_tag {};
 * using max_parallel_connections_t = tagged_scalar_t<
 * 		std::size_t, max_parallel_connections_tag >;
 *
 * struct max_active_accepts_tag {};
 * using max_active_accepts_t = tagged_scalar_t<
 * 		std::size_t, max_active_accepts_tag >;
 *
 * class limiter_t
 * {
 * public:
 * 	limiter_t(
 * 		max_parallel_connections_t parallel_connections,
 * 		max_active_accepts_t active_accepts);
 * 	...
 * };
 * @endcode
 *
 * @since v.0.6.12
 */
template< typename Scalar, typename Tag >
class tagged_scalar_t
{
	static_assert( std::is_scalar<Scalar>::value,
			"Scalar is expected to be scalar type" );

	Scalar m_value;

public:
	constexpr explicit tagged_scalar_t( Scalar value ) noexcept
		:	m_value{ value }
	{}

	RESTINIO_NODISCARD
	constexpr Scalar
	value() const noexcept { return m_value; }
};

} /* namespace utils */

} /* namespace restinio */

