/*
	restinio
*/

/*!
	Tests response coordinator.
*/

#include <catch2/catch.hpp>

#include <iterator>

#include <restinio/impl/write_group_output_ctx.hpp>

using namespace restinio;
using namespace restinio::impl;

using none_write_operation_t = write_group_output_ctx_t::none_write_operation_t;
using trivial_write_operation_t = write_group_output_ctx_t::trivial_write_operation_t;
using file_write_operation_t = write_group_output_ctx_t::file_write_operation_t;

std::string
make_string( const asio_ns::const_buffer & buf )
{
	return
		std::string{
			asio_ns::buffer_cast< const char * >( buf ),
			asio_ns::buffer_size( buf ) };
}

std::string
concat_bufs( const asio_bufs_container_t & bufs )
{
	std::string res;

	for( const auto & b : bufs )
	{
		res.append( make_string( b ) );
	}

	return res;
}

writable_items_container_t
make_buffers(
	std::vector< std::string > v )
{
	writable_items_container_t result;
	result.reserve( v.size() );

	for( auto & s : v )
	{
		result.emplace_back( std::move( s ) );
	}
	return result;
}

writable_items_container_t
make_buffers( sendfile_t sf )
{
	writable_items_container_t result;
	result.emplace_back( std::move( sf ) );
	return result;
}

writable_items_container_t
make_buffers( writable_items_container_t head, writable_items_container_t tail )
{
	for( auto & wi : tail )
	{
		head.emplace_back( std::move( wi ) );
	}

	return head;
}

template < typename... CONTAINERS >
writable_items_container_t
make_buffers(
	writable_items_container_t first,
	writable_items_container_t second,
	CONTAINERS &&... containers  )
{
	auto merged = make_buffers( std::move( first ), std::move( second ) );
	return
		make_buffers(
			std::move( merged ),
			std::forward< CONTAINERS >( containers )... );
}

