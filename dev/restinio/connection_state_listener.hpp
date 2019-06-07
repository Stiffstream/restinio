/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to connection state listeners.
 *
 * @since v.0.5.1
 */

#pragma once

#include <cstdint>

namespace restinio
{

//FIXME: document this!
using connection_id_t = std::uint64_t;

//FIXME: document this!
enum class connection_state_notify_t
{
	accepted,
	closed,
	upgraded_to_websocket
};

//FIXME: document this!
struct noop_connection_state_listener_t
{
	// empty type by design.
};

} /* namespace restinio */

