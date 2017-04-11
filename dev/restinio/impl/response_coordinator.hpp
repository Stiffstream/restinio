/*
	restinio
*/

/*!
	Coordinator for process od sending responses with
	respect to http pipeline technique and chunk transfer.
*/

#pragma once

#include <string>
#include <deque>

#include <fmt/format.h>

namespace restinio
{

namespace impl
{

using request_id_t = unsigned int;

//
// response_context_t
//

//! A context for a single response.
struct response_context_t
{
	response_context_t() {}

	response_context_t( request_id_t request_id )
		:	m_request_id{ request_id }
	{}

	request_id_t m_request_id{ 0 };

	//! Unsent responses parts.
	std::vector< std::string > m_bufs;

	//! Are existing bufs complete the response?
	bool m_response_complete{ false };
};

//
// response_context_table_t
//

//! Helper storage for responses' contexts.
class response_context_table_t
{
	public:
		response_context_table_t( unsigned int max_elements_count )
		{
			m_contexts.resize( max_elements_count );
		}

		//! If table is empty.
		bool
		empty() const
		{
			return 0UL == m_elements_exists;
		}

		//! If table is full.
		bool
		is_full() const
		{
			return m_contexts.size() == m_elements_exists;
		}

		//! Get first context.
		response_context_t &
		front()
		{
			return m_contexts[ m_first_element_indx ];
		}

		//! Get last context.
		response_context_t &
		back()
		{
			return m_contexts[
				(m_first_element_indx + (m_elements_exists - 1) ) %
					m_contexts.size() ];
		}

		//! Get context of specified request.
		response_context_t &
		operator [] ( request_id_t req_id )
		{
			if( empty() )
				throw std::runtime_error{
					"operator [] not allowed on "
					"empty response_context_table" };

			if( req_id < front().m_request_id ||
				req_id > back().m_request_id )
				throw std::runtime_error{
					fmt::format(
						"operator [] error: bad request id ({}) "
						"for response_context_table ({}..{})",
						req_id,
						front().m_request_id,
						back().m_request_id ) };

			return m_contexts[ get_real_index( req_id ) ];
		}

		//! Insert new context into queue.
		void
		push_response_context( request_id_t req_id )
		{
			if( m_contexts.size() == m_elements_exists )
				throw std::runtime_error{
					"unable to insert context because "
					"response_context_table is full" };

			auto & ctx =
				m_contexts[
						// Current next.
						( m_first_element_indx + m_elements_exists ) % m_contexts.size()
					];

				ctx.m_request_id = req_id;
				ctx.m_response_complete = false;

			// 1 more element added.
			++m_elements_exists;
		}

		void
		pop_response_context()
		{
			if( empty() )
				throw std::runtime_error{
					"unable to pop context because "
					"response_context_table is empty" };

			--m_elements_exists;
			++m_first_element_indx;
			if( m_contexts.size() == m_first_element_indx )
			{
				m_first_element_indx = 0UL;
			}
		}

	private:
		std::size_t
		get_real_index( request_id_t req_id )
		{
			const auto distance_from_first =
				req_id - front().m_request_id;

			return ( m_first_element_indx + distance_from_first ) % m_contexts.size();
		}

		std::vector< response_context_t > m_contexts;
		unsigned int m_first_element_indx{0};
		unsigned int m_elements_exists{0};
};

//
// response_coordinator_t
//

//! Coordinator for process od sending responses with
//! respect to http pipeline technique and chunk transfer.
/*
	Keeps track of maximum N (max_req_count) pipelined requests,
	gathers pieces (buffers) of responses and provides access to
	ready-to-send buffers on demand.
*/
class response_coordinator_t
{
	public:
		response_coordinator_t(
			//! Maximum count of requests to keep track of.
			unsigned int max_req_count )
			:	m_max_req_count{ max_req_count }
		{

		}

	private:
		const unsigned int m_max_req_count;
};

} /* namespace impl */

} /* namespace restinio */
