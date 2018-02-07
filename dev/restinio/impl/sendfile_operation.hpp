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

//
// sendfile_operation_base_t
//

class sendfile_operation_base_t
	:	public std::enable_shared_from_this< sendfile_operation_base_t >
{
	public:
		virtual void
		start() = 0;
};

using sendfile_operation_shared_ptr_t = std::shared_ptr< sendfile_operation_base_t >;


using after_sendfile_cb_t = std::function< void ( const asio_ns::error_code & , std::size_t ) >;

//
// sendfile_operation_runner_base_t
//
template < typename Socket >
class sendfile_operation_runner_base_t
	:	public sendfile_operation_base_t
{
	public:
		sendfile_operation_runner_base_t(
			const sendfile_options_t & sf_opts,
			asio_ns::executor executor,
			Socket & socket,
			after_sendfile_cb_t after_sendfile_cb )
			:	m_file_descriptor{ sf_opts.file_descriptor() }
			,	m_next_write_offset{ sf_opts.offset() }
			,	m_remained_size{ sf_opts.size() }
			,	m_chunk_size{ sf_opts.chunk_size() }
			,	m_expires_after{ std::chrono::steady_clock::now() + sf_opts.timelimit() }
			,	m_executor{ std::move( executor )}
			,	m_socket{ socket }
			,	m_after_sendfile_cb{ std::move( after_sendfile_cb ) }
		{}

		auto expires_after() const { return m_expires_after; }

	protected:
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

//
// sendfile_operation_runner_t
//

//! A runner of sendfile operation
template < typename Socket >
class sendfile_operation_runner_t final
	:	public sendfile_operation_runner_base_t< Socket >
{
	public:
		using base_type_t = sendfile_operation_runner_base_t< Socket >;

		// Reuse construstors from base.
		using base_type_t::base_type_t;

		virtual void
		start() override
		{
			auto const n = ::lseek( this->m_file_descriptor, this->m_next_write_offset, SEEK_SET );
			if( static_cast< off_t >( -1 ) != n )
			{
				this->init_next_write();
			}
			else
			{
				const asio_ns::error_code ec{ errno, asio_ns::error::get_system_category() };
				this->m_after_sendfile_cb( ec, this->m_transfered_size );
				return;
			}
		}

		void
		init_next_write()
		{
			while( true )
			{
				auto const n =
					::read(
						this->m_file_descriptor,
						this->m_buffer.get(),
						std::min< file_size_t >( this->m_remained_size, this->m_chunk_size ) );

				if( -1 == n )
				{
					if( errno == EINTR )
						continue;

					const asio_ns::error_code ec{ errno, asio_ns::error::get_system_category() };
					this->m_after_sendfile_cb( ec, this->m_transfered_size );
				}
				else if( 0 == n )
				{
					const asio_ns::error_code ec{ asio_ec::eof, asio_ns::error::get_system_category() };
					this->m_after_sendfile_cb( ec, this->m_transfered_size );
				}
				else
				{
					asio_ns::async_write(
						this->m_socket,
						asio_ns::const_buffer{ this->m_buffer.get(), static_cast< std::size_t >( n ) },
						asio_ns::bind_executor(
							this->m_executor,
							[ this, ctx = this->shared_from_this() ]
							( const asio_ns::error_code & ec, std::size_t written ){

								if( !ec )
								{
									this->m_remained_size -= written;
									this->m_transfered_size += written;
									if( 0 == this->m_remained_size )
									{
										this->m_after_sendfile_cb( ec, this->m_transfered_size );
									}
									else
									{
										this->init_next_write();
									}
								}
								else
								{
									this->m_after_sendfile_cb( ec, this->m_transfered_size );
								}
							}
						) );
				}
				break;
			}
		}

	private:
		std::unique_ptr< char[] > m_buffer{ new char [ this->m_chunk_size ] };
};

//! A specialization for plain tcp-socket using
//! linux sendfile() (http://man7.org/linux/man-pages/man2/sendfile.2.html).
template <>
class sendfile_operation_runner_t< asio_ns::ip::tcp::socket > final
	:	public sendfile_operation_runner_base_t< asio_ns::ip::tcp::socket >
{
	public:
		using base_type_t = sendfile_operation_runner_base_t< asio_ns::ip::tcp::socket >;

		// Reuse construstors from base.
		using base_type_t::base_type_t;

		virtual void
		start() override
		{
			init_next_write();
		}

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
					::sendfile(
						m_socket.native_handle(),
						m_file_descriptor,
						&m_next_write_offset,
						std::min< file_size_t >( m_remained_size, m_chunk_size ) );

				auto wait_write_is_possible =
					[&]{
						// We have to wait for the socket to become ready again.
						m_socket.async_wait(
							asio_ns::ip::tcp::socket::wait_write,
							asio_ns::bind_executor(
								m_executor,
								[ this, ctx = this->shared_from_this() ]
								( const asio_ns::error_code & ec ){
									if( ec || 0 == m_remained_size )
									{
										m_after_sendfile_cb( ec, m_transfered_size );
									}
									else
									{
										init_next_write();
									}
								} ) );
					};

				if( -1 == n )
				{
					if( errno == EAGAIN )
					{
						wait_write_is_possible();
					}
					else
					{
						ec = asio_ns::error_code{ errno, asio_ns::error::get_system_category() };
						m_after_sendfile_cb( ec, m_transfered_size );
					}

					break;
				}
				else if( 0 == n )
				{
					wait_write_is_possible();
					break;
				}
				else
				{
					m_remained_size -= n;
					m_transfered_size += n;
				}

				// Loop around to try calling sendfile again.
			}
		}
};

} /* namespace impl */

} /* namespace restinio */

