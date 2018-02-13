/*
	restinio
*/

/*!
	Sendfile routine.
*/

#pragma once

#if defined(RESTINIO_ASIO_HAS_WINDOWS_OVERLAPPED_PTR)

#include <cstdio>

namespace restinio
{

using file_descriptor_t = HANDLE;
constexpr file_descriptor_t null_file_descriptor(){ return INVALID_HANDLE_VALUE; }
using file_offset_t = std::size_t;
using file_size_t = std::size_t;

//! Open file.
inline file_descriptor_t
open_file( const char * file_path, open_file_errh_t err_handling )
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
		if( open_file_errh_t::throw_err == err_handling )
		{
			throw exception_t{
				fmt::format( "unable to openfile '{}': error({})", file_path, GetLastError() ) };
		}
		// else ignore
	}

	return file_descriptor;
}

//! Get file size.
inline file_size_t
size_of_file( file_descriptor_t fd, open_file_errh_t err_handling )
{
	file_size_t fsize = 0;

	if( null_file_descriptor() != fd )
	{
		// Obtain file size:
		fsize = GetFileSize( fd, NULL );
	}

	return fsize;
}

//! Close file by its descriptor proxy function.
inline void
close_file( file_descriptor_t fd )
{
	CloseHandle( fd );
}

} /* namespace restinio */

#else // #if defined(RESTINIO_ASIO_HAS_WINDOWS_OVERLAPPED_PTR)

#include <restinio/sendfile_defs_default.hpp>

#endif // #if defined(RESTINIO_ASIO_HAS_WINDOWS_OVERLAPPED_PTR)
