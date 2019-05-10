/*
	restinio
*/

/*!
	Tests for header objects.
*/

#include <catch2/catch.hpp>

#include <iterator>
#include <set>

#include <restinio/all.hpp>

using namespace restinio;

TEST_CASE( "Working with fields (by name)" , "[header][fields][by_name]" )
{
	http_header_fields_t fields;

	REQUIRE( 0 == fields.fields_count() ); // No fields yet.

	REQUIRE(
		fields.get_field_or( "Content-Type", "default-value" )
			== "default-value" );

	REQUIRE(
		fields.get_field_or( "CONTENT-Type", "default-value-2" )
			== "default-value-2" );

	REQUIRE(
		fields.get_field_or( "CONTENT-TYPE", "default-value-3" )
			== "default-value-3" );

	fields.set_field( "Content-Type", "text/plain" );
	REQUIRE( 1 == fields.fields_count() );

	REQUIRE( fields.get_field( "Content-Type" ) == "text/plain" );
	REQUIRE( fields.value_of( "Content-Type" ) == "text/plain" );
	REQUIRE( fields.get_field( "CONTENT-TYPE" ) == "text/plain" );
	REQUIRE( fields.value_of( "CONTENT-TYPE" ) == "text/plain" );
	REQUIRE( fields.get_field_or( "content-type", "WRONG1" ) == "text/plain" );
	// By id must also be available:
	REQUIRE( fields.get_field( http_field::content_type ) == "text/plain" );
	REQUIRE( fields.value_of( http_field::content_type ) == "text/plain" );
	REQUIRE( fields.get_field_or( http_field::content_type, "WRONG2" ) == "text/plain" );

	REQUIRE(
		fields.get_field_or( "Content-Type", "Default-Value" ) == "text/plain" );
	REQUIRE(
		fields.get_field_or( "CONTENT-TYPE", "DEFAULT-VALUE" ) == "text/plain" );
	REQUIRE(
		fields.get_field_or( "content-type", "default-value" ) == "text/plain" );

	REQUIRE(
		fields.get_field_or( "Content-Type-XXX", "default-value" )
			== "default-value" );

	impl::append_last_field_accessor( fields, "; charset=utf-8" );

	REQUIRE(
		fields.get_field( "Content-Type" ) == "text/plain; charset=utf-8" );
	REQUIRE(
		fields.value_of( "Content-Type" ) == "text/plain; charset=utf-8" );
	REQUIRE(
		fields.get_field_or( "Content-Type", "Default-Value" )
			== "text/plain; charset=utf-8" );

	fields.append_field( "Server", "Unit Test" );
	REQUIRE( 2 == fields.fields_count() );

	REQUIRE( fields.get_field( "server" ) == "Unit Test" );
	REQUIRE( fields.value_of( "server" ) == "Unit Test" );
	REQUIRE( fields.get_field_or( "SERVER", "EMPTY" ) == "Unit Test" );

	fields.append_field( "sERVER", "; Fields Test" );
	REQUIRE( fields.get_field( "sERVEr" ) == "Unit Test; Fields Test" );
	REQUIRE( fields.value_of( "sERVEr" ) == "Unit Test; Fields Test" );
	REQUIRE( fields.get_field_or( "SeRveR", "EMPTY" ) == "Unit Test; Fields Test" );

	{
		int counter = 0;
		fields.for_each_field( [&counter](const auto & f) {
			if( 0 == counter ) {
				REQUIRE( f.name() == "Content-Type" );
				REQUIRE( f.value() == "text/plain; charset=utf-8" );
			}
			else if( 1 == counter ) {
				REQUIRE( f.name() == "Server" );
				REQUIRE( f.value() == "Unit Test; Fields Test" );
			}
			// Just ignore all other values.

			++counter;
		} );
	}

	// Fields that don't exist
	REQUIRE_FALSE( fields.has_field( "Kontent-typo" ) );
	REQUIRE_FALSE( fields.has_field( "Zerver" ) );

	REQUIRE_THROWS( fields.get_field( "Kontent-typo" ) );
	REQUIRE_THROWS( fields.get_field( "Zerver" ) );

	fields.remove_field( "Kontent-typo" );
	fields.remove_field( "Zerver" );
	REQUIRE( 2 == fields.fields_count() );

	fields.remove_field( "Content-TYPE" );
	REQUIRE( 1 == fields.fields_count() );
	fields.remove_field( "ServeR" );
	REQUIRE( 0 == fields.fields_count() );
}

TEST_CASE( "get_field_or(name, value) overloads" ,
		"[header][get_field_or][by_name][overloads]" )
{
	http_header_fields_t fields;

	REQUIRE(
		fields.get_field_or( "Content-Type", "default-value" )
			== "default-value" );

	REQUIRE(
		fields.get_field_or( "Content-Type",
			restinio::string_view_t{ "default-value" } ) == "default-value" );

	std::string dv1{ "default-value" };
	const std::string dv2{ "default-value" };

	REQUIRE(
		fields.get_field_or( "Content-Type", dv1 ) == "default-value" );
	REQUIRE(
		fields.get_field_or( "Content-Type", dv2 ) == "default-value" );

	REQUIRE(
		fields.get_field_or( "Content-Type",
			std::string{ "default-value" } ) == "default-value" );

	{
		std::string long_value{
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
		};
		const std::string long_value2{ long_value };

		REQUIRE( long_value == long_value2 );
		REQUIRE( long_value.data() != long_value2.data() );

		const char * long_value_ptr{ long_value.data() };

		auto v = fields.get_field_or( "Content-Type", std::move(long_value) );
		REQUIRE( v == long_value2 );
		REQUIRE( v.data() == long_value_ptr );
	}
}

