/*
 * SObjectizer-5
 */

/*!
 * \since v.5.4.0
 * \brief A helper for limit execution of code block to some time.
 */

#pragma once

#include <cstdlib>
#include <functional>
#include <future>
#include <thread>
#include <iostream>
#include <string>

template< typename FUNCTOR >
inline void
run_with_time_limit(
	FUNCTOR action,
	unsigned int seconds,
	std::string block_name = std::string() )
	{
		std::promise< void > finish;
		std::thread sentinel( [&finish, seconds, block_name]() {
				auto f = finish.get_future();
				auto status = f.wait_for( std::chrono::seconds( seconds ) );
				if( std::future_status::timeout == status )
					{
						if( !block_name.empty() )
							std::cerr << block_name << ": " << std::flush;
						std::cerr << "block of code not finished within "
							<< seconds << " seconds. Aborting..." << std::endl;
						std::abort();
					}
			} );

		struct sentinel_finisher
			{
				std::thread m_s;
				std::promise< void > & m_p;

				sentinel_finisher( std::thread && s, std::promise< void > & p )
					:	m_s( std::move(s) ), m_p( p )
					{}
				~sentinel_finisher()
					{
						m_p.set_value();
						m_s.join();
					}
			}
		sentinel_finisher( std::move(sentinel), finish );

		action();
	}

