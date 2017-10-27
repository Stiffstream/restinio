/*
	restinio
*/

/*!
	Timer handlers invocation context.
*/

#pragma once

#include <restinio/timer_common.hpp>

namespace restinio
{

namespace impl
{

//! Data related to timer handlers invocation.
template < typename Timer_Guard >
struct timer_invocation_ctx_t
{
	timer_invocation_ctx_t( Timer_Guard timer_guard )
		:	m_timer_guard{ std::move( timer_guard ) }
	{}

	//! Create tag for next timeout operation.
	auto create_invocation_tag() { return ++m_invocation_tag; }

	//! Checks if current tag matches the one in parameter.
	bool is_same_tag( timer_invocation_tag_t tag ) const { return m_invocation_tag == tag; }

	//! Cancel timer.
	void cancel(){ m_timer_guard.cancel(); }

	//! Operation timeout guard.
	Timer_Guard m_timer_guard;
	timer_invocation_tag_t m_invocation_tag{ 0 };
};

} /* namespace impl */

} /* namespace restinio */
