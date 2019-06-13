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
//FIXME: document this!
enum class inspection_result_t
{
	deny,
	allow
};

//FIXME: document this!
inline constexpr inspection_result_t
deny() noexcept { return inspection_result_t::deny; }

//FIXME: document this!
inline constexpr inspection_result_t
allow() noexcept { return inspection_result_t::allow; }

//
// incoming_info_t
//
//FIXME: document this!
class incoming_info_t
{
	endpoint_t m_remote_endpoint;

public :
	incoming_info_t(
		endpoint_t remote_endpoint )
		:	m_remote_endpoint{ remote_endpoint }
	{}

	endpoint_t
	remote_endpoint() const noexcept { return m_remote_endpoint; }
};

//
// noop_ip_blocker_t
//
//FIXME: document this!
struct noop_ip_blocker_t
{
	// empty type by design.
};

} /* namespace ip_blocker */

} /* namespace restinio */

