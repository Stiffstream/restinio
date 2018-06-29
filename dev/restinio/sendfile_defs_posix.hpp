/*
	restinio
*/

/*!
	Sendfile routine definitions (posix implementation).

	@since v.0.4.3
*/

#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <iostream>

namespace restinio
{

#if defined( __FreeBSD__ )
	#define RESTINIO_FREEBSD_TARGET
#elif defined(__APPLE__) && defined( __MACH__ )
	#define RESTINIO_MACOS_TARGET
#endif

/** @name Aliases for sendfile operation.
 */
///@{
using file_descriptor_t = int;
using file_offset_t = std::int64_t;
using file_size_t = std::uint64_t;
///@}

/** @name File operations.
 * @brief A minimal set of file operations.
 *
 * Incapsulates details of native API for a set of file operations neccessary
 * for sendfile_t class implementation.
 */
///@{

//! Get file descriptor which stands for null.
constexpr file_descriptor_t null_file_descriptor(){ return -1; }

//! Open file.
inline file_descriptor_t
open_file( const char * file_path)
{
#if defined( RESTINIO_FREEBSD_TARGET ) || defined( RESTINIO_MACOS_TARGET )
	file_descriptor_t file_descriptor = ::open( file_path, O_RDONLY );
#else
	file_descriptor_t file_descriptor = ::open( file_path, O_RDONLY | O_LARGEFILE );
#endif
	if( null_file_descriptor() == file_descriptor )
	{
		throw exception_t{
			fmt::format( "unable to openfile '{}': {}", file_path, strerror( errno ) ) };
	}
	return file_descriptor;
}

//! Get file meta.
template < typename META >
META
get_file_meta( file_descriptor_t fd )
{
	if( null_file_descriptor() == fd )
	{
		throw exception_t{ "invalid file descriptor" };
	}

#if defined( RESTINIO_FREEBSD_TARGET ) || defined( RESTINIO_MACOS_TARGET )
	struct stat file_stat;

	const auto fstat_rc = ::fstat( fd, &file_stat );
#else
	struct stat64 file_stat;

	const auto fstat_rc = fstat64( fd, &file_stat );
#endif

	if( 0 != fstat_rc )
	{
		throw exception_t{
			fmt::format( "unable to get file stat : {}", strerror( errno ) ) };
	}

	const std::chrono::system_clock::time_point
		last_modified{
#if defined( RESTINIO_MACOS_TARGET )
			std::chrono::seconds( file_stat.st_mtimespec.tv_sec ) +
				std::chrono::microseconds( file_stat.st_mtimespec.tv_nsec / 1000 )
#else
			std::chrono::seconds( file_stat.st_mtim.tv_sec ) +
				std::chrono::microseconds( file_stat.st_mtim.tv_nsec / 1000 )
#endif
		};

	return META{ static_cast< file_size_t >( file_stat.st_size ), last_modified };
}

//! Close file by its descriptor.
inline void
close_file( file_descriptor_t fd )
{
	::close( fd );
}
///@}

} /* namespace restinio */
