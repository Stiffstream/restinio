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

		/*!
		 * @note
		 * This method is noexcept since v.0.6.0.
		 */
		void
		init_next_write() noexcept
		{
			// A note about noexcept for that method.
			// It seems that there is no exceptions thrown by the method itself.
			// The only dangerous place is a call to m_after_sendfile_cb.
			// But the main code behind m_after_sendfile_cb is going from
			// connection_t class and that code is noexcept since v.0.6.0.
			//
			while( true )
			{
				auto const n = ::read(
						this->m_file_descriptor,
						this->m_buffer.get(),
						std::min< file_size_t >(
								this->m_remained_size, this->m_chunk_size ) );

				if( -1 == n )
				{
					if( errno == EINTR )
						continue;

					this->m_after_sendfile_cb(
							asio_ns::error_code{
									errno,
									asio_ns::error::get_system_category() },
							this->m_transfered_size );
				}
				else if( 0 == n )
				{
					this->m_after_sendfile_cb(
							asio_ns::error_code{
									asio_ec::eof,
									asio_ns::error::get_system_category() },
							this->m_transfered_size );
				}
				else
				{
					// If asio_ns::async_write fails we'll call m_after_sendfile_cb.
					try
					{
						asio_ns::async_write(
							this->m_socket,
							asio_ns::const_buffer{
									this->m_buffer.get(),
									static_cast< std::size_t >( n ) },
							asio_ns::bind_executor(
								this->m_executor,
								make_async_write_handler() ) );
					}
					catch( ... )
					{
						this->m_after_sendfile_cb(
							make_asio_compaible_error(
								asio_convertible_error_t::async_write_call_failed ),
							this->m_transfered_size );
					}
				}

				break;
			}
		}

	private:
		std::unique_ptr< char[] > m_buffer{ new char [ this->m_chunk_size ] };

		//! Helper method for making a lambda for async_write completion handler.
		auto
		make_async_write_handler() noexcept
		{
			return [this, ctx = this->shared_from_this()]
				// NOTE: this lambda is noexcept since v.0.6.0.
				( const asio_ns::error_code & ec, std::size_t written ) noexcept
				{
					if( !ec )
					{
						this->m_remained_size -= written;
						this->m_transfered_size += written;
						if( 0 == this->m_remained_size )
						{
							this->m_after_sendfile_cb(
									ec,
									this->m_transfered_size );
						}
						else
						{
							this->init_next_write();
						}
					}
					else
					{
						this->m_after_sendfile_cb(
								ec,
								this->m_transfered_size );
					}
				};
		}
};

//! A specialization for plain tcp-socket using
//! linux sendfile() (http://man7.org/linux/man-pages/man2/sendfile.2.html).
template <>
class sendfile_operation_runner_t< asio_ns::ip::tcp::socket > final
	:	public sendfile_operation_runner_base_t< asio_ns::ip::tcp::socket >
{
	private:

		RESTINIO_NODISCARD
		bool
		try_turn_non_blocking_mode() noexcept
		{
			bool result = true;

			if( !m_socket.native_non_blocking() )
			{
				asio_ns::error_code ec;
				m_socket.native_non_blocking( true, ec );
				if( ec )
				{
					// We assume that m_after_sendfile_cb doesn't throw;
					m_after_sendfile_cb( ec, m_transfered_size );
					result = false;
				}
			}

			return result;
		}

#if defined( RESTINIO_FREEBSD_TARGET )
		RESTINIO_NODISCARD
		auto
		call_native_sendfile() noexcept
		{
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

			return n;
		}
#elif defined( RESTINIO_MACOS_TARGET )
		RESTINIO_NODISCARD
		auto
		call_native_sendfile() noexcept
		{
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

			return n;
		}
#else
		RESTINIO_NODISCARD
		auto
		call_native_sendfile() noexcept
		{
			return ::sendfile64(
					m_socket.native_handle(),
					m_file_descriptor,
					&m_next_write_offset,
					std::min< file_size_t >( m_remained_size, m_chunk_size ) );
		}
#endif

		RESTINIO_NODISCARD
		bool
		try_initiate_waiting_for_write_readiness() noexcept
		{
			bool result = true;

			try
			{
				// We have to wait for the socket to become ready again.
				m_socket.async_wait(
					asio_ns::ip::tcp::socket::wait_write,
					asio_ns::bind_executor(
						m_executor,
						[ this, ctx = this->shared_from_this() ]
						// NOTE: this lambda is noexcept since v.0.6.0.
						( const asio_ns::error_code & ec ) noexcept {
							if( ec || 0 == m_remained_size )
							{
								m_after_sendfile_cb( ec, m_transfered_size );
							}
							else
							{
								init_next_write();
							}
						} ) );
			}
			catch( ... )
			{
				m_after_sendfile_cb(
						make_asio_compaible_error(
								asio_convertible_error_t::async_write_call_failed ),
						m_transfered_size );
				result = false;
			}

			return result;
		}

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

		/*!
		 * @note
		 * This method is noexcept since v.0.6.0.
		 */
		void
		init_next_write() noexcept
		{
			if( !try_turn_non_blocking_mode() )
				return;

			while( true )
			{
				// Try the system call.
				errno = 0;

				if( 0 == m_remained_size )
				{
					// We are done.
					// Result of try_initiate_waiting_for_write_readiness can
					// be ignored here.
					(void)try_initiate_waiting_for_write_readiness();
					break;
				}

				const auto n = call_native_sendfile();

				if( -1 == n )
				{
					if( errno == EAGAIN || errno == EINTR )
					{
						if( !try_initiate_waiting_for_write_readiness() )
							return;
					}
					else
					{
						m_after_sendfile_cb(
								asio_ns::error_code{
										errno, asio_ns::error::get_system_category() },
								m_transfered_size );
					}

					break;
				}
				else if( 0 == n )
				{
					// Result of try_initiate_waiting_for_write_readiness can
					// be ignored here.
					(void)try_initiate_waiting_for_write_readiness();
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

