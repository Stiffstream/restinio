/*
	restinio
*/

/*!
	Transformator of data streams using zlib.

	@since v.0.4.4
*/

#pragma once

#include <restinio/impl/include_fmtlib.hpp>

#include <restinio/impl/string_caseless_compare.hpp>

#include <restinio/exception.hpp>
#include <restinio/string_view.hpp>
#include <restinio/message_builders.hpp>
#include <restinio/request_handler.hpp>

#include <zlib.h>

#include <string>
#include <cstring>

namespace restinio
{

namespace transforms
{

namespace zlib
{

//! Default reserve buffer size for zlib transformator.
//! @since v.0.4.4
constexpr std::size_t default_output_reserve_buffer_size = 256 * 1024;

/** @name Default values for zlib tuning parameters.
 * @brief Constants are defined with values provided by zlib.
 *
 * @since v.0.4.4
*/
///@{
constexpr int default_window_bits = MAX_WBITS;
constexpr int default_mem_level = MAX_MEM_LEVEL;
constexpr int default_strategy = Z_DEFAULT_STRATEGY;
///@}

//
// params_t
//

//! Parameters of performing data transformation with zlib.
/*
	@since v.0.4.4

	\note There is a special case for compression format: format_t::identity.
	If this format is set that zlib transformator is transparently copies
	input to output and so all other params are ignored.
*/
class params_t
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
			gzip,
			//! Identity. With semantics descrobed here: https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Accept-Encoding
			/*
				Means that no compression will be used and no header/trailer will be applied.
			*/
			identity
		};

