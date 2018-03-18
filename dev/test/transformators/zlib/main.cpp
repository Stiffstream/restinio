/*
	restinio
*/

/*!
	Echo server.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <restinio/transformator/zlib.hpp>

TEST_CASE( "Create parameters for zlib transformators" , "[zlib][params][create_params]" )
{
	namespace rt = restinio::transformator;

	{
		auto params = rt::deflate_compress();

		REQUIRE( rt::zlib_params_t::operation_t::compress == params.operation() );
		REQUIRE( rt::zlib_params_t::format_t::deflate == params.format() );
		REQUIRE( -1 == params.level() );
	}

	{
		auto params = rt::deflate_compress( 9 );

		REQUIRE( rt::zlib_params_t::operation_t::compress == params.operation() );
		REQUIRE( rt::zlib_params_t::format_t::deflate == params.format() );
		REQUIRE( 9 == params.level() );
	}

	{
		auto params = rt::gzip_compress();

		REQUIRE( rt::zlib_params_t::operation_t::compress == params.operation() );
		REQUIRE( rt::zlib_params_t::format_t::gzip == params.format() );
		REQUIRE( -1 == params.level() );
	}

	{
		auto params = rt::gzip_compress( 3 );

		REQUIRE( rt::zlib_params_t::operation_t::compress == params.operation() );
		REQUIRE( rt::zlib_params_t::format_t::gzip == params.format() );
		REQUIRE( 3 == params.level() );
	}

	{
		auto params = rt::deflate_decompress();

		REQUIRE( rt::zlib_params_t::operation_t::decompress == params.operation() );
		REQUIRE( rt::zlib_params_t::format_t::deflate == params.format() );
	}

	{
		auto params = rt::gzip_decompress();

		REQUIRE( rt::zlib_params_t::operation_t::decompress == params.operation() );
		REQUIRE( rt::zlib_params_t::format_t::gzip == params.format() );
	}
}

TEST_CASE( "Default parameters for zlib transformators" , "[zlib][params][defaults]" )
{
	namespace rt = restinio::transformator;

	auto params = rt::deflate_compress();

	REQUIRE( rt::default_zlib_window_bits == params.window_bits() );
	REQUIRE( rt::default_zlib_mem_level == params.mem_level() );
	REQUIRE( rt::default_zlib_strategy == params.strategy() );
	REQUIRE( rt::default_zlib_output_reserve_buffer_size ==
				params.reserve_buffer_size() );
}

TEST_CASE( "Setting parameters for zlib transformators: window_bits" , "[zlib][params][window_bits]" )
{
	namespace rt = restinio::transformator;

	{
		auto params = rt::deflate_compress();

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
		auto params = rt::deflate_decompress();
		REQUIRE_NOTHROW( params.window_bits( 0 ) );
		REQUIRE( 0 == params.window_bits() );
	}
}

TEST_CASE( "Setting parameters for zlib transformators: mem_level" , "[zlib][params][mem_level]" )
{
	namespace rt = restinio::transformator;

	{
		auto params = rt::deflate_compress();

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
	namespace rt = restinio::transformator;

	{
		auto params = rt::deflate_compress();

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
	namespace rt = restinio::transformator;

	{
		auto params = rt::deflate_compress();

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

std::string
create_random_text( std::size_t n, std::size_t repeat_max = 1 )
{
	restinio::string_view_t symbols{
		// " \t\r\n"
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		// ",.;:'\"!@#$%^&*~-+\\/"
		"<>{}()"};

	std::string result;
	result.reserve( n );

	while( 0 != n )
	{
		const char c = symbols[ std::rand() % symbols.size() ];
		std::size_t repeats = 1 + std::min< std::size_t >( n - 1, std::rand() % repeat_max );
		result.append( repeats, c );
		n -= repeats;
	}

	return result;
}

std::string
create_random_binary( std::size_t n, std::size_t repeat_max = 1 )
{
	std::string result;
	result.reserve( n );

	while( 0 != n )
	{
		const unsigned char c = std::rand() % 256;
		std::size_t repeats = 1 + std::min< std::size_t >( n - 1, std::rand() % repeat_max );
		result.append( repeats, c );
		n -= repeats;
	}

	return result;
}

TEST_CASE( "deflate" , "[zlib][compress][decompress][deflate]" )
{
	namespace rt = restinio::transformator;

	std::srand( std::time( nullptr ) );

	{
		rt::zlib_t zc{ rt::deflate_compress() };

		std::string
			input_data{
				"The zlib compression library provides "
				"in-memory compression and decompression functions, "
				"including integrity checks of the uncompressed data." };

		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_NOTHROW( zc.complete() );

		const auto out_size = zc.output_size();
		const auto out_data = zc.givaway_output();
		REQUIRE( out_size == out_data.size() );
		REQUIRE( 10 < out_data.size() );

		rt::zlib_t zd{ rt::deflate_decompress() };

		REQUIRE_NOTHROW( zd.write( out_data ) );
		REQUIRE_NOTHROW( zd.complete() );

		const auto decompression_out_size = zd.output_size();
		const auto decompression_out_data = zd.givaway_output();
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

			rt::zlib_t zc{
				rt::deflate_compress()
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
			const auto out_data = zc.givaway_output();
			REQUIRE( out_size == out_data.size() );
			REQUIRE( 10 < out_data.size() );

			rt::zlib_t zd{
				rt::deflate_decompress()
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
			const auto decompression_out_data = zd.givaway_output();
			REQUIRE( decompression_out_size == decompression_out_data.size() );
			REQUIRE( decompression_out_data == input_data );
		}
	}
}

TEST_CASE( "gzip" , "[zlib][compress][decompress][gzip]" )
{
	namespace rt = restinio::transformator;

	std::srand( std::time( nullptr ) );

	{
		rt::zlib_t zc{ rt::gzip_compress() };

		std::string
			input_data{
				"The zlib compression library provides "
				"in-memory compression and decompression functions, "
				"including integrity checks of the uncompressed data." };

		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_NOTHROW( zc.complete() );

		const auto out_size = zc.output_size();
		const auto out_data = zc.givaway_output();
		REQUIRE( out_size == out_data.size() );
		REQUIRE( 10 < out_data.size() );

		rt::zlib_t zd{ rt::gzip_decompress() };

		REQUIRE_NOTHROW( zd.write( out_data ) );
		REQUIRE_NOTHROW( zd.complete() );

		const auto decompression_out_size = zd.output_size();
		const auto decompression_out_data = zd.givaway_output();
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

			rt::zlib_t zc{
				rt::gzip_compress()
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
			const auto out_data = zc.givaway_output();
			REQUIRE( out_size == out_data.size() );
			REQUIRE( 10 < out_data.size() );

			rt::zlib_t zd{
				rt::gzip_decompress()
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
			const auto decompression_out_data = zd.givaway_output();
			REQUIRE( decompression_out_size == decompression_out_data.size() );
			REQUIRE( decompression_out_data == input_data );
		}
	}
}

TEST_CASE( "complete" , "[zlib][compress][decompress][commplete]" )
{
	namespace rt = restinio::transformator;

	std::srand( std::time( nullptr ) );

	{
		rt::zlib_t zc{ rt::gzip_compress() };
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
		const auto out_data = zc.givaway_output();
		REQUIRE( out_size == out_data.size() );

		rt::zlib_t zd{ rt::gzip_decompress() };
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
		const auto decompression_out_data = zd.givaway_output();
		REQUIRE( decompression_out_size == decompression_out_data.size() );
		REQUIRE( decompression_out_data == input_data + input_data );
	}
}

TEST_CASE( "take output" , "[zlib][compress][decompress][output]" )
{
	namespace rt = restinio::transformator;

	std::srand( std::time( nullptr ) );

	{
		rt::zlib_t zc{ rt::gzip_compress() };
		REQUIRE_FALSE( zc.is_completed() );

		std::string
			input_data{
				"The zlib compression library provides "
				"in-memory compression and decompression functions, "
				"including integrity checks of the uncompressed data." };

		std::string out_data;
		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_NOTHROW( out_data += zc.givaway_output() );
		REQUIRE_NOTHROW( zc.givaway_output() == "" );

		REQUIRE_NOTHROW( zc.flush() );
		REQUIRE_NOTHROW( out_data += zc.givaway_output() );
		REQUIRE_NOTHROW( zc.givaway_output() == "" );

		REQUIRE_NOTHROW( zc.write( input_data ) );
		REQUIRE_NOTHROW( out_data += zc.givaway_output() );
		REQUIRE_NOTHROW( zc.givaway_output() == "" );

		REQUIRE_NOTHROW( zc.complete() );
		REQUIRE_NOTHROW( out_data += zc.givaway_output() );
		REQUIRE_NOTHROW( zc.givaway_output() == "" );

		rt::zlib_t zd{ rt::gzip_decompress() };
		std::string decompression_out_data;

		REQUIRE_NOTHROW( zd.write( out_data.substr(0, out_data.size()/2 ) ) );
		REQUIRE_NOTHROW( decompression_out_data += zd.givaway_output() );
		REQUIRE_NOTHROW( zd.givaway_output() == "" );

		REQUIRE_NOTHROW( zd.flush() );
		REQUIRE_NOTHROW( decompression_out_data += zd.givaway_output() );
		REQUIRE_NOTHROW( zd.givaway_output() == "" );

		REQUIRE_NOTHROW( zd.write( out_data.substr( out_data.size()/2 ) ) );
		REQUIRE_NOTHROW( decompression_out_data += zd.givaway_output() );
		REQUIRE_NOTHROW( zd.givaway_output() == "" );

		REQUIRE_NOTHROW( zd.complete() );
		REQUIRE_NOTHROW( decompression_out_data += zd.givaway_output() );
		REQUIRE_NOTHROW( zd.givaway_output() == "" );

		REQUIRE( decompression_out_data == input_data + input_data );
	}
}
