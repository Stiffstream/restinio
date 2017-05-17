/*
	restinio
*/

/*!
	Tests for header objects.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <iterator>

#include <restinio/router/first_match.hpp>

using namespace restinio;
using namespace restinio::router;

request_handle_t
create_fake_request( std::string target )
{
	return
		std::make_shared< request_t >(
			0,
			http_request_header_t{ http_method_get(), std::move( target ) },
			"",
			connection_handle_t{} );
}

TEST_CASE( "Exact match" , "[first_match][exact]" )
{
	first_match_router_t< exact_target_matcher_t > router;

	int last_handler_called = -1;

	auto extract_last_handler_called = [&]{
		int result = last_handler_called;
		last_handler_called = -1;
		return result;
	};

	router.add_handler(
		{ http_method_get(), "/" },
		[&]( auto ){ last_handler_called = 0; return request_accepted(); } );

	router.add_handler(
		{ http_method_get(), "/abcd" },
		[&]( auto ){ last_handler_called = 1; return request_accepted(); } );

	router.add_handler(
		{ http_method_get(), "/abc" },
		[&]( auto ){ last_handler_called = 2; return request_accepted(); } );

	router.add_handler(
		{ http_method_get(), "/ab" },
		[&]( auto ){ last_handler_called = 3; return request_accepted(); } );

	router.add_handler(
		{ http_method_get(), "/a" },
		[&]( auto ){ last_handler_called = 4; return request_accepted(); } );

	router.add_handler(
		{ http_method_get(), "/a" },
		[&]( auto ){
			REQUIRE( false ); // Duplicate: must never be called!
			return request_accepted();
		} );

	router.add_handler(
		{ http_method_get(), "/a/b/c/d/1/2/3/4/5/6/7/8/9/0" },
		[&]( auto ){ last_handler_called = 5; return request_accepted(); } );


	REQUIRE( request_rejected() == router( create_fake_request( "/xxx" ) ) );
	REQUIRE( -1 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/" ) ) );
	REQUIRE( 0 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/abcd" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/abc" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/ab" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/a" ) ) );
	REQUIRE( 4 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/a/b/c/d/1/2/3/4/5/6/7/8/9/0" ) ) );
	REQUIRE( 5 == extract_last_handler_called() );

	REQUIRE( request_rejected() == router( create_fake_request( "/a/b/c/d/1/2/3/4/5/6/7/8/9/0/?" ) ) );
	REQUIRE( -1 == extract_last_handler_called() );
}

TEST_CASE( "Beginning match" , "[first_match][beginning]" )
{
	first_match_router_t< begins_with_target_matcher_t > router;

	int last_handler_called = -1;

	auto extract_last_handler_called = [&]{
		int result = last_handler_called;
		last_handler_called = -1;
		return result;
	};

	router.add_handler(
		{ http_method_get(), "/alpha/" },
		[&]( auto ){ last_handler_called = 1; return request_accepted(); } );

	router.add_handler(
		{ http_method_get(), "/alpha/" },
		[&]( auto ){
			REQUIRE( false ); // Duplicate: must never be called!
			return request_accepted(); } );

	router.add_handler(
		{ http_method_get(), "/beta/" },
		[&]( auto ){ last_handler_called = 2; return request_accepted(); } );

	router.add_handler(
		{ http_method_get(), "/gamma/" },
		[&]( auto ){ last_handler_called = 3; return request_accepted(); } );

	// Matches all, so appended last
	router.add_handler(
		{ http_method_get(), "/" },
		[&]( auto ){ last_handler_called = 1000; return request_accepted(); } );

	router.add_handler(
		{ http_method_get(), "" },
		[&]( auto ){
			REQUIRE( false ); // Must never be called!
			return request_accepted(); } );

	REQUIRE( request_accepted() == router( create_fake_request( "/" ) ) );
	REQUIRE( 1000 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/a" ) ) );
	REQUIRE( 1000 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/b" ) ) );
	REQUIRE( 1000 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/g" ) ) );
	REQUIRE( 1000 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/omega/123" ) ) );
	REQUIRE( 1000 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/omega/aplha" ) ) );
	REQUIRE( 1000 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/alpha" ) ) );
	REQUIRE( 1000 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/beta" ) ) );
	REQUIRE( 1000 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/gamma" ) ) );
	REQUIRE( 1000 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/alpha/" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/beta/" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/gamma/" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/alpha/1" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/alpha/123" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/alpha/abc" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/alpha/def" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/alpha/foo/bar" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/alpha/bar/foo" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/beta/1" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/beta/123" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/beta/abc" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/beta/def" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/beta/foo/bar" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/beta/bar/foo" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/gamma/1" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/gamma/123" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/gamma/abc" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/gamma/def" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/gamma/foo/bar" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/gamma/bar/foo" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
}


TEST_CASE( "Regex match" , "[first_match][regex]" )
{
	first_match_router_t< regex_target_matcher_t > router;

	int last_handler_called = -1;

	auto extract_last_handler_called = [&]{
		int result = last_handler_called;
		last_handler_called = -1;
		return result;
	};

	router.add_handler(
		{ http_method_get(), std::regex( R"(\/(\d)+)") },
		[&]( auto ){ last_handler_called = 1; return request_accepted(); } );

	REQUIRE( request_accepted() == router( create_fake_request( "/0" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/1" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/12" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/0123456789" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/0123456789" ) ) );
	REQUIRE( 1 == extract_last_handler_called() );

	REQUIRE( request_rejected() == router( create_fake_request( "/0123456789/" ) ) );
	REQUIRE( -1 == extract_last_handler_called() );
	REQUIRE( request_rejected() == router( create_fake_request( "/012345678a" ) ) );
	REQUIRE( -1 == extract_last_handler_called() );

	router.add_handler(
		{ http_method_get(), std::regex( R"(\/(\d)+.)") },
		[&]( auto ){ last_handler_called = 2; return request_accepted(); } );

	REQUIRE( request_accepted() == router( create_fake_request( "/0123456789/" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/012345678a" ) ) );
	REQUIRE( 2 == extract_last_handler_called() );

	router.add_handler(
		{ http_method_get(), std::regex( R"(X(\d)+X)"), regex_target_matcher_t::search_algo },
		[&]( auto ){ last_handler_called = 3; return request_accepted(); } );

	REQUIRE( request_accepted() == router( create_fake_request( "/X0123456789X/" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/b012X34X5678a" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/a0X1X2" ) ) );
	REQUIRE( 3 == extract_last_handler_called() );
	REQUIRE( request_rejected() == router( create_fake_request( "/a/ss/XX123/qwe" ) ) );
	REQUIRE( -1 == extract_last_handler_called() );

	router.add_handler(
		{ http_method_get(), std::regex( R"(\d{4}\/)"), regex_target_matcher_t::search_algo },
		[&]( auto ){ last_handler_called = 4; return request_accepted(); } );

	REQUIRE( request_accepted() == router( create_fake_request( "/a0123456789/" ) ) );
	REQUIRE( 4 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/b012345678/a" ) ) );
	REQUIRE( 4 == extract_last_handler_called() );

	REQUIRE( request_accepted() == router( create_fake_request( "/0a012/8877/" ) ) );
	REQUIRE( 4 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/a/ss/123/qwe0123456/" ) ) );
	REQUIRE( 4 == extract_last_handler_called() );

	router.add_handler(
		{ http_method_get(), std::regex( R"(\d{4}-\d{2}-\d{2})"), regex_target_matcher_t::search_algo },
		[&]( auto ){ last_handler_called = 5; return request_accepted(); } );

	REQUIRE( request_accepted() == router( create_fake_request( "/2017-05-17" ) ) );
	REQUIRE( 5 == extract_last_handler_called() );
	REQUIRE( request_accepted() == router( create_fake_request( "/asd/zxc/2017-05-17/qwe" ) ) );
	REQUIRE( 5 == extract_last_handler_called() );
}
