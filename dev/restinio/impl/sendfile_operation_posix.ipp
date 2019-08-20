/*
	restinio
*/

/*!
	sendfile routine.
*/

#if defined( RESTINIO_FREEBSD_TARGET ) || defined( RESTINIO_MACOS_TARGET )
	#include <sys/uio.h>
#else
	#include <sys/sendfile.h>
#endif

namespace restinio
{

namespace impl
{

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

		sendfile_operation_runner_t( const sendfile_operation_runner_t & ) = delete;
		sendfile_operation_runner_t( sendfile_operation_runner_t && ) = delete;
		sendfile_operation_runner_t & operator = ( const sendfile_operation_runner_t & ) = delete;
		sendfile_operation_runner_t & operator = ( sendfile_operation_runner_t && ) = delete;

		// Reuse construstors from base.
		using base_type_t::base_type_t;

		virtual void
		start() override
		{
#if defined( RESTINIO_FREEBSD_TARGET ) || defined( RESTINIO_MACOS_TARGET )
			auto const n = ::lseek( this->m_file_descriptor, this->m_next_write_offset, SEEK_SET );
#else
			auto const n = ::lseek64( this->m_file_descriptor, this->m_next_write_offset, SEEK_SET );
#endif

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
							( const asio_ns::error_code & ec, std::size_t written )
							{
//FIXME: this lambda should be noexcept.
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

		sendfile_operation_runner_t( const sendfile_operation_runner_t & ) = delete;
		sendfile_operation_runner_t( sendfile_operation_runner_t && ) = delete;
		sendfile_operation_runner_t & operator = ( const sendfile_operation_runner_t & ) = delete;
		sendfile_operation_runner_t & operator = ( sendfile_operation_runner_t && ) = delete;

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
			}

			while( true )
			{
				// Try the system call.
				errno = 0;
				auto wait_write_is_possible =
					[&]{
						// We have to wait for the socket to become ready again.
						m_socket.async_wait(
							asio_ns::ip::tcp::socket::wait_write,
							asio_ns::bind_executor(
								m_executor,
								[ this, ctx = this->shared_from_this() ]
								( const asio_ns::error_code & ec ) {
//FIXME: this lambda should be noexcept.
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

				if( 0 == m_remained_size )
				{
					// We are done.
					wait_write_is_possible();
					break;
				}

#if defined( RESTINIO_FREEBSD_TARGET )
				// FreeBSD sendfile signature:
				// int sendfile(int fd, int s, off_t offset, size_t nbytes,
				//			struct sf_hdtr	*hdtr, off_t *sbytes, int flags);
				// https://www.freebsd.org/cgi/man.cgi?query=sendfile

				off_t n{ 0 };
				auto rc =
					::sendfile(
						m_file_descriptor,
						m_socket.native_handle(),
						m_next_write_offset,
						static_cast< size_t >(
							std::min< file_size_t >( m_remained_size, m_chunk_size ) ),
						nullptr, // struct	sf_hdtr	*hdtr
						&n, // sbytes
						// Is 16 a reasonable constant here.
#if __FreeBSD__ >= 11
						SF_FLAGS( 16, SF_NOCACHE )
#else
						SF_MNOWAIT
#endif
						);

				// Shift the number of bytes successfully sent.
				m_next_write_offset += n;

				if( -1 == rc )
				{
					// It is still possible that some bytes had been sent.
					m_remained_size -= static_cast< file_size_t >( n );
					m_transfered_size += static_cast< file_size_t >( n );

					n = -1;
				}
#elif defined( RESTINIO_MACOS_TARGET )
				// macOS sendfile signature:
				// in sendfile(int fd, int s, off_t offset,
				//				off_t *len, struct sf_hdtr *hdtr, int flags);

				off_t n =
					static_cast< off_t >(
						std::min< file_size_t >( m_remained_size, m_chunk_size ) );

				auto rc =
					::sendfile(
						m_file_descriptor,
						m_socket.native_handle(),
						m_next_write_offset,
						&n,
						nullptr, // struct	sf_hdtr	*hdtr
						0 );

				// Shift the number of bytes successfully sent.
				m_next_write_offset += n;

				if( -1 == rc )
				{
					// It is still possible that some bytes had been sent.
					m_remained_size -= static_cast< file_size_t >( n );
					m_transfered_size += static_cast< file_size_t >( n );

					n = -1;
				}
#else
				auto n =
					::sendfile64(
						m_socket.native_handle(),
						m_file_descriptor,
						&m_next_write_offset,
						std::min< file_size_t >( m_remained_size, m_chunk_size ) );
#endif

				if( -1 == n )
				{
					if( errno == EAGAIN || errno == EINTR )
					{
						wait_write_is_possible();
					}
					else
					{
						const asio_ns::error_code ec{ errno, asio_ns::error::get_system_category() };
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
					m_remained_size -= static_cast< file_size_t >( n );
					m_transfered_size += static_cast< file_size_t >( n );
				}

				// Loop around to try calling sendfile again.
			}
		}
};

} /* namespace impl */

} /* namespace restinio */