		//! Init constructor.
		/*!
			It's better to use special functions to cunstruct
			initial params_t, see:
			deflate_compress(), deflate_decompress(),
			gzip_compress(), gzip_decompress(),
		*/
		params_t(
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

		//! Default constructor for identiry transformator
		params_t()
			:	m_operation{ operation_t::compress }
			,	m_format{ format_t::identity }
		{
			level( -1 );
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
		params_t &
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
		params_t &&
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
		params_t &
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
		params_t &&
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
			Must be an integer value in the range of 1 to 9.
			1 stands for minimum memory usage and
			9 stands for maximum memory usage.
			The amount of memory that can be used
			affects the quality of compression.

			\note Makes sense only for compression operation.
		*/
		params_t &
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
		params_t &&
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
			Must be an integer value defined by one of
			zlib constants: Z_FILTERED, Z_HUFFMAN_ONLY,
			Z_RLE, Z_DEFAULT_STRATEGY.

			\note Makes sense only for compression operation.
		*/
		params_t &
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
		params_t &&
		strategy( int strategy_value ) &&
		{
			return std::move( this->strategy( strategy_value ) );
		}

		//! Get the size initially reserved for buffer.
		/*!
			When using zlib transformator the outout buffer is used.
			The initial size of such buffer must be defined.
			zlib_t instance will use this parameter
			as the initial size of out buffer and as an increment size
			if out buffer must be enlarged.
		*/
		std::size_t reserve_buffer_size() const { return m_reserve_buffer_size; }

		//! Set the size initially reserved for buffer.
		params_t &
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
		params_t &&
		reserve_buffer_size( std::size_t size ) &&
		{
			return std::move( this->reserve_buffer_size( size ) );
		}

	private:
		//! Get the reference to self.
		params_t & reference_to_self() { return *this; }

		//! Transformation type.
		operation_t m_operation;

		//! Format to be used for compressed data.
		format_t m_format;

		//! Compression level.
		/*!
			An integer value in the range of -1 to 9.
		*/
		int m_level;

		int m_window_bits{ default_window_bits };
		int m_mem_level{ default_mem_level };
		int m_strategy{ default_strategy };

		//! Size initially reserved for buffer.
		std::size_t m_reserve_buffer_size{ default_output_reserve_buffer_size };
};

/** @name Create parameters for zlib transformators.
 * @brief A set of function helping to create params_t objects
 * ommiting some verbose deteils.
 *
 * Instead of writing something like this:
 * \code
 * params_t params{
 *     restinio::transforms::zlib::params_t::operation_t::compress,
 *     restinio::transforms::zlib::params_t::format_t::gzip,
 *     -1 };
 * \endcode
 * It is better to write the following:
 * \code
 * params_t params = restinio::transforms::zlib::make_gzip_compress_params();
 * \endcode
 *
 * @since v.0.4.4
*/
///@{
inline params_t
make_deflate_compress_params( int compression_level = -1 )
{
	return params_t{
			params_t::operation_t::compress,
			params_t::format_t::deflate,
			compression_level };
}

inline params_t
make_deflate_decompress_params()
{
	return params_t{
			params_t::operation_t::decompress,
			params_t::format_t::deflate };
}

inline params_t
make_gzip_compress_params( int compression_level = -1 )
{
	return params_t{
			params_t::operation_t::compress,
			params_t::format_t::gzip,
			compression_level };
}

inline params_t
make_gzip_decompress_params()
{
	return params_t{
			params_t::operation_t::decompress,
			params_t::format_t::gzip };
}

inline params_t
make_identity_params()
{
	return params_t{};
}
///@}

//
// zlib_t
//

//! Zlib transformator.
/*!
	Uses zlib (https://zlib.net) for gzip/deflate compresion/decompression.
	All higher level functionality that minimizes boilerplate code and makes
	compression and decompression logic less verbose is based on using zlib_t.

	Simple usage:
	\code
		namespace rtz = restinio::transforms::zlib;
		rtz::zlib_t z{ rtz::make_gzip_compress_params( compression_level ).level( 9 ) };
		z.write( input_data );
		z.complete();

		auto gziped_data = z.giveaway_output();
	\endcode

	Advanced usage:
	\code
	namespace rtz = restinio::transforms::zlib;
	rtz::zlib_t z{ rtz::make_gzip_compress_params( compression_level ).level( 9 ) };

	std::size_t processed_data = 0;
	for( const auto d : data_pieces )
	{
		z.write( d );

		// Track how much data is already processed:
		processed_data += d.size();

		if( processed_data > 1024 * 1024 )
		{
			// If more than 1Mb  is processed do flush.
			z.flush();
		}

		if( z.output_size() > 100 * 1024 )
		{
			// If more than 100Kb of data is ready, then append it to something.
			append_output( z.giveaway_output() );
		}
	}

	// Complete the stream and append remeining putput data.
	z.complete();
	append_output( z.giveaway_output() );
	\endcode


	@since v.0.4.4
*/
class zlib_t
{
	public:
		zlib_t( const params_t & transformation_params )
			:	m_params{ transformation_params }
		{
			if( !is_identity() )
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

				if( params_t::format_t::gzip == m_params.format() )
				{
					current_window_bits += 16;
				}

				if( params_t::operation_t::compress == m_params.operation() )
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
							"Failed to initialize zlib stream: {}, {}",
							init_result,
							get_error_msg() ) };
				}

				m_zlib_stream_initialized = true;

