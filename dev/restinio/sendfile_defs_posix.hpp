/*
	restinio
*/

/*!
	Sendfile routine.
*/

#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace restinio
{

using file_descriptor_t = int;
constexpr file_descriptor_t null_file_descriptor = -1;
using file_offset_t = off_t;
using file_size_t = size_t;

//! Open file.
inline file_descriptor_t
open_file( const char * file_path, open_file_errh_t err_handling )
{
	file_descriptor_t file_descriptor = open( file_path, O_RDONLY );

	if( null_file_descriptor == file_descriptor )
	{
		if( open_file_errh_t::throw_err == err_handling )
		{
			throw exception_t{
				fmt::format( "unable to openfile '{}': {}", file_path, strerror( errno ) ) };
		}
	}
	return file_descriptor;
}

//! Get file size.
inline file_size_t
size_of_file( file_descriptor_t fd, open_file_errh_t err_handling )
{
	if( null_file_descriptor == fd )
		return 0;

	struct stat64 file_stat;
	if( 0 != fstat64( fd, &file_stat ) )
	{
		if( open_file_errh_t::throw_err == err_handling )
			throw exception_t{
				fmt::format( "unable to get file size : {}", strerror( errno ) ) };
		else
			return 0;
	}

	return file_stat.st_size;
}

//! Close file by its descriptor proxy function.
inline void
close_file( file_descriptor_t fd )
{
	::close( fd );
}

} /* namespace restinio */
