/*
	restinio
*/

/*!
	Transformator of data streams using zlib.

	@since v.0.4.4
*/

#pragma once

#include <string>
#include <cstring>

#include <fmt/format.h>

#include <zlib.h>

#include <restinio/exception.hpp>
#include <restinio/string_view.hpp>

namespace restinio
{

namespace transformator
{

//! Default reserve buffer size for zlib transformator.
//! @since v.0.4.4
constexpr std::size_t default_zlib_output_reserve_buffer_size = 256 * 1024;

/** @name Default values for zlib tuning parameters.
 * @brief Constants are defined with values provided by zlib.
 *
 * @since v.0.4.4
*/
///@{
constexpr int default_zlib_window_bits = MAX_WBITS;
constexpr int default_zlib_mem_level = MAX_MEM_LEVEL;
constexpr int default_zlib_strategy = Z_DEFAULT_STRATEGY;
///@}

//
// zlib_settings_t
//

//! Parameters of performing data transformation with zlib.
/*
	@since v.0.4.4
*/
class zlib_params_t
{
	public:
		//! Types of transformation.
		enum class operation_t
		{
			//! Compress the input data.
			compress,
			//! Decompress input data.
			decompress
		};

		//! Formats of compressed data.
		enum class format_t
		{
			//! zlib format.
			deflate,
			//! gzip format
			gzip
		};

		//! Init constructor.
		/*!
			It's better to use special functions to cunstruct
			initial zlib_params_t, see:
			deflate_compress(), deflate_decompress(),
			gzip_compress(), gzip_decompress(),
		*/
		zlib_params_t(
			//! Operation: compress or decompress.
			operation_t op,
			//! Foramt: deflate or gzip.
			format_t f,
			//! Compression level.
			int l = -1 )
			:	m_operation{ op }
			,	m_format{ f }
		{
			level( l );
		}

		//! Get operation.
		operation_t operation() const { return m_operation; }

		//! Get format.
		format_t format() const { return m_format; }

		//! Get compression level.
		/*!
			\note Makes sense only for compression operation.
		*/
		int level() const { return m_level; }

		//! Set compression level.
		/*!
			Must be an integer value in the range of -1 to 9.

			\note Makes sense only for compression operation.
		*/
		zlib_params_t &
		level( int level_value ) &
		{
			if( level_value < -1 || level_value > 9 )
			{
				throw exception_t{
					fmt::format(
						"invalid compression level: {}, must be "
						"an integer value in the range of -1 to 9",
						level_value ) };
			}

			m_level = level_value;

			return reference_to_self();
		}

		//! Set compression level.
		zlib_params_t &&
		level( int level_value ) &&
		{
			return std::move( this->level( level_value ) );
		}

		//! Get window_bits.
		int window_bits() const { return m_window_bits; }

		//! Set window_bits.
		/*!
			Must be an integer value in the range of 8 to MAX_WBITS.

			\note For descompress operation it is better to use default
			value of windows beats. See https://zlib.net/manual.html
			inflateInit2() description for details.
		*/
		zlib_params_t &
		window_bits( int window_bits_value ) &
		{
			// From https://zlib.net/manual.html:
			// For the current implementation of deflate(),
			// a windowBits value of 8 (a window size of 256 bytes)
			// is not supported. As a result, a request for 8 will result in 9
			// (a 512-byte window). In that case, providing 8 to inflateInit2()
			// will result in an error when the zlib header with 9 is
			// checked against the initialization of inflate().
			// The remedy is to not use 8 with deflateInit2()
			// with this initialization, or at least in that case use 9
			// with inflateInit2().
			// ...
			// windowBits can also be zero to request that inflate
			// use the window size in the zlib header of the compressed
			// stream.

			if( ( window_bits_value < 8 || window_bits_value > MAX_WBITS ) &&
				( 0 != window_bits_value || operation_t::decompress != operation() ) )
			{
				throw exception_t{
					fmt::format(
						"invalid window_bits: {}, must be "
						"an integer value in the range of 8 to {} or "
						"0 for decompress operation",
						window_bits_value,
						MAX_WBITS ) };
			}

			if( 8 == window_bits_value )
				window_bits_value = 9;

			m_window_bits = window_bits_value;

			return reference_to_self();
		}

		//! Set window_bits.
		zlib_params_t &&
		window_bits( int window_bits_value ) &&
		{
			return std::move( this->window_bits( window_bits_value ) );
		}

		//! Get compression mem_level.
		/*!
			\note Makes sense only for compression operation.
		*/
		int mem_level() const { return m_mem_level; }

