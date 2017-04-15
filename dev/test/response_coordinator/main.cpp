/*
	restinio
*/

/*!
	Tests for header objects.
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

		REQUIRE( table.get_by_req_id( 42UL )->m_request_id == 42UL );

		REQUIRE( response_parts_attr_t::not_final_parts ==
			table.get_by_req_id( 42UL )->m_response_output_flags.m_response_parts );
		REQUIRE(
			response_connection_attr_t::connection_keepalive ==
			table.get_by_req_id( 42UL )->m_response_output_flags.m_response_connection );

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
	std::vector< std::string > out_bufs;
	auto concat_bufs =
		[ & ](){
			std::string res;
			for( const auto & s : out_bufs )
			{
				res += s;
			}
			return res;
		};

	SECTION( "simple" )
	{
		response_coordinator_t coordinator{ 2 };
		REQUIRE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE_FALSE( coordinator.closed() );

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 0UL == out_bufs.size() );

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
			{ "a", "b", "c" } ) );

		// #0: "a", "b", "c"
		// #1: <nothing>

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 3UL == out_bufs.size() );
		REQUIRE( out_bufs[ 0UL ] == "a" );
		REQUIRE( out_bufs[ 1UL ] == "b" );
		REQUIRE( out_bufs[ 2UL ] == "c" );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			{ "A", "B", "C" } ) );
		// #0: "A", "B", "C"
		// #1: <nothing>

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			{ "X", "Y", "Z" } ) );
		// #0: "A", "B", "C"
		// #1: "X", "Y", "Z"

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 3UL == out_bufs.size() );
		REQUIRE( out_bufs[ 0UL ] == "A" );
		REQUIRE( out_bufs[ 1UL ] == "B" );
		REQUIRE( out_bufs[ 2UL ] == "C" );

		// #0: <nothing>
		// #1: "X", "Y", "Z"

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 10UL, out_bufs ) );
		REQUIRE( 0UL == out_bufs.size() );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "LAST", "PARTS" } ) );
		// #0: <nothing>
		// #1: "X", "Y", "Z", "LAST", "PARTS"

		// Append complete response error:
		CHECK_THROWS( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			{ "!LAST!", "!PARTS!" } ) );


		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "LAST", "PARTS" } ) );

		// Append complete response error:
		CHECK_THROWS( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "!LAST!", "!PARTS!" } ) );

		// #0: "LAST", "PARTS"
		// #1: "X", "Y", "Z", "LAST", "PARTS"

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 5UL, out_bufs ) );
		REQUIRE( 5UL == out_bufs.size() );
		REQUIRE( out_bufs[ 0UL ] == "LAST" );
		REQUIRE( out_bufs[ 1UL ] == "PARTS" );
		REQUIRE( out_bufs[ 2UL ] == "X" );
		REQUIRE( out_bufs[ 3UL ] == "Y" );
		REQUIRE( out_bufs[ 4UL ] == "Z" );

		// #1: "LAST", "PARTS"

		// Response doesn't exist any more error:
		CHECK_THROWS( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "!NO!", "!WAY!" } ) );

		CHECK_NOTHROW( req_id[ 0 ] = coordinator.register_new_request() );
		REQUIRE( req_id[ 0 ] == 2UL );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE( coordinator.is_full() );

		// #1: "LAST", "PARTS"
		// #2: <nothing>

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 5UL, out_bufs ) );
		REQUIRE( 2UL == out_bufs.size() );
		REQUIRE( out_bufs[ 0UL ] == "LAST" );
		REQUIRE( out_bufs[ 1UL ] == "PARTS" );

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
			{ "0a", "0b", "0c", "0a", "0b", "0c",
			  "0a", "0b", "0c", "0a", "0b", "0c",
			  "0a", "0b", "0c", "0a", "0b", "0c",
			  "0a", "0b", "0c", "0a", "0b", "0c" } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			{ "1a", "1b", "1c", "1a", "1b", "1c",
			  "1a", "1b", "1c", "1a", "1b", "1c",
			  "1a", "1b", "1c", "1a", "1b", "1c",
			  "1a", "1b", "1c", "1a", "1b", "1c" } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 2 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			{ "2a", "2b", "2c", "2a", "2b", "2c",
			  "2a", "2b", "2c", "2a", "2b", "2c",
			  "2a", "2b", "2c", "2a", "2b", "2c",
			  "2a", "2b", "2c", "2a", "2b", "2c" } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 3 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			{ "3a", "3b", "3c", "3a", "3b", "3c",
			  "3a", "3b", "3c", "3a", "3b", "3c",
			  "3a", "3b", "3c", "3a", "3b", "3c",
			  "3a", "3b", "3c", "3a", "3b", "3c" } ) );

		// #0: ["0a", "0b", "0c"] * 2*4
		// #1: ["0a", "0b", "0c"] * 2*4
		// #2: ["2a", "2b", "2c"] * 2*4
		// #3: ["3a", "3b", "3c"] * 2*4

		// Only bufs for #0 response mast be presented:

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 64UL, out_bufs ) );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE( coordinator.is_full() );
		REQUIRE( 24UL == out_bufs.size() );
		REQUIRE( concat_bufs() == "0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c" );

		// #0: <nothing>
		// #1: ["0a", "0b", "0c"] * 2*4
		// #2: ["2a", "2b", "2c"] * 2*4
		// #3: ["3a", "3b", "3c"] * 2*4

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 64UL, out_bufs ) );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE( coordinator.is_full() );
		REQUIRE( 0UL == out_bufs.size() );
		REQUIRE( concat_bufs() == "" );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "LAST", "PARTS",
			  "LAST", "PARTS",
			  "LAST", "PARTS",
			  "LAST", "PARTS"  } ) );

		// #0: ["LAST", "PARTS"] * 4
		// #1: ["0a", "0b", "0c"] * 2*4
		// #2: ["2a", "2b", "2c"] * 2*4
		// #3: ["3a", "3b", "3c"] * 2*4

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "LAST", "PARTS",
			  "LAST", "PARTS",
			  "LAST", "PARTS",
			  "LAST", "PARTS"  } ) );

		// #0: ["LAST", "PARTS"] * 4
		// #1: ["0a", "0b", "0c"] * 2*4 + ["LAST", "PARTS"] * 4
		// #2: ["2a", "2b", "2c"] * 2*4
		// #3: ["3a", "3b", "3c"] * 2*4

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 64UL, out_bufs ) );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE( 64UL == out_bufs.size() );
		REQUIRE( concat_bufs() ==
			"LASTPARTSLASTPARTSLASTPARTSLASTPARTS"
			"1a1b1c1a1b1c1a1b1c1a1b1c1a1b1c1a1b1c1a1b1c1a1b1c"
			"LASTPARTSLASTPARTSLASTPARTSLASTPARTS"
			"2a2b2c2a2b2c2a2b2c2a2b2c2a2b2c2a2b2c2a2b2c2a2b2c" );

		// #2: <nothing>
		// #3: ["3a", "3b", "3c"] * 2*4

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 64UL, out_bufs ) );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE( 0UL == out_bufs.size() );
		REQUIRE( concat_bufs() == "" );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 2 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "LAST", "PARTS",
			  "LAST", "PARTS",
			  "LAST", "PARTS",
			  "LAST", "PARTS"  } ) );

		// #2: ["LAST", "PARTS"] * 4
		// #3: ["3a", "3b", "3c"] * 2*4

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 4UL, out_bufs ) );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE( 4UL == out_bufs.size() );
		REQUIRE( concat_bufs() == "LASTPARTSLASTPARTS" );

		// #2: ["LAST", "PARTS"] * 2
		// #3: ["3a", "3b", "3c"] * 2*4

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 16UL, out_bufs ) );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE( 16UL == out_bufs.size() );
		REQUIRE( concat_bufs() ==
			"LASTPARTSLASTPARTS"
			"3a3b3c3a3b3c3a3b3c3a3b3c" );

		// #3: ["3a", "3b", "3c"] * 1*4

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

		// #3: ["3a", "3b", "3c"] * 1*4
		// #4: <nothing>
		// #5: <nothing>
		// #6: <nothing>


		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "4a", "4b", "4c", "4a", "4b", "4c",
			  "4a", "4b", "4c", "4a", "4b", "4c",
			  "4a", "4b", "4c", "4a", "4b", "4c",
			  "4a", "4b", "4c", "4a", "4b", "4c" } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			{ "5a", "5b", "5c", "5a", "5b", "5c",
			  "5a", "5b", "5c", "5a", "5b", "5c",
			  "5a", "5b", "5c", "5a", "5b", "5c",
			  "5a", "5b", "5c", "5a", "5b", "5c" } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 2 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "6a", "6b", "6c", "6a", "6b", "6c",
			  "6a", "6b", "6c", "6a", "6b", "6c",
			  "6a", "6b", "6c", "6a", "6b", "6c",
			  "6a", "6b", "6c", "6a", "6b", "6c" } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 3 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "LAST", "PARTS",
			  "LAST", "PARTS",
			  "LAST", "PARTS",
			  "LAST", "PARTS" } ) );

		// #3: ["3a", "3b", "3c"] 1 *4 + [ "LAST", "PARTS" ] * 4
		// #4: ["4a", "4b", "4c"] * 2*4
		// #5: ["5a", "5b", "5c"] * 2*4
		// #6: ["6a", "6b", "6c"] * 2*4

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 64UL, out_bufs ) );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE( 64UL == out_bufs.size() );
		REQUIRE( concat_bufs() ==
			// 20 from #3
			"3a3b3c3a3b3c3a3b3c3a3b3c"
			"LASTPARTSLASTPARTSLASTPARTSLASTPARTS"
			// 24 from #4
			"4a4b4c4a4b4c4a4b4c4a4b4c4a4b4c4a4b4c4a4b4c4a4b4c"
			// 20 from #5
			"5a5b5c5a5b5c5a5b5c5a5b5c"
			"5a5b5c5a5b5c5a5b" );

		// #5: ["5c", "5a", "5b", "5c"]
		// #6: ["6a", "6b", "6c"] * 2*4


		CHECK_THROWS( coordinator.append_response(
			req_id[ 3 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "NO", "WAY" } ) );

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 64UL, out_bufs ) );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE( 4UL == out_bufs.size() );
		REQUIRE( concat_bufs() ==
			// 4 from #5 (and it is not complete)
			"5c5a5b5c" );

		// #5: <nothing>
		// #6: ["6a", "6b", "6c"] * 2*4

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "LAST", "PARTS",
			  "LAST", "PARTS",
			  "LAST", "PARTS",
			  "LAST", "PARTS" } ) );

		// #5: [ "LAST", "PARTS" ] * 4
		// #6: ["6a", "6b", "6c"] * 2*4

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 64UL, out_bufs ) );
		REQUIRE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE( 32UL == out_bufs.size() );
		REQUIRE( concat_bufs() ==
			"LASTPARTSLASTPARTSLASTPARTSLASTPARTS"
			"6a6b6c6a6b6c6a6b6c6a6b6c6a6b6c6a6b6c6a6b6c6a6b6c" );

		// EMPTY

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 64UL, out_bufs ) );
		REQUIRE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE( 0UL == out_bufs.size() );

		CHECK_THROWS( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "NO", "WAY" } ) );

		CHECK_THROWS( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "NO", "WAY" } ) );

		CHECK_THROWS( coordinator.append_response(
			req_id[ 2 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "NO", "WAY" } ) );

		CHECK_THROWS( coordinator.append_response(
			req_id[ 3 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "NO", "WAY" } ) );

		REQUIRE_FALSE( coordinator.closed() );
	}
}

TEST_CASE( "response_coordinator_with_close" , "[response_coordinator][connection_close]" )
{
	std::vector< std::string > out_bufs;
	auto concat_bufs =
		[ & ](){
			std::string res;
			for( const auto & s : out_bufs )
			{
				res += s;
			}
			return res;
		};

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
			{ "0a", "0b", "0c", "0a", "0b", "0c",
			  "0a", "0b", "0c", "0a", "0b", "0c",
			  "0a", "0b", "0c", "0a", "0b", "0c",
			  "0a", "0b", "0c", "0a", "0b", "0c" } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			{ "1a", "1b", "1c", "1a", "1b", "1c",
			  "1a", "1b", "1c", "1a", "1b", "1c",
			  "1a", "1b", "1c", "1a", "1b", "1c",
			  "1a", "1b", "1c", "1a", "1b", "1c" } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 2 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			{ "2a", "2b", "2c", "2a", "2b", "2c",
			  "2a", "2b", "2c", "2a", "2b", "2c",
			  "2a", "2b", "2c", "2a", "2b", "2c",
			  "2a", "2b", "2c", "2a", "2b", "2c" } ) );


		// #0: ["0a", "0b", "0c"] * 2*4
		// #1: ["0a", "0b", "0c"] * 2*4
		// #2: ["2a", "2b", "2c"] * 2*4
		// #3: ["3a", "3b", "3c"] * 2*4

		// Only bufs for #0 response mast be presented:

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 64UL, out_bufs ) );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE( coordinator.is_full() );
		REQUIRE_FALSE( coordinator.closed() );

		REQUIRE( 24UL == out_bufs.size() );
		REQUIRE( concat_bufs() == "0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c" );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_close() }, // CLOSE CONNECTION AT RESP #0
			{ "LAST", " ", "PARTS" } ) );

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 64UL, out_bufs ) );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE( coordinator.closed() );

		REQUIRE( 3UL == out_bufs.size() );
		REQUIRE( concat_bufs() == "LAST PARTS" );

		CHECK_THROWS( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "LAST", " ", "PARTS" } ) );

		CHECK_THROWS( coordinator.append_response(
			req_id[ 2 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "LAST", " ", "PARTS" } ) );


		out_bufs.clear();
		CHECK_THROWS(
			coordinator.pop_ready_buffers( 64UL, out_bufs ) );

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
			{ "0a", "0b", "0c", "0a", "0b", "0c",
			  "0a", "0b", "0c", "0a", "0b", "0c",
			  "0a", "0b", "0c", "0a", "0b", "0c",
			  "0a", "0b", "0c", "0a", "0b", "0c" } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 1 ],
			response_output_flags_t{
				response_is_complete(), // Complete from beggining.
				connection_should_close() },  // CLOSE CONNECTION AT RESP #1
			{ "1a", "1b", "1c", "1a", "1b", "1c",
			  "1a", "1b", "1c", "1a", "1b", "1c",
			  "1a", "1b", "1c", "1a", "1b", "1c",
			  "1a", "1b", "1c", "LAST", " ", "PARTS" } ) );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 2 ],
			response_output_flags_t{
				response_is_not_complete(),
				connection_should_keep_alive() },
			{ "2a", "2b", "2c", "2a", "2b", "2c",
			  "2a", "2b", "2c", "2a", "2b", "2c",
			  "2a", "2b", "2c", "2a", "2b", "2c",
			  "2a", "2b", "2c", "2a", "2b", "2c" } ) );


		// #0: ["0a", "0b", "0c"] * 2*4
		// #1: ["0a", "0b", "0c"] * 2*4
		// #2: ["2a", "2b", "2c"] * 2*4
		// #3: ["3a", "3b", "3c"] * 2*4

		// Only bufs for #0 response mast be presented:

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 64UL, out_bufs ) );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE( coordinator.is_full() );
		REQUIRE_FALSE( coordinator.closed() );

		REQUIRE( 24UL == out_bufs.size() );
		REQUIRE( concat_bufs() == "0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c0a0b0c" );

		CHECK_NOTHROW( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "LAST", " ", "PARTS" } ) );

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 24UL, out_bufs ) );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE_FALSE( coordinator.closed() );

		REQUIRE( 24UL == out_bufs.size() );
		REQUIRE( concat_bufs() ==
			"LAST PARTS"
			"1a1b1c1a1b1c1a1b1c1a1b1c1a1b1c1a1b1c1a1b1c" );

		out_bufs.clear();
		CHECK_NOTHROW(
			coordinator.pop_ready_buffers( 24UL, out_bufs ) );
		REQUIRE_FALSE( coordinator.empty() );
		REQUIRE_FALSE( coordinator.is_full() );
		REQUIRE( coordinator.closed() );


		REQUIRE( 3UL == out_bufs.size() );
		REQUIRE( concat_bufs() == "LAST PARTS" );

		CHECK_THROWS( coordinator.append_response(
			req_id[ 0 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "NO", " ", "WAY" } ) );

		CHECK_THROWS( coordinator.append_response(
			req_id[ 2 ],
			response_output_flags_t{
				response_is_complete(),
				connection_should_keep_alive() },
			{ "REQUEST", "IS", "ACTUAL",
			  "BUT", "REQ #1", "SIGNALED", "CONNECTION-CLOSE" } ) );

		out_bufs.clear();
		CHECK_THROWS(
			coordinator.pop_ready_buffers( 64UL, out_bufs ) );

		REQUIRE( coordinator.closed() );
	}
}
