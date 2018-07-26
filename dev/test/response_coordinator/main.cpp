/*
	restinio
*/

/*!
	Tests response coordinator.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <iterator>

#include <restinio/impl/response_coordinator.hpp>

using namespace restinio;
using namespace restinio::impl;

constexpr response_parts_attr_t
response_is_not_complete() { return response_parts_attr_t::not_final_parts; }

constexpr response_parts_attr_t
response_is_complete() { return response_parts_attr_t::final_parts; }

constexpr response_connection_attr_t
connection_should_keep_alive() { return response_connection_attr_t::connection_keepalive; }

constexpr response_connection_attr_t
connection_should_close() { return response_connection_attr_t::connection_close; }

template < typename BUF >
std::string
make_string( const BUF & buf )
{
	return
		std::string{
			asio_ns::buffer_cast< const char * >( buf.buf() ),
			asio_ns::buffer_size( buf.buf() ) };
}

std::string
concat_bufs( write_group_t & wg )
{
	std::string res;

	for( const auto & b : wg.items() )
	{
		res.append( make_string( b ) );
	}

	return res;
};


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

TEST_CASE( "response_context_t" , "[response_context]" )
{
	{
		// Merge possible.

		response_context_t ctx;

		ctx.enqueue_group( write_group_t{ make_buffers( {"0123456789", "0123456789" } ) } );
		ctx.enqueue_group( write_group_t{ make_buffers( {"qqqqqqqqqq", "wwwwwwwwww" } ) } );

		REQUIRE( 1 == utest_access( ctx ).size() );
		REQUIRE( 4 == utest_access( ctx ).front().items_count() );
		REQUIRE( 0 == utest_access( ctx ).front().status_line_size() );
		REQUIRE_FALSE( utest_access( ctx ).front().after_write_notificator() );

		ctx.enqueue_group( write_group_t{ make_buffers( {"eeeeeeeeeee", "333333333" } ) } );

		REQUIRE( 1 == utest_access( ctx ).size() );
		REQUIRE( 6 == utest_access( ctx ).front().items_count() );
		REQUIRE( 0 == utest_access( ctx ).front().status_line_size() );
		REQUIRE_FALSE( utest_access( ctx ).front().after_write_notificator() );

		ctx.dequeue_group();
		REQUIRE( 0 == utest_access( ctx ).size() );
	}

	{
		// Merge possible.
		response_context_t ctx;

		write_group_t wg{ make_buffers( {"0123456789", "0123456789" } ) };
		wg.status_line_size( 4 );

		ctx.enqueue_group( std::move( wg ) );
		ctx.enqueue_group( write_group_t{ make_buffers( {"qqqqqqqqqq", "wwwwwwwwww" } ) } );

		REQUIRE( 1 == utest_access( ctx ).size() );
		REQUIRE( 4 == utest_access( ctx ).front().items_count() );
		REQUIRE( 4 == utest_access( ctx ).front().status_line_size() );
		REQUIRE_FALSE( utest_access( ctx ).front().after_write_notificator() );

		write_group_t wg2{ make_buffers( {"0123456789", "0123456789" } ) };
		wg2.after_write_notificator( []( const auto & ec ){} );
		ctx.enqueue_group( std::move( wg2 ) );

		REQUIRE( 1 == utest_access( ctx ).size() );
		REQUIRE( 6 == utest_access( ctx ).front().items_count() );
		REQUIRE( 4 == utest_access( ctx ).front().status_line_size() );
		REQUIRE( static_cast< bool >( utest_access( ctx ).front().after_write_notificator() ) );
	}

	{
		// Merge is not possible.

		response_context_t ctx;

		write_group_t wg{ make_buffers( {"0123456789", "0123456789" } ) };
		wg.after_write_notificator( []( const auto & ec ){} );

		ctx.enqueue_group( std::move( wg ) );
		ctx.enqueue_group( write_group_t{ make_buffers( {"qqqqqqqqqq", "wwwwwwwwww" } ) } );

		REQUIRE( 2 == utest_access( ctx ).size() );
		REQUIRE( 2 == utest_access( ctx ).front().items_count() );
		REQUIRE( static_cast< bool >( utest_access( ctx ).front().after_write_notificator() ) );
	}

	{
		// Merge is not possible.

		response_context_t ctx;

		ctx.enqueue_group( write_group_t{ make_buffers( {"qqqqqqqqqq", "wwwwwwwwww" } ) } );
		write_group_t wg{ make_buffers( {"0123456789", "0123456789" } ) };
		wg.status_line_size( 4 );
		ctx.enqueue_group( std::move( wg ) );

		REQUIRE( 2 == utest_access( ctx ).size() );
		REQUIRE( 4 == utest_access( ctx ).back().status_line_size() );
	}
}

TEST_CASE( "response_context_table" , "[response_context][response_context_table]" )
{
	SECTION( "empty" )
	{
		constexpr auto max_elements_count = 16UL;

		response_context_table_t table( max_elements_count );

		REQUIRE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
		REQUIRE_THROWS( table.pop_response_context() );
		REQUIRE( nullptr == table.get_by_req_id( 0UL ) );
		REQUIRE( nullptr == table.get_by_req_id( 1UL ) );
	}

	SECTION( "single" )
	{
		constexpr auto max_elements_count = 1UL;

		response_context_table_t table( max_elements_count );

		REQUIRE( table.empty() );
		REQUIRE_FALSE( table.is_full() );

		CHECK_NOTHROW( table.push_response_context( 42UL ) );
		CHECK_THROWS( table.push_response_context( 43UL ) );

		REQUIRE_FALSE( table.empty() );
		REQUIRE( table.is_full() );
		REQUIRE( table.get_by_req_id( 42UL ) == &table.front() );
		REQUIRE( table.get_by_req_id( 42UL ) == &table.back() );

		REQUIRE( nullptr != table.get_by_req_id( 42UL ) );
		REQUIRE( nullptr == table.get_by_req_id( 41UL ) );
		REQUIRE( nullptr == table.get_by_req_id( 43UL ) );

		REQUIRE( table.get_by_req_id( 42UL )->request_id() == 42UL );

		REQUIRE( response_parts_attr_t::not_final_parts ==
			table.get_by_req_id( 42UL )->response_output_flags().m_response_parts );
		REQUIRE(
			response_connection_attr_t::connection_keepalive ==
			table.get_by_req_id( 42UL )->response_output_flags().m_response_connection );

		CHECK_NOTHROW( table.pop_response_context() );
	}

	SECTION( "max_elements_count=4" )
	{
		constexpr auto max_elements_count = 4UL;

		response_context_table_t table( max_elements_count );

		REQUIRE( table.empty() );
		REQUIRE_FALSE( table.is_full() );

		CHECK_NOTHROW( table.push_response_context( 42UL ) );

		REQUIRE_FALSE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
		REQUIRE( table.get_by_req_id( 42UL ) == &table.front() );
		REQUIRE( table.get_by_req_id( 42UL ) == &table.back() );

		CHECK_NOTHROW( table.push_response_context( 43UL ) );

		REQUIRE_FALSE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
		REQUIRE( table.get_by_req_id( 42UL ) == &table.front() );
		REQUIRE( table.get_by_req_id( 43UL ) == &table.back() );

		CHECK_NOTHROW( table.push_response_context( 44UL ) );

		REQUIRE_FALSE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
		REQUIRE( table.get_by_req_id( 42UL ) == &table.front() );
		REQUIRE( table.get_by_req_id( 44UL ) == &table.back() );

		CHECK_NOTHROW( table.push_response_context( 45UL ) );

		REQUIRE_FALSE( table.empty() );
		REQUIRE( table.is_full() );
		REQUIRE( table.get_by_req_id( 42UL ) == &table.front() );
		REQUIRE( table.get_by_req_id( 45UL ) == &table.back() );

		CHECK_THROWS( table.push_response_context( 46UL ) );

		CHECK_NOTHROW( table.pop_response_context() );

		REQUIRE_FALSE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
		REQUIRE( table.get_by_req_id( 43UL ) == &table.front() );
		REQUIRE( table.get_by_req_id( 45UL ) == &table.back() );

		CHECK_NOTHROW( table.push_response_context( 46UL ) );
		REQUIRE_FALSE( table.empty() );
		REQUIRE( table.is_full() );
		REQUIRE( table.get_by_req_id( 43UL ) == &table.front() );
		REQUIRE( table.get_by_req_id( 46UL ) == &table.back() );

		CHECK_THROWS( table.push_response_context( 47UL ) );
		CHECK_NOTHROW( table.pop_response_context() );
		CHECK_NOTHROW( table.push_response_context( 47UL ) );
		REQUIRE_FALSE( table.empty() );
		REQUIRE( table.is_full() );
		REQUIRE( table.get_by_req_id( 44UL ) == &table.front() );
		REQUIRE( table.get_by_req_id( 47UL ) == &table.back() );

		CHECK_THROWS( table.push_response_context( 48UL ) );
		CHECK_NOTHROW( table.pop_response_context() );
		CHECK_NOTHROW( table.push_response_context( 48UL ) );
		REQUIRE_FALSE( table.empty() );
		REQUIRE( table.is_full() );
		REQUIRE( table.get_by_req_id( 45UL ) == &table.front() );
		REQUIRE( table.get_by_req_id( 48UL ) == &table.back() );

		CHECK_THROWS( table.push_response_context( 49UL ) );
		CHECK_NOTHROW( table.pop_response_context() );
		CHECK_NOTHROW( table.push_response_context( 49UL ) );
		REQUIRE_FALSE( table.empty() );
		REQUIRE( table.is_full() );
		REQUIRE( table.get_by_req_id( 46UL ) == &table.front() );
		REQUIRE( table.get_by_req_id( 49UL ) == &table.back() );

		CHECK_NOTHROW( table.pop_response_context() );
		REQUIRE_FALSE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
		REQUIRE( table.get_by_req_id( 47UL ) == &table.front() );
		REQUIRE( table.get_by_req_id( 49UL ) == &table.back() );

		CHECK_NOTHROW( table.pop_response_context() );
		REQUIRE_FALSE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
		REQUIRE( table.get_by_req_id( 48UL ) == &table.front() );
		REQUIRE( table.get_by_req_id( 49UL ) == &table.back() );

		CHECK_NOTHROW( table.pop_response_context() );
		REQUIRE_FALSE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
		REQUIRE( table.get_by_req_id( 49UL ) == &table.front() );
		REQUIRE( table.get_by_req_id( 49UL ) == &table.back() );

		CHECK_NOTHROW( table.pop_response_context() );
		REQUIRE( table.empty() );
		REQUIRE_FALSE( table.is_full() );
	}
}

TEST_CASE( "response_coordinator" , "[response_coordinator]" )
{
	SECTION( "simple" )
	{
		response_coordinator_t coordinator{ 2 };
		REQUIRE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE_FALSE( coordinator.closed() );

		auto popped_wg = coordinator.pop_ready_buffers();
		REQUIRE_FALSE( popped_wg );

		request_id_t req_id[ 2 ];

		CHECK_NOTHROW( req_id[ 0 ] = coordinator.register_new_request() );
		REQUIRE( req_id[ 0 ] == 0UL );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );

		CHECK_NOTHROW( req_id[ 1 ] = coordinator.register_new_request() );
		REQUIRE( req_id[ 1 ] == 1UL );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE( coordinator.is_full() );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "a", "b", "c" } ) } ) );

		// #0: ("a", "b", "c")
		// #1: <nothing>

		popped_wg = coordinator.pop_ready_buffers();

		REQUIRE( popped_wg );
		REQUIRE( 3UL == popped_wg->items_count() );
		REQUIRE( make_string( popped_wg->items()[ 0UL ] ) == "a" );
		REQUIRE( make_string( popped_wg->items()[ 1UL ] ) == "b" );
		REQUIRE( make_string( popped_wg->items()[ 2UL ] ) == "c" );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "A", "B", "C" } ) } ) );
		// #0: ("A", "B", "C")
		// #1: <nothing>

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "X", "Y", "Z" } ) } ) );
		// #0: ("A", "B", "C")
		// #1: ("X", "Y", "Z")

		popped_wg = coordinator.pop_ready_buffers();

		REQUIRE( popped_wg );
		REQUIRE( 3UL == popped_wg->items_count() );
		REQUIRE( make_string( popped_wg->items()[ 0UL ] ) == "A" );
		REQUIRE( make_string( popped_wg->items()[ 1UL ] ) == "B" );
		REQUIRE( make_string( popped_wg->items()[ 2UL ] ) == "C" );

		// #0: <nothing>
		// #1: ("X", "Y", "Z")

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE_FALSE( popped_wg );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "LAST", "PARTS" } ) } ) );
		// #0: <nothing>
		// #1: ("X", "Y", "Z", "LAST", "PARTS") groups merged

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE_FALSE( popped_wg );

		// Append complete response error:
		CHECK_THROWS( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "!LAST!", "!PARTS!" } ) } ) );


		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "LAST", "PARTS" } ) } ) );

		// Append complete response error:
		CHECK_THROWS( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "!LAST!", "!PARTS!" } ) } ) );

		// #0: ("LAST", "PARTS")
		// #1: ("X", "Y", "Z", "LAST", "PARTS")

		popped_wg = coordinator.pop_ready_buffers();

		REQUIRE( popped_wg );
		REQUIRE( 2UL == popped_wg->items_count() );
		REQUIRE( make_string( popped_wg->items()[ 0UL ] ) == "LAST" );
		REQUIRE( make_string( popped_wg->items()[ 1UL ] ) == "PARTS" );

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE( 5UL == popped_wg->items_count() );
		REQUIRE( make_string( popped_wg->items()[ 0UL ] ) == "X" );
		REQUIRE( make_string( popped_wg->items()[ 1UL ] ) == "Y" );
		REQUIRE( make_string( popped_wg->items()[ 2UL ] ) == "Z" );
		REQUIRE( make_string( popped_wg->items()[ 3UL ] ) == "LAST" );
		REQUIRE( make_string( popped_wg->items()[ 4UL ] ) == "PARTS" );

		// Response doesn't exist any more error:
		CHECK_THROWS( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "!NO!", "!WAY!" } ) } ) );

		CHECK_NOTHROW( req_id[ 0 ] = coordinator.register_new_request() );
		REQUIRE( req_id[ 0 ] == 2UL );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );

		// #2: <nothing>

		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		CHECK_NOTHROW( req_id[ 1 ] = coordinator.register_new_request() );
		REQUIRE( req_id[ 1 ] == 3UL );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE( coordinator.is_full() );

		// #2: <nothing>
		// #3: <nothing>

		REQUIRE_FALSE( coordinator.closed() );
	}

	SECTION( "complex scenario")
	{
		response_coordinator_t coordinator{ 4 };
		REQUIRE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE_FALSE( coordinator.closed() );

		request_id_t req_id[ 4 ];

		CHECK_NOTHROW( req_id[ 0 ] = coordinator.register_new_request() );
		REQUIRE( req_id[ 0 ] == 0UL );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		CHECK_NOTHROW( req_id[ 1 ] = coordinator.register_new_request() );
		REQUIRE( req_id[ 1 ] == 1UL );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		CHECK_NOTHROW( req_id[ 2 ] = coordinator.register_new_request() );
		REQUIRE( req_id[ 2 ] == 2UL );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		CHECK_NOTHROW( req_id[ 3 ] = coordinator.register_new_request() );
		REQUIRE( req_id[ 3 ] == 3UL );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE( coordinator.is_full() );

		// #0: <nothing>
		// #1: <nothing>
		// #2: <nothing>
		// #3: <nothing>

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{ "0a", "0b", "0c", "0a", "0b", "0c",
					  "0a", "0b", "0c", "0a", "0b", "0c",
					  "0a", "0b", "0c", "0a", "0b", "0c",
					  "0a", "0b", "0c", "0a", "0b", "0c" } ) } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{ "1a", "1b", "1c", "1a", "1b", "1c",
					  "1a", "1b", "1c", "1a", "1b", "1c",
					  "1a", "1b", "1c", "1a", "1b", "1c",
					  "1a", "1b", "1c", "1a", "1b", "1c" } ) } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 2 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{ "2a", "2b", "2c", "2a", "2b", "2c",
					  "2a", "2b", "2c", "2a", "2b", "2c",
					  "2a", "2b", "2c", "2a", "2b", "2c",
					  "2a", "2b", "2c", "2a", "2b", "2c" } ) } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 3 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{ "3a", "3b", "3c", "3a", "3b", "3c",
					  "3a", "3b", "3c", "3a", "3b", "3c",
					  "3a", "3b", "3c", "3a", "3b", "3c",
					  "3a", "3b", "3c", "3a", "3b", "3c" } ) } ) );

		// #0: ["0a", "0b", "0c"] * 2*4
		// #1: ["1a", "1b", "1c"] * 2*4
		// #2: ["2a", "2b", "2c"] * 2*4
		// #3: ["3a", "3b", "3c"] * 2*4

		// Only bufs for #0 response mast be presented:

		auto popped_wg = coordinator.pop_ready_buffers();

		REQUIRE( popped_wg );
		REQUIRE( 24UL == popped_wg->items_count() );
		REQUIRE( concat_bufs( *popped_wg ) ==
				"0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c" );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE( coordinator.is_full() );

		// // #0: <nothing>
		// // #1: ["1a", "1b", "1c"] * 2*4
		// // #2: ["2a", "2b", "2c"] * 2*4
		// // #3: ["3a", "3b", "3c"] * 2*4

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE_FALSE( popped_wg );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE( coordinator.is_full() );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{ "LAST", "PARTS",
					  "LAST", "PARTS",
					  "LAST", "PARTS",
					  "LAST", "PARTS"  } ) } ) );

		// #0: ["LAST", "PARTS"] * 4
		// #1: ["1a", "1b", "1c"] * 2*4
		// #2: ["2a", "2b", "2c"] * 2*4
		// #3: ["3a", "3b", "3c"] * 2*4

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{ "LAST", "PARTS",
					  "LAST", "PARTS",
					  "LAST", "PARTS",
					  "LAST", "PARTS" } ) } ) );

		// #0: ["LAST", "PARTS"] * 4
		// #1: ["1a", "1b", "1c"] * 2*4 + ["LAST", "PARTS"] * 4
		// #2: ["2a", "2b", "2c"] * 2*4
		// #3: ["3a", "3b", "3c"] * 2*4

		popped_wg = coordinator.pop_ready_buffers();

		REQUIRE( popped_wg );
		REQUIRE( 8UL == popped_wg->items_count() );
		REQUIRE( concat_bufs( *popped_wg ) ==
				"LASTPARTSLASTPARTSLASTPARTSLASTPARTS" );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );


		popped_wg = coordinator.pop_ready_buffers();

		REQUIRE( popped_wg );
		REQUIRE( 32UL == popped_wg->items_count() );
		REQUIRE( concat_bufs( *popped_wg ) ==
				"1a1b1c1a1b1c1a1b1c1a1b1c1a1b1c1a1b1c1a1b1c1a1b1c"
				"LASTPARTSLASTPARTSLASTPARTSLASTPARTS" );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );

		popped_wg = coordinator.pop_ready_buffers();

		REQUIRE( popped_wg );
		REQUIRE( 24UL == popped_wg->items_count() );
		REQUIRE( concat_bufs( *popped_wg ) ==
				"2a2b2c2a2b2c2a2b2c2a2b2c2a2b2c2a2b2c2a2b2c2a2b2c" );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );

		// #2: <nothing>
		// #3: ["3a", "3b", "3c"] * 2*4

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE_FALSE( popped_wg );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 2 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{ "LAST", "PARTS",
					  "LAST", "PARTS",
					  "LAST", "PARTS",
					  "LAST", "PARTS"  } ) } ) );

		// #2: ["LAST", "PARTS"] * 4
		// #3: ["3a", "3b", "3c"] * 2*4

		popped_wg = coordinator.pop_ready_buffers();

		REQUIRE( popped_wg );
		REQUIRE( 8UL == popped_wg->items_count() );
		REQUIRE( concat_bufs( *popped_wg ) ==
				"LASTPARTSLASTPARTSLASTPARTSLASTPARTS" );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );

		// #3: ["3a", "3b", "3c"] * 2*4

		popped_wg = coordinator.pop_ready_buffers();

		REQUIRE( popped_wg );
		REQUIRE( 24UL == popped_wg->items_count() );
		REQUIRE( concat_bufs( *popped_wg ) ==
				"3a3b3c3a3b3c3a3b3c3a3b3c3a3b3c3a3b3c3a3b3c3a3b3c" );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );

		// #3: <nothing>

		CHECK_NOTHROW( req_id[ 0 ] = coordinator.register_new_request() );
		REQUIRE( req_id[ 0 ] == 4UL );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		CHECK_NOTHROW( req_id[ 1 ] = coordinator.register_new_request() );
		REQUIRE( req_id[ 1 ] == 5UL );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		CHECK_NOTHROW( req_id[ 2 ] = coordinator.register_new_request() );
		REQUIRE( req_id[ 2 ] == 6UL );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE( coordinator.is_full() );


		// #3: <nothing>
		// #4: <nothing>
		// #5: <nothing>
		// #6: <nothing>

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE_FALSE( popped_wg );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{
					  "4a", "4b", "4c", "4a", "4b", "4c",
					  "4a", "4b", "4c", "4a", "4b", "4c",
					  "4a", "4b", "4c", "4a", "4b", "4c",
					  "4a", "4b", "4c", "4a", "4b", "4c",

					  "4a", "4b", "4c", "4a", "4b", "4c",
					  "4a", "4b", "4c", "4a", "4b", "4c",
					  "4a", "4b", "4c", "4a", "4b", "4c",
					  "4a", "4b", "4c", "4a", "4b", "4c",

					  "4a", "4b", "4c", "4a", "4b", "4c",
					  "4a", "4b", "4c", "4a", "4b", "4c",
					  "4a", "4b", "4c", "4a", "4b", "4c",
					  "4a", "4b", "4c", "4a", "4b", "4c",

					  "LAST", "PARTS" } ) } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{ "5a", "5b", "5c", "5a", "5b", "5c",
					  "5a", "5b", "5c", "5a", "5b", "5c",
					  "5a", "5b", "5c", "5a", "5b", "5c",
					  "5a", "5b", "5c", "5a", "5b", "5c" } ) }) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 2 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{ "6a", "6b", "6c", "6a", "6b", "6c",
					  "6a", "6b", "6c", "6a", "6b", "6c",
					  "6a", "6b", "6c", "6a", "6b", "6c",
					  "6a", "6b", "6c", "6a", "6b", "6c" } ) } ) );


		// #3: <nothing>
		// #4: ["4a", "4b", "4c"] * 2*4 *3 + ["LAST", "PARTS"]
		// #5: ["5a", "5b", "5c"] * 2*4
		// #6: ["6a", "6b", "6c"] * 2*4

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE_FALSE( popped_wg );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 3 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{ "LAST", "PARTS",
					  "LAST", "PARTS",
					  "LAST", "PARTS",
					  "LAST", "PARTS" } ) } ) );

		// #3: [ "LAST", "PARTS" ] * 4
		// #4: ["4a", "4b", "4c"] * 2*4 *3 + ["LAST", "PARTS"]
		// #5: ["5a", "5b", "5c"] * 2*4
		// #6: ["6a", "6b", "6c"] * 2*4

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE( popped_wg );

		REQUIRE( 8UL == popped_wg->items_count() );
		REQUIRE( concat_bufs( *popped_wg ) ==
				"LASTPARTSLASTPARTSLASTPARTSLASTPARTS" );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );

		// #3 already completed.
		CHECK_THROWS( coordinator.append_response(
			req_id[ 3 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "NO", "WAY" } ) } ) );

		// #4: ["4a", "4b", "4c"] * 2*4 *3 + ["LAST", "PARTS"]
		// #5: ["5a", "5b", "5c"] * 2*4
		// #6: ["6a", "6b", "6c"] * 2*4

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE( popped_wg );

		REQUIRE( 74UL  == popped_wg->items_count() );
		REQUIRE( concat_bufs( *popped_wg ) ==
				"4a4b4c4a4b4c4a4b4c4a4b4c4a4b4c4a4b4c4a4b4c4a4b4c"
				"4a4b4c4a4b4c4a4b4c4a4b4c4a4b4c4a4b4c4a4b4c4a4b4c"
				"4a4b4c4a4b4c4a4b4c4a4b4c4a4b4c4a4b4c4a4b4c4a4b4c"
				"LASTPARTS" );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );

		// #5: ["5a", "5b", "5c"] * 2*4
		// #6: ["6a", "6b", "6c"] * 2*4

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE( popped_wg );

		REQUIRE( 24UL  == popped_wg->items_count() );
		REQUIRE( concat_bufs( *popped_wg ) ==
				"5a5b5c5a5b5c5a5b5c5a5b5c5a5b5c5a5b5c5a5b5c5a5b5c" );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );

		// #5: <nothing>
		// #6: ["6a", "6b", "6c"] * 2*4

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{ "LAST", "PARTS",
					  "LAST", "PARTS",
					  "LAST", "PARTS",
					  "LAST", "PARTS" } ) } ) );

		// #5: [ "LAST", "PARTS" ] * 4
		// #6: ["6a", "6b", "6c"] * 2*4

		// #5 already response_is_complete().
		CHECK_THROWS( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "NO", "WAY" } ) } ) );

		// #5: [ "LAST", "PARTS" ] * 4
		// #6: ["6a", "6b", "6c"] * 2*4

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE( popped_wg );

		REQUIRE( 8UL == popped_wg->items_count() );
		REQUIRE( concat_bufs( *popped_wg ) ==
				"LASTPARTSLASTPARTSLASTPARTSLASTPARTS" );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );

		// #6: ["6a", "6b", "6c"] * 2*4

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE( popped_wg );

		REQUIRE( 24UL == popped_wg->items_count() );
		REQUIRE( concat_bufs( *popped_wg ) ==
				"6a6b6c6a6b6c6a6b6c6a6b6c6a6b6c6a6b6c6a6b6c6a6b6c" );
		REQUIRE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );

		// EMPTY

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE_FALSE( popped_wg );

		CHECK_THROWS( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "NO", "WAY" } ) } ) );


		CHECK_THROWS( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "NO", "WAY" } ) } ) );


		CHECK_THROWS( coordinator.append_response(
			req_id[ 2 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "NO", "WAY" } ) } ) );


		CHECK_THROWS( coordinator.append_response(
			req_id[ 3 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "NO", "WAY" } ) } ) );


		REQUIRE_FALSE( coordinator. closed() );
	}
}

TEST_CASE( "response_coordinator_with_close" , "[response_coordinator][connection_close]" )
{
	SECTION( "first request causes close" )
	{
		response_coordinator_t coordinator{ 3 };
		REQUIRE_FALSE( coordinator.closed() );

		request_id_t req_id[ 3 ];

		CHECK_NOTHROW( req_id[ 0 ] = coordinator.register_new_request() );
		CHECK_NOTHROW( req_id[ 1 ] = coordinator.register_new_request() );
		CHECK_NOTHROW( req_id[ 2 ] = coordinator.register_new_request() );

		// #0: <nothing>
		// #1: <nothing>
		// #2: <nothing>

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_close() }, // CLOSE CONNECTION AT RESP #0
			write_group_t{
				make_buffers(
					{ "0a", "0b", "0c", "0a", "0b", "0c",
					  "0a", "0b", "0c", "0a", "0b", "0c",
					  "0a", "0b", "0c", "0a", "0b", "0c",
					  "0a", "0b", "0c", "0a", "0b", "0c" } ) } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{ "1a", "1b", "1c", "1a", "1b", "1c",
					  "1a", "1b", "1c", "1a", "1b", "1c",
					  "1a", "1b", "1c", "1a", "1b", "1c",
					  "1a", "1b", "1c", "1a", "1b", "1c" } ) } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 2 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{ "2a", "2b", "2c", "2a", "2b", "2c",
					  "2a", "2b", "2c", "2a", "2b", "2c",
					  "2a", "2b", "2c", "2a", "2b", "2c",
					  "2a", "2b", "2c", "2a", "2b", "2c" } ) } ) );


		// #0: ["0a", "0b", "0c"] * 2*4
		// #1: ["0a", "0b", "0c"] * 2*4
		// #2: ["2a", "2b", "2c"] * 2*4

		auto popped_wg = coordinator.pop_ready_buffers();
		REQUIRE( popped_wg );

		REQUIRE( 24UL == popped_wg->items_count() );
		REQUIRE( concat_bufs( *popped_wg ) ==
				"0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c" );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE( coordinator.is_full() );
		REQUIRE_FALSE( coordinator.closed() );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_close() }, // CLOSE CONNECTION AT RESP #0
			write_group_t{ make_buffers( { "LAST", " ", "PARTS" } ) } ) );

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE( popped_wg );

		REQUIRE( 3UL == popped_wg->items_count() );
		REQUIRE( concat_bufs( *popped_wg ) == "LAST PARTS" );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE( coordinator.closed() );

		CHECK_THROWS( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "LAST", " ", "PARTS" } ) } ) );

		CHECK_THROWS( coordinator.append_response(
			req_id[ 2 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "LAST", " ", "PARTS" } ) } ) );

		CHECK_THROWS(
			popped_wg = coordinator.pop_ready_buffers() );

		REQUIRE( coordinator.closed() );
	}

	SECTION( "second request causes close" )
	{
		response_coordinator_t coordinator{ 3 };
		REQUIRE_FALSE( coordinator.closed() );

		request_id_t req_id[ 3 ];

		CHECK_NOTHROW( req_id[ 0 ] = coordinator.register_new_request() );
		CHECK_NOTHROW( req_id[ 1 ] = coordinator.register_new_request() );
		CHECK_NOTHROW( req_id[ 2 ] = coordinator.register_new_request() );

		// #0: <nothing>
		// #1: <nothing>
		// #2: <nothing>

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{ "0a", "0b", "0c", "0a", "0b", "0c",
					  "0a", "0b", "0c", "0a", "0b", "0c",
					  "0a", "0b", "0c", "0a", "0b", "0c",
					  "0a", "0b", "0c", "0a", "0b", "0c" } ) } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_complete(), // Complete from beggining.
				connection_should_close() },  // CLOSE CONNECTION AT RESP #1
			write_group_t{
				make_buffers(
					{ "1a", "1b", "1c", "1a", "1b", "1c",
					  "1a", "1b", "1c", "1a", "1b", "1c",
					  "1a", "1b", "1c", "1a", "1b", "1c",
					  "1a", "1b", "1c", "LAST", " ", "PARTS" } ) } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 2 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{ "2a", "2b", "2c", "2a", "2b", "2c",
					  "2a", "2b", "2c", "2a", "2b", "2c",
					  "2a", "2b", "2c", "2a", "2b", "2c",
					  "2a", "2b", "2c", "2a", "2b", "2c" } ) } ) );

		// #0: ["0a", "0b", "0c"] * 2*4
		// #1: ["1a", "1b", "1c"] * 2*3 + ["1a", "1b", "1c", "LAST", " ", "PARTS"]
		// #2: ["2a", "2b", "2c"] * 2*4

		auto popped_wg = coordinator.pop_ready_buffers();
		REQUIRE( popped_wg );

		REQUIRE( 24UL == popped_wg->items_count() );
		REQUIRE( concat_bufs( *popped_wg ) ==
				"0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c" );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE( coordinator.is_full() );
		REQUIRE_FALSE( coordinator.closed() );

		// #0: <nothing>
		// #1: ["1a", "1b", "1c"] * 2*3 + ["1a", "1b", "1c", "LAST", " ", "PARTS"]
		// #2: ["2a", "2b", "2c"] * 2*4

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE_FALSE( popped_wg );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "LAST", " ", "PARTS" } ) } ) );

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE( popped_wg );

		REQUIRE( 3UL == popped_wg->items_count() );
		REQUIRE( concat_bufs( *popped_wg ) == "LAST PARTS" );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE_FALSE( coordinator.closed() );

		// #1: ["1a", "1b", "1c"] * 2*3 + ["1a", "1b", "1c", "LAST", " ", "PARTS"]
		// #2: ["2a", "2b", "2c"] * 2*4

		popped_wg = coordinator.pop_ready_buffers();
		REQUIRE( popped_wg );

		REQUIRE( 24UL == popped_wg->items_count() );
		REQUIRE( concat_bufs( *popped_wg ) ==
			"1a1b1c1a1b1c1a1b1c1a1b1c1a1b1c1a1b1c1a1b1c"
			"LAST PARTS" );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE( coordinator.closed() );

		CHECK_THROWS( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{ make_buffers( { "NO", " ", "WAY" } ) } ) );

		CHECK_THROWS( coordinator.append_response(
			req_id[ 2 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			write_group_t{
				make_buffers(
					{ "REQUEST", "IS", "ACTUAL",
					  "BUT", "REQ #1", "SIGNALED", "CONNECTION-CLOSE" } ) } ) );

		CHECK_THROWS(
			popped_wg = coordinator.pop_ready_buffers() );

		REQUIRE( coordinator.closed() );
	}
}

#if 0 /* Needs to be changed */

