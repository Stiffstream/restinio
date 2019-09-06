/*
	restinio
*/

/*!
	sendfile routine.
*/

#pragma once

#include <memory>

#include <restinio/sendfile.hpp>

namespace restinio
{

namespace impl
{

//
// sendfile_operation_base_t
//

//!Base class for storing sendfile operation context.
class sendfile_operation_base_t
	:	public std::enable_shared_from_this< sendfile_operation_base_t >
{
	public:
		virtual ~sendfile_operation_base_t() = default;

		virtual void
		start() = 0;
};

using sendfile_operation_shared_ptr_t = std::shared_ptr< sendfile_operation_base_t >;

//! Callback type for invocation when sendfile operation completes.
using after_sendfile_cb_t =
	std::function< void ( const asio_ns::error_code & , file_size_t ) >;

//
// sendfile_operation_runner_base_t
//

//! A base runner of sendfile operation (keeps all the data).
template < typename Socket >
class sendfile_operation_runner_base_t
	:	public sendfile_operation_base_t
{
	public:
		sendfile_operation_runner_base_t() = delete;

		sendfile_operation_runner_base_t(
			const sendfile_t & sf,
			asio_ns::executor executor,
			Socket & socket,
			after_sendfile_cb_t after_sendfile_cb )
			:	m_file_descriptor{ sf.file_descriptor() }
			,	m_next_write_offset{ sf.offset() }
			,	m_remained_size{ sf.size() }
			,	m_chunk_size{ sf.chunk_size() }
			,	m_expires_after{ std::chrono::steady_clock::now() + sf.timelimit() }
			,	m_executor{ std::move( executor )}
			,	m_socket{ socket }
			,	m_after_sendfile_cb{ std::move( after_sendfile_cb ) }
		{}

		auto expires_after() const noexcept { return m_expires_after; }

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

template<typename Error_Type>
auto
make_error_code( const Error_Type & e ) noexcept
{
	return asio_ns::error_code{ static_cast<int>(e), asio_ns::error::get_system_category() };
}

} /* namespace impl */

} /* namespace restinio */

/*
	Concrete implementations.
*/

#if defined( _MSC_VER ) || defined( __MINGW32__ )
	#include "sendfile_operation_win.ipp"
#elif (defined( __clang__ ) || defined( __GNUC__ )) && !defined(__WIN32__)
	#include "sendfile_operation_posix.ipp"
#else
	#if defined (RESTINIO_ENABLE_SENDFILE_DEFAULT_IMPL)
		#include "sendfile_operation_default.ipp"
	#else
		#error "Sendfile not supported, to enable default implementation define RESTINIO_ENABLE_SENDFILE_DEFAULT_IMPL macro"
	#endif
#endif

