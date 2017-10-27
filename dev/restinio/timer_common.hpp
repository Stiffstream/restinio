/*
	restinio
*/

/*!
	A base class for all classes that deal with connection context.
*/

#pragma once

#include <restinio/tcp_connection_ctx_base.hpp>

namespace restinio
{

// A weak pointer to a context object that is shceduled to be invoked at some time point.
using tcp_connection_ctx_weak_handle_t = std::weak_ptr< tcp_connection_ctx_base_t >;

//! An invokation tag, for controlling no more actual timers in corner cases.
using timer_invocation_tag_t = std::uint32_t;

//! A pointer to invocation callback for a specific context object.
using timer_invocation_cb_t =
	std::add_pointer< void ( timer_invocation_tag_t , tcp_connection_ctx_weak_handle_t ) >::type
	;

} /* namespace restinio */
