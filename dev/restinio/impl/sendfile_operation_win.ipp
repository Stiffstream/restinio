/*
	restinio
*/

/*!
	sendfile routine.
*/

#if defined(RESTINIO_ASIO_HAS_WINDOWS_OVERLAPPED_PTR)

#include <cstdio>

namespace restinio
{

namespace impl
{

namespace asio_details
{

#if RESTINIO_ASIO_VERSION < 101300

template<typename Socket >
decltype(auto)
executor_or_context_from_socket( Socket & socket )
{
	return socket.get_executor().context();
}

#else

template<typename Socket >
decltype(auto)
executor_or_context_from_socket( Socket & socket )
{
	return socket.get_executor();
}

#endif

} /* namespace asio_details */

//
// sendfile_operation_runner_t
//

//! A runner of sendfile operation.
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

		sendfile_operation_runner_t(
			sendfile_t & sf,
			asio_ns::executor executor,
			Socket & socket,
			after_sendfile_cb_t after_sendfile_cb )
			:	base_type_t{ sf, std::move( executor), socket, std::move( after_sendfile_cb ) }
		{
			// We have passed sf.file_descriptor() to m_file_handle object.
			// It means that file description will be closed automatically
			// by m_file_handle.
			// But sf still holds the same file_descriptor. Because of that
			// we should tell sf to release this file_descriptor.
			takeaway_file_descriptor(sf).release();
		}

		virtual void
		start() override
		{
			init_next_read_some_from_file();
		}

		/*!
		 * @note
		 * This method is noexcept since v.0.6.0.
		 */
		void
		init_next_read_some_from_file() noexcept
		{
			const auto desired_size =
				std::min< file_size_t >( this->m_remained_size, this->m_chunk_size );

			try
			{
				this->m_file_handle.async_read_some_at(
					this->m_next_write_offset,
					asio_ns::buffer(
						this->m_buffer.get(),
						static_cast< std::size_t >( desired_size ) ),
					asio_ns::bind_executor(
						this->m_executor,
						make_async_read_some_at_handler() ) );
			}
			catch( ... )
			{
				this->m_after_sendfile_cb(
						make_asio_compaible_error(
								asio_convertible_error_t::async_read_some_at_call_failed ),
						this->m_transfered_size );
			}
		}

