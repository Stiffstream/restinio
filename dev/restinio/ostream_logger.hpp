/*
	restinio
*/

/*!
	Ready to use logger implementation for using with std::ostream.
*/

#pragma once

#include <string>
#include <iostream>
#include <chrono>
#include <mutex>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/time.h>

namespace restinio
{

//
// null_lock_t
//

//! Fake lock.
struct null_lock_t
{
	constexpr void
	lock() const
	{}

	constexpr bool
	try_lock() const
	{
		return true;
	}

	constexpr void
	unlock() const
	{}
};

//
// ostream_logger_t
//

//! Logger for std::ostream.
/*!
	\note It is not efficient.
*/
template < typename LOCK >
class ostream_logger_t
{
	public:
		ostream_logger_t( const ostream_logger_t & ) = delete;
		void operator = ( const ostream_logger_t & ) = delete;

		ostream_logger_t()
			:	m_out{ &std::cout }
		{}

		ostream_logger_t( std::ostream & out )
			:	m_out{ &out }
		{}

		template< typename MSG_BUILDER >
		void
		trace( MSG_BUILDER && msg_builder )
		{
			log_message( "TRACE", msg_builder() );
		}

		template< typename MSG_BUILDER >
		void
		info( MSG_BUILDER && msg_builder )
		{
			log_message( " INFO", msg_builder() );
		}

		template< typename MSG_BUILDER >
		void
		warn( MSG_BUILDER && msg_builder )
		{
			log_message( " WARN", msg_builder() );
		}

		template< typename MSG_BUILDER >
		void
		error( MSG_BUILDER && msg_builder )
		{
			log_message( "ERROR", msg_builder() );
		}

	private:
		void
		log_message( const char * tag, const std::string & msg )
		{
			std::unique_lock< LOCK > lock{ m_lock };

			using namespace std;
			using namespace chrono;

			auto now = system_clock::now();
			auto ms = duration_cast< milliseconds >( now.time_since_epoch() );
			time_t unix_time = duration_cast< seconds >( ms ).count();

			( *m_out )
				<< fmt::format(
						"[{:%Y-%m-%d %H:%M:%S}.{:03d}] {}: {}",
						make_localtime( unix_time ),
						static_cast< int >( ms.count() % 1000u ),
						tag,
						msg )
				<< std::endl;
		}

		LOCK m_lock;
		std::ostream * m_out;
};

using single_threaded_ostream_logger_t = ostream_logger_t< null_lock_t >;
using shared_ostream_logger_t = ostream_logger_t< std::mutex >;

} /* namespace restinio */
