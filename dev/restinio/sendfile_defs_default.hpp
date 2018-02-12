/*
	restinio
*/

/*!
	Sendfile routine.
*/

#pragma once

#include <cstdio>

namespace restinio
{

using file_descriptor_t = std::FILE*;
constexpr file_descriptor_t null_file_descriptor = nullptr;
using file_offset_t = std::size_t;
using file_size_t = std::size_t;


//! Open file.
inline file_descriptor_t
open_file( const char * file_path, open_file_errh_t err_handling )
{
	file_descriptor_t file_descriptor = std::fopen( file_path, "rb" );

	if( null_file_descriptor == file_descriptor )
	{
		if( open_file_errh_t::throw_err == err_handling )
		{
			throw exception_t{
				fmt::format( "unable to openfile '{}'", file_path ) };
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

	if( null_file_descriptor != fd )
	{
		// Obtain file size:
		std::fseek( fd , 0 , SEEK_END );
		fsize = std::ftell( fd );
		std::rewind( fd );
	}

	return fsize;
}

//! Close file by its descriptor proxy function.
inline void
close_file( file_descriptor_t fd )
{
	std::fclose( fd );
}

} /* namespace restinio */
