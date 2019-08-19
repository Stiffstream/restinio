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

#include <restinio/impl/include_fmtlib.hpp>

namespace restinio
{

//
// null_lock_t
//

//! Fake lock.
struct null_lock_t
{
	constexpr void lock() const noexcept {}

	constexpr bool try_lock() const noexcept { return true; }

	constexpr void unlock() const noexcept {}
};

//
// ostream_logger_t
//

//! Logger for std::ostream.
/*!
	\note It is not efficient.
*/
template < typename Lock >
class ostream_logger_t
{
	public:
		ostream_logger_t( const ostream_logger_t & ) = delete;
		ostream_logger_t & operator = ( const ostream_logger_t & ) = delete;

		ostream_logger_t() noexcept
			:	m_out{ &std::cout }
		{}

		ostream_logger_t( std::ostream & out ) noexcept
			:	m_out{ &out }
		{}

		template< typename Message_Builder >
		void
		trace( Message_Builder && msg_builder )
		{
			log_message( "TRACE", msg_builder() );
		}

		template< typename Message_Builder >
		void
		info( Message_Builder && msg_builder )
		{
			log_message( " INFO", msg_builder() );
		}

		template< typename Message_Builder >
		void
		warn( Message_Builder && msg_builder )
		{
			log_message( " WARN", msg_builder() );
		}

		template< typename Message_Builder >
		void
		error( Message_Builder && msg_builder )
		{
			log_message( "ERROR", msg_builder() );
		}

	private:
		void
		log_message( const char * tag, const std::string & msg )
		{
			std::unique_lock< Lock > lock{ m_lock };

			namespace stdchrono = std::chrono;

			auto now = stdchrono::system_clock::now();
			auto ms = stdchrono::duration_cast<
					stdchrono::milliseconds >( now.time_since_epoch() );
			std::time_t unix_time = stdchrono::duration_cast<
					stdchrono::seconds >( ms ).count();

			( *m_out )
				<< fmt::format(
						"[{:%Y-%m-%d %H:%M:%S}.{:03d}] {}: {}",
						make_localtime( unix_time ),
						static_cast< int >( ms.count() % 1000u ),
						tag,
						msg )
				<< std::endl;
		}

		Lock m_lock;
		std::ostream * m_out;
};

using single_threaded_ostream_logger_t = ostream_logger_t< null_lock_t >;
using shared_ostream_logger_t = ostream_logger_t< std::mutex >;

} /* namespace restinio */