TEST_CASE( "Working with optional fields (by name)",
		"[header][opt_fields][by_name]" )
{
	http_header_fields_t fields;

	REQUIRE( 0 == fields.fields_count() ); // No fields yet.

	{
		const auto f = fields.try_get_field( "Content-Type" );
		REQUIRE( !f );
	}
	{
		const auto f = fields.opt_value_of( "Content-Type" );
		REQUIRE( !f );
	}

	fields.set_field( "Content-Type", "text/plain" );
	REQUIRE( 1 == fields.fields_count() );

	{
		const auto f = fields.try_get_field( "Content-Type" );
		REQUIRE( f );
		REQUIRE( *f == "text/plain" );
	}
	{
		const auto f = fields.opt_value_of( "Content-Type" );
		REQUIRE( f );
		REQUIRE( *f == "text/plain" );
	}
	{
		const auto f = fields.try_get_field( "CONTENT-Type" );
		REQUIRE( f );
		REQUIRE( *f == "text/plain" );
	}
	{
		const auto f = fields.opt_value_of( "CONTENT-Type" );
		REQUIRE( f );
		REQUIRE( *f == "text/plain" );
	}
}

TEST_CASE( "Working with fields (by id)" , "[header][fields][by_id]" )
{
	http_header_fields_t fields;

	REQUIRE( 0 == fields.fields_count() ); // No fields yet.

	REQUIRE(
		fields.get_field_or( http_field::content_type, "default-value" )
			== "default-value" );

	REQUIRE(
		fields.get_field_or( http_field::content_type, "default-value-2" )
			== "default-value-2" );

	fields.set_field( http_field::content_type, "text/plain" );
	REQUIRE( 1 == fields.fields_count() );

	REQUIRE( fields.get_field( http_field::content_type ) == "text/plain" );
	REQUIRE( fields.value_of( http_field::content_type ) == "text/plain" );
	REQUIRE( fields.get_field_or( http_field::content_type, "WRONG1" ) == "text/plain" );
	// By name must be also availabl.
	REQUIRE( fields.get_field( "Content-Type" ) == "text/plain" );
	REQUIRE( fields.get_field_or( "CONTENT-TYPE", "WRONG2" ) == "text/plain" );
	REQUIRE( fields.get_field( "content-type" ) == "text/plain" );

	impl::append_last_field_accessor( fields, "; charset=utf-8" );

	REQUIRE(
		fields.get_field( http_field::content_type ) == "text/plain; charset=utf-8" );
	REQUIRE(
		fields.value_of( http_field::content_type ) == "text/plain; charset=utf-8" );
	REQUIRE(
		fields.get_field_or( http_field::content_type, "Default-Value" )
			== "text/plain; charset=utf-8" );

	fields.append_field( http_field::server, "Unit Test" );
	REQUIRE( 2 == fields.fields_count() );

	REQUIRE( fields.get_field( http_field::server ) == "Unit Test" );
	REQUIRE( fields.value_of( http_field::server ) == "Unit Test" );
	REQUIRE( fields.get_field_or( http_field::server, "EMPTY" ) == "Unit Test" );

	fields.append_field( http_field::server, "; Fields Test" );
	REQUIRE( fields.get_field( http_field::server ) == "Unit Test; Fields Test" );
	REQUIRE( fields.value_of( http_field::server ) == "Unit Test; Fields Test" );
	REQUIRE( fields.get_field_or( http_field::server, "EMPTY" ) == "Unit Test; Fields Test" );
	REQUIRE( fields.get_field( "sERVEr" ) == "Unit Test; Fields Test" );
	REQUIRE( fields.get_field_or( "SeRveR", "EMPTY" ) == "Unit Test; Fields Test" );

	// Must add nothing.
	fields.set_field( http_field::field_unspecified, "NOWAY" );
	REQUIRE( 2 == fields.fields_count() );
	fields.append_field( http_field::field_unspecified, "STILLNOWAY" );
	REQUIRE( 2 == fields.fields_count() );

	// Add unspecified field.
	fields.append_field( "Non-Specified-Field-Unit-Test", "UNSPECIFIED" );
	REQUIRE( 3 == fields.fields_count() );
	REQUIRE( fields.get_field( "Non-Specified-Field-Unit-Test" ) == "UNSPECIFIED" );

	fields.append_field( http_field::field_unspecified, "WRONG" );
	REQUIRE( fields.get_field( "Non-Specified-Field-Unit-Test" ) == "UNSPECIFIED" );

	REQUIRE_THROWS( fields.get_field( http_field::field_unspecified ) );

	REQUIRE( fields.get_field_or( http_field::field_unspecified, "?" ) == "?" );

	{
		int counter = 0;
		fields.for_each_field( [&counter](const auto & f) {
			if( 0 == counter ) {
				REQUIRE( f.field_id() == http_field::content_type );
				REQUIRE( f.name() == field_to_string( http_field::content_type ) );
				REQUIRE( f.value() == "text/plain; charset=utf-8" );
			}
			else if( 1 == counter ) {
				REQUIRE( f.field_id() == http_field::server );
				REQUIRE( f.name() == field_to_string( http_field::server ) );
				REQUIRE( f.value() == "Unit Test; Fields Test" );
			}
			// Just ignore all other values.

			++counter;
		} );
	}

	// Fields that don't exist
	REQUIRE_FALSE( fields.has_field( http_field::date ) );
	REQUIRE_FALSE( fields.has_field( http_field::accept_encoding ) );
	REQUIRE_FALSE( fields.has_field( http_field::authorization ) );

	REQUIRE_THROWS( fields.get_field( http_field::date ) );
	REQUIRE_THROWS( fields.get_field( http_field::accept_encoding ) );
	REQUIRE_THROWS( fields.get_field( http_field::authorization ) );

	fields.remove_field( http_field::date );
	fields.remove_field( http_field::accept_encoding );
	fields.remove_field( http_field::field_unspecified );

	REQUIRE( 3 == fields.fields_count() );

	fields.remove_field( http_field::content_type );
	REQUIRE( 2 == fields.fields_count() );
	fields.remove_field( http_field::server );
	REQUIRE( 1 == fields.fields_count() );
}

