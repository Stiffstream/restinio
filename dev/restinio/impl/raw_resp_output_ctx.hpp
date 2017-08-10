/*
	restinio
*/

/*!
	Helper output context for writing buffers to output stream (socket).
*/

#pragma once

#include <vector>

#include <asio.hpp>

#include <restinio/buffers.hpp>

namespace restinio
{

namespace impl
{

//
// raw_resp_output_ctx_t
//

//! Helper class for writting response data.
struct raw_resp_output_ctx_t
{
	static constexpr auto
	max_iov_len()
	{
		using len_t = decltype( asio::detail::max_iov_len );
		return std::min< len_t >( asio::detail::max_iov_len, 64 );
	}

	raw_resp_output_ctx_t()
	{
		m_asio_bufs.reserve( max_iov_len() );
		m_bufs.reserve( max_iov_len() );
	}

	const std::vector< asio::const_buffer > &
	create_bufs()
	{
		for( const auto & buf : m_bufs )
		{
			m_asio_bufs.emplace_back( buf.buf() );
		}

		m_transmitting = true;
		return m_asio_bufs;
	}

	void
	done()
	{
		m_asio_bufs.resize( 0 );
		m_bufs.resize( 0 );
		m_transmitting = false;
	}

	bool
	transmitting() const
	{
		return m_transmitting;
	}

	//! Obtains ready buffers if any.
	/*!
		\note READY_BUFFERS_SOURCE must have
		pop_ready_buffers() member function.
	*/
	template < class READY_BUFFERS_SOURCE >
	bool
	obtain_bufs(
		READY_BUFFERS_SOURCE & ready_buffers_source )
	{
		ready_buffers_source.pop_ready_buffers(
			max_iov_len(),
			m_bufs );

		return !m_bufs.empty();
	}

	private:
		//! Is transmition running?
		bool m_transmitting{ false };

		//! Asio buffers.
		std::vector< asio::const_buffer > m_asio_bufs;

		//! Real buffers with data.
		buffers_container_t m_bufs;
};

} /* namespace impl */

} /* namespace restinio */