TEST_CASE( "response_coordinator sendfile" , "[response_coordinator][sendfile][size1]" )
{
	//
	// response_coordinator with 1 item.
	//

	{
		// close after sendfile.
		writable_items_container_t out_bufs;
		response_coordinator_t coordinator{ 1 };

		auto req_id = coordinator.register_new_request();
		REQUIRE( coordinator.is_full() );

		CHECK_NOTHROW( coordinator.append_response(
			req_id,
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_close() },
			make_buffers( { "header1", "header2", "header3" } ) ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id,
			response_output_flags_t{
				response_is_complete(),
				connection_should_close() },
			make_buffers( restinio::sendfile(
							restinio::null_file_descriptor() /* fake not real */,
							restinio::file_meta_t{1024, std::chrono::system_clock::now() } ) ) ) );

		REQUIRE_FALSE( coordinator.closed() );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::trivial_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 3UL == out_bufs.size() );
		REQUIRE( make_string( out_bufs[ 0UL ] ) == "header1" );
		REQUIRE( make_string( out_bufs[ 1UL ] ) == "header2" );
		REQUIRE( make_string( out_bufs[ 2UL ] ) == "header3" );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::file_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 1UL == out_bufs.size() );

		CHECK_NOTHROW( out_bufs[ 0UL ].sendfile_operation() );

		REQUIRE( coordinator.closed() );
	}

	{
		// keep-alive after sendfile.
		writable_items_container_t out_bufs;
		response_coordinator_t coordinator{ 1 };

		auto req_id = coordinator.register_new_request();
		REQUIRE( coordinator.is_full() );

		CHECK_NOTHROW( coordinator.append_response(
			req_id,
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			make_buffers( { "header1", "header2", "header3" } ) ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id,
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			make_buffers( restinio::sendfile(
							restinio::null_file_descriptor() /* fake not real */,
							restinio::file_meta_t{ 1024, std::chrono::system_clock::now() } ) ) ) );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::trivial_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 3UL == out_bufs.size() );
		REQUIRE( make_string( out_bufs[ 0UL ] ) == "header1" );
		REQUIRE( make_string( out_bufs[ 1UL ] ) == "header2" );
		REQUIRE( make_string( out_bufs[ 2UL ] ) == "header3" );

		REQUIRE_FALSE( coordinator.closed() );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::file_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 1UL == out_bufs.size() );

		CHECK_NOTHROW( out_bufs[ 0UL ].sendfile_operation() );

		REQUIRE_FALSE( coordinator.closed() );
	}

	{
		// several sendfiles and close.
		writable_items_container_t out_bufs;
		response_coordinator_t coordinator{ 1 };

		auto req_id = coordinator.register_new_request();
		REQUIRE( coordinator.is_full() );

		CHECK_NOTHROW( coordinator.append_response(
			req_id,
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_close() },
			make_buffers( { "header1", "header2", "header3" } ) ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id,
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_close() },
			make_buffers( restinio::sendfile(
							restinio::null_file_descriptor() /* fake not real */,
							restinio::file_meta_t{ 1024, std::chrono::system_clock::now() } ) ) ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id,
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_close() },
			make_buffers( restinio::sendfile(
							restinio::null_file_descriptor() /* fake not real */,
							restinio::file_meta_t{ 2048, std::chrono::system_clock::now() } ) ) ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id,
			response_output_flags_t{
				response_is_complete(),
				connection_should_close() },
			make_buffers( restinio::sendfile(
								restinio::null_file_descriptor() /* fake not real */,
								restinio::file_meta_t{ 4096, std::chrono::system_clock::now() } ) ) ) );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::trivial_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 3UL == out_bufs.size() );
		REQUIRE( make_string( out_bufs[ 0UL ] ) == "header1" );
		REQUIRE( make_string( out_bufs[ 1UL ] ) == "header2" );
		REQUIRE( make_string( out_bufs[ 2UL ] ) == "header3" );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::file_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 1UL == out_bufs.size() );
		REQUIRE_FALSE( coordinator.closed() );

		CHECK_NOTHROW( out_bufs[ 0UL ].sendfile_operation() );
		REQUIRE( out_bufs[ 0UL ].size() == 1024 );
		REQUIRE_FALSE( coordinator.closed() );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::file_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 1UL == out_bufs.size() );
		REQUIRE_FALSE( coordinator.closed() );

		CHECK_NOTHROW( out_bufs[ 0UL ].sendfile_operation() );
		REQUIRE( out_bufs[ 0UL ].size() == 2048 );
		REQUIRE_FALSE( coordinator.closed() );


		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::file_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 1UL == out_bufs.size() );

		CHECK_NOTHROW( out_bufs[ 0UL ].sendfile_operation() );
		REQUIRE( out_bufs[ 0UL ].size() == 4096 );
		REQUIRE( coordinator.closed() );
	}

	{
		// several sendfiles trivial buf and keep-alive.
		writable_items_container_t out_bufs;
		response_coordinator_t coordinator{ 1 };

		auto req_id = coordinator.register_new_request();
		REQUIRE( coordinator.is_full() );

		CHECK_NOTHROW( coordinator.append_response(
			req_id,
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_close() },
			make_buffers( { "header1", "header2", "header3" } ) ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id,
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_close() },
			make_buffers( restinio::sendfile(
							restinio::null_file_descriptor() /* fake not real */,
							restinio::file_meta_t{ 1024, std::chrono::system_clock::now() } ) ) ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id,
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_close() },
			make_buffers( restinio::sendfile(
							restinio::null_file_descriptor() /* fake not real */,
							restinio::file_meta_t{ 2048, std::chrono::system_clock::now() } ) ) ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id,
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_close() },
			make_buffers( restinio::sendfile(
							restinio::null_file_descriptor() /* fake not real */,
							restinio::file_meta_t{ 4096, std::chrono::system_clock::now() } ) ) ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id,
			response_output_flags_t{
				response_is_complete(),
				connection_should_close() },
			make_buffers( { "END", "OF", "RESPONSE" } ) ) );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::trivial_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 3UL == out_bufs.size() );
		REQUIRE( make_string( out_bufs[ 0UL ] ) == "header1" );
		REQUIRE( make_string( out_bufs[ 1UL ] ) == "header2" );
		REQUIRE( make_string( out_bufs[ 2UL ] ) == "header3" );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::file_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 1UL == out_bufs.size() );
		REQUIRE_FALSE( coordinator.closed() );

		CHECK_NOTHROW( out_bufs[ 0UL ].sendfile_operation() );
		REQUIRE( out_bufs[ 0UL ].size() == 1024 );
		REQUIRE_FALSE( coordinator.closed() );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::file_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 1UL == out_bufs.size() );
		REQUIRE_FALSE( coordinator.closed() );

		CHECK_NOTHROW( out_bufs[ 0UL ].sendfile_operation() );
		REQUIRE( out_bufs[ 0UL ].size() == 2048 );
		REQUIRE_FALSE( coordinator.closed() );


		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::file_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 1UL == out_bufs.size() );

		CHECK_NOTHROW( out_bufs[ 0UL ].sendfile_operation() );
		REQUIRE( out_bufs[ 0UL ].size() == 4096 );
		REQUIRE_FALSE( coordinator.closed() );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::trivial_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 3UL == out_bufs.size() );
		REQUIRE( make_string( out_bufs[ 0UL ] ) == "END" );
		REQUIRE( make_string( out_bufs[ 1UL ] ) == "OF" );
		REQUIRE( make_string( out_bufs[ 2UL ] ) == "RESPONSE" );
		REQUIRE( coordinator.closed() );
	}
}