TEST_CASE( "get_field_or(field_id, value) overloads" ,
		"[header][get_field_or][by_id][overloads]" )
{
	http_header_fields_t fields;

	REQUIRE(
		fields.get_field_or( http_field::content_type, "default-value" )
			== "default-value" );

	REQUIRE(
		fields.get_field_or( http_field::content_type,
			restinio::string_view_t{ "default-value" } ) == "default-value" );

	std::string dv1{ "default-value" };
	const std::string dv2{ "default-value" };

	REQUIRE(
		fields.get_field_or( http_field::content_type, dv1 ) == "default-value" );
	REQUIRE(
		fields.get_field_or( http_field::content_type, dv2 ) == "default-value" );

	REQUIRE(
		fields.get_field_or( http_field::content_type,
			std::string{ "default-value" } ) == "default-value" );

	{
		std::string long_value{
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
			"This is a long value to avoid SSO. "
		};
		const std::string long_value2{ long_value };

		REQUIRE( long_value == long_value2 );
		REQUIRE( long_value.data() != long_value2.data() );

		const char * long_value_ptr{ long_value.data() };

		auto v = fields.get_field_or(
				http_field::content_type, std::move(long_value) );
		REQUIRE( v == long_value2 );
		REQUIRE( v.data() == long_value_ptr );
	}
}

TEST_CASE( "Working with optional fields (by id)",
		"[header][opt_fields][by_id]" )
{
	http_header_fields_t fields;

	REQUIRE( 0 == fields.fields_count() ); // No fields yet.

	{
		const auto f = fields.try_get_field( http_field::content_type );
		REQUIRE( !f );
	}

	{
		const auto f = fields.opt_value_of( http_field::content_type );
		REQUIRE( !f );
	}

	fields.set_field( "Content-Type", "text/plain" );
	REQUIRE( 1 == fields.fields_count() );

	{
		const auto f = fields.try_get_field( http_field::content_type );
		REQUIRE( f );
		REQUIRE( *f == "text/plain" );
	}

	{
		const auto f = fields.opt_value_of( http_field::content_type );
		REQUIRE( f );
		REQUIRE( *f == "text/plain" );
	}
}

TEST_CASE( "Working with fields (by http_header_field_t)" , "[header][fields][by_http_header_field_t]" )
{
	http_header_fields_t fields;

	fields.set_field( http_header_field_t{ std::string{"Content-Type"}, std::string{ "text/plain"} } );
	fields.set_field( http_header_field_t{ http_field::server, std::string{"UNIT-TEST"} } );

	REQUIRE( fields.get_field( http_field::content_type ) == "text/plain" );
	REQUIRE( fields.get_field( "Server" ) == "UNIT-TEST" );
}

