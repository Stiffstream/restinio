/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to IP blockers.
 *
 * @since v.0.5.1
 */

#pragma once

#include <restinio/common_types.hpp>

namespace restinio
{

namespace ip_blocker
{

//
// inspection_result_t
//
/*!
 * @brief Enumeration of result of inspecting new incoming connection.
 *
 * @since v.0.5.1
 */
enum class inspection_result_t
{
	//! New connection is disabled and should be closed.
	deny,
	//! New connection is allowed to be processed further.
	allow
};

/*!
 * @brief Shorthand for inspection_result_t::deny.
 *
 * @since v.0.5.1
 */
inline constexpr inspection_result_t
deny() noexcept { return inspection_result_t::deny; }

/*!
 * @brief Shorthand for inspection_result_t::allow.
 *
 * @since v.0.5.1
 */
inline constexpr inspection_result_t
allow() noexcept { return inspection_result_t::allow; }

//
// incoming_info_t
//
/*!
 * @brief An information about new incoming connection to be passed
 * to IP-blocker object.
 *
 * @since v.0.5.1
 */
class incoming_info_t
{
	endpoint_t m_remote_endpoint;

public :
	//! Initializing constructor.
	incoming_info_t(
		endpoint_t remote_endpoint )
		:	m_remote_endpoint{ remote_endpoint }
	{}

	//! Remote endpoint of the new connection.
	endpoint_t
	remote_endpoint() const noexcept { return m_remote_endpoint; }
};

//
// noop_ip_blocker_t
//
/*!
 * @brief The default no-op IP-blocker.
 *
 * This type is used for ip_blocker_t trait by default.
 * 
 * NOTE. When this type if used no calls to IP-blocker will be generated.
 * It means that there won't be any performance penalties related to
 * invoking of IP-blocker's inspect() method.
 *
 * @since v.0.5.1
 */
struct noop_ip_blocker_t
{
	// empty type by design.
};

} /* namespace ip_blocker */

} /* namespace restinio */

