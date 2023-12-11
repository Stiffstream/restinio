/*
	restinio
*/

/*!
	Sendfile routine definitions (win implementation).

	@since v.0.4.3
*/

#pragma once

//eao197: this code has to be uncommented to check the default
//implementation of sendfile operation.
//#if defined(RESTINIO_ASIO_HAS_WINDOWS_OVERLAPPED_PTR)
//#undef RESTINIO_ASIO_HAS_WINDOWS_OVERLAPPED_PTR
//#endif

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
[[nodiscard]]
inline file_descriptor_t null_file_descriptor(){ return INVALID_HANDLE_VALUE; }

//! Open file.
[[nodiscard]]
inline file_descriptor_t
open_file( const char * file_path )
{
	file_descriptor_t file_descriptor =
		// We don't support Unicode on Windows, so call Ansi-version of
		// CreateFile directly.
		::CreateFileA(
			file_path,
			GENERIC_READ,
			FILE_SHARE_READ,
			0,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			0 );

	if( null_file_descriptor() == file_descriptor )
	{
		throw exception_t{
			fmt::format(
					RESTINIO_FMT_FORMAT_STRING( "unable to openfile '{}': error({})" ),
					file_path, GetLastError() )
		};
	}

	return file_descriptor;
}

/*!
 * @brief Version of %open_file that accepts std::filesystem::path.
 *
 * @attention
 * It uses std::filesystem::path::wstring() to get the file name and
 * calls CreateFileW. We assume that @a file_path contains a valid
 * file name constructed from a wide-char string or from utf-8 string
 * literal (as `const std::char8_t[N]` in C++20). Or @a file_path is
 * specified as a narrow string, but it can be automatically converted
 * to wide-string in the current code page.
 *
 * @since v.0.7.1
 */
[[nodiscard]]
inline file_descriptor_t
open_file( const std::filesystem::path & file_path )
{
	const auto wide_file_path = file_path.wstring();
	file_descriptor_t file_descriptor =
		// Use wide-char version of CreateFile.
		::CreateFileW(
			wide_file_path.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ,
			0,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			0 );

	if( null_file_descriptor() == file_descriptor )
	{
		//NOTE(eao197): I don't know a simple way to add `file_path` value into
		//error message (with respect to the fact that file_path can contain name
		//in Unicode, in UCS-2, but not in UTF-8).
		//Because of that the current version doesn't include file name in the
		//error description.
		throw exception_t{
			fmt::format(
					RESTINIO_FMT_FORMAT_STRING(
							"open_file(std::filesystem::path) "
							"unable to openfile: error({})" ),
					GetLastError() )
		};
	}

	return file_descriptor;
}


//! Get file meta.
template < typename META >
[[nodiscard]]
META
get_file_meta( file_descriptor_t fd )
{
	file_size_t fsize = 0;
	std::chrono::system_clock::time_point flastmodified;

	if( null_file_descriptor() != fd )
	{
		LARGE_INTEGER file_size;
		// Obtain file size:
		if( GetFileSizeEx( fd, &file_size ) )
		{
			fsize = static_cast< file_size_t >( file_size.QuadPart );
		}
		else
		{
			throw exception_t{
				fmt::format(
						RESTINIO_FMT_FORMAT_STRING(
							"unable to get file size: error code:{}" ),
						GetLastError() )
			};
		}

		FILETIME ftWrite;
		if( GetFileTime( fd, NULL, NULL, &ftWrite ) )
		{
			// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724284(v=vs.85).aspx

			// Microseconds between 1601-01-01 00:00:00 UTC and 1970-01-01 00:00:00 UTC
			constexpr std::uint64_t nanosec100_in_microsec = 10;
			constexpr std::uint64_t epoch_difference_in_microsec = 
				11644473600ULL * 1000 *1000;

			// First convert 100-ns intervals to microseconds, then adjust for the
			// epoch difference
			ULARGE_INTEGER ull;
			ull.LowPart = ftWrite.dwLowDateTime;
			ull.HighPart = ftWrite.dwHighDateTime;

			flastmodified = 
				std::chrono::system_clock::time_point{
					std::chrono::microseconds( 
						ull.QuadPart / nanosec100_in_microsec - epoch_difference_in_microsec ) };
		}
		else
		{
			throw exception_t{
				fmt::format( 
					RESTINIO_FMT_FORMAT_STRING(
						"unable to get file last modification: error code:{}" ),
					GetLastError() )
			};
		}
	}

	return META{ fsize, flastmodified};
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