TEST_CASE( "response_coordinator sendfile 2" , "[response_coordinator][sendfile][sizeN]" )
{
	//
	// response_coordinator with N item.
	//

	{
		// first response goes with connection close flag.
		// 2 sequential sendfile writables for distinct requests.
		writable_items_container_t out_bufs;
		response_coordinator_t coordinator{ 2 };

		request_id_t req_id[ 2 ];
		req_id[0] = coordinator.register_new_request();
		REQUIRE_FALSE( coordinator.is_full() );
		req_id[1] = coordinator.register_new_request();
		REQUIRE( coordinator.is_full() );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[0],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_close() },
			make_buffers( { "header1", "header2", "header3" } ) ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[0],
			response_output_flags_t{
				response_is_complete(),
				connection_should_close() },
			make_buffers( restinio::sendfile(
								restinio::null_file_descriptor() /* fake not real */,
								restinio::file_meta_t{ 1024, std::chrono::system_clock::now() } ) ) ) );

		// Previous response goes with connection close flag
		// So it would not be selected.
		CHECK_NOTHROW( coordinator.append_response(
			req_id[1],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_close() },
			make_buffers( restinio::sendfile(
								restinio::null_file_descriptor() /* fake not real */,
								restinio::file_meta_t{ 2048, std::chrono::system_clock::now() } ) ) ) );

		REQUIRE_FALSE( coordinator.closed() );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::trivial_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 3UL == out_bufs.size() );
		REQUIRE( make_string( out_bufs[ 0UL ] ) == "header1" );
		REQUIRE( make_string( out_bufs[ 1UL ] ) == "header2" );
		REQUIRE( make_string( out_bufs[ 2UL ] ) == "header3" );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::file_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 1UL == out_bufs.size() );

		CHECK_NOTHROW( out_bufs[ 0UL ].sendfile_operation() );

		REQUIRE( coordinator.closed() );

		out_bufs.clear();
		CHECK_THROWS( coordinator.pop_ready_buffers( 10UL, out_bufs ) );
	}

	{
		// 2 sequential sendfile writables for distinct requests.
		writable_items_container_t out_bufs;
		response_coordinator_t coordinator{ 2 };

		request_id_t req_id[ 2 ];
		req_id[0] = coordinator.register_new_request();
		REQUIRE_FALSE( coordinator.is_full() );
		req_id[1] = coordinator.register_new_request();
		REQUIRE( coordinator.is_full() );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[0],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			make_buffers( { "header1", "header2", "header3" } ) ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[0],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			make_buffers( restinio::sendfile(
								restinio::null_file_descriptor() /* fake not real */,
								restinio::file_meta_t{ 1024, std::chrono::system_clock::now() } ) ) ) );

		// Previous response goes with connection close flag
		// So it would not be selected.
		CHECK_NOTHROW( coordinator.append_response(
			req_id[1],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			make_buffers( restinio::sendfile(
								restinio::null_file_descriptor() /* fake not real */,
								restinio::file_meta_t{ 2048, std::chrono::system_clock::now() } ) ) ) );

		REQUIRE_FALSE( coordinator.closed() );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::trivial_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 3UL == out_bufs.size() );
		REQUIRE( make_string( out_bufs[ 0UL ] ) == "header1" );
		REQUIRE( make_string( out_bufs[ 1UL ] ) == "header2" );
		REQUIRE( make_string( out_bufs[ 2UL ] ) == "header3" );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::file_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 1UL == out_bufs.size() );
		CHECK_NOTHROW( out_bufs[ 0UL ].sendfile_operation() );
		REQUIRE( out_bufs[ 0UL ].size() == 1024 );

		REQUIRE_FALSE( coordinator.closed() );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::file_write_operation ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 1UL == out_bufs.size() );
		CHECK_NOTHROW( out_bufs[ 0UL ].sendfile_operation() );
		REQUIRE( out_bufs[ 0UL ].size() == 2048 );

		out_bufs.clear();
		REQUIRE(
			writable_item_type_t::none ==
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
	}
}

#endif