				// Reserve initial buffer.
				inc_buffer();
			}
			// else => Nothing to initialize and to reserve.
		}

		zlib_t( const zlib_t & ) = delete;
		zlib_t( zlib_t && ) = delete;
		zlib_t & operator = ( const zlib_t & ) = delete;
		zlib_t & operator = ( zlib_t && ) = delete;

		~zlib_t()
		{
			if( m_zlib_stream_initialized )
			{
				if( params_t::operation_t::compress == m_params.operation() )
				{
					deflateEnd( &m_zlib_stream );
				}
				else
				{
					inflateEnd( &m_zlib_stream );
				}
			}
		}

		//! Get parameters of current transformation.
		const params_t & params() const { return m_params; }

		//! Append input data.
		/*!
			Pushes a given data to zlib transform.
			\a input is the data to be compressed or decompressed.
		*/
		void
		write( string_view_t input )
		{
			ensure_operation_in_not_completed();

			if( is_identity() )
			{
				m_out_buffer.append( input.data(), input.size() );
				m_write_pos = m_out_buffer.size();
			}
			else
			{
				if( std::numeric_limits< decltype( m_zlib_stream.avail_in ) >::max() < input.size() )
				{
					throw exception_t{
						fmt::format(
							"input data is too large: {} (max possible: {}), "
							"try to break large data into pieces",
							input.size(),
							std::numeric_limits< decltype( m_zlib_stream.avail_in ) >::max() ) };
				}

				if( 0 < input.size() )
				{
					m_zlib_stream.next_in =
						reinterpret_cast< Bytef* >( const_cast< char* >( input.data() ) );

					m_zlib_stream.avail_in = static_cast< uInt >( input.size() );

					if( params_t::operation_t::compress == m_params.operation() )
					{
						write_compress_impl( Z_NO_FLUSH );
					}
					else
					{
						write_decompress_impl( Z_NO_FLUSH );
					}
				}
			}
		}

		//! Flush the zlib stream.
		/*!
			Flushes underlying zlib stream.
			All pending output is flushed to the output buffer.
		*/
		void
		flush()
		{
			ensure_operation_in_not_completed();

			if( !is_identity() )
			{
				m_zlib_stream.next_in = nullptr;
				m_zlib_stream.avail_in = static_cast< uInt >( 0 );

				if( params_t::operation_t::compress == m_params.operation() )
				{
					write_compress_impl( Z_SYNC_FLUSH );
				}
				else
				{
					write_decompress_impl( Z_SYNC_FLUSH );
				}
			}
		}

		//! Complete the stream.
		void
		complete()
		{
			ensure_operation_in_not_completed();

			if( !is_identity() )
			{
				m_zlib_stream.next_in = nullptr;
				m_zlib_stream.avail_in = static_cast< uInt >( 0 );

				if( params_t::operation_t::compress == m_params.operation() )
				{
					write_compress_impl( Z_FINISH );
				}
				else
				{
					write_decompress_impl( Z_FINISH );
				}
			}

			m_operation_is_complete = true;
		}

		//! Get current accumulated output data
		/*!
			On this request a current accumulated output data is reterned.
			Move semantics is applied. Once current output is fetched
			zlib_t object resets its internal out buffer.

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
		bool is_identity() const
		{
			return params_t::format_t::identity == m_params.format();
		}

		//! Get zlib error message if it exists.
		const char *
		get_error_msg() const
		{
			const char * err_msg = "<no zlib error description>";
			if( m_zlib_stream.msg )
				err_msg = m_zlib_stream.msg;

			return err_msg;
		}

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
					throw exception_t{
						fmt::format(
							"unexpected result of inflate() (zlib): {}, {}",
							operation_result,
							get_error_msg() ) };
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
		const params_t m_params;

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

/** @name Helper functions for doing zlib transformation with less boilerplate.
 * @brief A set of handy functions helping to perform zlib transform in one line.
 *
 * Instead of writing something like this:
 * \code
 * restinio::transforms::zlib::zlib_t z{ restinio::transforms::zlib::gzip_compress() };
 * z.write( data );
 * z.complete();
 * body = z.giveaway_output();
 * \endcode
 * It is possible to write the following:
 * \code
 * body = restinio::transformators::zlib::gzip_compress( data );
 * \endcode
 *
 * @since v.0.4.4
*/
///@{

//! Do a specified zlib transformation.
inline std::string
transform( string_view_t input, const params_t & params )
{
	zlib_t z{ params };
	z.write( input );
	z.complete();

	return z.giveaway_output();
}

inline std::string
deflate_compress( string_view_t input, int compression_level = -1 )
{
	return transform( input, make_deflate_compress_params( compression_level ) );
}

inline std::string
deflate_decompress( string_view_t input )
{
	return transform( input, make_deflate_decompress_params() );
}

inline std::string
gzip_compress( string_view_t input, int compression_level = -1 )
{
	return transform( input, make_gzip_compress_params( compression_level ) );
}