TEST_CASE( "Enumeration of fields" , "[header][fields][for_each]" )
{
	http_header_fields_t fields;

	fields.set_field( "Content-Type", "text/plain" );
	fields.set_field( "Accept-Encoding", "utf-8" );
	fields.set_field( "Server", "Unknown" );

	REQUIRE( 3 == fields.fields_count() );

	std::set< std::string > names, values;

	fields.for_each_field( [&](const auto & hf) {
			names.insert( hf.name() );
			values.insert( hf.value() );
		} );

	REQUIRE( names == std::set< std::string >{
			"Content-Type", "Accept-Encoding", "Server"
		} );
	REQUIRE( values == std::set< std::string >{
			"text/plain", "utf-8", "Unknown"
		} );
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
		REQUIRE( restinio::status_code::ok == h.status_code() );

		h.status_code( restinio::status_code::not_found );
		REQUIRE( restinio::status_code::not_found == h.status_code() );
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

TEST_CASE( "working with string_to_field()" , "[header][string_to_field]" )
{

#define RESTINIO_FIELD_FROM_STRIN_TEST( field_id, field_name ) \
	REQUIRE( http_field:: field_id == string_to_field( #field_name ) );

	RESTINIO_FIELD_FROM_STRIN_TEST( a_im,                         A-IM )
	RESTINIO_FIELD_FROM_STRIN_TEST( accept,                       Accept )
	RESTINIO_FIELD_FROM_STRIN_TEST( accept_additions,             Accept-Additions )
	RESTINIO_FIELD_FROM_STRIN_TEST( accept_charset,               Accept-Charset )
	RESTINIO_FIELD_FROM_STRIN_TEST( accept_datetime,              Accept-Datetime )
	RESTINIO_FIELD_FROM_STRIN_TEST( accept_encoding,              Accept-Encoding )
	RESTINIO_FIELD_FROM_STRIN_TEST( accept_features,              Accept-Features )
	RESTINIO_FIELD_FROM_STRIN_TEST( accept_language,              Accept-Language )
	RESTINIO_FIELD_FROM_STRIN_TEST( accept_patch,                 Accept-Patch )
	RESTINIO_FIELD_FROM_STRIN_TEST( accept_post,                  Accept-Post )
	RESTINIO_FIELD_FROM_STRIN_TEST( accept_ranges,                Accept-Ranges )
	RESTINIO_FIELD_FROM_STRIN_TEST( age,                          Age )
	RESTINIO_FIELD_FROM_STRIN_TEST( allow,                        Allow )
	RESTINIO_FIELD_FROM_STRIN_TEST( alpn,                         ALPN )
	RESTINIO_FIELD_FROM_STRIN_TEST( alt_svc,                      Alt-Svc )
	RESTINIO_FIELD_FROM_STRIN_TEST( alt_used,                     Alt-Used )
	RESTINIO_FIELD_FROM_STRIN_TEST( alternates,                   Alternates )
	RESTINIO_FIELD_FROM_STRIN_TEST( apply_to_redirect_ref,        Apply-To-Redirect-Ref )
	RESTINIO_FIELD_FROM_STRIN_TEST( authentication_control,       Authentication-Control )
	RESTINIO_FIELD_FROM_STRIN_TEST( authentication_info,          Authentication-Info )
	RESTINIO_FIELD_FROM_STRIN_TEST( authorization,                Authorization )
	RESTINIO_FIELD_FROM_STRIN_TEST( c_ext,                        C-Ext )
	RESTINIO_FIELD_FROM_STRIN_TEST( c_man,                        C-Man )
	RESTINIO_FIELD_FROM_STRIN_TEST( c_opt,                        C-Opt )
	RESTINIO_FIELD_FROM_STRIN_TEST( c_pep,                        C-PEP )
	RESTINIO_FIELD_FROM_STRIN_TEST( c_pep_info,                   C-PEP-Info )
	RESTINIO_FIELD_FROM_STRIN_TEST( cache_control,                Cache-Control )
	RESTINIO_FIELD_FROM_STRIN_TEST( caldav_timezones,             CalDAV-Timezones )
	RESTINIO_FIELD_FROM_STRIN_TEST( close,                        Close )
	RESTINIO_FIELD_FROM_STRIN_TEST( content_base,                 Content-Base )
	RESTINIO_FIELD_FROM_STRIN_TEST( content_disposition,          Content-Disposition )
	RESTINIO_FIELD_FROM_STRIN_TEST( content_encoding,             Content-Encoding )
	RESTINIO_FIELD_FROM_STRIN_TEST( content_id,                   Content-ID )
	RESTINIO_FIELD_FROM_STRIN_TEST( content_language,             Content-Language )
	RESTINIO_FIELD_FROM_STRIN_TEST( content_location,             Content-Location )
	RESTINIO_FIELD_FROM_STRIN_TEST( content_md5,                  Content-MD5 )
	RESTINIO_FIELD_FROM_STRIN_TEST( content_range,                Content-Range )
	RESTINIO_FIELD_FROM_STRIN_TEST( content_script_type,          Content-Script-Type )
	RESTINIO_FIELD_FROM_STRIN_TEST( content_style_type,           Content-Style-Type )
	RESTINIO_FIELD_FROM_STRIN_TEST( content_type,                 Content-Type )
	RESTINIO_FIELD_FROM_STRIN_TEST( content_version,              Content-Version )
	RESTINIO_FIELD_FROM_STRIN_TEST( cookie,                       Cookie )
	RESTINIO_FIELD_FROM_STRIN_TEST( cookie2,                      Cookie2 )
	RESTINIO_FIELD_FROM_STRIN_TEST( dasl,                         DASL )
	RESTINIO_FIELD_FROM_STRIN_TEST( dav,                          DAV )
	RESTINIO_FIELD_FROM_STRIN_TEST( date,                         Date )
	RESTINIO_FIELD_FROM_STRIN_TEST( default_style,                Default-Style )
	RESTINIO_FIELD_FROM_STRIN_TEST( delta_base,                   Delta-Base )
	RESTINIO_FIELD_FROM_STRIN_TEST( depth,                        Depth )
	RESTINIO_FIELD_FROM_STRIN_TEST( derived_from,                 Derived-From )
	RESTINIO_FIELD_FROM_STRIN_TEST( destination,                  Destination )
	RESTINIO_FIELD_FROM_STRIN_TEST( differential_id,              Differential-ID )
	RESTINIO_FIELD_FROM_STRIN_TEST( digest,                       Digest )
	RESTINIO_FIELD_FROM_STRIN_TEST( etag,                         ETag )
	RESTINIO_FIELD_FROM_STRIN_TEST( expect,                       Expect )
	RESTINIO_FIELD_FROM_STRIN_TEST( expires,                      Expires )
	RESTINIO_FIELD_FROM_STRIN_TEST( ext,                          Ext )
	RESTINIO_FIELD_FROM_STRIN_TEST( forwarded,                    Forwarded )
	RESTINIO_FIELD_FROM_STRIN_TEST( from,                         From )
	RESTINIO_FIELD_FROM_STRIN_TEST( getprofile,                   GetProfile )
	RESTINIO_FIELD_FROM_STRIN_TEST( hobareg,                      Hobareg )
	RESTINIO_FIELD_FROM_STRIN_TEST( host,                         Host )
	RESTINIO_FIELD_FROM_STRIN_TEST( http2_settings,               HTTP2-Settings )
	RESTINIO_FIELD_FROM_STRIN_TEST( im,                           IM )
	RESTINIO_FIELD_FROM_STRIN_TEST( if_,                          If )
	RESTINIO_FIELD_FROM_STRIN_TEST( if_match,                     If-Match )
	RESTINIO_FIELD_FROM_STRIN_TEST( if_modified_since,            If-Modified-Since )
	RESTINIO_FIELD_FROM_STRIN_TEST( if_none_match,                If-None-Match )
	RESTINIO_FIELD_FROM_STRIN_TEST( if_range,                     If-Range )
	RESTINIO_FIELD_FROM_STRIN_TEST( if_schedule_tag_match,        If-Schedule-Tag-Match )
	RESTINIO_FIELD_FROM_STRIN_TEST( if_unmodified_since,          If-Unmodified-Since )
	RESTINIO_FIELD_FROM_STRIN_TEST( keep_alive,                   Keep-Alive )
	RESTINIO_FIELD_FROM_STRIN_TEST( label,                        Label )
	RESTINIO_FIELD_FROM_STRIN_TEST( last_modified,                Last-Modified )
	RESTINIO_FIELD_FROM_STRIN_TEST( link,                         Link )
	RESTINIO_FIELD_FROM_STRIN_TEST( location,                     Location )
	RESTINIO_FIELD_FROM_STRIN_TEST( lock_token,                   Lock-Token )
	RESTINIO_FIELD_FROM_STRIN_TEST( man,                          Man )
	RESTINIO_FIELD_FROM_STRIN_TEST( max_forwards,                 Max-Forwards )
	RESTINIO_FIELD_FROM_STRIN_TEST( memento_datetime,             Memento-Datetime )
	RESTINIO_FIELD_FROM_STRIN_TEST( meter,                        Meter )
	RESTINIO_FIELD_FROM_STRIN_TEST( mime_version,                 MIME-Version )
	RESTINIO_FIELD_FROM_STRIN_TEST( negotiate,                    Negotiate )
	RESTINIO_FIELD_FROM_STRIN_TEST( opt,                          Opt )
	RESTINIO_FIELD_FROM_STRIN_TEST( optional_www_authenticate,    Optional-WWW-Authenticate )
	RESTINIO_FIELD_FROM_STRIN_TEST( ordering_type,                Ordering-Type )
	RESTINIO_FIELD_FROM_STRIN_TEST( origin,                       Origin )
	RESTINIO_FIELD_FROM_STRIN_TEST( overwrite,                    Overwrite )
	RESTINIO_FIELD_FROM_STRIN_TEST( p3p,                          P3P )
	RESTINIO_FIELD_FROM_STRIN_TEST( pep,                          PEP )
	RESTINIO_FIELD_FROM_STRIN_TEST( pics_label,                   PICS-Label )
	RESTINIO_FIELD_FROM_STRIN_TEST( pep_info,                     Pep-Info )
	RESTINIO_FIELD_FROM_STRIN_TEST( position,                     Position )
	RESTINIO_FIELD_FROM_STRIN_TEST( pragma,                       Pragma )
	RESTINIO_FIELD_FROM_STRIN_TEST( prefer,                       Prefer )
	RESTINIO_FIELD_FROM_STRIN_TEST( preference_applied,           Preference-Applied )
	RESTINIO_FIELD_FROM_STRIN_TEST( profileobject,                ProfileObject )
	RESTINIO_FIELD_FROM_STRIN_TEST( protocol,                     Protocol )
	RESTINIO_FIELD_FROM_STRIN_TEST( protocol_info,                Protocol-Info )
	RESTINIO_FIELD_FROM_STRIN_TEST( protocol_query,               Protocol-Query )
	RESTINIO_FIELD_FROM_STRIN_TEST( protocol_request,             Protocol-Request )
	RESTINIO_FIELD_FROM_STRIN_TEST( proxy_authenticate,           Proxy-Authenticate )
	RESTINIO_FIELD_FROM_STRIN_TEST( proxy_authentication_info,    Proxy-Authentication-Info )
	RESTINIO_FIELD_FROM_STRIN_TEST( proxy_authorization,          Proxy-Authorization )
	RESTINIO_FIELD_FROM_STRIN_TEST( proxy_features,               Proxy-Features )
	RESTINIO_FIELD_FROM_STRIN_TEST( proxy_instruction,            Proxy-Instruction )
	RESTINIO_FIELD_FROM_STRIN_TEST( public_,                      Public )
	RESTINIO_FIELD_FROM_STRIN_TEST( public_key_pins,              Public-Key-Pins )
	RESTINIO_FIELD_FROM_STRIN_TEST( public_key_pins_report_only,  Public-Key-Pins-Report-Only )
	RESTINIO_FIELD_FROM_STRIN_TEST( range,                        Range )
	RESTINIO_FIELD_FROM_STRIN_TEST( redirect_ref,                 Redirect-Ref )
	RESTINIO_FIELD_FROM_STRIN_TEST( referer,                      Referer )
	RESTINIO_FIELD_FROM_STRIN_TEST( retry_after,                  Retry-After )
	RESTINIO_FIELD_FROM_STRIN_TEST( safe,                         Safe )
	RESTINIO_FIELD_FROM_STRIN_TEST( schedule_reply,               Schedule-Reply )
	RESTINIO_FIELD_FROM_STRIN_TEST( schedule_tag,                 Schedule-Tag )
	RESTINIO_FIELD_FROM_STRIN_TEST( sec_websocket_accept,         Sec-WebSocket-Accept )
	RESTINIO_FIELD_FROM_STRIN_TEST( sec_websocket_extensions,     Sec-WebSocket-Extensions )
	RESTINIO_FIELD_FROM_STRIN_TEST( sec_websocket_key,            Sec-WebSocket-Key )
	RESTINIO_FIELD_FROM_STRIN_TEST( sec_websocket_protocol,       Sec-WebSocket-Protocol )
	RESTINIO_FIELD_FROM_STRIN_TEST( sec_websocket_version,        Sec-WebSocket-Version )
	RESTINIO_FIELD_FROM_STRIN_TEST( security_scheme,              Security-Scheme )
	RESTINIO_FIELD_FROM_STRIN_TEST( server,                       Server )
	RESTINIO_FIELD_FROM_STRIN_TEST( set_cookie,                   Set-Cookie )
	RESTINIO_FIELD_FROM_STRIN_TEST( set_cookie2,                  Set-Cookie2 )
	RESTINIO_FIELD_FROM_STRIN_TEST( setprofile,                   SetProfile )
	RESTINIO_FIELD_FROM_STRIN_TEST( slug,                         SLUG )
	RESTINIO_FIELD_FROM_STRIN_TEST( soapaction,                   SoapAction )
	RESTINIO_FIELD_FROM_STRIN_TEST( status_uri,                   Status-URI )
	RESTINIO_FIELD_FROM_STRIN_TEST( strict_transport_security,    Strict-Transport-Security )
	RESTINIO_FIELD_FROM_STRIN_TEST( surrogate_capability,         Surrogate-Capability )
	RESTINIO_FIELD_FROM_STRIN_TEST( surrogate_control,            Surrogate-Control )
	RESTINIO_FIELD_FROM_STRIN_TEST( tcn,                          TCN )
	RESTINIO_FIELD_FROM_STRIN_TEST( te,                           TE )
	RESTINIO_FIELD_FROM_STRIN_TEST( timeout,                      Timeout )
	RESTINIO_FIELD_FROM_STRIN_TEST( topic,                        Topic )
	RESTINIO_FIELD_FROM_STRIN_TEST( trailer,                      Trailer )
	RESTINIO_FIELD_FROM_STRIN_TEST( transfer_encoding,            Transfer-Encoding )
	RESTINIO_FIELD_FROM_STRIN_TEST( ttl,                          TTL )
	RESTINIO_FIELD_FROM_STRIN_TEST( urgency,                      Urgency )
	RESTINIO_FIELD_FROM_STRIN_TEST( uri,                          URI )
	RESTINIO_FIELD_FROM_STRIN_TEST( upgrade,                      Upgrade )
	RESTINIO_FIELD_FROM_STRIN_TEST( user_agent,                   User-Agent )
	RESTINIO_FIELD_FROM_STRIN_TEST( variant_vary,                 Variant-Vary )
	RESTINIO_FIELD_FROM_STRIN_TEST( vary,                         Vary )
	RESTINIO_FIELD_FROM_STRIN_TEST( via,                          Via )
	RESTINIO_FIELD_FROM_STRIN_TEST( www_authenticate,             WWW-Authenticate )
	RESTINIO_FIELD_FROM_STRIN_TEST( want_digest,                  Want-Digest )
	RESTINIO_FIELD_FROM_STRIN_TEST( warning,                      Warning )
	RESTINIO_FIELD_FROM_STRIN_TEST( x_frame_options,              X-Frame-Options )

	// Since v0.4.7
	RESTINIO_FIELD_FROM_STRIN_TEST( access_control,               Access-Control )
	RESTINIO_FIELD_FROM_STRIN_TEST( access_control_allow_credentials, Access-Control-Allow-Credentials )
	RESTINIO_FIELD_FROM_STRIN_TEST( access_control_allow_headers, Access-Control-Allow-Headers )
	RESTINIO_FIELD_FROM_STRIN_TEST( access_control_allow_methods, Access-Control-Allow-Methods )
	RESTINIO_FIELD_FROM_STRIN_TEST( access_control_allow_origin,  Access-Control-Allow-Origin )
	RESTINIO_FIELD_FROM_STRIN_TEST( access_control_max_age,       Access-Control-Max-Age )
	RESTINIO_FIELD_FROM_STRIN_TEST( access_control_request_method,    Access-Control-Request-Method )
	RESTINIO_FIELD_FROM_STRIN_TEST( access_control_request_headers,   Access-Control-Request-Headers )
	RESTINIO_FIELD_FROM_STRIN_TEST( compliance,                   Compliance )
	RESTINIO_FIELD_FROM_STRIN_TEST( content_transfer_encoding,    Content-Transfer-Encoding )
	RESTINIO_FIELD_FROM_STRIN_TEST( cost,                         Cost )
	RESTINIO_FIELD_FROM_STRIN_TEST( ediint_features,              EDIINT-Features )
	RESTINIO_FIELD_FROM_STRIN_TEST( message_id,                   Message-ID )
	RESTINIO_FIELD_FROM_STRIN_TEST( method_check,                 Method-Check )
	RESTINIO_FIELD_FROM_STRIN_TEST( method_check_expires,         Method-Check-Expires )
	RESTINIO_FIELD_FROM_STRIN_TEST( non_compliance,               Non-Compliance )
	RESTINIO_FIELD_FROM_STRIN_TEST( optional,                     Optional )
	RESTINIO_FIELD_FROM_STRIN_TEST( referer_root,                 Referer-Root )
	RESTINIO_FIELD_FROM_STRIN_TEST( resolution_hint,              Resolution-Hint )
	RESTINIO_FIELD_FROM_STRIN_TEST( resolver_location,            Resolver-Location )
	RESTINIO_FIELD_FROM_STRIN_TEST( subok,                        SubOK )
	RESTINIO_FIELD_FROM_STRIN_TEST( subst,                        Subst )
	RESTINIO_FIELD_FROM_STRIN_TEST( title,                        Title )
	RESTINIO_FIELD_FROM_STRIN_TEST( ua_color,                     UA-Color )
	RESTINIO_FIELD_FROM_STRIN_TEST( ua_media,                     UA-Media )
	RESTINIO_FIELD_FROM_STRIN_TEST( ua_pixels,                    UA-Pixels )
	RESTINIO_FIELD_FROM_STRIN_TEST( ua_resolution,                UA-Resolution )
	RESTINIO_FIELD_FROM_STRIN_TEST( ua_windowpixels,              UA-Windowpixels )
	RESTINIO_FIELD_FROM_STRIN_TEST( version,                      Version )
	RESTINIO_FIELD_FROM_STRIN_TEST( x_device_accept,              X-Device-Accept )
	RESTINIO_FIELD_FROM_STRIN_TEST( x_device_accept_charset,      X-Device-Accept-Charset )
	RESTINIO_FIELD_FROM_STRIN_TEST( x_device_accept_encoding,     X-Device-Accept-Encoding )
	RESTINIO_FIELD_FROM_STRIN_TEST( x_device_accept_language,     X-Device-Accept-Language )
	RESTINIO_FIELD_FROM_STRIN_TEST( x_device_user_agent,          X-Device-User-Agent )
#undef RESTINIO_FIELD_FROM_STRIN_TEST
}

TEST_CASE( "Connection" , "[header][connection]" )
{
	using namespace Catch;

	{
		// Default.
		http_response_header_t h;
		const auto serialized =
			impl::create_header_string( h,
				impl::content_length_field_presence_t::skip_content_length );

		REQUIRE_THAT(
			serialized,
			Contains( "Connection: close" ) ||
			!Contains( "Content-Length" ) );
	}
	{
		// Default.
		http_response_header_t h;
		const auto serialized =
			impl::create_header_string( h );

		REQUIRE_THAT(
			serialized,
			Contains( "Connection: close" ) ||
			Contains( "Content-Length: 0" ) );
	}

	{
		http_response_header_t h;
		h.should_keep_alive( false );
		const auto serialized =
			impl::create_header_string( h );

		REQUIRE_THAT(
			serialized,
			Contains( "Connection: close" ) );
	}

	{
		http_response_header_t h;
		h.should_keep_alive( true );
		const auto serialized =
			impl::create_header_string( h );

		REQUIRE_THAT(
			serialized,
			Contains( "Connection: keep-alive" ) );
	}

	{
		http_response_header_t h;
		h.connection( http_connection_header_t::upgrade );
		const auto serialized =
			impl::create_header_string( h );

		REQUIRE_THAT(
			serialized,
			Contains( "Connection: Upgrade" ) );
	}
}


TEST_CASE( "Query" , "[header][query string][query path]" )
{
	auto append = []( http_request_header_t & h, const std::string & part ){
		h.append_request_target( part.data(), part.size() );
	};
	// Default.
	http_request_header_t h;

	h.request_target( "/sobjectizerteam" );

	REQUIRE( h.request_target() == "/sobjectizerteam" );
	REQUIRE( h.path() == "/sobjectizerteam" );
	REQUIRE( h.query() == "" );
	REQUIRE( h.fragment() == "" );

	append( h, "/json_dto-0.2" );

	REQUIRE( h.request_target() == "/sobjectizerteam/json_dto-0.2" );
	REQUIRE( h.path() == "/sobjectizerteam/json_dto-0.2" );
	REQUIRE( h.query() == "" );
	REQUIRE( h.fragment() == "" );



	append( h, "#markdown-header" );

	REQUIRE( h.request_target() == "/sobjectizerteam/json_dto-0.2#markdown-header" );
	REQUIRE( h.path() == "/sobjectizerteam/json_dto-0.2" );
	REQUIRE( h.query() == "" );
	REQUIRE( h.fragment() == "markdown-header" );

	append( h, "-what-is-json_dto" );

	REQUIRE( h.request_target() == "/sobjectizerteam/json_dto-0.2#markdown-header-what-is-json_dto" );
	REQUIRE( h.path() == "/sobjectizerteam/json_dto-0.2" );
	REQUIRE( h.query() == "" );
	REQUIRE( h.fragment() == "markdown-header-what-is-json_dto" );


	h.request_target( "/sobjectizerteam/json_dto-0.2#markdown-header-what-is-json_dto" );
	REQUIRE( h.request_target() == "/sobjectizerteam/json_dto-0.2#markdown-header-what-is-json_dto" );

	REQUIRE( h.path() == "/sobjectizerteam/json_dto-0.2" );
	REQUIRE( h.query() == "" );
	REQUIRE( h.fragment() == "markdown-header-what-is-json_dto" );


	h.request_target( "/sobjectizerteam/json_dto-0.2?#" );
	REQUIRE( h.request_target() == "/sobjectizerteam/json_dto-0.2?#" );

	REQUIRE( h.path() == "/sobjectizerteam/json_dto-0.2" );
	REQUIRE( h.query() == "" );
	REQUIRE( h.fragment() == "" );

	h.request_target( "/sobjectizerteam/json_dto-0.2" );
	REQUIRE( h.request_target() == "/sobjectizerteam/json_dto-0.2" );
	REQUIRE( h.path() == "/sobjectizerteam/json_dto-0.2" );
	REQUIRE( h.query() == "" );
	REQUIRE( h.fragment() == "" );

	append( h, "?" );
	REQUIRE( h.request_target() == "/sobjectizerteam/json_dto-0.2?" );
	REQUIRE( h.path() == "/sobjectizerteam/json_dto-0.2" );
	REQUIRE( h.query() == "" );
	REQUIRE( h.fragment() == "" );

	append( h, "#" );
	REQUIRE( h.request_target() == "/sobjectizerteam/json_dto-0.2?#" );
	REQUIRE( h.path() == "/sobjectizerteam/json_dto-0.2" );
	REQUIRE( h.query() == "" );
	REQUIRE( h.fragment() == "" );

	append( h, "123" );
	REQUIRE( h.request_target() == "/sobjectizerteam/json_dto-0.2?#123" );
	REQUIRE( h.path() == "/sobjectizerteam/json_dto-0.2" );
	REQUIRE( h.query() == "" );
	REQUIRE( h.fragment() == "123" );


	h.request_target( "/weather/temperature?from=2012-01-01&to=2012-01-10" );
	REQUIRE( h.request_target() == "/weather/temperature?from=2012-01-01&to=2012-01-10" );
	REQUIRE( h.path() == "/weather/temperature" );
	REQUIRE( h.query() == "from=2012-01-01&to=2012-01-10" );
	REQUIRE( h.fragment() == "" );

	h.request_target( "/weather/temperature" );
	append( h, "?" );
	append( h, "from=2012-01-01" );
	append( h, "&" );
	append( h, "to=2012-01-10" );
	REQUIRE( h.request_target() == "/weather/temperature?from=2012-01-01&to=2012-01-10" );
	REQUIRE( h.path() == "/weather/temperature" );
	REQUIRE( h.query() == "from=2012-01-01&to=2012-01-10" );
	REQUIRE( h.fragment() == "" );

	h.request_target( "/weather/temperature" );
	append( h, "?" );
	append( h, "from=2012-01-01" );
	append( h, "&" );
	append( h, "to=2012-01-10" );
	append( h, "#" );
	append( h, "Celsius" );
	REQUIRE( h.request_target() == "/weather/temperature?from=2012-01-01&to=2012-01-10#Celsius" );
	REQUIRE( h.path() == "/weather/temperature" );
	REQUIRE( h.query() == "from=2012-01-01&to=2012-01-10" );
	REQUIRE( h.fragment() == "Celsius" );


	h.request_target( "/weather/temperature#Celsius?from=2012-01-01&to=2012-01-10" );
	REQUIRE( h.request_target() == "/weather/temperature#Celsius?from=2012-01-01&to=2012-01-10" );
	REQUIRE( h.path() == "/weather/temperature" );
	REQUIRE( h.query() == "" );
	REQUIRE( h.fragment() == "Celsius?from=2012-01-01&to=2012-01-10" );

	h.request_target( "/weather" );
	append( h, "/temperature" );
	append( h, "#" );
	append( h, "Celsius" );
	append( h, "?" );
	append( h, "from=2012-01-01" );
	append( h, "&" );
	append( h, "to=2012-01-10" );
	REQUIRE( h.request_target() == "/weather/temperature#Celsius?from=2012-01-01&to=2012-01-10" );
	REQUIRE( h.path() == "/weather/temperature" );
	REQUIRE( h.query() == "" );
	REQUIRE( h.fragment() == "Celsius?from=2012-01-01&to=2012-01-10" );
}

