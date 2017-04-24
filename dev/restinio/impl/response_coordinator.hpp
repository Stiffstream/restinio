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

#include <restinio/request_handler.hpp>

namespace restinio
{

namespace impl
{

//
// response_context_t
//

//! A context for a single response.
struct response_context_t
{
	void
	reinit( request_id_t request_id )
	{
		m_request_id = request_id;
		m_total_bufs_count = 0;
		m_response_output_flags =
			response_output_flags_t{
				response_parts_attr_t::not_final_parts,
				response_connection_attr_t::connection_keepalive };
	}

	request_id_t m_request_id{ 0 };

	//! Unsent responses parts.
	std::vector< std::string > m_bufs;

	//! Total used bufs, historical count of bufs used to send this response.
	std::size_t m_total_bufs_count{ 0 };

	//! Response flags
	response_output_flags_t
		m_response_output_flags{
			response_parts_attr_t::not_final_parts,
			response_connection_attr_t::connection_keepalive };
};

//
// response_context_table_t
//

//! Helper storage for responses' contexts.
class response_context_table_t
{
	public:
		response_context_table_t( std::size_t max_elements_count )
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
		response_context_t *
		get_by_req_id( request_id_t req_id )
		{
			if( empty() ||
				req_id < front().m_request_id ||
				req_id > back().m_request_id )
			{
				return nullptr;
			}

			return &m_contexts[ get_real_index( req_id ) ];
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

				ctx.reinit( req_id );

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
		std::size_t m_first_element_indx{0};
		std::size_t m_elements_exists{0};
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
			std::size_t max_req_count )
			:	m_context_table{ max_req_count }
		{}

		bool
		closed() const
		{
			return m_connection_closed_response_occured;
		}

		bool
		empty() const
		{
			return m_context_table.empty();
		}

		bool
		is_full() const
		{
			return m_context_table.is_full();
		}

		//! Create a new request and reserve context for its response.
		request_id_t
		register_new_request()
		{
			m_context_table.push_response_context( m_request_id_counter );

			return m_request_id_counter++;
		}

		//! Add outgoing data for specified request.
		void
		append_response(
			//! Request id the responses parts are for.
			request_id_t req_id,
			//! Resp output flag.
			response_output_flags_t response_output_flags,
			//! The parts of response.
			std::vector< std::string > bufs )
		{
			// Nothing to do if already closed response emitted.
			if( closed() )
				throw std::runtime_error{
					"unable to append response parts, "
					"response coordinator is closed" };

			auto * ctx = m_context_table.get_by_req_id( req_id );

			if( nullptr == ctx )
			{
				// Request is unknown...
				throw std::runtime_error{
					fmt::format(
						"no context associated with request {}",
						req_id ) };
			}

			if( response_parts_attr_t::final_parts ==
				ctx->m_response_output_flags.m_response_parts )
			{
				// Request is already completed...
				throw std::runtime_error{
					"unable to append response, "
					"it marked as complete" };
			}

			ctx->m_response_output_flags = response_output_flags;

			if( ctx->m_bufs.empty() )
				ctx->m_bufs = std::move( bufs );
			else
			{
				ctx->m_bufs.reserve(
					ctx->m_bufs.size() +  bufs.size() );

				for( auto & buf : bufs )
					ctx->m_bufs.emplace_back( std::move( buf ) );
			}
		}

		//! Get ready to send buffers
		void
		pop_ready_buffers(
			//! The maximum count of buffers to obtain.
			unsigned int max_buf_count,
			//! Receiver for buffers.
			std::vector< std::string > & bufs )
		{
			if( closed() )
				throw std::runtime_error{
					"unable to prepare output buffers, "
					"response coordinator is closed" };

			// Select buffers one by one while
			// it is possible to follow the order of the data
			// that must be sent to client
			// and buf count not exceed max_buf_count.

			while(
				0 != max_buf_count &&
				!m_context_table.empty() )
			{
				auto & current_ctx = m_context_table.front();
				const auto bufs_to_get_from_current_context = std::min(
						static_cast<decltype(max_buf_count)>(current_ctx.m_bufs.size()),
						max_buf_count );

				max_buf_count -= bufs_to_get_from_current_context;

				auto extracted_bufs_end = std::begin( current_ctx.m_bufs );
				std::advance(
					extracted_bufs_end,
					bufs_to_get_from_current_context );

				std::for_each(
					std::begin( current_ctx.m_bufs ),
					extracted_bufs_end,
					[ & ]( auto & buf ){
						bufs.emplace_back( std::move( buf ) );
					} );

				if( current_ctx.m_bufs.end() == extracted_bufs_end )
				{
					current_ctx.m_bufs.clear();

					// All existing parts for current response were
					// selected for output, so it might be the case
					// entire response was selected.

					if( response_parts_attr_t::final_parts ==
						current_ctx.m_response_output_flags.m_response_parts )
					{
						// Response for currently first tracked
						// request is completed.
						m_context_table.pop_response_context();

						if( response_connection_attr_t::connection_close ==
							current_ctx.m_response_output_flags.m_response_connection )
						{
							// Not onle the response is complete
							// but it has a connection-close property.
							// So the response coordinator must
							// stop its work.

							m_connection_closed_response_occured = true;
							break;
						}
					}
					else
					{
						// All existing parts of current response were selected
						// but there must be more parts for current response
						// that a not already received by coordinator
						// so breake selection loop.
						break;
					}
				}
				else
				{
					// Current response is definetely not over
					// but max_buf_count bufers are obtained
					// while condition will fail.
					current_ctx.m_bufs.erase(
						std::begin( current_ctx.m_bufs ),
						extracted_bufs_end );
				}
			}
		}

	private:
		//! Counter for asigining id to new requests.
		request_id_t m_request_id_counter{ 0 };

		response_context_table_t m_context_table;

		//! Indicate whether a response with connection close flag was emitted.
		bool m_connection_closed_response_occured{ false };
};

} /* namespace impl */

} /* namespace restinio */