inline std::string
gzip_decompress( string_view_t input )
{
	return transform( input, make_gzip_decompress_params() );
}
///@}

//
// body_appender_t
//

template < typename Response_Output_Strategy >
class body_appender_t
{
	body_appender_t() = delete;
};

namespace impl
{

//! Check if operation is a copression, and throw if it is not.
inline void ensure_is_compression_operation( params_t::operation_t op )
{
	if( params_t::operation_t::compress != op )
	{
		throw exception_t{ "operation is not copress" };
	}
}

//! Check if a pointer to zlib transformator is valid.
inline void ensure_valid_transforator( zlib_t * ztransformator )
{
	if( nullptr == ztransformator )
	{
		throw exception_t{ "invalid body appender" };
	}
}

//! Get token for copression format.
inline std::string content_encoding_token( params_t::format_t f )
{
	std::string result{ "identity" };

	if( params_t::format_t::deflate == f )
	{
		result.assign( "deflate" );
	}
	if( params_t::format_t::gzip == f )
	{
		result.assign( "gzip" );
	}

	return result;
}

} /* namespace impl */

//
// body_appender_base_t
//

//! Base class for body appenders.
template < typename Response_Output_Strategy, typename Descendant >
class body_appender_base_t
{
	public:
		using resp_t = response_builder_t< Response_Output_Strategy >;

		body_appender_base_t( const params_t & params, resp_t & resp )
			:	m_ztransformator{ std::make_unique< zlib_t >( params ) }
			,	m_resp{ resp }
		{
			impl::ensure_is_compression_operation(
				m_ztransformator->params().operation() );

			m_resp.append_header(
				restinio::http_field::content_encoding,
				impl::content_encoding_token(
					m_ztransformator->params().format() ) );
		}

		body_appender_base_t( const body_appender_base_t & ) = delete;
		body_appender_base_t & operator = ( const body_appender_base_t & ) = delete;
		body_appender_base_t & operator = ( body_appender_base_t && ) = delete;

		body_appender_base_t( body_appender_base_t && ba ) noexcept
			:	m_ztransformator{ std::move( ba.m_ztransformator ) }
			,	m_resp{ ba.m_resp }
		{}

		virtual ~body_appender_base_t() {}

	protected:
		std::unique_ptr< zlib_t > m_ztransformator;
		resp_t & m_resp;
};

//! Base class for body appenders with restinio or user controlled output.
template < typename X_Controlled_Output, typename Descendant >
class x_controlled_output_body_appender_base_t
	:	public body_appender_base_t< X_Controlled_Output, Descendant >
{
	public:
		using base_type_t = body_appender_base_t< X_Controlled_Output, Descendant >;

		using base_type_t::base_type_t;

		//! Append a piece of data to response.
		Descendant &
		append( string_view_t input )
		{
			impl::ensure_valid_transforator( this->m_ztransformator.get() );
			this->m_ztransformator->write( input );
			return static_cast< Descendant & >( *this );
		}

		//! Complete zlib transformation operation.
		void
		complete()
		{
			impl::ensure_valid_transforator( this->m_ztransformator.get() );

			this->m_ztransformator->complete();

			this->m_resp.append_body( this->m_ztransformator->giveaway_output() );
		}
};

/** @name Body appender.
 * @brief Helper class for setting the body of response_builder_t<restinio_controlled_output_t>.
 *
 * Sample usage:
 * \code
 * namespace rtz = restinio::transforms::zlib;
 * auto resp = req->create_response();
 *
 * resp.append_header( restinio::http_field::server, "RESTinio" )
 *   .append_header_date_field()
 *   .append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" );
 *
 * auto ba = rtz::gzip_body_appender( resp );
 * ba.append( some_data );
 * // ...
 * ba.append( some_more_data );
 * ba.complete();
 * resp.done();
 * \endcode
 *
 * @since v.0.4.4
*/
template <>
class body_appender_t< restinio_controlled_output_t >
	:	public x_controlled_output_body_appender_base_t<
					restinio_controlled_output_t,
					body_appender_t< restinio_controlled_output_t > >
{
	public:
		using base_type_t =
			x_controlled_output_body_appender_base_t<
				restinio_controlled_output_t,
				body_appender_t< restinio_controlled_output_t > >;

		//! Get the size of transformed body.
		auto
		size() const
		{
			impl::ensure_valid_transforator( m_ztransformator.get() );

			return m_ztransformator->output_size();
		}

		using base_type_t::base_type_t;
};