TEST_CASE( "write_group_output_ctx_t simple" , "[write_group_output_ctx_t][trivial]" )
{
	{
		write_group_output_ctx_t wg_output{};

		REQUIRE_FALSE( wg_output.transmitting() );
	}

	{
		write_group_output_ctx_t wg_output{};

		wg_output.start_next_write_group(
			write_group_t{
				make_buffers( { "BUFFER" } ) } );

		REQUIRE( wg_output.transmitting() );

		write_group_output_ctx_t::solid_write_operation_variant_t wo{};

		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< trivial_write_operation_t >( wo ) );
		REQUIRE(
			concat_bufs(
				get< trivial_write_operation_t >( wo )
					.get_trivial_bufs() ) == "BUFFER" );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );

		REQUIRE_NOTHROW( wg_output.finish_write_group() );
		REQUIRE_FALSE( wg_output.transmitting() );
	}

	{
		write_group_output_ctx_t wg_output{};

		wg_output.start_next_write_group(
			write_group_t{
				make_buffers( {
					"0", "1", "2", "3", "4", "5", "6", "7",
					"8", "9", "A", "B", "C", "D", "E", "F",
					"0", "1", "2", "3", "4", "5", "6", "7",
					"8", "9", "A", "B", "C", "D", "E", "F",
					"0", "1", "2", "3", "4", "5", "6", "7",
					"8", "9", "A", "B", "C", "D", "E", "F",
					"0", "1", "2", "3", "4", "5", "6", "7",
					"8", "9", "A", "B", "C", "D", "E", "F",
					 } ) } );

		REQUIRE( wg_output.transmitting() );

		write_group_output_ctx_t::solid_write_operation_variant_t wo{};

		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< trivial_write_operation_t >( wo ) );
		REQUIRE(
			concat_bufs(
				get< trivial_write_operation_t >( wo )
					.get_trivial_bufs() ) ==
						"0123456789ABCDEF"
						"0123456789ABCDEF"
						"0123456789ABCDEF"
						"0123456789ABCDEF" );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );

		REQUIRE_NOTHROW( wg_output.finish_write_group() );
		REQUIRE_FALSE( wg_output.transmitting() );
	}

	{
		write_group_output_ctx_t wg_output{};

		wg_output.start_next_write_group(
			write_group_t{
				make_buffers( {
					"0", "1", "2", "3", "4", "5", "6", "7",
					"8", "9", "A", "B", "C", "D", "E", "F",
					"0", "1", "2", "3", "4", "5", "6", "7",
					"8", "9", "A", "B", "C", "D", "E", "F",
					"0", "1", "2", "3", "4", "5", "6", "7",
					"8", "9", "A", "B", "C", "D", "E", "F",
					"0", "1", "2", "3", "4", "5", "6", "7",
					"8", "9", "A", "B", "C", "D", "E", "F",

					"*", "1", "2", "3", "4", "5", "6", "7",
					"8", "9", "A", "B", "C", "D", "E", "F",
					"*", "1", "2", "3", "4", "5", "6", "7",
					"8", "9", "A", "B", "C", "D", "E", "F",
					"*", "1", "2", "3", "4", "5", "6", "7",
					"8", "9", "A", "B", "C", "D", "E", "F",
					"*", "1", "2", "3", "4", "5", "6", "7",
					"8", "9", "A", "B", "C", "D", "E", "F",
					 } ) } );

		REQUIRE( wg_output.transmitting() );

		write_group_output_ctx_t::solid_write_operation_variant_t wo{};

		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );
		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< trivial_write_operation_t >( wo ) );
		REQUIRE(
			concat_bufs(
				get< trivial_write_operation_t >( wo )
					.get_trivial_bufs() ) ==
						"0123456789ABCDEF"
						"0123456789ABCDEF"
						"0123456789ABCDEF"
						"0123456789ABCDEF" );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< trivial_write_operation_t >( wo ) );
		REQUIRE(
			concat_bufs(
				get< trivial_write_operation_t >( wo )
					.get_trivial_bufs() ) ==
						"*123456789ABCDEF"
						"*123456789ABCDEF"
						"*123456789ABCDEF"
						"*123456789ABCDEF" );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );

		REQUIRE_NOTHROW( wg_output.finish_write_group() );
		REQUIRE_FALSE( wg_output.transmitting() );
	}
	{
		write_group_output_ctx_t wg_output{};

		wg_output.start_next_write_group(
			write_group_t{
				make_buffers( {
					"0", "1", "2", "3", "4", "5", "6", "7",
					"8", "9", "A", "B", "C", "D", "E", "F",
					"0", "1", "2", "3", "4", "5", "6", "7",
					"8", "9", "A", "B", "C", "D", "E", "F",
					"0", "1", "2", "3", "4", "5", "6", "7",
					"8", "9", "A", "B", "C", "D", "E", "F",
					"0", "1", "2", "3", "4", "5", "6", "7",
					"8", "9", "A", "B", "C", "D", "E", "F",

					"*****"
					 } ) } );

		REQUIRE( wg_output.transmitting() );

		write_group_output_ctx_t::solid_write_operation_variant_t wo{};

		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< trivial_write_operation_t >( wo ) );
		REQUIRE(
			concat_bufs(
				get< trivial_write_operation_t >( wo )
					.get_trivial_bufs() ) ==
						"0123456789ABCDEF"
						"0123456789ABCDEF"
						"0123456789ABCDEF"
						"0123456789ABCDEF" );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< trivial_write_operation_t >( wo ) );
		REQUIRE(
			concat_bufs(
				get< trivial_write_operation_t >( wo )
					.get_trivial_bufs() ) == "*****" );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );

		REQUIRE_NOTHROW( wg_output.finish_write_group() );
		REQUIRE_FALSE( wg_output.transmitting() );
	}
}

