/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/multipart_body.hpp>

TEST_CASE( "Basics", "[basics]" )
{
	using namespace restinio::multipart_body;

	const restinio::string_view_t boundary{ "--boundary" };

	{
		auto result = split_multipart_body( "", boundary );
		REQUIRE( result.empty() );
	}

	{
		auto result = split_multipart_body(
				"Some text with boundary word inside",
				boundary );
		REQUIRE( result.empty() );
	}

	{
		auto result = split_multipart_body(
				"Some text with --boundary inside",
				boundary );
		REQUIRE( result.empty() );
	}

	{
		auto result = split_multipart_body(
				"--boundary\r\n"
				"The first part"
				"--boundary--",
				boundary );
		REQUIRE( result.empty() );
	}

	{
		auto result = split_multipart_body(
				"--boundary\r\n"
				"The first part\r\n",
				boundary );
		REQUIRE( result.empty() );
	}

	{
		auto result = split_multipart_body(
				"--boundary\r\n"
				"The first part\r\n\r\n"
				"--boundary--",
				boundary );
		REQUIRE( result.empty() );
	}

	{
		auto result = split_multipart_body(
				"--boundary\r\n"
				"The first part\r\n\r\n"
				"--boundary--\r\n",
				boundary );
		REQUIRE( 1u == result.size() );
		REQUIRE( "The first part\r\n" == result[0] );
	}

	{
		auto result = split_multipart_body(
				"--boundary\r\n"
				"The first part\r\n"
				"--boundary--\r\n",
				boundary );
		REQUIRE( 1u == result.size() );
		REQUIRE( "The first part" == result[0] );
	}

	{
		auto result = split_multipart_body(
				"Some preamble\r\n"
				"--boundary\r\n"
				"The first part\r\n"
				"--boundary--\r\n",
				boundary );
		REQUIRE( 1u == result.size() );
		REQUIRE( "The first part" == result[0] );
	}

	{
		auto result = split_multipart_body(
				"Some preamble\r\n"
				"--boundary\r\n"
				"The first part\r\n"
				"--boundary--\r\n"
				"Some epilog\r\n"
				"--boundary\r\n"
				"This should be ignored\r\n",
				boundary );
		REQUIRE( 1u == result.size() );
		REQUIRE( "The first part" == result[0] );
	}

	{
		auto result = split_multipart_body(
				"Some preamble\r\n"
				"--boundary\r\n"
				"The first part\r\n"
				"--boundary\r\n"
				"The second part\r\n"
				"\r\n"
				"With empty string inside\r\n"
				"--boundary--\r\n"
				"Some epilog\r\n"
				"--boundary\r\n"
				"This should be ignored\r\n",
				boundary );
		REQUIRE( 2u == result.size() );
		REQUIRE( "The first part" == result[0] );
		REQUIRE( "The second part\r\n"
				"\r\n"
				"With empty string inside" == result[1] );
	}

	{
		auto result = split_multipart_body(
				"Some preamble\r\n"
				"--boundary\r\n"
				"The first part\r\n"
				"--boundary\r\n"
				"The second part\r\n"
				"\r\n"
				"With empty string inside and wrong --boundary mark\r\n"
				"--boundary--\r\n"
				"Some epilog\r\n"
				"--boundary\r\n"
				"This should be ignored\r\n",
				boundary );

		REQUIRE( result.empty() );
	}
}

TEST_CASE( "try_parse_part", "[try_parse_part]" )
{
	using namespace restinio::multipart_body;

	{
		const auto r = try_parse_part( "" );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse_part( " " );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse_part( " body" );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse_part( "content-type: text/plain" );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse_part(
				"content-type: text/plain\r\n"
				"body." );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse_part(
				"content-type: text/plain;\r\n"
				"  boundary=12345567\r\n"
				"\r\n"
				"body." );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse_part(
				"content-type: text/plain;\r\n"
				" content-disposition: form-data;\r\n"
				"\r\n"
				"body." );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse_part( "\r\n" );

		REQUIRE( r );
		REQUIRE( 0u == r->fields.fields_count() );
		REQUIRE( "" == r->body );
	}

	{
		const auto r = try_parse_part(
				"content-type: text/plain\r\n"
				"\r\n"
				"body." );

		REQUIRE( r );
		REQUIRE( 1u == r->fields.fields_count() );
		REQUIRE( r->fields.has_field( "content-type" ) );
		REQUIRE( "text/plain" == r->fields.value_of( "content-type" ) );
		REQUIRE( "body." == r->body );
	}

	{
		const auto r = try_parse_part(
				"content-type: text/plain\r\n"
				"content-disposition: form-data; name=value\r\n"
				"\r\n"
				"body." );

		REQUIRE( r );
		REQUIRE( 2u == r->fields.fields_count() );

		REQUIRE( r->fields.has_field( "content-type" ) );
		REQUIRE( "text/plain" == r->fields.value_of( "content-type" ) );

		REQUIRE( r->fields.has_field( "content-disposition" ) );
		REQUIRE( "form-data; name=value" ==
				r->fields.value_of( "content-disposition" ) );

		REQUIRE( "body." == r->body );
	}

	{
		const auto r = try_parse_part(
				"content-type: text/plain\r\n"
				"content-disposition: form-data; name=value\r\n"
				"\r\n"
				"\r\n"
				"\r\n"
				"body." );

		REQUIRE( r );
		REQUIRE( 2u == r->fields.fields_count() );

		REQUIRE( r->fields.has_field( "content-type" ) );
		REQUIRE( "text/plain" == r->fields.value_of( "content-type" ) );

		REQUIRE( r->fields.has_field( "content-disposition" ) );
		REQUIRE( "form-data; name=value" ==
				r->fields.value_of( "content-disposition" ) );

		REQUIRE( "\r\n\r\nbody." == r->body );
	}
}

