/*
	restinio
*/

/*!
	Sendfile routine definitions (default implementation via <cstdio>).

	@since v.0.4.3
*/

#pragma once

#include <cstdio>
#include <cerrno>

// for fixing #199 (https://github.com/Stiffstream/restinio/issues/199)
// fopen_s seems to be defined in the global namespace.
#include <stdio.h>

namespace restinio
{

/** @name Aliases for sendfile operation.
 */
///@{
using file_descriptor_t = std::FILE*;
using file_offset_t = std::int64_t;
using file_size_t = std::uint64_t;
///@}


/** @name File operations.
 * @brief A minimal set of file operations.
 *
 * Incapsulates the details *cstdio* API for a set of file operations neccessary
 * for sendfile_t class implementation.
 */
///@{

//! Get file descriptor which stands for null.
[[nodiscard]]
constexpr file_descriptor_t null_file_descriptor(){ return nullptr; }

//! Open file.
/*!
 * @note
 * It uses `fopen_s` if the compiler is VC++.
 *
 * @note
 * It's expected that @a file_path contains names in ANSI encoding
 * on Windows platform.
 */
[[nodiscard]]
inline file_descriptor_t
open_file( const char * file_path )
{
//NOTE: fopen_s is only used for VC++ compiler.
#if defined(_MSC_VER)
	file_descriptor_t file_descriptor{};
	const auto result = fopen_s( &file_descriptor, file_path, "rb" );

	if( result )
	{
		const auto err_code = errno;
		throw exception_t{
			fmt::format(
					RESTINIO_FMT_FORMAT_STRING( "fopen_s('{}') failed; errno={}" ),
					file_path, err_code )
		};
	}

	return file_descriptor;
#else
	file_descriptor_t file_descriptor = std::fopen( file_path, "rb" );

	if( null_file_descriptor() == file_descriptor )
	{
		throw exception_t{
			fmt::format(
					RESTINIO_FMT_FORMAT_STRING( "std::fopen failed: '{}'" ),
					file_path )
		};
	}

	return file_descriptor;
#endif
}

/*!
 * @brief Helper function that accepts std::filesystem::path.
 *
 * @note
 * It uses `_wfopen_s` if the compiler is VC++.
 *
 * @note
 * It's expected that @a file_path contains names in ANSI encoding
 * on Windows platform.
 *
 * @since v.0.7.1
 */
[[nodiscard]]
inline file_descriptor_t
open_file( const std::filesystem::path & file_path )
{
//NOTE: _wfopen_s is only used for VC++ compiler.
#if defined(_MSC_VER)
	file_descriptor_t file_descriptor{};
	const auto result = _wfopen_s( &file_descriptor, file_path.c_str(), L"rb" );

	if( result )
	{
		const auto err_code = errno;
		throw exception_t{
			fmt::format(
					RESTINIO_FMT_FORMAT_STRING( "_wfopen_s failed; errno={}" ),
					err_code )
		};
	}

	return file_descriptor;
#else
	// Just delegate to ordinary open_file assuming that file_path is in UTF-8.
	return open_file( file_path.c_str() );
#endif
}

//! Get file size.
template < typename META >
[[nodiscard]]
META
get_file_meta( file_descriptor_t fd )
{
	file_size_t fsize = 0;

	if( null_file_descriptor() != fd )
	{
		// Obtain file size:
		if( 0 == std::fseek( fd , 0 , SEEK_END ) )
		{
			const auto end_pos = std::ftell( fd );

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

	// No way to get last modification,
	// Use current time instead.
	return META{ fsize, std::chrono::system_clock::now() };
}

//! Close file by its descriptor.
inline void
close_file( file_descriptor_t fd )
{
	std::fclose( fd );
}
///@}

} /* namespace restinio */