		//! Set compression mem_level.
		/*!
			Must be an integer value in the range of -1 to 9.

			\note Makes sense only for compression operation.
		*/
		zlib_params_t &
		mem_level( int mem_level_value ) &
		{
			if( mem_level_value < 1 || mem_level_value > MAX_MEM_LEVEL )
			{
				throw exception_t{
					fmt::format(
						"invalid compression mem_level: {}, must be "
						"an integer value in the range of 1 to {}",
						mem_level_value,
						MAX_MEM_LEVEL ) };
			}

			m_mem_level = mem_level_value;

			return reference_to_self();
		}

		//! Set compression mem_level.
		zlib_params_t &&
		mem_level( int mem_level_value ) &&
		{
			return std::move( this->mem_level( mem_level_value ) );
		}

		//! Get compression strategy.
		/*!
			\note Makes sense only for compression operation.
		*/
		int strategy() const { return m_strategy; }

		//! Set compression strategy.
		/*!
			Must be an integer value in the range of -1 to 9.

			\note Makes sense only for compression operation.
		*/
		zlib_params_t &
		strategy( int strategy_value ) &
		{
			if( Z_DEFAULT_STRATEGY != strategy_value &&
				Z_FILTERED != strategy_value &&
				Z_HUFFMAN_ONLY != strategy_value &&
				Z_RLE != strategy_value )
			{
				throw exception_t{
					fmt::format(
						"invalid compression strategy: {}, must be "
						"one of: "
						"Z_DEFAULT_STRATEGY({}), ",
						"Z_FILTERED({}), ",
						"Z_HUFFMAN_ONLY({}), ",
						"Z_RLE({})",
						strategy_value,
						Z_DEFAULT_STRATEGY,
						Z_FILTERED,
						Z_HUFFMAN_ONLY,
						Z_RLE ) };
			}

			m_strategy = strategy_value;

			return reference_to_self();
		}

		//! Set compression strategy.
		zlib_params_t &&
		strategy( int strategy_value ) &&
		{
			return std::move( this->strategy( strategy_value ) );
		}

		//! Get the size initially reserved for buffer.
		/*!
			When using zlib transformator out buffer is used.
			The initial size of such buffer must be defined.
			zlib_t instance will use this parameter
			as the initial size of out buffer and as an increment size
			if out buffer must be enlarged.
		*/
		std::size_t reserve_buffer_size() const { return m_reserve_buffer_size; }

		//! Set the size initially reserved for buffer.
		zlib_params_t &
		reserve_buffer_size( std::size_t size ) &
		{
			if( size < 10UL )
			{
				throw exception_t{ "too small reserve buffer size" };
			}

			m_reserve_buffer_size = size;

			return reference_to_self();
		}

		//! Set the size initially reserved for buffer.
		zlib_params_t &&
		reserve_buffer_size( std::size_t size ) &&
		{
			return std::move( this->reserve_buffer_size( size ) );
		}

	private:
		//! Get the reference to self.
		zlib_params_t & reference_to_self() { return *this; }

		//! Transformation type.
		operation_t m_operation;

		//! Format to be used for compressed data.
		format_t m_format;

		//! Compression level.
		/*!
			An integer value in the range of -1 to 9.
		*/
		int m_level;

		int m_window_bits{ default_zlib_window_bits };
		int m_mem_level{ default_zlib_mem_level };
		int m_strategy{ default_zlib_strategy };

