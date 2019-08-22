/*
	restinio
*/

/*!
	sendfile routine.
*/

#include <cstdio>

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
			const auto n =
				std::fseek(
					this->m_file_descriptor,
					this->m_next_write_offset,
					SEEK_SET );

			if( 0 == n )
			{
				this->init_next_write();
			}
			else
			{
				this->m_after_sendfile_cb(
					make_error_code( std::ferror( this->m_file_descriptor ) ),
				 	this->m_transfered_size );
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
			const auto desired_size =
				std::min< file_size_t >( this->m_remained_size, this->m_chunk_size );

			const auto n =
				std::fread(
					this->m_buffer.get(),
					1,
					desired_size,
					this->m_file_descriptor );

			if( desired_size != n )
			{
				this->m_after_sendfile_cb(
					make_error_code( std::ferror( this->m_file_descriptor ) ),
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
							static_cast< std::size_t >( desired_size ) },
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
		}

	private:
		std::unique_ptr< char[] > m_buffer{ new char [ this->m_chunk_size ] };

		//! Helper method for making a lambda for async_write completion handler.
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
				};
		}
};

} /* namespace impl */

} /* namespace restinio */

