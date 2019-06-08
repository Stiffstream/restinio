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

namespace connection_state
{

//FIXME: document this!
enum class notice_t
{
	accepted,
	closed,
	upgraded_to_websocket
};

//FIXME: document this!
struct noop_listener_t
{
	// empty type by design.
};

} /* namespace connection_state */

} /* namespace restinio */