		/*!
		 * @note
		 * This method is noexcept since v.0.6.0.
		 */
		void
		init_next_write( std::size_t len ) noexcept
		{
			try
			{
				asio_ns::async_write(
					this->m_socket,
					asio_ns::const_buffer{
						this->m_buffer.get(),
						static_cast< std::size_t >( len ) },
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

	private:
		std::unique_ptr< char[] > m_buffer{ new char [ this->m_chunk_size ] };
		asio_ns::windows::random_access_handle m_file_handle{
				asio_details::executor_or_context_from_socket(this->m_socket),
				this->m_file_descriptor
		};

		auto
		make_async_read_some_at_handler() noexcept
		{
			return [this, ctx = this->shared_from_this()]
				// NOTE: this lambda is noexcept since v.0.6.0.
				( const asio_ns::error_code & ec, std::size_t len ) noexcept
				{
					if( ec || 0 == this->m_remained_size )
					{
						this->m_after_sendfile_cb( ec, this->m_transfered_size );
					}
					if( !ec )
					{
						if( 0 != len )
							init_next_write( len );
						else
						{
							this->m_after_sendfile_cb(
								make_error_code( asio_ec::eof ),
								this->m_transfered_size );
						}
					}
					else
					{
						this->m_after_sendfile_cb( ec, this->m_transfered_size );
					}
				};
		}

		auto
		make_async_write_handler() noexcept
		{
			return [ this, ctx = this->shared_from_this() ]
				// NOTE: this lambda is noexcept since v.0.6.0.
				( const asio_ns::error_code & ec, std::size_t written ) noexcept
				{
					if( !ec )
					{
						this->m_remained_size -= written;
						this->m_transfered_size += written;
						this->m_next_write_offset += written;

						if( 0 == this->m_remained_size )
						{
							this->m_after_sendfile_cb( ec, this->m_transfered_size );
						}
						else
						{
							this->init_next_read_some_from_file();
						}
					}
					else
					{
						this->m_after_sendfile_cb( ec, this->m_transfered_size );
					}
				};
		}
};

//! A runner of sendfile operation for raw socket.
template <>
class sendfile_operation_runner_t < asio_ns::ip::tcp::socket > final
	:	public sendfile_operation_runner_base_t< asio_ns::ip::tcp::socket >
{
		auto
		make_completion_handler() noexcept
		{
			return [this, ctx = shared_from_this() ]
				// NOTE: this lambda is noexcept since v.0.6.0.
				( const asio_ns::error_code & ec, std::size_t written )
				{
					if( !ec )
					{
						m_remained_size -= written;
						m_transfered_size += written;
						m_next_write_offset += written;

						if( 0 == m_remained_size )
						{
							m_after_sendfile_cb( ec, m_transfered_size );
						}
						else
						{
							init_next_write();
						}
					}
					else
					{
						m_after_sendfile_cb( ec, m_transfered_size );
					}
				};
		}

	public:
		using base_type_t = sendfile_operation_runner_base_t< asio_ns::ip::tcp::socket >;

		sendfile_operation_runner_t( const sendfile_operation_runner_t & ) = delete;
		sendfile_operation_runner_t( sendfile_operation_runner_t && ) = delete;
		sendfile_operation_runner_t & operator = ( const sendfile_operation_runner_t & ) = delete;
		sendfile_operation_runner_t & operator = ( sendfile_operation_runner_t && ) = delete;

		sendfile_operation_runner_t(
			sendfile_t & sf,
			asio_ns::executor executor,
			asio_ns::ip::tcp::socket & socket,
			after_sendfile_cb_t after_sendfile_cb )
			:	base_type_t{ sf, std::move( executor), socket, std::move( after_sendfile_cb ) }
		{
			// We have passed sf.file_descriptor() to m_file_handle object.
			// It means that file description will be closed automatically
			// by m_file_handle.
			// But sf still holds the same file_descriptor. Because of that
			// we should tell sf to release this file_descriptor.
			takeaway_file_descriptor(sf).release();
		}

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
			// In this function bind_executor is the main suspect
			// for throwing an exception. Because of that the whole
			// function's logic is wrapped by try-catch.
			try
			{
				asio_ns::windows::overlapped_ptr overlapped{
					asio_details::executor_or_context_from_socket( m_socket ),
					asio_ns::bind_executor(
						m_executor,
						make_completion_handler() )
				};

				// Set offset.
				overlapped.get()->Offset =
					static_cast< DWORD >( m_next_write_offset & 0xFFFFFFFFULL );
				overlapped.get()->OffsetHigh =
					static_cast< DWORD >( (m_next_write_offset>>32) & 0xFFFFFFFFULL );

				// Amount of data to transfer.
				const auto desired_size =
					std::min< file_size_t >( this->m_remained_size, this->m_chunk_size );

				// Initiate the TransmitFile operation.
				BOOL ok =
					::TransmitFile(
						m_socket.native_handle(),
						m_file_handle.native_handle(),
						static_cast< DWORD >( desired_size ),
						0,
						overlapped.get(),
						nullptr,
						0 );

				DWORD last_error = ::GetLastError();

				// Check if the operation completed immediately.
				if( !ok && last_error != ERROR_IO_PENDING )
				{
					// The operation completed immediately, so a completion notification needs
					// to be posted. When complete() is called, ownership of the OVERLAPPED-
					// derived object passes to the io_context.
					overlapped.complete( make_error_code( last_error ) , 0 );
				}
				else
				{
					// The operation was successfully initiated, so ownership of the
					// OVERLAPPED-derived object has passed to the io_context.
					overlapped.release();
				}
			}
			catch( ... )
			{
				// Report that error as a failure of async_write.
				this->m_after_sendfile_cb(
						make_asio_compaible_error(
								asio_convertible_error_t::async_write_call_failed ),
						this->m_transfered_size );
			}
		}

	private:
		std::unique_ptr< char[] > m_buffer{ new char [ m_chunk_size ] };
		asio_ns::windows::random_access_handle m_file_handle{
				asio_details::executor_or_context_from_socket(m_socket),
				m_file_descriptor
		};
};

} /* namespace impl */

} /* namespace restinio */

#else // #if defined(RESTINIO_ASIO_HAS_WINDOWS_OVERLAPPED_PTR)

#include "sendfile_operation_default.ipp"

#endif // #if defined(RESTINIO_ASIO_HAS_WINDOWS_OVERLAPPED_PTR)