/** @name Body appender.
 * @brief Helper class for setting the body of response_builder_t<user_controlled_output_t>.
 *
 * Sample usage:
 * \code
 * namespace rtz = restinio::transforms::zlib;
 * auto resp = req->create_response<user_controlled_output_t>();
 *
 * resp.append_header( restinio::http_field::server, "RESTinio" )
 *   .append_header_date_field()
 *   .append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" );
 *
 * auto ba = rtz::gzip_body_appender( resp );
 * ba.append( some_data );
 * ba.flush();
 * // ...
 * ba.append( some_more_data );
 * ba.complete();
 * resp.done();
 * \endcode
 *
 * @since v.0.4.4
*/
template <>
class body_appender_t< user_controlled_output_t > final
	:	public x_controlled_output_body_appender_base_t<
					user_controlled_output_t,
					body_appender_t< user_controlled_output_t > >
{
	public:
		using base_type_t =
			x_controlled_output_body_appender_base_t<
				user_controlled_output_t,
				body_appender_t< user_controlled_output_t > >;

		using base_type_t::base_type_t;

		auto &
		flush()
		{
			impl::ensure_valid_transforator( m_ztransformator.get() );
			m_ztransformator->flush();
			m_resp
				.append_body( m_ztransformator->giveaway_output() )
				.flush();

			return *this;
		}
};


/** @name Body appender.
 * @brief Helper class for setting the body of response_builder_t<chunked_output_t>.
 *
 * Sample usage:
 * \code
 * namespace rtz = restinio::transforms::zlib;
 * auto resp = req->create_response<chunked_output_t>();
 *
 * resp.append_header( restinio::http_field::server, "RESTinio" )
 *   .append_header_date_field()
 *   .append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" );
 *
 * auto ba = rtz::gzip_body_appender( resp );
 * ba.append( some_data );
 * ba.append( some_more_data );
 * ba.make_chunk(); // Flush copressed data and creates a chunk with it.
 * ba.flush(); // Send currently prepared chunks to client
 * // ...
 * // Copress the data and creates a chunk with it.
 * ba.make_chunk( even_more_data );
 * ba.flush(); // Send currently prepared chunks to client
 * // ...
 * ba.append( yet_even_more_data );
 * ba.append( last_data );
 * ba.complete(); // Creates last chunk, but doesn't send it to client.
 * ba.flush(); // Send chunk created by complete() call
 * // ...
 * resp.done();
 * \endcode
 *
 * @since v.0.4.4
*/
template <>
class body_appender_t< chunked_output_t > final
	:	public body_appender_base_t<
					chunked_output_t,
					body_appender_t< chunked_output_t > >
{
	public:
		using base_type_t =
			body_appender_base_t<
				chunked_output_t,
				body_appender_t< chunked_output_t > >;

		using base_type_t::base_type_t;

		//! Append data to be compressed.
		/*!
			Function only adds data to anderlying zlib stream
			and it doesn't affect target response right on here.
		*/
		auto &
		append( string_view_t input )
		{
			impl::ensure_valid_transforator( m_ztransformator.get() );

			m_ztransformator->write( input );
			return *this;
		}

		//! Append data to be compressed and adds current zlib transformator output
		//! as a new chunk.
		/*!
			Adds data and flushes zlib transformator.
			Then ready compressed data is taken and used as a new chunk
			of target response.
		*/
		auto &
		make_chunk( string_view_t input = string_view_t{} )
		{
			append( input ); // m_ztransformator is checked here.

			m_ztransformator->flush(); // Flush already compressed data.

			// Create a chunk with current output.
			m_resp.append_chunk( m_ztransformator->giveaway_output() );

			return *this;
		}

		//! Flushes currently available compressed data with possibly creating new chunk
		//! and then flushes target response.
		void
		flush()
		{
			impl::ensure_valid_transforator( m_ztransformator.get() );

			if( !m_ztransformator->is_completed() )
			{
				m_ztransformator->flush();
				m_resp.append_chunk( m_ztransformator->giveaway_output() );
			}

			m_resp.flush();
		}

		//! Complete zlib transformation operation.
		void
		complete()
		{
			impl::ensure_valid_transforator( m_ztransformator.get() );
			m_ztransformator->complete();
			m_resp.append_chunk( m_ztransformator->giveaway_output() );
		}
};