		//! Size initially reserved for buffer.
		std::size_t m_reserve_buffer_size{ default_zlib_output_reserve_buffer_size };
};

/** @name Create parameters for zlib transformators.
 * @brief A set of function helping to create zlib_params_t objects
 * ommiting some verbose deteils.
 *
 * Instead of writing something like this:
 * \code
 * zlib_params_t params{
 *     restinio::transformators::zlib_params_t::operation_t::compress,
 *     restinio::transformators::zlib_params_t::format_t::gzip,
 *     -1 };
 * \endcode
 * It is better to write the following:
 * \code
 * zlib_params_t params = restinio::transformators::gzip_compress();
 * \endcode
 *
 * @since v.0.4.4
*/
///@{
inline zlib_params_t
deflate_compress( int level = -1 )
{
	return zlib_params_t{
			zlib_params_t::operation_t::compress,
			zlib_params_t::format_t::deflate,
			level };
}

inline zlib_params_t
deflate_decompress()
{
	return zlib_params_t{
			zlib_params_t::operation_t::decompress,
			zlib_params_t::format_t::deflate };
}

inline zlib_params_t
gzip_compress( int level = -1)
{
	return zlib_params_t{
			zlib_params_t::operation_t::compress,
			zlib_params_t::format_t::gzip,
			level };
}

inline zlib_params_t
gzip_decompress()
{
	return zlib_params_t{
			zlib_params_t::operation_t::decompress,
			zlib_params_t::format_t::gzip };
}
///@}

//
// zlib_t
//

//! Zlib transformator.
/*!
	Uses zlib (https://zlib.net) for gzip/deflate compresion/decompression.

	@since v.0.4.4
*/
class zlib_t
{
	public:
		zlib_t( const zlib_params_t & params )
			:	m_params{ params }
		{
			// Setting allocator stuff before initializing
			// TODO: allocation can be done with user defined allocator.
			m_zlib_stream.zalloc = Z_NULL;
			m_zlib_stream.zfree = Z_NULL;
			m_zlib_stream.opaque = Z_NULL;

			// Track initialization result.
			int init_result;

			// Compression.
			auto current_window_bits = m_params.window_bits();

			if( zlib_params_t::format_t::gzip == m_params.format() )
			{
				current_window_bits += 16;
			}

			if( zlib_params_t::operation_t::compress == m_params.operation() )
			{

				// zlib format.
				init_result =
					deflateInit2(
						&m_zlib_stream,
						m_params.level(),
						Z_DEFLATED,
						current_window_bits,
						m_params.mem_level(),
						m_params.strategy() );
			}
			else
			{
				init_result =
					inflateInit2(
						&m_zlib_stream,
						current_window_bits );
			}

			if( Z_OK != init_result )
			{
				throw exception_t{
					fmt::format(
						"Failed to initialize zlib stream: {}",
						init_result ) };
			}

			m_zlib_stream_initialized = true;

			// Reserve initial buffer.
			inc_buffer();
		}

		zlib_t( const zlib_t & ) = delete;
		zlib_t( zlib_t && ) = delete;
		const zlib_t & operator = ( const zlib_t & ) = delete;
		zlib_t & operator = ( zlib_t && ) = delete;

		~zlib_t()
		{
			if( m_zlib_stream_initialized )
			{
				if( zlib_params_t::operation_t::compress == m_params.operation() )
				{
					deflateEnd( &m_zlib_stream );
				}
				else
				{
					inflateEnd( &m_zlib_stream );
				}
			}
		}

		//! Append input data.
		void
		write( string_view_t sv )
		{
			ensure_operation_in_not_completed();

			if( std::numeric_limits< decltype( m_zlib_stream.avail_in ) >::max() < sv.size() )
			{
				throw exception_t{
					fmt::format(
						"input data is too large: {} (max possible: {}), "
						"try to break large data into pieces",
						sv.size(),
						std::numeric_limits< decltype( m_zlib_stream.avail_in ) >::max() ) };
			}

			if( 0 < sv.size() )
			{
				// const Bytef* x = reinterpret_cast< const Bytef* >( sv.data() );
				m_zlib_stream.next_in =
					reinterpret_cast< Bytef* >( const_cast< char* >( sv.data() ) );

				m_zlib_stream.avail_in = static_cast< uInt >( sv.size() );

				if( zlib_params_t::operation_t::compress == m_params.operation() )
				{
					write_compress_impl( Z_NO_FLUSH );
				}
				else
				{
					write_decompress_impl( Z_NO_FLUSH );
				}
			}
		}

		//! Flush the zlib stream.
		/*!
			Flushes underlying zlib stream.
		*/
		void
		flush()
		{
			ensure_operation_in_not_completed();

			m_zlib_stream.next_in = nullptr;
			m_zlib_stream.avail_in = static_cast< uInt >( 0 );

			if( zlib_params_t::operation_t::compress == m_params.operation() )
			{
				write_compress_impl( Z_SYNC_FLUSH );
			}
			else
			{
				write_decompress_impl( Z_SYNC_FLUSH );
			}
		}

		//! Complete the stream.
		void
		complete()
		{
			ensure_operation_in_not_completed();

			m_zlib_stream.next_in = nullptr;
			m_zlib_stream.avail_in = static_cast< uInt >( 0 );

			if( zlib_params_t::operation_t::compress == m_params.operation() )
			{
				write_compress_impl( Z_FINISH );
			}
			else
			{
				write_decompress_impl( Z_FINISH );
			}

			m_operation_is_complete = true;
		}

		//! Get current output.
		/*!
			On this request a currently accumulated output data is reterned.
			Move semantics is applied. Once current output is fetched
			zlib_t object reset its internal out buffer.

			In the following code:
			\code
			restinio::transformator::zlib_t z{ restinio::transformator::gzip_compress() };

			z.write( A );
			consume_out( z.giveaway_output() ); // (1)

			z.write( B );
			z.write( C );
			consume_out( z.giveaway_output() ); // (2)

			\endcode
			At the point (2) `consume_out()` function receives
			a string that is not an appended version of a string
			received in point (1).
		*/
		std::string
		giveaway_output()
		{
			std::string result;
			const auto data_size = m_write_pos;
			std::swap( result, m_out_buffer );
			m_write_pos = 0;
			result.resize( data_size ); // Shrink output data.
			return result;
		}