TEST_CASE( "write_group_output_ctx_t simple sf" , "[write_group_output_ctx_t][sendfile]" )
{
	{
		write_group_output_ctx_t wg_output{};

		wg_output.start_next_write_group(
			write_group_t{
				make_buffers( restinio::sendfile(
					restinio::null_file_descriptor() /* fake not real */,
					restinio::file_meta_t{ 1024, std::chrono::system_clock::now() } ) ) } );

		REQUIRE( wg_output.transmitting() );

		write_group_output_ctx_t::solid_write_operation_variant_t wo{};

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< file_write_operation_t >( wo ) );
		REQUIRE( 1024 == get< file_write_operation_t >( wo ).size() );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );

		REQUIRE_NOTHROW( wg_output.finish_write_group() );
		REQUIRE_FALSE( wg_output.transmitting() );
	}

	{
		write_group_output_ctx_t wg_output{};

		wg_output.start_next_write_group(
			write_group_t{
				make_buffers(
					make_buffers( restinio::sendfile(
						restinio::null_file_descriptor() /* fake not real */,
						restinio::file_meta_t{ 1024, std::chrono::system_clock::now() } ) ),
					make_buffers( restinio::sendfile(
						restinio::null_file_descriptor() /* fake not real */,
						restinio::file_meta_t{ 2048, std::chrono::system_clock::now() } ) ),
					make_buffers( restinio::sendfile(
						restinio::null_file_descriptor() /* fake not real */,
						restinio::file_meta_t{ 4096, std::chrono::system_clock::now() } ) )
				) } );

		REQUIRE( wg_output.transmitting() );

		write_group_output_ctx_t::solid_write_operation_variant_t wo{};

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< file_write_operation_t >( wo ) );
		REQUIRE( 1024 == get< file_write_operation_t >( wo ).size() );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< file_write_operation_t >( wo ) );
		REQUIRE( 2048 == get< file_write_operation_t >( wo ).size() );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< file_write_operation_t >( wo ) );
		REQUIRE( 4096 == get< file_write_operation_t >( wo ).size() );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );

		REQUIRE_NOTHROW( wg_output.finish_write_group() );
		REQUIRE_FALSE( wg_output.transmitting() );
	}
}

TEST_CASE( "write_group_output_ctx_t mixed" , "[write_group_output_ctx_t][mix][trivial][sendfile]" )
{
	{
		write_group_output_ctx_t wg_output{};

		wg_output.start_next_write_group(
			write_group_t{
				make_buffers(
					make_buffers( {
						"0", "1", "2", "3", "4", "5", "6", "7",
						"8", "9", "A", "B", "C", "D", "E", "F",
						"0", "1", "2", "3", "4", "5", "6", "7",
						"8", "9", "A", "B", "C", "D", "E", "F",
						"0", "1", "2", "3", "4", "5", "6", "7",
						"8", "9", "A", "B", "C", "D", "E", "F",
						"0", "1", "2", "3", "4", "5", "6", "7",
						"8", "9", "A", "B", "C", "D", "E", "F",
						 } ),
					make_buffers( restinio::sendfile(
						restinio::null_file_descriptor() /* fake not real */,
						restinio::file_meta_t{ 1024, std::chrono::system_clock::now() } ) ),
					make_buffers( {
						"0", "1", "2", "3", "4", "5", "6", "7",
						"8", "9", "A", "B", "C", "D", "E", "F",
						 } ),
					make_buffers( restinio::sendfile(
						restinio::null_file_descriptor() /* fake not real */,
						restinio::file_meta_t{ 2048, std::chrono::system_clock::now() } ) ),
					make_buffers( {
						"0", "1", "2", "3", "4", "5", "6", "7",
						"8", "9", "A", "B", "C", "D", "E", "F",
						"0", "1", "2", "3", "4", "5", "6", "7",
						"8", "9", "A", "B", "C", "D", "E", "F",
						"0", "1", "2", "3", "4", "5", "6", "7",
						"8", "9", "A", "B", "C", "D", "E", "F",
						"0", "1", "2", "3", "4", "5", "6", "7",
						"8", "9", "A", "B", "C", "D", "E", "F",

						"*", "*", "*", "*", "*", "*", "*", "*",
						"*", "*", "*", "*", "*", "*", "*", "*",
						 } ),
					make_buffers( restinio::sendfile(
						restinio::null_file_descriptor() /* fake not real */,
						restinio::file_meta_t{ 4096, std::chrono::system_clock::now() } ) ),
					make_buffers( {
						"THE", " ", "END" } ) ) } );

		REQUIRE( wg_output.transmitting() );

		write_group_output_ctx_t::solid_write_operation_variant_t wo{};

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< trivial_write_operation_t >( wo ) );
		REQUIRE(
			concat_bufs(
				get< trivial_write_operation_t >( wo )
					.get_trivial_bufs() ) ==
						"0123456789ABCDEF"
						"0123456789ABCDEF"
						"0123456789ABCDEF"
						"0123456789ABCDEF" );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< file_write_operation_t >( wo ) );
		REQUIRE( 1024 == get< file_write_operation_t >( wo ).size() );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< trivial_write_operation_t >( wo ) );
		REQUIRE(
			concat_bufs(
				get< trivial_write_operation_t >( wo )
					.get_trivial_bufs() ) == "0123456789ABCDEF" );


		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< file_write_operation_t >( wo ) );
		REQUIRE( 2048 == get< file_write_operation_t >( wo ).size() );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< trivial_write_operation_t >( wo ) );
		REQUIRE(
			concat_bufs(
				get< trivial_write_operation_t >( wo )
					.get_trivial_bufs() ) ==
						"0123456789ABCDEF"
						"0123456789ABCDEF"
						"0123456789ABCDEF"
						"0123456789ABCDEF" );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< trivial_write_operation_t >( wo ) );
		REQUIRE(
			concat_bufs(
				get< trivial_write_operation_t >( wo )
					.get_trivial_bufs() ) == "****************" );


		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< file_write_operation_t >( wo ) );
		REQUIRE( 4096 == get< file_write_operation_t >( wo ).size() );


		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< trivial_write_operation_t >( wo ) );
		REQUIRE(
			concat_bufs(
				get< trivial_write_operation_t >( wo )
					.get_trivial_bufs() ) == "THE END" );


		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );

		REQUIRE_NOTHROW( wg_output.finish_write_group() );
		REQUIRE_FALSE( wg_output.transmitting() );
	}
}

