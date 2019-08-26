/*
	restinio
*/

/*!
	Zlib.
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>
#include <restinio/transforms/zlib.hpp>

#include "../random_data_generators.ipp"

TEST_CASE( "Create parameters for zlib transformators" , "[zlib][params][create_params]" )
{
	namespace rtz = restinio::transforms::zlib;

	{
		auto params = rtz::make_deflate_compress_params();

		REQUIRE( rtz::params_t::operation_t::compress == params.operation() );
		REQUIRE( rtz::params_t::format_t::deflate == params.format() );
		REQUIRE( -1 == params.level() );
	}

	{
		auto params = rtz::make_deflate_compress_params( 9 );

		REQUIRE( rtz::params_t::operation_t::compress == params.operation() );
		REQUIRE( rtz::params_t::format_t::deflate == params.format() );
		REQUIRE( 9 == params.level() );
	}

	{
		auto params = rtz::make_gzip_compress_params();

		REQUIRE( rtz::params_t::operation_t::compress == params.operation() );
		REQUIRE( rtz::params_t::format_t::gzip == params.format() );
		REQUIRE( -1 == params.level() );
	}

	{
		auto params = rtz::make_gzip_compress_params( 3 );

		REQUIRE( rtz::params_t::operation_t::compress == params.operation() );
		REQUIRE( rtz::params_t::format_t::gzip == params.format() );
		REQUIRE( 3 == params.level() );
	}

	{
		auto params = rtz::make_deflate_decompress_params();

		REQUIRE( rtz::params_t::operation_t::decompress == params.operation() );
		REQUIRE( rtz::params_t::format_t::deflate == params.format() );
	}

	{
		auto params = rtz::make_gzip_decompress_params();

		REQUIRE( rtz::params_t::operation_t::decompress == params.operation() );
		REQUIRE( rtz::params_t::format_t::gzip == params.format() );
	}

	{
		auto params = rtz::make_identity_params();

		REQUIRE( rtz::params_t::format_t::identity == params.format() );
	}
}

TEST_CASE( "Default parameters for zlib transformators" , "[zlib][params][defaults]" )
{
	namespace rtz = restinio::transforms::zlib;

	auto params = rtz::make_deflate_compress_params();

	REQUIRE( rtz::default_window_bits == params.window_bits() );
	REQUIRE( rtz::default_mem_level == params.mem_level() );
	REQUIRE( rtz::default_strategy == params.strategy() );
	REQUIRE( rtz::default_output_reserve_buffer_size ==
				params.reserve_buffer_size() );
}

TEST_CASE( "Setting parameters for zlib transformators: window_bits" , "[zlib][params][window_bits]" )
{
	namespace rtz = restinio::transforms::zlib;

	{
		auto params = rtz::make_deflate_compress_params();

		REQUIRE_NOTHROW( params.window_bits( 12 ) );
		REQUIRE( 12 == params.window_bits() );
		REQUIRE_NOTHROW( params.window_bits( 8 ) );
		REQUIRE( 9 == params.window_bits() );

		REQUIRE_NOTHROW( params.window_bits( 13 ) );
		REQUIRE( 13 == params.window_bits() );

		REQUIRE_NOTHROW( params.window_bits( 9 ) );
		REQUIRE( 9 == params.window_bits() );

		REQUIRE_THROWS( params.window_bits( 0 ) );
		REQUIRE( 9 == params.window_bits() );

		REQUIRE_THROWS( params.window_bits( 1 ) );
		REQUIRE_THROWS( params.window_bits( 3 ) );
		REQUIRE_THROWS( params.window_bits( 16 ) );
		REQUIRE_THROWS( params.window_bits( 17 ) );
	}

	{
		auto params = rtz::make_deflate_decompress_params();
		REQUIRE_NOTHROW( params.window_bits( 0 ) );
		REQUIRE( 0 == params.window_bits() );
	}
}

TEST_CASE( "Setting parameters for zlib transformators: mem_level" , "[zlib][params][mem_level]" )
{
	namespace rtz = restinio::transforms::zlib;

	{
		auto params = rtz::make_deflate_compress_params();

		REQUIRE_NOTHROW( params.mem_level( 1 ) );
		REQUIRE( 1 == params.mem_level() );
		REQUIRE_NOTHROW( params.mem_level( 2 ) );
		REQUIRE( 2 == params.mem_level() );
		REQUIRE_NOTHROW( params.mem_level( 8 ) );
		REQUIRE( 8 == params.mem_level() );
		REQUIRE_NOTHROW( params.mem_level( MAX_MEM_LEVEL ) );
		REQUIRE( MAX_MEM_LEVEL == params.mem_level() );

		REQUIRE_THROWS( params.mem_level( -1 ) );
		REQUIRE_THROWS( params.mem_level( 0 ) );
		REQUIRE_THROWS( params.mem_level( MAX_MEM_LEVEL + 1 ) );
	}
}

TEST_CASE( "Setting parameters for zlib transformators: strategy" , "[zlib][params][strategy]" )
{
	namespace rtz = restinio::transforms::zlib;

	{
		auto params = rtz::make_deflate_compress_params();

		REQUIRE_NOTHROW( params.strategy( Z_DEFAULT_STRATEGY ) );
		REQUIRE( Z_DEFAULT_STRATEGY == params.strategy() );
		REQUIRE_NOTHROW( params.strategy( Z_FILTERED ) );
		REQUIRE( Z_FILTERED == params.strategy() );
		REQUIRE_NOTHROW( params.strategy( Z_HUFFMAN_ONLY ) );
		REQUIRE( Z_HUFFMAN_ONLY == params.strategy() );
		REQUIRE_NOTHROW( params.strategy( Z_RLE ) );
		REQUIRE( Z_RLE == params.strategy() );
		REQUIRE_THROWS( params.strategy( Z_RLE + 4242 ) );
	}
}

TEST_CASE( "Setting parameters for zlib transformators: reserve_buffer_size" , "[zlib][params][reserve_buffer_size]" )
{
	namespace rtz = restinio::transforms::zlib;

	{
		auto params = rtz::make_deflate_compress_params();

		REQUIRE_NOTHROW( params.reserve_buffer_size( 512UL ) );
		REQUIRE( 512UL == params.reserve_buffer_size() );
		REQUIRE_NOTHROW( params.reserve_buffer_size( 10UL ) );
		REQUIRE( 10UL == params.reserve_buffer_size() );
		REQUIRE_NOTHROW( params.reserve_buffer_size( 4096UL ) );
		REQUIRE( 4096UL == params.reserve_buffer_size() );
		REQUIRE_THROWS( params.reserve_buffer_size( 9 ) );
		REQUIRE_THROWS( params.reserve_buffer_size( 1 ) );
		REQUIRE_THROWS( params.reserve_buffer_size( 0 ) );
	}
}

TEST_CASE( "deflate" , "[zlib][compress][decompress][deflate]" )
{
	namespace rtz = restinio::transforms::zlib;

	std::srand( static_cast<unsigned int>(std::time( nullptr )) );

	{
		rtz::zlib_t zc{ rtz::make_deflate_compress_params() };

		std::string
			input_data{
				"The zlib compression library provides "
				"in-memory compression and decompression functions, "
				"including integrity checks of the uncompressed data." };

		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_NOTHROW( zc.complete() );

		const auto out_size = zc.output_size();
		const auto out_data = zc.giveaway_output();
		REQUIRE( out_size == out_data.size() );
		REQUIRE( 10 < out_data.size() );

		rtz::zlib_t zd{ rtz::make_deflate_decompress_params() };

		REQUIRE_NOTHROW( zd.write( out_data ) );
		REQUIRE_NOTHROW( zd.complete() );

		const auto decompression_out_size = zd.output_size();
		const auto decompression_out_data = zd.giveaway_output();
		REQUIRE( decompression_out_size == decompression_out_data.size() );
		REQUIRE( decompression_out_data == input_data+input_data );
	}

	{
		const std::size_t chunk_size = 1024;
		const std::size_t chunk_count = 128;

		struct test_setting_t
		{
			std::size_t m_reserve_buffer_size{ 1024UL * (1UL << (std::rand() % 11UL) ) };
			std::size_t m_repeats_level{ 1 + std::rand() % 42UL };
			int m_data_gen{ std::rand() % 2 };
			int m_do_flush{ std::rand() % 2 };
			int m_window_bits{ 9 + (std::rand() % (MAX_WBITS - 8) ) };
			int m_mem_level{ 1 + (std::rand() % ( MAX_MEM_LEVEL ) )};
		};

		unsigned int tests_count = 250;

		while( 0 != tests_count-- )
		{
			test_setting_t ts;
			// std::cout << "tests_count = " << tests_count
			// 	<< "; ts = "
			// 		<< "{" << ts.m_reserve_buffer_size
			// 		<< "," << ts.m_repeats_level
			// 		<< "," << ts.m_data_gen
			// 		<< "," << ts.m_do_flush
			// 		<< "," << ts.m_window_bits
			// 		<< "," << ts.m_mem_level
			// 		<< "}" << std::endl;

			const std::string input_data =
				ts.m_data_gen == 0 ?
					create_random_text( chunk_size * chunk_count, ts.m_repeats_level ) :
					create_random_binary( chunk_size * chunk_count, ts.m_repeats_level );

			rtz::zlib_t zc{
				rtz::make_deflate_compress_params()
					.reserve_buffer_size( ts.m_reserve_buffer_size + 512 )
					.window_bits( ts.m_window_bits )
					.mem_level( ts.m_mem_level ) };

			for( std::size_t i = 0; i < chunk_count; ++i )
			{
				const restinio::string_view_t
					chunk{ input_data.data() + i * chunk_size, chunk_size };

				REQUIRE_NOTHROW( zc.write( chunk ) );

				if( ts.m_do_flush ) zc.flush();
			}

			REQUIRE_NOTHROW( zc.complete() );
			const auto out_size = zc.output_size();
			const auto out_data = zc.giveaway_output();
			REQUIRE( out_size == out_data.size() );
			REQUIRE( 10 < out_data.size() );

			rtz::zlib_t zd{
				rtz::make_deflate_decompress_params()
					.reserve_buffer_size( ts.m_reserve_buffer_size + 512 )
					.window_bits( ts.m_window_bits )
					.mem_level( ts.m_mem_level ) };

			for(
				const char* buf = out_data.data();
				buf < out_data.data() + out_data.size();
				buf += chunk_size )
			{
				const restinio::string_view_t
					chunk{
						buf,
						std::min< std::size_t >(
							chunk_size,
							out_data.size() - (buf - out_data.data() ) ) };

				REQUIRE_NOTHROW( zd.write( chunk ) );

				if( ts.m_do_flush ) zd.flush();
			}

			zd.complete();
			const auto decompression_out_size = zd.output_size();
			const auto decompression_out_data = zd.giveaway_output();
			REQUIRE( decompression_out_size == decompression_out_data.size() );
			REQUIRE( decompression_out_data == input_data );
		}
	}
}

TEST_CASE( "gzip" , "[zlib][compress][decompress][gzip]" )
{
	namespace rtz = restinio::transforms::zlib;

	std::srand( static_cast<unsigned int>(std::time( nullptr )) );

	{
		rtz::zlib_t zc{ rtz::make_gzip_compress_params() };

		std::string
			input_data{
				"The zlib compression library provides "
				"in-memory compression and decompression functions, "
				"including integrity checks of the uncompressed data." };

		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_NOTHROW( zc.complete() );

		const auto out_size = zc.output_size();
		const auto out_data = zc.giveaway_output();
		REQUIRE( out_size == out_data.size() );
		REQUIRE( 10 < out_data.size() );

		rtz::zlib_t zd{ rtz::make_gzip_decompress_params() };

		REQUIRE_NOTHROW( zd.write( out_data ) );
		REQUIRE_NOTHROW( zd.complete() );

		const auto decompression_out_size = zd.output_size();
		const auto decompression_out_data = zd.giveaway_output();
		REQUIRE( decompression_out_size == decompression_out_data.size() );
		REQUIRE( decompression_out_data == input_data+input_data );
	}

	{
		const std::size_t chunk_size = 1024;
		const std::size_t chunk_count = 128;

		struct test_setting_t
		{
			std::size_t m_reserve_buffer_size{ 1024UL * (1UL << (std::rand() % 11UL) ) };
			std::size_t m_repeats_level{ 1 + std::rand() % 42UL };
			int m_data_gen{ std::rand() % 2 };
			int m_do_flush{ std::rand() % 2 };
			int m_window_bits{ 9 + (std::rand() % (MAX_WBITS - 8) ) };
			int m_mem_level{ 1 + (std::rand() % ( MAX_MEM_LEVEL ) )};
		};

		unsigned int tests_count = 250;

		while( 0 != tests_count-- )
		{
			test_setting_t ts;
			// std::cout << "tests_count = " << tests_count
			// 	<< "; ts = "
			// 		<< "{" << ts.m_reserve_buffer_size
			// 		<< "," << ts.m_repeats_level
			// 		<< "," << ts.m_data_gen
			// 		<< "," << ts.m_do_flush
			// 		<< "," << ts.m_window_bits
			// 		<< "," << ts.m_mem_level
			// 		<< "}" << std::endl;

			const std::string input_data =
				ts.m_data_gen == 0 ?
					create_random_text( chunk_size * chunk_count, ts.m_repeats_level ) :
					create_random_binary( chunk_size * chunk_count, ts.m_repeats_level );

			rtz::zlib_t zc{
				rtz::make_gzip_compress_params()
					.reserve_buffer_size( ts.m_reserve_buffer_size + 512 )
					.window_bits( ts.m_window_bits )
					.mem_level( ts.m_mem_level ) };

			for( std::size_t i = 0; i < chunk_count; ++i )
			{
				const restinio::string_view_t
					chunk{ input_data.data() + i * chunk_size, chunk_size };

				REQUIRE_NOTHROW( zc.write( chunk ) );

				if( ts.m_do_flush ) zc.flush();
			}

			REQUIRE_NOTHROW( zc.complete() );
			const auto out_size = zc.output_size();
			const auto out_data = zc.giveaway_output();
			REQUIRE( out_size == out_data.size() );
			REQUIRE( 10 < out_data.size() );

			rtz::zlib_t zd{
				rtz::make_gzip_decompress_params()
					.reserve_buffer_size( ts.m_reserve_buffer_size + 512 )
					.window_bits( ts.m_window_bits )
					.mem_level( ts.m_mem_level ) };

			for(
				const char* buf = out_data.data();
				buf < out_data.data() + out_data.size();
				buf += chunk_size )
			{
				const restinio::string_view_t
					chunk{
						buf,
						std::min< std::size_t >(
							chunk_size,
							out_data.size() - (buf - out_data.data() ) ) };

				REQUIRE_NOTHROW( zd.write( chunk ) );

				if( ts.m_do_flush ) zd.flush();
			}

			zd.complete();
			const auto decompression_out_size = zd.output_size();
			const auto decompression_out_data = zd.giveaway_output();
			REQUIRE( decompression_out_size == decompression_out_data.size() );
			REQUIRE( decompression_out_data == input_data );
		}
	}
}

TEST_CASE( "identity" , "[zlib][identity]" )
{
	namespace rtz = restinio::transforms::zlib;

	std::srand( static_cast<unsigned int>(std::time( nullptr )) );

	{
		rtz::zlib_t zc{ rtz::make_identity_params() };

		std::string
			input_data{
				"The zlib compression library provides "
				"in-memory compression and decompression functions, "
				"including integrity checks of the uncompressed data." };

		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_NOTHROW( zc.complete() );

		const auto out_data = zc.giveaway_output();
		REQUIRE( out_data == input_data + input_data );
	}
}

TEST_CASE( "transform functions" , "[zlib][transform]" )
{
	namespace rtz = restinio::transforms::zlib;

	const std::string
		input_data{
			"The zlib compression library provides "
			"in-memory compression and decompression functions, "
			"including integrity checks of the uncompressed data." };

	REQUIRE( input_data == rtz::transform( input_data, rtz::make_identity_params() ) );

	REQUIRE( input_data ==
		rtz::transform(
			rtz::transform(
				input_data,
				rtz::make_deflate_compress_params() ),
			rtz::make_deflate_decompress_params() ) );

	REQUIRE( input_data ==
		rtz::transform(
			rtz::transform(
				input_data,
				rtz::make_gzip_compress_params() ),
			rtz::make_gzip_decompress_params() ) );

	REQUIRE( input_data ==
		rtz::deflate_decompress( rtz::deflate_compress( input_data ) ) );

	REQUIRE( input_data ==
		rtz::gzip_decompress( rtz::gzip_compress( input_data ) ) );
}

TEST_CASE( "complete" , "[zlib][compress][decompress][commplete]" )
{
	namespace rtz = restinio::transforms::zlib;

	std::srand( static_cast<unsigned int>(std::time( nullptr )) );

	{
		rtz::zlib_t zc{ rtz::make_gzip_compress_params() };
		REQUIRE_FALSE( zc.is_completed() );

		std::string
			input_data{
				"The zlib compression library provides "
				"in-memory compression and decompression functions, "
				"including integrity checks of the uncompressed data." };

		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_FALSE( zc.is_completed() );

		REQUIRE_NOTHROW( zc.flush() );
		REQUIRE_FALSE( zc.is_completed() );

		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_FALSE( zc.is_completed() );

		REQUIRE_NOTHROW( zc.complete() );
		REQUIRE( zc.is_completed() );
		REQUIRE_THROWS( zc.complete() );
		REQUIRE( zc.is_completed() );

		const auto out_size = zc.output_size();
		const auto out_data = zc.giveaway_output();
		REQUIRE( out_size == out_data.size() );

		rtz::zlib_t zd{ rtz::make_gzip_decompress_params() };
		REQUIRE_FALSE( zd.is_completed() );

		REQUIRE_NOTHROW( zd.write( out_data.substr(0, out_data.size()/2 ) ) );
		REQUIRE_FALSE( zd.is_completed() );

		REQUIRE_NOTHROW( zd.flush() );
		REQUIRE_FALSE( zd.is_completed() );

		REQUIRE_NOTHROW( zd.write( out_data.substr( out_data.size()/2 ) ) );
		REQUIRE_FALSE( zd.is_completed() );

		REQUIRE_NOTHROW( zd.complete() );
		REQUIRE( zd.is_completed() );
		REQUIRE_THROWS( zd.complete() );
		REQUIRE( zd.is_completed() );

		const auto decompression_out_size = zd.output_size();
		const auto decompression_out_data = zd.giveaway_output();
		REQUIRE( decompression_out_size == decompression_out_data.size() );
		REQUIRE( decompression_out_data == input_data + input_data );
	}
}

TEST_CASE( "take output" , "[zlib][compress][decompress][output]" )
{
	namespace rtz = restinio::transforms::zlib;

	std::srand( static_cast<unsigned int>(std::time( nullptr )) );

	{
		rtz::zlib_t zc{ rtz::make_gzip_compress_params() };

		std::string
			input_data{
				"The zlib compression library provides "
				"in-memory compression and decompression functions, "
				"including integrity checks of the uncompressed data." };

		std::string out_data;
		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_NOTHROW( out_data += zc.giveaway_output() );
		REQUIRE_NOTHROW( zc.giveaway_output() == "" );

		REQUIRE_NOTHROW( zc.flush() );
		REQUIRE_NOTHROW( out_data += zc.giveaway_output() );
		REQUIRE_NOTHROW( zc.giveaway_output() == "" );

		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_NOTHROW( out_data += zc.giveaway_output() );
		REQUIRE_NOTHROW( zc.giveaway_output() == "" );

		REQUIRE_NOTHROW( zc.complete() );
		REQUIRE_NOTHROW( out_data += zc.giveaway_output() );
		REQUIRE_NOTHROW( zc.giveaway_output() == "" );

		rtz::zlib_t zd{ rtz::make_gzip_decompress_params() };
		std::string decompression_out_data;

		REQUIRE_NOTHROW( zd.write( out_data.substr(0, out_data.size()/2 ) ) );
		REQUIRE_NOTHROW( decompression_out_data += zd.giveaway_output() );
		REQUIRE_NOTHROW( zd.giveaway_output() == "" );

		REQUIRE_NOTHROW( zd.flush() );
		REQUIRE_NOTHROW( decompression_out_data += zd.giveaway_output() );
		REQUIRE_NOTHROW( zd.giveaway_output() == "" );

		REQUIRE_NOTHROW( zd.write( out_data.substr( out_data.size()/2 ) ) );
		REQUIRE_NOTHROW( decompression_out_data += zd.giveaway_output() );
		REQUIRE_NOTHROW( zd.giveaway_output() == "" );

		REQUIRE_NOTHROW( zd.complete() );
		REQUIRE_NOTHROW( decompression_out_data += zd.giveaway_output() );
		REQUIRE_NOTHROW( zd.giveaway_output() == "" );

		REQUIRE( decompression_out_data == input_data + input_data );
	}

	{
		rtz::zlib_t zc{ rtz::make_identity_params() };

		std::string
			input_data{
				"The zlib compression library provides "
				"in-memory compression and decompression functions, "
				"including integrity checks of the uncompressed data." };

		std::string out_data;
		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_NOTHROW( out_data += zc.giveaway_output() );
		REQUIRE( out_data == input_data );
		REQUIRE_NOTHROW( zc.giveaway_output() == "" );

		REQUIRE_NOTHROW( zc.flush() );
		REQUIRE_NOTHROW( out_data += zc.giveaway_output() );
		REQUIRE( out_data == input_data );
		REQUIRE_NOTHROW( zc.giveaway_output() == "" );

		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_NOTHROW( out_data += zc.giveaway_output() );
		REQUIRE( out_data == input_data + input_data );
		REQUIRE_NOTHROW( zc.giveaway_output() == "" );

		REQUIRE_NOTHROW( zc.complete() );
		REQUIRE_NOTHROW( out_data += zc.giveaway_output() );
		REQUIRE( out_data == input_data + input_data );
		REQUIRE_NOTHROW( zc.giveaway_output() == "" );
	}
}

TEST_CASE( "write check input size" , "[zlib][write][large input]" )
{
	namespace rtz = restinio::transforms::zlib;

	std::srand( static_cast<unsigned int>(std::time( nullptr )) );

	if( sizeof( restinio::string_view_t::size_type ) == 8 )
	{
		rtz::zlib_t zc{ rtz::make_gzip_compress_params() };

		const char * s =
			"The zlib compression library provides "
			"in-memory compression and decompression functions, "
			"including integrity checks of the uncompressed data.";

		restinio::string_view_t large_input{
			s,
			restinio::string_view_t::size_type{ 1 } + std::numeric_limits< decltype( z_stream::avail_in ) >::max() };

		REQUIRE_THROWS( zc.write( large_input ) );
	}
}
