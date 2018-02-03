/*
	restinio
*/

/*!
	sendfile routine.
*/

#pragma once

#include <sys/sendfile.h>

#include <memory>

#include <restinio/sendfile.hpp>

namespace restinio
{

namespace impl
{

using after_sendfile_cb_t = std::function< void ( const asio_ns::error_code & , std::size_t ) >;

//
// sendfile_operation_runner_t
//

//! A runner of sendfile operation
template < typename Socket >
class sendfile_operation_runner_t
	:	public std::enable_shared_from_this< sendfile_operation_runner_t< Socket > >
{
	public:
		sendfile_operation_runner_t(
			const sendfile_options_t & sf_opts,
			asio_ns::executor executor,
			Socket & socket,
			after_sendfile_cb_t after_sendfile_cb )
			:	m_file_descriptor{ sf_opts.file_descriptor() }
			,	m_next_write_offset{ sf_opts.offset() }
			,	m_remained_size{ sf_opts.size() }
			,	m_chunk_size{ sf_opts.m_chunk_size() }
			,	m_expires_after{ std::chrono::steady_clock::now() + sf_opts.timelimit() }
			,	m_executor{ std::move( executor )}
			,	m_socket{ socket }
			,	m_after_sendfile_cb{ std::move( after_sendfile_cb ) }
		{}

		auto expires_after() const { return m_expires_after; }

		void
		init_next_write()
		{
			asio_ns::error_code ec;

			if( !m_socket.native_non_blocking() )
			{
				m_socket.native_non_blocking( true, ec );
			}

			if( ec )
			{
				m_after_sendfile_cb( ec, m_transfered_size );
				return;
			}

			while( true )
			{
				// Try the system call.
				errno = 0;
				auto n =
					sendfile(
						m_socket.native_handle(),
						m_file_descriptor,
						&m_next_write_offset,
						std::min< file_size_t >( m_remained_size, m_chunk_size ) );

				if( -1 == n )
				{
					if( errno == EAGAIN )
					{
						// We have to wait for the socket to become ready again.
						m_socket.async_wait(
							asio_ns::ip::tcp::socket::wait_write,
							asio_ns::bind_executor(
								m_executor,
								[ ctx = this->enable_shared_from_this() ]( const asio_ns::error_code & ec ){
									if( ec || 0 == ctx->m_remained_size )
									{
										ctx->m_after_sendfile_cb( ec, ctx->m_transfered_size );
									}
									else
									{
										ctx->init_next_write();
									}
								} ) );
					}
					else
					{
						ec = asio_ns::error_code{ errno, asio_ns::error::get_system_category() };
						m_after_sendfile_cb( ec, m_transfered_size );
					}
				}
				else
				{
					m_remained_size -= n;
					m_transfered_size += n;
				}

				// Loop around to try calling sendfile again.
			}
		}

	private:
		file_descriptor_t m_file_descriptor;
		file_offset_t m_next_write_offset;
		file_size_t m_remained_size;
		file_size_t m_transfered_size{ 0 };

		const file_size_t m_chunk_size;

		const std::chrono::steady_clock::time_point m_expires_after;

		asio_ns::executor m_executor;
		Socket & m_socket;
		after_sendfile_cb_t m_after_sendfile_cb;
};

} /* namespace impl */

} /* namespace restinio */

