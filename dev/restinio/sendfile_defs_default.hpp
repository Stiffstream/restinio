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
constexpr file_descriptor_t null_file_descriptor(){ return nullptr; }

using file_offset_t = std::int64_t;
using file_size_t = std::uint64_t;

//! Open file.
inline file_descriptor_t
open_file( const char * file_path )
{
	file_descriptor_t file_descriptor = std::fopen( file_path, "rb" );

	if( null_file_descriptor() == file_descriptor )
	{
		throw exception_t{ fmt::format( "std::fopen failed: '{}'", file_path ) };
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
		// Obtain file size:
		if( 0 == std::fseek( fd , 0 , SEEK_END ) )
		{
			const auto end_pos = std::ftell( fd );

			static_assert(
				sizeof( end_pos ) == sizeof(file_size_t),
				"Different values width are not allowed" );

			if( -1 != end_pos )
			{
				fsize = static_cast< file_size_t >( end_pos );
				std::rewind( fd );
			}
			else
			{
				throw exception_t{ "std::ftell failed" };
			}
		}
		else
		{
			throw exception_t{ "std::fseek failed" };
		}
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