		//! Get current output size.
		auto output_size() const { return m_write_pos; }

		//! Is operation complete?
		bool is_completed() const { return m_operation_is_complete; }

	private:
		//! Checks completion flag and throws if operation is is already completed.
		void
		ensure_operation_in_not_completed() const
		{
			if( is_completed() )
				throw exception_t{ "zlib operation is already completed" };
		}

		//! Increment internal buffer for receiving output.
		void
		inc_buffer()
		{
			m_out_buffer.resize(
				m_out_buffer.size() + m_params.reserve_buffer_size() );
		}

		//! Prepare out buffer for receiving data.
		auto
		prepare_out_buffer()
		{
			m_zlib_stream.next_out =
				reinterpret_cast< Bytef* >(
					const_cast< char* >( m_out_buffer.data() + m_write_pos ) );

			const auto provided_out_buffer_size =
				m_out_buffer.size() - m_write_pos;
			m_zlib_stream.avail_out =
				static_cast<uInt>( provided_out_buffer_size );

			return provided_out_buffer_size;
		}

		//! Handle incoming data for compression operation.
		/*
			Data and its size must be already in
			`m_zlib_stream.next_in`, `m_zlib_stream.avail_in`.
		*/
		void
		write_compress_impl( int flush )
		{
			while( true )
			{
				const auto provided_out_buffer_size = prepare_out_buffer();

				int operation_result = deflate( &m_zlib_stream, flush );

				if( !( Z_OK == operation_result ||
						Z_BUF_ERROR == operation_result ||
						( Z_STREAM_END == operation_result && Z_FINISH == flush ) ) )
				{
					const char * err_msg = "<no error desc>";
					if( m_zlib_stream.msg )
						err_msg = m_zlib_stream.msg;

					throw exception_t{
						fmt::format(
							"unexpected result of deflate() (zlib): {}, {}",
							operation_result,
							err_msg ) };
				}

				m_write_pos += provided_out_buffer_size - m_zlib_stream.avail_out;

				if( 0 == m_zlib_stream.avail_out && Z_STREAM_END != operation_result )
				{
					// Looks like not all the output was obtained.
					// There is a minor chance that it just happened to
					// occupy exactly the same space that was available,
					// in that case it would go for a one redundant call to deflate.
					inc_buffer();
					continue;
				}

				if( 0 == m_zlib_stream.avail_in )
				{
					// All the input was consumed.
					break;
				}
			}
		}

		//! Handle incoming data for decompression operation.
		/*
			Data and its size must be already in
			`m_zlib_stream.next_in`, `m_zlib_stream.avail_in`.
		*/
		void
		write_decompress_impl( int flush )
		{
			while( true )
			{
				const auto provided_out_buffer_size = prepare_out_buffer();

				int operation_result = inflate( &m_zlib_stream, flush );
				if( !( Z_OK == operation_result ||
						Z_BUF_ERROR == operation_result ||
						Z_STREAM_END == operation_result ) )
				{
					const char * err_msg = "<no error desc>";
					if( m_zlib_stream.msg )
						err_msg = m_zlib_stream.msg;

					throw exception_t{
						fmt::format(
							"unexpected result of inflate() (zlib): {}, {}",
							operation_result,
							err_msg ) };
				}

				m_write_pos += provided_out_buffer_size - m_zlib_stream.avail_out;

				if( 0 == m_zlib_stream.avail_out && Z_STREAM_END != operation_result )
				{
					// Looks like not all the output was obtained.
					// There is a minor chance that it just happened to
					// occupy exactly the same space that was available,
					// in that case it would go for a one redundant call to deflate.
					inc_buffer();
					continue;
				}

				if( 0 == m_zlib_stream.avail_in )
				{
					// All the input was consumed.
					break;
				}
			}
		}

		//! Parameters for zlib.
		const zlib_params_t m_params;

		//! Flag: was m_zlib_stream initialized properly.
		/*!
			If deflateInit2()/inflateInit2() was completed successfully
			it is needed to call deflateEnd()/inflateEnd().
		*/
		bool m_zlib_stream_initialized{ false };

		//! zlib stream.
		z_stream m_zlib_stream;

		//! Output buffer.
		std::string m_out_buffer;
		//! Next write pos in out buffer.
		std::size_t m_write_pos{ 0 };

		bool m_operation_is_complete{ false };
};

} /* namespace transformator */

} /* namespace restinio */