TEST_CASE( "write_group_output_ctx_t two groups" , "[write_group_output_ctx_t][trivial][restart]" )
{
	{
		write_group_output_ctx_t wg_output{};

		wg_output.start_next_write_group(
			write_group_t{
				make_buffers( { "BUFFER1" } ) } );

		REQUIRE( wg_output.transmitting() );

		write_group_output_ctx_t::solid_write_operation_variant_t wo{};

		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );
		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< trivial_write_operation_t >( wo ) );
		REQUIRE(
			concat_bufs(
				get< trivial_write_operation_t >( wo )
					.get_trivial_bufs() ) == "BUFFER1" );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );

		REQUIRE_NOTHROW( wg_output.finish_write_group() );
		REQUIRE_FALSE( wg_output.transmitting() );

		// Start with new group.

		wg_output.start_next_write_group(
			write_group_t{
				make_buffers( { "BUFFER2" } ) } );

		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );
		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< trivial_write_operation_t >( wo ) );
		REQUIRE(
			concat_bufs(
				get< trivial_write_operation_t >( wo )
					.get_trivial_bufs() ) == "BUFFER2" );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );

		REQUIRE_NOTHROW( wg_output.finish_write_group() );
		REQUIRE_FALSE( wg_output.transmitting() );


		// Start with new group.
		wg_output.start_next_write_group(
			write_group_t{
				make_buffers( { "BUFFER3" } ) } );

		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );
		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< trivial_write_operation_t >( wo ) );
		REQUIRE(
			concat_bufs(
				get< trivial_write_operation_t >( wo )
					.get_trivial_bufs() ) == "BUFFER3" );

		REQUIRE_NOTHROW( wo = wg_output.extract_next_write_operation() );
		REQUIRE( holds_alternative< none_write_operation_t >( wo ) );

		REQUIRE_NOTHROW( wg_output.finish_write_group() );
		REQUIRE_FALSE( wg_output.transmitting() );
	}
}
