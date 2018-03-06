/*
	restinio
*/

/*!
	Sendfile routine definitions (win implementation).

	@since v.0.4.3
*/

#pragma once

#if defined(RESTINIO_ASIO_HAS_WINDOWS_OVERLAPPED_PTR)

#include <cstdio>

namespace restinio
{

/** @name Aliases for sendfile operation.
 */
///@{
using file_descriptor_t = HANDLE;
using file_offset_t = std::uint64_t;
using file_size_t = std::uint64_t;
///@}

/** @name File operations.
 * @brief A minimal set of file operations.
 *
 * Incapsulates details of windows API for a set of file operations neccessary
 * for sendfile_t class implementation.
 */
///@{
//! Get file descriptor which stands for null.
inline file_descriptor_t null_file_descriptor(){ return INVALID_HANDLE_VALUE; }

//! Open file.
inline file_descriptor_t
open_file( const char * file_path )
{
	file_descriptor_t file_descriptor =
		::CreateFile(
			file_path,
			GENERIC_READ,
			0,
			0,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			0 );

	if( null_file_descriptor() == file_descriptor )
	{
		throw exception_t{
			fmt::format( "unable to openfile '{}': error({})", file_path, GetLastError() ) };
	}

	return file_descriptor;
}

//! Get file size.
inline file_size_t
size_of_file( file_descriptor_t fd )
{
	file_size_t fsize = 0;

	if( null_file_descriptor() != fd )
	{
		LARGE_INTEGER file_size;
		// Obtain file size:
		if( GetFileSizeEx( fd, &file_size ) )
		{
			fsize = static_cast< file_size_t >( file_size.QuadPart );
		}
	}

	return fsize;
}

//! Close file by its descriptor.
inline void
close_file( file_descriptor_t fd )
{
	CloseHandle( fd );
}
///@}

} /* namespace restinio */

#else // #if defined(RESTINIO_ASIO_HAS_WINDOWS_OVERLAPPED_PTR)

#include <restinio/sendfile_defs_default.hpp>

#endif // #if defined(RESTINIO_ASIO_HAS_WINDOWS_OVERLAPPED_PTR)
