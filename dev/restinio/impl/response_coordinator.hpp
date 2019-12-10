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

#include <restinio/impl/include_fmtlib.hpp>

#include <restinio/utils/suppress_exceptions.hpp>

#include <restinio/exception.hpp>
#include <restinio/request_handler.hpp>
#include <restinio/buffers.hpp>
#include <restinio/optional.hpp>

namespace restinio
{

namespace impl
{

using write_groups_container_t = std::vector< write_group_t >;

//
// response_context_t
//

//! A context for a single response.
class response_context_t
{
	public:
		//! Access write-groups container (used in unit tests)
		friend write_groups_container_t &
		utest_access( response_context_t & ctx )
		{
			return ctx.m_write_groups;
		}

		//! Reinitialize context.
		void
		reinit(
			//! New request id.
			request_id_t request_id ) noexcept
		{
			m_request_id = request_id;
			m_response_output_flags =
				response_output_flags_t{
					response_parts_attr_t::not_final_parts,
					response_connection_attr_t::connection_keepalive };
		}

		//! Put write group to data queue.
		void
		enqueue_group( write_group_t wg )
		{
			// There is at least one group.
			// So we check if this group can be merged with existing (the last one).
			if( !m_write_groups.empty() &&
				!m_write_groups.back().has_after_write_notificator() &&
				std::size_t{ 0 } == wg.status_line_size() )
			{
				m_write_groups.back().merge( std::move( wg ) );
			}
			else
			{
				m_write_groups.emplace_back( std::move( wg ) );
			}

		}

		//! Is context empty.
		bool empty() const noexcept { return m_write_groups.empty(); }

		//! Extract write group from data queue.
		write_group_t
		dequeue_group() noexcept
		{
			assert( !m_write_groups.empty() );

			// Move constructor for write_group_t shouldn't throw.
			RESTINIO_STATIC_ASSERT_NOEXCEPT(
					write_group_t{ std::declval<write_group_t>() } );

			write_group_t result{ std::move( m_write_groups.front() ) };

			// Some STL implementation can have std::vector::erase that
			// doesn't throw. So we use a kind of static if to select
			// an appropriate behaviour.
			static_if_else< noexcept(m_write_groups.erase(m_write_groups.begin())) >(
					// This is for the case when std::vector::erase doesn't throw.
					[this]() noexcept {
						m_write_groups.erase( m_write_groups.begin() );
					},
					// This is for the case when std::vector::erase does throw.
					[this]() {
						restinio::utils::suppress_exceptions_quietly( [this] {
								m_write_groups.erase( m_write_groups.begin() );
						} );
					} );

			return result;
		}

		//! Get id of associated request.
		auto request_id() const noexcept { return m_request_id; }

		//! Get flags of corrent response data flow.
		void
		response_output_flags( response_output_flags_t flags ) noexcept
		{
			m_response_output_flags = flags;
		}

		//! Get flags of corrent response data flow.
		auto
		response_output_flags() const noexcept
		{
			return m_response_output_flags;
		}

		//! Is response data of a given request is complete.
		bool
		is_complete() const noexcept
		{
			return m_write_groups.empty() &&
				response_parts_attr_t::final_parts ==
					m_response_output_flags.m_response_parts;
		}

	private:
		request_id_t m_request_id{ 0 };

		//! Unsent responses parts.
		write_groups_container_t m_write_groups;

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
		empty() const noexcept
		{
			return !m_elements_exists;
		}

		//! If table is full.
		bool
		is_full() const noexcept
		{
			return m_contexts.size() == m_elements_exists;
		}

		//! Get first context.
		response_context_t &
		front() noexcept
		{
			return m_contexts[ m_first_element_index ];
		}

		//! Get last context.
		response_context_t &
		back() noexcept
		{
			return m_contexts[
				(m_first_element_index + (m_elements_exists - 1) ) %
					m_contexts.size() ];
		}

		//! Get context of specified request.
		response_context_t *
		get_by_req_id( request_id_t req_id ) noexcept
		{
			if( empty() ||
				req_id < front().request_id() ||
				req_id > back().request_id() )
			{
				return nullptr;
			}

			return &m_contexts[ get_real_index( req_id ) ];
		}

		//! Insert new context into queue.
		void
		push_response_context( request_id_t req_id )
		{
			if( is_full() )
				throw exception_t{
					"unable to insert context because "
					"response_context_table is full" };

			auto & ctx =
				m_contexts[
						// Current next.
						( m_first_element_index + m_elements_exists ) % m_contexts.size()
					];

				ctx.reinit( req_id );

			// 1 more element added.
			++m_elements_exists;
		}

		//! Remove the first context from queue.
		void
		pop_response_context()
		{
			if( empty() )
				throw exception_t{
					"unable to pop context because "
					"response_context_table is empty" };

			pop_response_context_nonchecked();
		}

