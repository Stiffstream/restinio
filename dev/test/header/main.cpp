/*
	restinio
*/

/*!
	Tests for header objects.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <iterator>

#include <restinio/all.hpp>

using namespace restinio;

TEST_CASE( "Working with fields" , "[header][fields]" )
{
	http_header_fields_t fields;

	REQUIRE( 0 == fields.fields_count() ); // No fields yet.

	REQUIRE(
		fields.get_field( "Content-Type", "default-value" )
			== "default-value" );

	REQUIRE(
		fields.get_field( "CONTENT-Type", "default-value-2" )
			== "default-value-2" );

	REQUIRE(
		fields.get_field( "CONTENT-TYPE", "default-value-3" )
			== "default-value-3" );


	fields.set_field( "Content-Type", "text/plain" );
	REQUIRE( 1 == fields.fields_count() );

	REQUIRE( fields.get_field( "Content-Type" ) == "text/plain" );
	REQUIRE( fields.get_field( "CONTENT-TYPE" ) == "text/plain" );
	REQUIRE( fields.get_field( "content-type" ) == "text/plain" );

	REQUIRE(
		fields.get_field( "Content-Type", "Default-Value" ) == "text/plain" );
	REQUIRE(
		fields.get_field( "CONTENT-TYPE", "DEFAULT-VALUE" ) == "text/plain" );
	REQUIRE(
		fields.get_field( "content-type", "default-value" ) == "text/plain" );

	REQUIRE(
		fields.get_field( "Content-Type-XXX", "default-value" )
			== "default-value" );

	fields.append_last_field( "; charset=utf-8" );

	REQUIRE(
		fields.get_field( "Content-Type" ) == "text/plain; charset=utf-8" );
	REQUIRE(
		fields.get_field( "Content-Type", "Default-Value" )
			== "text/plain; charset=utf-8" );

	fields.append_field( "Server", "Unit Test" );
	REQUIRE( 2 == fields.fields_count() );

	REQUIRE( fields.get_field( "server" ) == "Unit Test" );
	REQUIRE( fields.get_field( "SERVER", "EMPTY" ) == "Unit Test" );

	fields.append_field( "sERVER", "; Fields Test" );
	REQUIRE( fields.get_field( "sERVEr" ) == "Unit Test; Fields Test" );
	REQUIRE( fields.get_field( "SeRveR", "EMPTY" ) == "Unit Test; Fields Test" );

	{
		const auto & f = *( fields.begin() );
		REQUIRE( f.m_name == "Content-Type" );
		REQUIRE( f.m_value == "text/plain; charset=utf-8" );
	}
	{
		const auto & f = *( std::next( fields.begin() ) );
		REQUIRE( f.m_name == "Server" );
		REQUIRE( f.m_value == "Unit Test; Fields Test" );
	}

	// Fields that don't exist
	REQUIRE_FALSE( fields.has_field( "Kontent-typo" ) );
	REQUIRE_FALSE( fields.has_field( "Zerver" ) );

	fields.remove_field( "Kontent-typo" );
	fields.remove_field( "Zerver" );
	REQUIRE( 2 == fields.fields_count() );

	fields.remove_field( "Content-TYPE" );
	REQUIRE( 1 == fields.fields_count() );
	fields.remove_field( "ServeR" );
	REQUIRE( 0 == fields.fields_count() );
}

TEST_CASE( "Working with common header" , "[header][common]" )
{
	SECTION( "http version" )
	{
		http_header_common_t h;
		REQUIRE( 1 == h.http_major() );
		REQUIRE( 1 == h.http_minor() );
	}

	SECTION( "content length" )
	{
		http_header_common_t h;
		REQUIRE( 0 == h.content_length() );

		h.content_length( 128 );
		REQUIRE( 128 == h.content_length() );
	}

	SECTION( "keep alive" )
	{
		http_header_common_t h;
		REQUIRE_FALSE( h.should_keep_alive() );

		h.should_keep_alive( true );
		REQUIRE( h.should_keep_alive() );
	}
}

TEST_CASE( "working with request header" , "[header][request]" )
{
	SECTION( "method" )
	{
		http_request_header_t h;
		REQUIRE( http_method_get() == h.method() );

		h.method( http_method_post() );
		REQUIRE( http_method_post() == h.method() );
	}

	SECTION( "request target" )
	{
		http_request_header_t h;
		REQUIRE( h.request_target() == "" );

		h.request_target( "/" );
		REQUIRE( h.request_target() == "/" );

		h.append_request_target( "197", 3 );
		REQUIRE( h.request_target() == "/197" );
	}
}

TEST_CASE( "working with response header" , "[header][response]" )
{
	SECTION( "status code" )
	{
		http_response_header_t h;
		REQUIRE( 200 == h.status_code() );

		h.status_code( 404 );
		REQUIRE( 404 == h.status_code() );
	}

	SECTION( "request target" )
	{
		http_response_header_t h;
		REQUIRE( h.reason_phrase() == "" );

		h.reason_phrase( "OK" );
		REQUIRE( h.reason_phrase() == "OK" );

		h.reason_phrase( "Not Found" );
		REQUIRE( h.reason_phrase() == "Not Found" );
	}
}
