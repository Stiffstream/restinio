/*
	restinio
*/

/*!
	Sendfile routine.
*/

#pragma once

#include <string>
#include <chrono>
#include <array>

#include <fmt/format.h>

#include <restinio/string_view.hpp>
#include <restinio/exception.hpp>

/*
	Defenitions for:
		file_descriptor_t
		file_offset_t
		file_size_t
*/

#if defined( _MSC_VER )
	#include "sendfile_defs_win.hpp"
#elif (defined( __clang__ ) || defined( __GNUC__ )) && !defined(__WIN32__)
	#include "sendfile_defs_posix.hpp"
#else
	#include "sendfile_defs_default.hpp"
#endif

namespace restinio
{

//! Default chunk size for sendfile operation.
constexpr file_size_t sendfile_default_chunk_size = 1024 * 1024;

//! Maximum size of a chunk
constexpr file_size_t sendfile_max_chunk_size = 1024 * 1024 * 1024;

//
// sendfile_chunk_size_guarded_value_t
//

//! A guard class for setting chunk size.
/*!
	If chunk_size_value does not fit in [1, sendfile_max_chunk_size].
	interval then it is shrinked to fit in the interval.
*/
class sendfile_chunk_size_guarded_value_t
{
		static file_size_t
		clarify_chunk_size( file_size_t chunk_size_value )
		{
			if( 0 == chunk_size_value )
				return sendfile_default_chunk_size;

			if( sendfile_max_chunk_size < chunk_size_value )
				return sendfile_max_chunk_size;

			return chunk_size_value;
		}

	public:
		sendfile_chunk_size_guarded_value_t( file_size_t chunk_size_value )
			:	m_chunk_size{ clarify_chunk_size( chunk_size_value ) }
		{}

		file_size_t
		value( ) const
		{
			return m_chunk_size;
		}

	private:
		const file_size_t m_chunk_size;
};

//
// file_descriptor_holder_t
//

//! Wrapper class for working with native file handler.
/*
	Class is responsible for managing file descriptor as resource.
*/
class file_descriptor_holder_t
{
	public:
		friend void
		swap( file_descriptor_holder_t & left, file_descriptor_holder_t & right ) noexcept
		{
			using std::swap;
			swap( left.m_file_descriptor, right.m_file_descriptor );
		}

		file_descriptor_holder_t( file_descriptor_t fd )
			:	m_file_descriptor{ fd }
		{}

		file_descriptor_holder_t( const file_descriptor_holder_t & ) = delete;
		const file_descriptor_holder_t & operator = ( const file_descriptor_holder_t & ) = delete;

		file_descriptor_holder_t( file_descriptor_holder_t && fdh ) noexcept
			:	m_file_descriptor{ fdh.m_file_descriptor }
		{
			fdh.release();
		}

		file_descriptor_holder_t & operator = ( file_descriptor_holder_t && fdh ) noexcept
		{
			if( this != &fdh )
			{
				file_descriptor_holder_t tmp{ std::move( fdh ) };
				swap( *this, tmp );
			}
			return *this;
		}

		~file_descriptor_holder_t()
		{
			if( is_valid() )
				close_file( m_file_descriptor );
		}

		//! Check if file descriptor is valid.
		bool is_valid() const noexcept
		{
			return null_file_descriptor() != m_file_descriptor;
		}

		//Get file descriptor.
		file_descriptor_t fd() const noexcept
		{
			return m_file_descriptor;
		}

		// Release stored descriptor.
		void release() noexcept
		{
			m_file_descriptor = null_file_descriptor();
		}

	private:
		//! Target file descriptor.
		file_descriptor_t m_file_descriptor;
};

//
// sendfile_t
//

//! Send file write operation description.
/*!
	Class gives a fluen-interface for setting various parameters
	for performing send file operation.
*/
class sendfile_t
{
		friend sendfile_t sendfile( file_descriptor_holder_t , file_size_t , file_size_t );

		sendfile_t(
			file_descriptor_holder_t fdh,
			file_size_t file_total_size,
			sendfile_chunk_size_guarded_value_t chunk )
			:	m_file_descriptor{ std::move( fdh ) }
			,	m_file_total_size{ file_total_size }
			,	m_offset{ 0 }
			,	m_size{ file_total_size }
			,	m_chunk_size{ chunk.value() }
			,	m_timelimit{ std::chrono::steady_clock::duration::zero() }
		{}

	public:
		friend void swap( sendfile_t & left, sendfile_t & right ) noexcept
		{
			using std::swap;
			std::swap( left.m_file_descriptor, right.m_file_descriptor );
			std::swap( left.m_file_total_size, right.m_file_total_size );
			std::swap( left.m_offset, right.m_offset );
			std::swap( left.m_size, right.m_size );
			std::swap( left.m_chunk_size, right.m_chunk_size );
			std::swap( left.m_timelimit, right.m_timelimit );
		}

		sendfile_t( const sendfile_t & ) = delete;
		const sendfile_t & operator = ( const sendfile_t & ) = delete;

