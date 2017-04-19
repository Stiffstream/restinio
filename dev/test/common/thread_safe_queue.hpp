#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

// Results of pop from queue
enum class queue_pop_result_t
{
	obtain_item,
	obtain_queue_is_closed
};

// Simple thread safe queue for using with unit tests.
template < typename ITEM >
class thread_safe_queue_t
{
	public:
		queue_pop_result_t
		pop( ITEM & item )
		{
			std::unique_lock< decltype( m_lock ) > lock{ m_lock };

			while( !m_is_closed && m_queue.empty() )
			{
				m_cv.wait( lock );
			}

			if( m_is_closed )
				return queue_pop_result_t::obtain_queue_is_closed;

			item = std::move( m_queue.front() );
			m_queue.pop();

			return queue_pop_result_t::obtain_item;
		}

		void
		push( ITEM item )
		{
			{
				std::unique_lock< decltype( m_lock ) > lock{ m_lock };
				m_queue.emplace( std::move( item ) );
			}
			m_cv.notify_one();
		}

		void
		close_queue()
		{
			std::unique_lock< decltype( m_lock ) > lock{ m_lock };
			m_is_closed = true;
			m_cv.notify_all();
			std::queue< ITEM > tmp{ std::move( m_queue ) };
		}

		void
		reset()
		{
			std::unique_lock< decltype( m_lock ) > lock{ m_lock };
			m_is_closed = false;
		}

	private:
		bool m_is_closed{ false };
		std::queue< ITEM > m_queue;
		std::mutex m_lock;
		std::condition_variable m_cv;
};
