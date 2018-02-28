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
		const sendfile_operation_runner_t & operator = ( const sendfile_operation_runner_t & ) = delete;
		sendfile_operation_runner_t & operator = ( sendfile_operation_runner_t && ) = delete;

		// Reuse construstors from base.
		using base_type_t::base_type_t;

		virtual void
		start() override
		{
			init_next_read_some_from_file();
		}

		void
		init_next_read_some_from_file()
		{
			const auto desired_size =
				std::min< file_size_t >( this->m_remained_size, this->m_chunk_size );

			this->m_file_handle.async_read_some_at(
				this->m_next_write_offset, 
				asio_ns::buffer(
					this->m_buffer.get(),
					static_cast< std::size_t >( desired_size ) ),
					asio_ns::bind_executor(
						this->m_executor,
						[ this, ctx = this->shared_from_this() ]
						( const asio_ns::error_code & ec, std::size_t len ){

							if( ec || 0 == m_remained_size )
							{
								m_after_sendfile_cb( ec, m_transfered_size );
							}
							if( !ec ) 
							{
								if( 0 != len )
									init_next_write( len );
								else
								{
									const asio_ns::error_code ec{ asio_ec::eof, asio_ns::error::get_system_category() };
									this->m_after_sendfile_cb( ec, this->m_transfered_size );
								}
							}
							else
							{
								this->m_after_sendfile_cb( ec, this->m_transfered_size );
							}
						} )	);
		}

		void
		init_next_write( std::size_t len )
		{
			asio_ns::async_write(
				this->m_socket,
				asio_ns::const_buffer{
					this->m_buffer.get(),
					static_cast< std::size_t >( len ) },
				asio_ns::bind_executor(
					this->m_executor,
					[ this, ctx = this->shared_from_this() ]
					( const asio_ns::error_code & ec, std::size_t written ){

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
					}
				) );
		}

	private:
		std::unique_ptr< char[] > m_buffer{ new char [ this->m_chunk_size ] };
		asio_ns::windows::random_access_handle 
			m_file_handle{ this->m_socket.get_executor().context(), this->m_file_descriptor };
};

//! A runner of sendfile operation for raw socket.
template <>
class sendfile_operation_runner_t < asio_ns::ip::tcp::socket > final
	:	public sendfile_operation_runner_base_t< asio_ns::ip::tcp::socket >
{
	public:
		using base_type_t = sendfile_operation_runner_base_t< asio_ns::ip::tcp::socket >;

		sendfile_operation_runner_t( const sendfile_operation_runner_t & ) = delete;
		sendfile_operation_runner_t( sendfile_operation_runner_t && ) = delete;
		const sendfile_operation_runner_t & operator = ( const sendfile_operation_runner_t & ) = delete;
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

			asio_ns::windows::overlapped_ptr overlapped{ 
				m_socket.get_executor().context(),
				asio_ns::bind_executor(
					m_executor,
					[this, ctx = shared_from_this() ]
					( const asio_ns::error_code & ec, std::size_t written ){
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
					} ) };
					
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
				  		0, 
				  		0 );

				DWORD last_error = ::GetLastError();

				// Check if the operation completed immediately.
				if( !ok && last_error != ERROR_IO_PENDING )
				{
					// The operation completed immediately, so a completion notification needs
					// to be posted. When complete() is called, ownership of the OVERLAPPED-
					// derived object passes to the io_context.
					asio::error_code ec( static_cast<int>( last_error ), asio::error::get_system_category() );
					overlapped.complete( ec, 0 );
				}
				else
				{
					// The operation was successfully initiated, so ownership of the
					// OVERLAPPED-derived object has passed to the io_context.
					overlapped.release();
				}
		}

	private:
		std::unique_ptr< char[] > m_buffer{ new char [ m_chunk_size ] };
		asio_ns::windows::random_access_handle 
			m_file_handle{ m_socket.get_executor().context(), m_file_descriptor };
};

} /* namespace impl */

} /* namespace restinio */

#else // #if defined(RESTINIO_ASIO_HAS_WINDOWS_OVERLAPPED_PTR)

#include "sendfile_operation_default.inl"

#endif // #if defined(RESTINIO_ASIO_HAS_WINDOWS_OVERLAPPED_PTR)