		sendfile_t( sendfile_t && sf ) noexcept
			:	m_file_descriptor{ std::move( sf.m_file_descriptor ) }
			,	m_file_total_size{ sf.m_file_total_size }
			,	m_offset{ sf.m_offset }
			,	m_size{ sf.m_size }
			,	m_chunk_size{ sf.m_chunk_size }
			,	m_timelimit{ sf.m_timelimit }
		{}

		sendfile_t & operator = ( sendfile_t && sf ) noexcept
		{
			if( this != &sf )
			{
				sendfile_t tmp{ std::move( sf ) };
				swap( *this, tmp );
			}

			return *this;
		}

		//! Check if file is valid.
		bool is_valid() const noexcept { return m_file_descriptor.is_valid(); }

		//! Get total file size.
		auto file_total_size() const noexcept { return m_file_total_size; }

		//! Get offset of data to write.
		auto offset() const noexcept { return m_offset; }

		//! Get size of data to write.
		auto size() const noexcept { return m_size; }

		//! Set file offset and size.
		//! \{

		sendfile_t &
		offset_and_size(
			file_offset_t offset_value,
			file_size_t size_value = std::numeric_limits< file_size_t >::max() ) &
		{
			check_file_is_valid();

			if( static_cast< file_size_t >( offset_value ) > m_file_total_size )
			{
				throw exception_t{
					fmt::format(
						"invalid file offset: {}, while file size is {}",
						offset_value,
						m_file_total_size ) };
			}

			m_offset = offset_value;
			m_size =
				std::min< file_size_t >(
					m_file_total_size - static_cast< file_size_t >( offset_value ),
					size_value );

			return *this;
		}

		sendfile_t &&
		offset_and_size(
			file_offset_t offset_value,
			file_size_t size_value = std::numeric_limits< file_size_t >::max() ) &&
		{
			return std::move( this->offset_and_size( offset_value, size_value ) );
		}
		//! \}


		auto chunk_size() const { return m_chunk_size; }

		//! Set prefered chunk size to use in  write operation.
		//! \{
		sendfile_t &
		chunk_size( sendfile_chunk_size_guarded_value_t chunk ) &
		{
			check_file_is_valid();

			m_chunk_size = chunk.value();
			return *this;
		}

		sendfile_t &&
		chunk_size( sendfile_chunk_size_guarded_value_t chunk ) &&
		{
			return std::move( this->chunk_size( chunk ) );
		}
		//! \}

		auto timelimit() const noexcept { return m_timelimit; }

		//! Set timelimit on  write operation.
		//! \{
		sendfile_t &
		timelimit( std::chrono::steady_clock::duration timelimit_value ) &
		{
			check_file_is_valid();

			m_timelimit = std::max( timelimit_value, std::chrono::steady_clock::duration::zero() );
			return *this;
		}

		sendfile_t &&
		timelimit( std::chrono::steady_clock::duration timelimit_value ) &&
		{
			return std::move( this->timelimit( timelimit_value ) );
		}
		//! \}

		file_descriptor_t
		file_descriptor() const noexcept
		{
			return m_file_descriptor.fd();
		}

	private:
		void
		check_file_is_valid() const
		{
			if( !is_valid() )
			{
				throw exception_t{ "invalid file descriptor" };
			}
		}

		file_descriptor_holder_t m_file_descriptor;
		file_size_t m_file_total_size;

		//! Data.
		//! \{
		file_offset_t m_offset;
		file_size_t m_size;
		//! \}

		//! A prefered chunk size for a single write call.
		file_size_t m_chunk_size;

		//! Timelimit for writing all the given data.
		/*!
			Zero value stands for default write operation timeout.
		*/
		std::chrono::steady_clock::duration m_timelimit{ std::chrono::steady_clock::duration::zero() };
};

//
// sendfile()
//

//! A group of function to create sendfile_t, that is convertad to writable items
//! used as a part of response.
//! \{

//! Create sendfile optionswith a given file and its given size.
/*!
	\note Parameters are not checked and are trusted as is.
*/
inline sendfile_t
sendfile(
	file_descriptor_holder_t fd,
	file_size_t total_file_size,
	file_size_t chunk_size = sendfile_default_chunk_size )
{
	return sendfile_t{ std::move( fd ), total_file_size, chunk_size };
}

inline sendfile_t
sendfile(
	const char * file_path,
	file_size_t chunk_size = sendfile_default_chunk_size )
{
	file_descriptor_holder_t fd{ open_file( file_path ) };

	auto total_file_size = size_of_file( fd.fd() );

	return sendfile( std::move( fd ), total_file_size , chunk_size );
}

inline sendfile_t
sendfile(
	const std::string & file_path,
	file_size_t chunk_size = sendfile_default_chunk_size )
{
	return sendfile( file_path.c_str(), chunk_size );
}

inline sendfile_t
sendfile(
	string_view_t file_path,
	file_size_t chunk_size = sendfile_default_chunk_size )
{
	return
		sendfile(
			std::string{ file_path.data(), file_path.size() },
			chunk_size );
}

//! \}

} /* namespace restinio */
