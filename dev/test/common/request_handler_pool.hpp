#pragma once

#include <array>
#include <thread>

#include <restinio/all.hpp>

#include "thread_safe_queue.hpp"

template < std::size_t N >
class request_handler_pool_t
{
	public:

		void
		start(
			std::function< void ( restinio::request_handle_t ) >
				handler )
		{
			m_request_queue.reset();
			m_handler = std::move( handler );

			for( auto & t : m_pool )
			{
				t = std::thread{ [ this ](){
					restinio::request_handle_t req;

					while( queue_pop_result_t::obtain_item ==
						m_request_queue.pop( req ) )
					{
						m_handler( std::move( req ) );
					}
				} };
			}
		}

		void
		stop()
		{
			m_request_queue.close_queue();

			for( auto & t : m_pool )
			{
				t.join();
			}
		}

		void
		enqueue( restinio::request_handle_t req )
		{
			m_request_queue.push( std::move( req ) );
		}

	public:
		thread_safe_queue_t< restinio::request_handle_t > m_request_queue;
		std::array< std::thread, N > m_pool;
		std::function< void ( restinio::request_handle_t ) > m_handler;
};
