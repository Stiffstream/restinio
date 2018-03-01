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
	Ensures that the value of chunk size is at least 1 and at most sendfile_max_chunk_size.
*/
struct sendfile_chunk_size_guarded_value_t
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

		sendfile_chunk_size_guarded_value_t( const sendfile_chunk_size_guarded_value_t & ) noexcept = default;
		sendfile_chunk_size_guarded_value_t( sendfile_chunk_size_guarded_value_t && ) noexcept = default;
		const sendfile_chunk_size_guarded_value_t & operator = ( const sendfile_chunk_size_guarded_value_t & ) = delete;
		sendfile_chunk_size_guarded_value_t & operator = ( sendfile_chunk_size_guarded_value_t && ) = delete;

		file_size_t
		value( ) const
		{
			return m_chunk_size;
		}

	private:
		const file_size_t m_chunk_size;
};

//
// sendfile_t
//

//! Send file write operation description.
/*!
	Class gives a fluen-interface for setting various parameters
	for performin send file operation.
*/
class sendfile_t
{
		friend sendfile_t sendfile( file_descriptor_t , file_size_t , file_size_t );

		sendfile_t(
			file_descriptor_t fd,
			file_size_t file_total_size,
			sendfile_chunk_size_guarded_value_t chunk )
			:	m_file_descriptor{ fd }
			,	m_file_total_size{ file_total_size }
			,	m_offset{ 0 }
			,	m_size{ file_total_size }
			,	m_chunk_size{ chunk.value() }
			,	m_timelimit{ std::chrono::steady_clock::duration::zero() }
		{}

	public:
		sendfile_t( const sendfile_t & ) = delete;
		const sendfile_t & operator = ( const sendfile_t & ) = delete;

		sendfile_t(
			sendfile_t && sf ) noexcept
			:	m_file_descriptor{ sf.m_file_descriptor }
			,	m_file_total_size{ sf.m_file_total_size }
			,	m_offset{ sf.m_offset }
			,	m_size{ sf.m_size }
			,	m_chunk_size{ sf.m_chunk_size }
			,	m_timelimit{ sf.m_timelimit }
		{
			sf.m_file_descriptor = null_file_descriptor();
		}

		sendfile_t & operator = ( sendfile_t && sf ) noexcept
		{
			sendfile_t tmp{ std::move( sf ) };
			swap( tmp );
			return *this;
		}

		~sendfile_t()
		{
			if( is_valid() )
			{
				close_file( m_file_descriptor );
			}
		}

		void
		swap( sendfile_t & sf ) noexcept
		{
			if( this != &sf )
			{
				std::swap( m_file_descriptor, sf.m_file_descriptor );
				std::swap( m_file_total_size, sf.m_file_total_size );
				std::swap( m_offset, sf.m_offset );
				std::swap( m_size, sf.m_size );
				std::swap( m_chunk_size, sf.m_chunk_size );
				std::swap( m_timelimit, sf.m_timelimit );
			}
		}

		//! Check if file is valid.
		bool is_valid() const { return null_file_descriptor() != m_file_descriptor; }

		//! Get total file size.
		auto file_total_size() const { return m_file_total_size; }

		//! Get offset of data to write.
		auto offset() const { return m_offset; }

		//! Get size of data to write.
		auto size() const { return m_size; }

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

		auto timelimit() const { return m_timelimit; }

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
		file_descriptor() const
		{
			return m_file_descriptor;
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

		file_descriptor_t m_file_descriptor;
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

inline void
swap( sendfile_t & sf_opts1, sendfile_t & sf_opts2 ) noexcept
{
	sf_opts1.swap( sf_opts2 );
}

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
	file_descriptor_t fd,
	file_size_t total_file_size,
	file_size_t chunk_size = sendfile_default_chunk_size )
{
	return sendfile_t{ fd, total_file_size, chunk_size };
}

inline sendfile_t
sendfile(
	const char * file_path,
	file_size_t chunk_size = sendfile_default_chunk_size )
{
	const file_descriptor_t fd = open_file( file_path );

	file_size_t total_file_size;

	try
	{
		total_file_size = size_of_file( fd );
	}
	catch( ... )
	{
		// Close the file as there is no owner after rethrow.
		close_file( fd );
		throw;
	}

	return sendfile( fd, total_file_size , chunk_size );
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
