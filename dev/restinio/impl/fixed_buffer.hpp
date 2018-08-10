/*
	restinio
*/

/*!
	Fixed size buffer.
*/

#pragma once

#include <vector>

#include <restinio/asio_include.hpp>

namespace restinio
{

namespace impl
{

//
// fixed_buffer_t
//

//! Helper class for reading bytes and feeding them to parser.
class fixed_buffer_t
{
	public:
		fixed_buffer_t( const fixed_buffer_t & ) = delete;
		fixed_buffer_t & operator = ( const fixed_buffer_t & ) = delete;
		fixed_buffer_t( fixed_buffer_t && ) = delete;
		fixed_buffer_t & operator = ( fixed_buffer_t && ) = delete;

		explicit fixed_buffer_t( std::size_t size )
		{
			m_buf.resize( size );
		}

		//! Make asio buffer for reading bytes from socket.
		auto
		make_asio_buffer() noexcept
		{
			return asio_ns::buffer( m_buf.data(), m_buf.size() );
		}

		//! Mark how many bytes were obtained.
		void
		obtained_bytes( std::size_t length ) noexcept
		{
			m_ready_length = length; // Current bytes in buffer.
			m_ready_pos = 0; // Reset current pos.
		}

		//! Mark how many bytes were obtained.
		void
		consumed_bytes( std::size_t length ) noexcept
		{
			m_ready_length -= length; // decrement buffer length.
			m_ready_pos += length; // Shift current pos.
		}

		//! How many unconsumed bytes are there in buffer.
		std::size_t length() const noexcept { return m_ready_length; }

		//! Get pointer to unconsumed bytes.
		/*!
			\note To check that buffer has unconsumed bytes use length().
		*/
		const char * bytes() const noexcept { return m_buf.data() + m_ready_pos; }

	private:
		//! Buffer for io operation.
		std::vector< char > m_buf;

		//! unconsumed data left in buffer:
		//! \{
		//! Start of data in buffer.
		std::size_t m_ready_pos{0};

		//! Data size.
		std::size_t m_ready_length{0};
		//! \}
};

} /* namespace impl */

} /* namespace restinio */