//! Create body appender with given zlib transformation parameters.
//! @since v.0.4.4
template < typename Response_Output_Strategy >
body_appender_t< Response_Output_Strategy >
body_appender(
	response_builder_t< Response_Output_Strategy > & resp,
	const params_t & params )
{
	return body_appender_t< Response_Output_Strategy >{ params, resp };
}

//! Create body appender with deflate transformation and a given compression level.
//! @since v.0.4.4
template < typename Response_Output_Strategy >
body_appender_t< Response_Output_Strategy >
deflate_body_appender(
	response_builder_t< Response_Output_Strategy > & resp,
	int compression_level = -1 )
{
	return body_appender( resp, make_deflate_compress_params( compression_level ) );
}

//! Create body appender with gzip transformation and a given compression level.
//! @since v.0.4.4
template < typename Response_Output_Strategy >
inline body_appender_t< Response_Output_Strategy >
gzip_body_appender(
	response_builder_t< Response_Output_Strategy > & resp,
	int compression_level = -1 )
{
	return body_appender( resp, make_gzip_compress_params( compression_level ) );
}

//! Create body appender with gzip transformation and a given compression level.
//! @since v.0.4.4
template < typename Response_Output_Strategy >
inline body_appender_t< Response_Output_Strategy >
identity_body_appender(
	response_builder_t< Response_Output_Strategy > & resp,
	int = -1 )
{
	return body_appender( resp, make_identity_params() );
}

//! Call a handler over a request body.
/*!
 * If body is encoded with either 'deflate' or 'gzip'
 * then it is decompressed and the handler is called.
 *
 * If body is encoded with 'identity' (or not specified)
 * the handler is called with original body.
 *
 * In other cases an exception is thrown.
 *
 * \tparam Handler a function object capable to handle std::string as an argument.
 *
 * Sample usage:
 * \code
 * namespace rtz = restinio::transforms::zlib;
 * auto decompressed_echo_handler( restinio::request_handle_t req )
 * {
 *   return
 *     rtz::handle_body(
 *       *req,
 *       [&]( auto body ){
 *         return
 *           req->create_response()
 *             .append_header( restinio::http_field::server, "RESTinio" )
 *             .append_header_date_field()
 *             .append_header(
 *               restinio::http_field::content_type,
 *               "text/plain; charset=utf-8" );
 *             .set_body( std::move( body ) )
 *             .done();
 *       } );
 * }
 * \endcode
 * @since v.0.4.4
*/
template < typename Handler >
decltype(auto)
handle_body( const request_t & req, Handler handler )
{
	using restinio::impl::is_equal_caseless;

	const auto content_encoding =
		req.header().get_field_or( restinio::http_field::content_encoding, "identity" );

	if( is_equal_caseless( content_encoding, "deflate" ) )
	{
		return handler( deflate_decompress( req.body() ) );
	}
	else if( is_equal_caseless( content_encoding, "gzip" ) )
	{
		return handler( gzip_decompress( req.body() ) );
	}
	else if( !is_equal_caseless( content_encoding, "identity" ) )
	{
		throw exception_t{
			fmt::format( "content-encoding '{}' not supported", content_encoding ) };
	}

	return handler( req.body() );
}

} /* namespace zlib */

} /* namespace transforms */

} /* namespace restinio */