		//! Remove the first context from queue with the check for
		//! emptiness of the queue.
		/*!
		 * @note
		 * This method is noexcept and indended to be used in noexcept
		 * context. But the emptiness of the queue should be checked
		 * before the call of this method.
		 *
		 * @since v.0.6.0
		 */
		void
		pop_response_context_nonchecked() noexcept
		{
			--m_elements_exists;
			++m_first_element_index;
			if( m_contexts.size() == m_first_element_index )
			{
				m_first_element_index = std::size_t{0};
			}
		}

	private:
		std::size_t
		get_real_index( request_id_t req_id ) noexcept
		{
			const auto distance_from_first =
				req_id - front().request_id();

			return ( m_first_element_index + distance_from_first ) % m_contexts.size();
		}

		std::vector< response_context_t > m_contexts;
		std::size_t m_first_element_index{0};
		std::size_t m_elements_exists{0};
};

//
// response_coordinator_t
//

//! Coordinator for process of sending responses with
//! respect to http pipeline technique and chunk transfer.
/*
	Keeps track of maximum N (max_req_count) pipelined requests,
	gathers pieces (write groups) of responses and provides access to
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

		/** @name Response coordinator state.
		 * @brief Various state flags.
		*/
		///@{
		bool closed() const noexcept { return m_connection_closed_response_occured; }
		bool empty() const noexcept { return m_context_table.empty(); }
		bool is_full() const noexcept { return m_context_table.is_full(); }
		///@}

		//! Check if it is possible to accept more requests.
		bool
		is_able_to_get_more_messages() const noexcept
		{
			return !closed() && !is_full();
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
			write_group_t wg )
		{
			// Nothing to do if already closed response emitted.
			if( closed() )
				throw exception_t{
					"unable to append response parts, "
					"response coordinator is closed" };

			auto * ctx = m_context_table.get_by_req_id( req_id );

			if( nullptr == ctx )
			{
				// Request is unknown...
				throw exception_t{
					fmt::format(
						"no context associated with request {}",
						req_id ) };
			}

			if( response_parts_attr_t::final_parts ==
				ctx->response_output_flags().m_response_parts )
			{
				// Request is already completed...
				throw exception_t{
					"unable to append response, "
					"it marked as complete" };
			}

			ctx->response_output_flags( response_output_flags );

			ctx->enqueue_group( std::move( wg ) );
		}

		//! Extract a portion of data available for write.
		/*!
			Data (if available) is wrapped in an instance of write_group_t.
			It can have a stats line mark (that is necessary for logging)
			and a notificator that must be invoked after the write operation
			of a given group completes.
		*/
		optional_t< std::pair< write_group_t, request_id_t > >
		pop_ready_buffers()
		{
			if( closed() )
				throw exception_t{
					"unable to prepare output buffers, "
					"response coordinator is closed" };

			optional_t< std::pair< write_group_t, request_id_t > > result;

			// Check for custom write operation.
			if( !m_context_table.empty() )
			{
				auto & current_ctx = m_context_table.front();

				if( !current_ctx.empty() )
				{
					result =
						std::make_pair(
							current_ctx.dequeue_group(),
							current_ctx.request_id() );

					if( current_ctx.is_complete() )
					{
						m_connection_closed_response_occured =
							( response_parts_attr_t::final_parts ==
								current_ctx.response_output_flags().m_response_parts )
							&&( response_connection_attr_t::connection_close ==
								current_ctx.response_output_flags().m_response_connection );

						m_context_table.pop_response_context();
					}
				}
			}

			return result;
		}

		//! Remove all contexts.
		/*!
			Invoke write groups after-write callbacks with error status.

			@note
			Since v.0.6.0 this method is noexcept
		*/
		void
		reset() noexcept
		{
			RESTINIO_STATIC_ASSERT_NOEXCEPT(m_context_table.empty());
			RESTINIO_STATIC_ASSERT_NOEXCEPT(
					m_context_table.pop_response_context_nonchecked());
			RESTINIO_STATIC_ASSERT_NOEXCEPT(m_context_table.front());
			RESTINIO_STATIC_ASSERT_NOEXCEPT(m_context_table.front().dequeue_group());
                                        
			RESTINIO_STATIC_ASSERT_NOEXCEPT(make_asio_compaible_error(
					asio_convertible_error_t::write_was_not_executed));

			for(; !m_context_table.empty();
				m_context_table.pop_response_context_nonchecked() )
			{
				const auto ec =
					make_asio_compaible_error(
						asio_convertible_error_t::write_was_not_executed );

				auto & current_ctx = m_context_table.front();
				while( !current_ctx.empty() )
				{
					auto wg = current_ctx.dequeue_group();

					restinio::utils::suppress_exceptions_quietly( [&] {
							wg.invoke_after_write_notificator_if_exists( ec );
						} );
				}
			}
		}

	private:
		//! Counter for asigining id to new requests.
		request_id_t m_request_id_counter{ 0 };

		//! Indicate whether a response with connection close flag was emitted.
		bool m_connection_closed_response_occured{ false };

		//! A storage for resp-context items.
		response_context_table_t m_context_table;
};

} /* namespace impl */

} /* namespace restinio */
