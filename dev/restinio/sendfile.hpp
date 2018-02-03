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

#include <string>
#include <chrono>

#include <fmt/format.h>

#include <restinio/exception.hpp>

namespace restinio
{

using file_descriptor_t = int;
using file_offset_t = off64_t;
using file_size_t = size_t;

//
// sendfile_options_t
//

//! Send file write operation settings.
/*!
	Class gives a fluen-interface for setting various parameters
	for performin send file operation.
*/
class sendfile_options_t
{
	public:
		sendfile_options_t( const char * filename )
		{
			m_file_descriptor = open( filename, O_RDONLY );

			if( -1 == m_file_descriptor )
			{
				throw exception_t{
					fmt::format( "unable to openfile '{}': {}", filename, strerror( errno ) ) };
			}

			struct stat file_stat;
			fstat( m_file_descriptor, &file_stat );

			m_size = m_file_total_size = file_stat.st_size;

			// TODO: mb use st_blksize for default chunk size?
		}

		sendfile_options_t( const sendfile_options_t & ) = delete;
		const sendfile_options_t & operator = ( const sendfile_options_t & ) = delete;

		sendfile_options_t( sendfile_options_t && sf_opts )
		{
			*this = std::move( sf_opts );
		}

		sendfile_options_t & operator = ( sendfile_options_t && sf_opts )
		{
			if( this != &sf_opts )
			{
				m_file_descriptor = sf_opts.m_file_descriptor;
				m_file_total_size = sf_opts.m_file_total_size;
				m_offset = sf_opts.m_offset;
				m_size = sf_opts.m_size;
				m_chunk_size = sf_opts.m_chunk_size;
				m_timelimit = sf_opts.m_timelimit;

				sf_opts.m_file_descriptor = -1;
			}

			return *this;
		}

		~sendfile_options_t()
		{
			if( -1 != m_file_descriptor )
			{
				close( m_file_descriptor );
			}
		}

		auto offset() const { return m_offset; }
		auto size() const { return m_size; }

		//! Set file offset and size.
		//! \{

		sendfile_options_t &
		offset_and_size(
			file_offset_t offset_,
			file_size_t size_ = std::numeric_limits< file_size_t >::max() ) &
		{
			if( offset_ > m_file_total_size )
			{
				throw exception_t{
					fmt::format(
						"invalid file offset: {}, while file size is {}",
						offset_,
						m_file_total_size ) };
			}

			m_offset = offset_;
			m_size = std::min< file_size_t >( m_file_total_size - offset_, size_ );

			return *this;
		}

		sendfile_options_t &&
		offset_and_size(
			file_offset_t offset_,
			file_size_t size_ = std::numeric_limits< file_size_t >::max() ) &&
		{
			return std::move( this->offset_and_size( offset_, size_ ) );
		}
		//! \}


		auto chunk_size() const { return m_chunk_size; }

		//! Set prefered chunk size to use in  write operation.
		//! \{
		sendfile_options_t &
		chunk_size( file_size_t chunk_size_ ) &
		{
			m_chunk_size = chunk_size_;
			return *this;
		}

		sendfile_options_t &&
		chunk_size( file_size_t chunk_size_ ) &&
		{
			return std::move( this->chunk_size( chunk_size_ ) );
		}
		//! \}

		auto timelimit() const { return m_timelimit; }

		//! Set timelimit on  write operation.
		//! \{
		sendfile_options_t &
		timelimit( std::chrono::steady_clock::duration timelimit_ ) &
		{
			m_timelimit = std::min( timelimit_, std::chrono::steady_clock::duration::zero() );
			return *this;
		}

		sendfile_options_t &&
		timelimit( std::chrono::steady_clock::duration timelimit_ ) &&
		{
			return std::move( this->timelimit( timelimit_ ) );
		}
		//! \}

		file_descriptor_t
		file_descriptor() const
		{
			return m_file_descriptor;
		}

	private:
		file_descriptor_t m_file_descriptor{ -1 };
		file_size_t m_file_total_size;

		//! Data.
		//! \{
		file_offset_t m_offset{ 0 };
		file_size_t m_size{ 0 }; // Zero means to the end of file.
		//! \}

		//! A prefered chunk size for a single write call.
		file_size_t m_chunk_size = 16 * 1024 * 1024;

		//! Timelimit for writing all the given data.
		/*!
			Zero value stands for default write operation timeout.
		*/
		std::chrono::steady_clock::duration m_timelimit{ std::chrono::steady_clock::duration::zero() };
};

} /* namespace restinio */
