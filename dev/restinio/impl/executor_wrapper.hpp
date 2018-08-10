/*
	restinio
*/

/*!
	Execution mixin.
*/

#pragma once


namespace restinio
{

namespace impl
{

//
// executor_wrapper_t
//

//! Wrapper for an executor (strand) used by connections.
template < typename Executor >
class executor_wrapper_t
{
	public:
		template < typename Init_Executor >
		executor_wrapper_t( Init_Executor && init_executor )
			:	m_executor{ std::forward< Init_Executor >( init_executor ) }
		{}

		virtual ~executor_wrapper_t() = default;

		//! An executor for callbacks on async operations.
		Executor & get_executor() noexcept { return m_executor; }

	private:
		//! Sync object for connection events.
		Executor m_executor;
};


} /* namespace impl */

} /* namespace restinio */
