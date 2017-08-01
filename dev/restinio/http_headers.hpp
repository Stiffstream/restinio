/*
	restinio
*/

/*!
	helpers for http communication.
*/

#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include <fmt/format.h>

#include <nodejs/http_parser/http_parser.h>

#include <restinio/exception.hpp>

namespace restinio
{

//
// caseless_cmp()
//

//! Comparator for fields names.
inline bool
caseless_cmp(
	const char * a,
	const char * b,
	std::size_t size )
{
	for( std::size_t i = 0; i < size; ++i )
		if( std::tolower( a[ i ] ) != std::tolower( b[ i ] ) )
			return false;

	return true;
}

//
// caseless_cmp()
//

//! Comparator for fields names.
inline bool
caseless_cmp(
	const char * a,
	std::size_t a_size,
	const char * b,
	std::size_t b_size )
{
	if( a_size == b_size )
	{
		return caseless_cmp( a, b, a_size );
	}

	return false;
}

//
// caseless_cmp()
//

//! Comparator for fields names.
inline bool
caseless_cmp( const std::string & a, const std::string & b )
{
	return caseless_cmp( a.data(), a.size(), b.data(), b.size() );
}

// Adopted header fields
// (https://www.iana.org/assignments/message-headers/message-headers.xml#perm-headers).
// Fields `Connection` and `Content-Length` are specieal cases, thus they are excluded from the list.
#define RESTINIO_HTTP_FIELDS_MAP( RESTINIO_GEN ) \
	RESTINIO_GEN( a_im,                         A-IM )                        \
	RESTINIO_GEN( accept,                       Accept )                      \
	RESTINIO_GEN( accept_additions,             Accept-Additions )            \
	RESTINIO_GEN( accept_charset,               Accept-Charset )              \
	RESTINIO_GEN( accept_datetime,              Accept-Datetime )             \
	RESTINIO_GEN( accept_encoding,              Accept-Encoding )             \
	RESTINIO_GEN( accept_features,              Accept-Features )             \
	RESTINIO_GEN( accept_language,              Accept-Language )             \
	RESTINIO_GEN( accept_patch,                 Accept-Patch )                \
	RESTINIO_GEN( accept_post,                  Accept-Post )                 \
	RESTINIO_GEN( accept_ranges,                Accept-Ranges )               \
	RESTINIO_GEN( age,                          Age )                         \
	RESTINIO_GEN( allow,                        Allow )                       \
	RESTINIO_GEN( alpn,                         ALPN )                        \
	RESTINIO_GEN( alt_svc,                      Alt-Svc )                     \
	RESTINIO_GEN( alt_used,                     Alt-Used )                    \
	RESTINIO_GEN( alternates,                   Alternates )                  \
	RESTINIO_GEN( apply_to_redirect_ref,        Apply-To-Redirect-Ref )       \
	RESTINIO_GEN( authentication_control,       Authentication-Control )      \
	RESTINIO_GEN( authentication_info,          Authentication-Info )         \
	RESTINIO_GEN( authorization,                Authorization )               \
	RESTINIO_GEN( c_ext,                        C-Ext )                       \
	RESTINIO_GEN( c_man,                        C-Man )                       \
	RESTINIO_GEN( c_opt,                        C-Opt )                       \
	RESTINIO_GEN( c_pep,                        C-PEP )                       \
	RESTINIO_GEN( c_pep_info,                   C-PEP-Info )                  \
	RESTINIO_GEN( cache_control,                Cache-Control )               \
	RESTINIO_GEN( caldav_timezones,             CalDAV-Timezones )            \
	RESTINIO_GEN( close,                        Close )                       \
	RESTINIO_GEN( content_base,                 Content-Base )                \
	RESTINIO_GEN( content_disposition,          Content-Disposition )         \
	RESTINIO_GEN( content_encoding,             Content-Encoding )            \
	RESTINIO_GEN( content_id,                   Content-ID )                  \
	RESTINIO_GEN( content_language,             Content-Language )            \
	RESTINIO_GEN( content_location,             Content-Location )            \
	RESTINIO_GEN( content_md5,                  Content-MD5 )                 \
	RESTINIO_GEN( content_range,                Content-Range )               \
	RESTINIO_GEN( content_script_type,          Content-Script-Type )         \
	RESTINIO_GEN( content_style_type,           Content-Style-Type )          \
	RESTINIO_GEN( content_type,                 Content-Type )                \
	RESTINIO_GEN( content_version,              Content-Version )             \
	RESTINIO_GEN( cookie,                       Cookie )                      \
	RESTINIO_GEN( cookie2,                      Cookie2 )                     \
	RESTINIO_GEN( dasl,                         DASL )                        \
	RESTINIO_GEN( dav,                          DAV )                         \
	RESTINIO_GEN( date,                         Date )                        \
	RESTINIO_GEN( default_style,                Default-Style )               \
	RESTINIO_GEN( delta_base,                   Delta-Base )                  \
	RESTINIO_GEN( depth,                        Depth )                       \
	RESTINIO_GEN( derived_from,                 Derived-From )                \
	RESTINIO_GEN( destination,                  Destination )                 \
	RESTINIO_GEN( differential_id,              Differential-ID )             \
	RESTINIO_GEN( digest,                       Digest )                      \
	RESTINIO_GEN( etag,                         ETag )                        \
	RESTINIO_GEN( expect,                       Expect )                      \
	RESTINIO_GEN( expires,                      Expires )                     \
	RESTINIO_GEN( ext,                          Ext )                         \
	RESTINIO_GEN( forwarded,                    Forwarded )                   \
	RESTINIO_GEN( from,                         From )                        \
	RESTINIO_GEN( getprofile,                   GetProfile )                  \
	RESTINIO_GEN( hobareg,                      Hobareg )                     \
	RESTINIO_GEN( host,                         Host )                        \
	RESTINIO_GEN( http2_settings,               HTTP2-Settings )              \
	RESTINIO_GEN( im,                           IM )                          \
	RESTINIO_GEN( if_,                          If )                          \
	RESTINIO_GEN( if_match,                     If-Match )                    \
	RESTINIO_GEN( if_modified_since,            If-Modified-Since )           \
	RESTINIO_GEN( if_none_match,                If-None-Match )               \
	RESTINIO_GEN( if_range,                     If-Range )                    \
	RESTINIO_GEN( if_schedule_tag_match,        If-Schedule-Tag-Match )       \
	RESTINIO_GEN( if_unmodified_since,          If-Unmodified-Since )         \
	RESTINIO_GEN( keep_alive,                   Keep-Alive )                  \
	RESTINIO_GEN( label,                        Label )                       \
	RESTINIO_GEN( last_modified,                Last-Modified )               \
	RESTINIO_GEN( link,                         Link )                        \
	RESTINIO_GEN( location,                     Location )                    \
	RESTINIO_GEN( lock_token,                   Lock-Token )                  \
	RESTINIO_GEN( man,                          Man )                         \
	RESTINIO_GEN( max_forwards,                 Max-Forwards )                \
	RESTINIO_GEN( memento_datetime,             Memento-Datetime )            \
	RESTINIO_GEN( meter,                        Meter )                       \
	RESTINIO_GEN( mime_version,                 MIME-Version )                \
	RESTINIO_GEN( negotiate,                    Negotiate )                   \
	RESTINIO_GEN( opt,                          Opt )                         \
	RESTINIO_GEN( optional_www_authenticate,    Optional-WWW-Authenticate )   \
	RESTINIO_GEN( ordering_type,                Ordering-Type )               \
	RESTINIO_GEN( origin,                       Origin )                      \
	RESTINIO_GEN( overwrite,                    Overwrite )                   \
	RESTINIO_GEN( p3p,                          P3P )                         \
	RESTINIO_GEN( pep,                          PEP )                         \
	RESTINIO_GEN( pics_label,                   PICS-Label )                  \
	RESTINIO_GEN( pep_info,                     Pep-Info )                    \
	RESTINIO_GEN( position,                     Position )                    \
	RESTINIO_GEN( pragma,                       Pragma )                      \
	RESTINIO_GEN( prefer,                       Prefer )                      \
	RESTINIO_GEN( preference_applied,           Preference-Applied )          \
	RESTINIO_GEN( profileobject,                ProfileObject )               \
	RESTINIO_GEN( protocol,                     Protocol )                    \
	RESTINIO_GEN( protocol_info,                Protocol-Info )               \
	RESTINIO_GEN( protocol_query,               Protocol-Query )              \
	RESTINIO_GEN( protocol_request,             Protocol-Request )            \
	RESTINIO_GEN( proxy_authenticate,           Proxy-Authenticate )          \
	RESTINIO_GEN( proxy_authentication_info,    Proxy-Authentication-Info )   \
	RESTINIO_GEN( proxy_authorization,          Proxy-Authorization )         \
	RESTINIO_GEN( proxy_features,               Proxy-Features )              \
	RESTINIO_GEN( proxy_instruction,            Proxy-Instruction )           \
	RESTINIO_GEN( public_,                      Public )                      \
	RESTINIO_GEN( public_key_pins,              Public-Key-Pins )             \
	RESTINIO_GEN( public_key_pins_report_only,  Public-Key-Pins-Report-Only ) \
	RESTINIO_GEN( range,                        Range )                       \
	RESTINIO_GEN( redirect_ref,                 Redirect-Ref )                \
	RESTINIO_GEN( referer,                      Referer )                     \
	RESTINIO_GEN( retry_after,                  Retry-After )                 \
	RESTINIO_GEN( safe,                         Safe )                        \
	RESTINIO_GEN( schedule_reply,               Schedule-Reply )              \
	RESTINIO_GEN( schedule_tag,                 Schedule-Tag )                \
	RESTINIO_GEN( sec_websocket_accept,         Sec-WebSocket-Accept )        \
	RESTINIO_GEN( sec_websocket_extensions,     Sec-WebSocket-Extensions )    \
	RESTINIO_GEN( sec_websocket_key,            Sec-WebSocket-Key )           \
	RESTINIO_GEN( sec_websocket_protocol,       Sec-WebSocket-Protocol )      \
	RESTINIO_GEN( sec_websocket_version,        Sec-WebSocket-Version )       \
	RESTINIO_GEN( security_scheme,              Security-Scheme )             \
	RESTINIO_GEN( server,                       Server )                      \
	RESTINIO_GEN( set_cookie,                   Set-Cookie )                  \
	RESTINIO_GEN( set_cookie2,                  Set-Cookie2 )                 \
	RESTINIO_GEN( setprofile,                   SetProfile )                  \
	RESTINIO_GEN( slug,                         SLUG )                        \
	RESTINIO_GEN( soapaction,                   SoapAction )                  \
	RESTINIO_GEN( status_uri,                   Status-URI )                  \
	RESTINIO_GEN( strict_transport_security,    Strict-Transport-Security )   \
	RESTINIO_GEN( surrogate_capability,         Surrogate-Capability )        \
	RESTINIO_GEN( surrogate_control,            Surrogate-Control )           \
	RESTINIO_GEN( tcn,                          TCN )                         \
	RESTINIO_GEN( te,                           TE )                          \
	RESTINIO_GEN( timeout,                      Timeout )                     \
	RESTINIO_GEN( topic,                        Topic )                       \
	RESTINIO_GEN( trailer,                      Trailer )                     \
	RESTINIO_GEN( transfer_encoding,            Transfer-Encoding )           \
	RESTINIO_GEN( ttl,                          TTL )                         \
	RESTINIO_GEN( urgency,                      Urgency )                     \
	RESTINIO_GEN( uri,                          URI )                         \
	RESTINIO_GEN( upgrade,                      Upgrade )                     \
	RESTINIO_GEN( user_agent,                   User-Agent )                  \
	RESTINIO_GEN( variant_vary,                 Variant-Vary )                \
	RESTINIO_GEN( vary,                         Vary )                        \
	RESTINIO_GEN( via,                          Via )                         \
	RESTINIO_GEN( www_authenticate,             WWW-Authenticate )            \
	RESTINIO_GEN( want_digest,                  Want-Digest )                 \
	RESTINIO_GEN( warning,                      Warning )                     \
	RESTINIO_GEN( x_frame_options,              X-Frame-Options )
	// SPECIAL CASE: RESTINIO_GEN( connection,                   Connection )
	// SPECIAL CASE: RESTINIO_GEN( content_length,               Content-Length )


//
// http_method_t
//

//! C++ enum that repeats nodejs c-style enum.
/*!
	\note Fields `Connection` and `Content-Length` are specieal cases,
	thus they are not present in the list.
*/
enum class http_field_t : std::uint8_t //By now 152 + 1 items fits to uint8_t
{
#define RESTINIO_HTTP_FIELD_GEN( name, ignored ) name,
	RESTINIO_HTTP_FIELDS_MAP( RESTINIO_HTTP_FIELD_GEN )
#undef RESTINIO_HTTP_FIELD_GEN
	// Unspecified field.
	field_unspecified
};

//! Helper alies to omitt `_t` suffix.
using http_field = http_field_t;

//
// string_to_field()
//

//! Helper function to get method string name.
//! \{
inline http_field_t
string_to_field( const char * field_name, std::size_t field_name_size )
{

#define RESTINIO_HTTP_CHECK_FOR_FIELD( field_id, candidate_field_name ) \
	if( caseless_cmp(field_name, #candidate_field_name , field_name_size ) ) \
		return http_field_t:: field_id;

	// TODO: make most popular fields to be checked first.

	switch( field_name_size )
	{
		case 2:
			RESTINIO_HTTP_CHECK_FOR_FIELD( if_,                          If )
			RESTINIO_HTTP_CHECK_FOR_FIELD( im,                           IM )
			RESTINIO_HTTP_CHECK_FOR_FIELD( te,                           TE )
			break;

		case 3:
			RESTINIO_HTTP_CHECK_FOR_FIELD( age,                          Age )
			RESTINIO_HTTP_CHECK_FOR_FIELD( dav,                          DAV )
			RESTINIO_HTTP_CHECK_FOR_FIELD( ext,                          Ext )
			RESTINIO_HTTP_CHECK_FOR_FIELD( man,                          Man )
			RESTINIO_HTTP_CHECK_FOR_FIELD( opt,                          Opt )
			RESTINIO_HTTP_CHECK_FOR_FIELD( p3p,                          P3P )
			RESTINIO_HTTP_CHECK_FOR_FIELD( pep,                          PEP )
			RESTINIO_HTTP_CHECK_FOR_FIELD( tcn,                          TCN )
			RESTINIO_HTTP_CHECK_FOR_FIELD( ttl,                          TTL )
			RESTINIO_HTTP_CHECK_FOR_FIELD( uri,                          URI )
			RESTINIO_HTTP_CHECK_FOR_FIELD( via,                          Via )
			break;

		case 4:
			RESTINIO_HTTP_CHECK_FOR_FIELD( a_im,                         A-IM )
			RESTINIO_HTTP_CHECK_FOR_FIELD( alpn,                         ALPN )
			RESTINIO_HTTP_CHECK_FOR_FIELD( dasl,                         DASL )
			RESTINIO_HTTP_CHECK_FOR_FIELD( date,                         Date )
			RESTINIO_HTTP_CHECK_FOR_FIELD( etag,                         ETag )
			RESTINIO_HTTP_CHECK_FOR_FIELD( from,                         From )
			RESTINIO_HTTP_CHECK_FOR_FIELD( host,                         Host )
			RESTINIO_HTTP_CHECK_FOR_FIELD( link,                         Link )
			RESTINIO_HTTP_CHECK_FOR_FIELD( safe,                         Safe )
			RESTINIO_HTTP_CHECK_FOR_FIELD( slug,                         SLUG )
			RESTINIO_HTTP_CHECK_FOR_FIELD( vary,                         Vary )

		case 5:
			RESTINIO_HTTP_CHECK_FOR_FIELD( allow,                        Allow )
			RESTINIO_HTTP_CHECK_FOR_FIELD( c_ext,                        C-Ext )
			RESTINIO_HTTP_CHECK_FOR_FIELD( c_man,                        C-Man )
			RESTINIO_HTTP_CHECK_FOR_FIELD( c_opt,                        C-Opt )
			RESTINIO_HTTP_CHECK_FOR_FIELD( c_pep,                        C-PEP )
			RESTINIO_HTTP_CHECK_FOR_FIELD( close,                        Close )
			RESTINIO_HTTP_CHECK_FOR_FIELD( depth,                        Depth )
			RESTINIO_HTTP_CHECK_FOR_FIELD( label,                        Label )
			RESTINIO_HTTP_CHECK_FOR_FIELD( meter,                        Meter )
			RESTINIO_HTTP_CHECK_FOR_FIELD( range,                        Range )
			RESTINIO_HTTP_CHECK_FOR_FIELD( topic,                        Topic )
			break;

		case 6:
			RESTINIO_HTTP_CHECK_FOR_FIELD( accept,                       Accept )
			RESTINIO_HTTP_CHECK_FOR_FIELD( cookie,                       Cookie )
			RESTINIO_HTTP_CHECK_FOR_FIELD( digest,                       Digest )
			RESTINIO_HTTP_CHECK_FOR_FIELD( expect,                       Expect )
			RESTINIO_HTTP_CHECK_FOR_FIELD( origin,                       Origin )
			RESTINIO_HTTP_CHECK_FOR_FIELD( pragma,                       Pragma )
			RESTINIO_HTTP_CHECK_FOR_FIELD( prefer,                       Prefer )
			RESTINIO_HTTP_CHECK_FOR_FIELD( public_,                      Public )
			RESTINIO_HTTP_CHECK_FOR_FIELD( server,                       Server )
			break;

		case 7:
			RESTINIO_HTTP_CHECK_FOR_FIELD( alt_svc,                      Alt-Svc )
			RESTINIO_HTTP_CHECK_FOR_FIELD( cookie2,                      Cookie2 )
			RESTINIO_HTTP_CHECK_FOR_FIELD( expires,                      Expires )
			RESTINIO_HTTP_CHECK_FOR_FIELD( hobareg,                      Hobareg )
			RESTINIO_HTTP_CHECK_FOR_FIELD( referer,                      Referer )
			RESTINIO_HTTP_CHECK_FOR_FIELD( timeout,                      Timeout )
			RESTINIO_HTTP_CHECK_FOR_FIELD( trailer,                      Trailer )
			RESTINIO_HTTP_CHECK_FOR_FIELD( urgency,                      Urgency )
			RESTINIO_HTTP_CHECK_FOR_FIELD( upgrade,                      Upgrade )
			RESTINIO_HTTP_CHECK_FOR_FIELD( warning,                      Warning )
			break;

		case 8:
			RESTINIO_HTTP_CHECK_FOR_FIELD( alt_used,                     Alt-Used )
			RESTINIO_HTTP_CHECK_FOR_FIELD( if_match,                     If-Match )
			RESTINIO_HTTP_CHECK_FOR_FIELD( if_range,                     If-Range )
			RESTINIO_HTTP_CHECK_FOR_FIELD( location,                     Location )
			RESTINIO_HTTP_CHECK_FOR_FIELD( pep_info,                     Pep-Info )
			RESTINIO_HTTP_CHECK_FOR_FIELD( position,                     Position )
			RESTINIO_HTTP_CHECK_FOR_FIELD( protocol,                     Protocol )
			break;

		case 9:
			RESTINIO_HTTP_CHECK_FOR_FIELD( forwarded,                    Forwarded )
			RESTINIO_HTTP_CHECK_FOR_FIELD( negotiate,                    Negotiate )
			RESTINIO_HTTP_CHECK_FOR_FIELD( overwrite,                    Overwrite )
			break;

		case 10:
			RESTINIO_HTTP_CHECK_FOR_FIELD( alternates,                   Alternates )
			RESTINIO_HTTP_CHECK_FOR_FIELD( c_pep_info,                   C-PEP-Info )
			RESTINIO_HTTP_CHECK_FOR_FIELD( content_id,                   Content-ID )
			RESTINIO_HTTP_CHECK_FOR_FIELD( delta_base,                   Delta-Base )
			RESTINIO_HTTP_CHECK_FOR_FIELD( getprofile,                   GetProfile )
			RESTINIO_HTTP_CHECK_FOR_FIELD( keep_alive,                   Keep-Alive )
			RESTINIO_HTTP_CHECK_FOR_FIELD( lock_token,                   Lock-Token )
			RESTINIO_HTTP_CHECK_FOR_FIELD( pics_label,                   PICS-Label )
			RESTINIO_HTTP_CHECK_FOR_FIELD( set_cookie,                   Set-Cookie )
			RESTINIO_HTTP_CHECK_FOR_FIELD( setprofile,                   SetProfile )
			RESTINIO_HTTP_CHECK_FOR_FIELD( soapaction,                   SoapAction )
			RESTINIO_HTTP_CHECK_FOR_FIELD( status_uri,                   Status-URI )
			RESTINIO_HTTP_CHECK_FOR_FIELD( user_agent,                   User-Agent )
			break;

		case 11:
			RESTINIO_HTTP_CHECK_FOR_FIELD( accept_post,                  Accept-Post )
			RESTINIO_HTTP_CHECK_FOR_FIELD( content_md5,                  Content-MD5 )
			RESTINIO_HTTP_CHECK_FOR_FIELD( destination,                  Destination )
			RESTINIO_HTTP_CHECK_FOR_FIELD( retry_after,                  Retry-After )
			RESTINIO_HTTP_CHECK_FOR_FIELD( set_cookie2,                  Set-Cookie2 )
			RESTINIO_HTTP_CHECK_FOR_FIELD( want_digest,                  Want-Digest )
			break;

		case 12:
			RESTINIO_HTTP_CHECK_FOR_FIELD( accept_patch,                 Accept-Patch )
			RESTINIO_HTTP_CHECK_FOR_FIELD( content_base,                 Content-Base )
			RESTINIO_HTTP_CHECK_FOR_FIELD( content_type,                 Content-Type )
			RESTINIO_HTTP_CHECK_FOR_FIELD( derived_from,                 Derived-From )
			RESTINIO_HTTP_CHECK_FOR_FIELD( max_forwards,                 Max-Forwards )
			RESTINIO_HTTP_CHECK_FOR_FIELD( mime_version,                 MIME-Version )
			RESTINIO_HTTP_CHECK_FOR_FIELD( schedule_tag,                 Schedule-Tag )
			RESTINIO_HTTP_CHECK_FOR_FIELD( redirect_ref,                 Redirect-Ref )
			RESTINIO_HTTP_CHECK_FOR_FIELD( variant_vary,                 Variant-Vary )
			break;

		case 13:
			RESTINIO_HTTP_CHECK_FOR_FIELD( accept_ranges,                Accept-Ranges )
			RESTINIO_HTTP_CHECK_FOR_FIELD( authorization,                Authorization )
			RESTINIO_HTTP_CHECK_FOR_FIELD( cache_control,                Cache-Control )
			RESTINIO_HTTP_CHECK_FOR_FIELD( content_range,                Content-Range )
			RESTINIO_HTTP_CHECK_FOR_FIELD( default_style,                Default-Style )
			RESTINIO_HTTP_CHECK_FOR_FIELD( if_none_match,                If-None-Match )
			RESTINIO_HTTP_CHECK_FOR_FIELD( last_modified,                Last-Modified )
			RESTINIO_HTTP_CHECK_FOR_FIELD( ordering_type,                Ordering-Type )
			RESTINIO_HTTP_CHECK_FOR_FIELD( profileobject,                ProfileObject )
			RESTINIO_HTTP_CHECK_FOR_FIELD( protocol_info,                Protocol-Info )
			break;

		case 14:
			RESTINIO_HTTP_CHECK_FOR_FIELD( accept_charset,               Accept-Charset )
			RESTINIO_HTTP_CHECK_FOR_FIELD( http2_settings,               HTTP2-Settings )
			RESTINIO_HTTP_CHECK_FOR_FIELD( protocol_query,               Protocol-Query )
			RESTINIO_HTTP_CHECK_FOR_FIELD( proxy_features,               Proxy-Features )
			RESTINIO_HTTP_CHECK_FOR_FIELD( schedule_reply,               Schedule-Reply )
			break;

		case 15:
			RESTINIO_HTTP_CHECK_FOR_FIELD( accept_encoding,              Accept-Encoding )
			RESTINIO_HTTP_CHECK_FOR_FIELD( accept_features,              Accept-Features )
			RESTINIO_HTTP_CHECK_FOR_FIELD( accept_language,              Accept-Language )
			RESTINIO_HTTP_CHECK_FOR_FIELD( accept_datetime,              Accept-Datetime )
			RESTINIO_HTTP_CHECK_FOR_FIELD( content_version,              Content-Version )
			RESTINIO_HTTP_CHECK_FOR_FIELD( differential_id,              Differential-ID )
			RESTINIO_HTTP_CHECK_FOR_FIELD( public_key_pins,              Public-Key-Pins )
			RESTINIO_HTTP_CHECK_FOR_FIELD( security_scheme,              Security-Scheme )
			RESTINIO_HTTP_CHECK_FOR_FIELD( x_frame_options,              X-Frame-Options )
			break;

		case 16:
			RESTINIO_HTTP_CHECK_FOR_FIELD( accept_additions,             Accept-Additions )
			RESTINIO_HTTP_CHECK_FOR_FIELD( caldav_timezones,             CalDAV-Timezones )
			RESTINIO_HTTP_CHECK_FOR_FIELD( content_encoding,             Content-Encoding )
			RESTINIO_HTTP_CHECK_FOR_FIELD( content_language,             Content-Language )
			RESTINIO_HTTP_CHECK_FOR_FIELD( content_location,             Content-Location )
			RESTINIO_HTTP_CHECK_FOR_FIELD( memento_datetime,             Memento-Datetime )
			RESTINIO_HTTP_CHECK_FOR_FIELD( protocol_request,             Protocol-Request )
			RESTINIO_HTTP_CHECK_FOR_FIELD( www_authenticate,             WWW-Authenticate )
			break;

		case 17:
			RESTINIO_HTTP_CHECK_FOR_FIELD( if_modified_since,            If-Modified-Since )
			RESTINIO_HTTP_CHECK_FOR_FIELD( proxy_instruction,            Proxy-Instruction )
			RESTINIO_HTTP_CHECK_FOR_FIELD( sec_websocket_key,            Sec-WebSocket-Key )
			RESTINIO_HTTP_CHECK_FOR_FIELD( surrogate_control,            Surrogate-Control )
			RESTINIO_HTTP_CHECK_FOR_FIELD( transfer_encoding,            Transfer-Encoding )
			break;

		case 18:
			RESTINIO_HTTP_CHECK_FOR_FIELD( content_style_type,           Content-Style-Type )
			RESTINIO_HTTP_CHECK_FOR_FIELD( preference_applied,           Preference-Applied )
			RESTINIO_HTTP_CHECK_FOR_FIELD( proxy_authenticate,           Proxy-Authenticate )
			break;

		case 19:
			RESTINIO_HTTP_CHECK_FOR_FIELD( authentication_info,          Authentication-Info )
			RESTINIO_HTTP_CHECK_FOR_FIELD( content_disposition,          Content-Disposition )
			RESTINIO_HTTP_CHECK_FOR_FIELD( content_script_type,          Content-Script-Type )
			RESTINIO_HTTP_CHECK_FOR_FIELD( if_unmodified_since,          If-Unmodified-Since )
			RESTINIO_HTTP_CHECK_FOR_FIELD( proxy_authorization,          Proxy-Authorization )
			break;

		case 20:
			RESTINIO_HTTP_CHECK_FOR_FIELD( sec_websocket_accept,         Sec-WebSocket-Accept )
			RESTINIO_HTTP_CHECK_FOR_FIELD( surrogate_capability,         Surrogate-Capability )
			break;

		case 21:
			RESTINIO_HTTP_CHECK_FOR_FIELD( apply_to_redirect_ref,        Apply-To-Redirect-Ref )
			RESTINIO_HTTP_CHECK_FOR_FIELD( if_schedule_tag_match,        If-Schedule-Tag-Match )
			RESTINIO_HTTP_CHECK_FOR_FIELD( sec_websocket_version,        Sec-WebSocket-Version )
			break;

		case 22:
			RESTINIO_HTTP_CHECK_FOR_FIELD( authentication_control,       Authentication-Control )
			RESTINIO_HTTP_CHECK_FOR_FIELD( sec_websocket_protocol,       Sec-WebSocket-Protocol )
			break;

		case 24:
			RESTINIO_HTTP_CHECK_FOR_FIELD( sec_websocket_extensions,     Sec-WebSocket-Extensions )
			break;

		case 25:
			RESTINIO_HTTP_CHECK_FOR_FIELD( optional_www_authenticate,    Optional-WWW-Authenticate )
			RESTINIO_HTTP_CHECK_FOR_FIELD( proxy_authentication_info,    Proxy-Authentication-Info )
			RESTINIO_HTTP_CHECK_FOR_FIELD( strict_transport_security,    Strict-Transport-Security )
			break;

		case 27:
			RESTINIO_HTTP_CHECK_FOR_FIELD( public_key_pins_report_only,  Public-Key-Pins-Report-Only )
			break;
	}

#undef RESTINIO_HTTP_CHECK_FOR_FIELD

	return http_field_t::field_unspecified;
}

inline http_field_t
string_to_field( const char * field_name )
{
	return string_to_field( field_name, std::strlen( field_name ) );
}

inline http_field_t
string_to_field( const std::string & field_name )
{
	return string_to_field( field_name.data(), field_name.size() );
}
//! \}

//
// field_to_string()
//

//! Helper sunction to get method string name.
inline const char *
field_to_string( http_field_t f )
{
	const char * result = "";
	switch( f )
	{
		#define RESTINIO_HTTP_FIELD_STR_GEN( name, string_name ) \
			case http_field_t::name: result = #string_name; break;

			RESTINIO_HTTP_FIELDS_MAP( RESTINIO_HTTP_FIELD_STR_GEN )
		#undef RESTINIO_HTTP_FIELD_STR_GEN

		case http_field_t::field_unspecified: break; // Ignore.
	};

	return result;
}

//
// http_header_field_t
//

//! A single header field.
/*!
	Fields m_name and m_field_id are kind of having the same meaning,
	and m_name field seems like can be omitted, but
	for the cases of custom header fields it is important to
	rely on the name only. And as the names of almoust all speified fields
	fits in SSO it doesn't involve much overhead on standard fields.
*/
struct http_header_field_t
{
	http_header_field_t()
		:	m_field_id{ http_field_t::field_unspecified }
	{}

	http_header_field_t(
		std::string name,
		std::string value )
		:	m_name{ std::move( name ) }
		,	m_value{ std::move( value ) }
		,	m_field_id{ string_to_field( m_name ) }
	{}

	http_header_field_t(
		http_field_t field_id,
		std::string value )
		:	m_name{ field_to_string( field_id ) }
		,	m_value{ std::move( value ) }
		,	m_field_id{ field_id }
	{}

	std::string m_name;
	std::string m_value;
	http_field_t m_field_id;
};

// Make neccessary forward declarations.
class http_header_fields_t;
namespace impl
{
void
append_last_field_accessor( http_header_fields_t &, const std::string & );
} /* namespace impl */

//
// http_header_fields_t
//

//! Header fields map.
/*!
	This class holds a collection of header fields.

	There are 2 special cases for fields: `Connection` and `Content-Length`
	This cases are handled separetely from the rest of the fields.
	And as the implementation of http_header_fields_t doesn't
	have checks on each field manipulation checking whether
	field name is `Connection` or `Content-Length` it is important
	to use proper member functions in derived classes for manipulating them.
*/
class http_header_fields_t
{
		friend void
		impl::append_last_field_accessor( http_header_fields_t &, const std::string & );

	public:
		using fields_container_t = std::vector< http_header_field_t >;

		http_header_fields_t() = default;
		http_header_fields_t(const http_header_fields_t &) = default;
		http_header_fields_t(http_header_fields_t &&) = default;
		virtual ~http_header_fields_t() {}

		http_header_fields_t & operator=(const http_header_fields_t &) = default;
		http_header_fields_t & operator=(http_header_fields_t &&) = default;

		//! Chack field by name.
		bool
		has_field( const std::string & field_name ) const
		{
			return m_fields.cend() != cfind( field_name );
		}

		//! Chack field by field-id.
		/*!
			\note If `field_id=http_field_t::field_unspecified`
			then function returns not more than just a fact
			whether there is at least one unspecified field.
		*/
		bool
		has_field( http_field_t field_id ) const
		{
			return m_fields.cend() != cfind( field_id );
		}

		//! Set field with string pair.
		void
		set_field(
			std::string field_name,
			std::string field_value )
		{
			const auto it = find( field_name );

			if( m_fields.end() != it )
			{
				it->m_name = std::move( field_name );
				it->m_value = std::move( field_value );
			}
			else
			{
				m_fields.emplace_back(
					std::move( field_name ),
					std::move( field_value ) );
			}
		}

		//! Set field with id-value pair.
		/*!
			If `field_id=http_field_t::field_unspecified`
			then function does nothing.
		*/
		void
		set_field(
			http_field_t field_id,
			std::string field_value )
		{
			if( http_field_t::field_unspecified != field_id )
			{
				const auto it = find( field_id );

				if( m_fields.end() != it )
				{
					it->m_value = std::move( field_value );
				}
				else
				{
					m_fields.emplace_back(
						field_id,
						std::move( field_value ) );
				}
			}
		}

		//! Append field with name.
		void
		append_field(
			const std::string & field_name,
			const std::string & field_value )
		{
			const auto it = find( field_name );

			if( m_fields.end() != it )
			{
				it->m_value.append( field_value );
			}
			else
			{
				m_fields.emplace_back( field_name, field_value );
			}
		}

		//! Append field with id.
		/*!
			If `field_id=http_field_t::field_unspecified`
			then function does nothing.
		*/
		void
		append_field(
			http_field_t field_id,
			const std::string & field_value )
		{
			if( http_field_t::field_unspecified != field_id )
			{
				const auto it = find( field_id );

				if( m_fields.end() != it )
				{
					it->m_value.append( field_value );
				}
				else
				{
					m_fields.emplace_back( field_id, field_value );
				}
			}
		}

		//! Get field by name.
		const std::string &
		get_field(
			const std::string & field_name ) const
		{
			const auto it = cfind( field_name );

			if( m_fields.end() == it )
				throw exception_t{
					fmt::format( "field '{}' doesn't exist", field_name ) };

			return it->m_value;
		}

		//! Get field by id.
		const std::string &
		get_field(
			http_field_t field_id ) const
		{
			if( http_field_t::field_unspecified == field_id )
				throw exception_t{
					fmt::format(
						"unspecified fields cannot be searched by id" ) };

			const auto it = cfind( field_id );

			if( m_fields.end() == it )
				throw exception_t{
					fmt::format(
						"field '{}' doesn't exist",
						field_to_string( field_id ) ) };

			return it->m_value;
		}

		//! Get field by name.
		/*!
			If field exists return field value, otherwise return default_value.
		*/
		const std::string &
		get_field(
			const std::string & field_name,
			const std::string & default_value ) const
		{
			const auto it = cfind( field_name );

			if( m_fields.end() == it )
				return default_value;

			return it->m_value;
		}

		//! Get field by id.
		/*!
			If field exists return field value, otherwise return default_value.
		*/
		const std::string &
		get_field(
			http_field_t field_id,
			const std::string & default_value ) const
		{
			if( http_field_t::field_unspecified != field_id )
			{
				const auto it = cfind( field_id );

				if( m_fields.end() != it )
					return it->m_value;
			}

			return default_value;
		}

		//! Remove field by name.
		void
		remove_field( const std::string & field_name )
		{
			const auto it = find( field_name );

			if( m_fields.end() != it )
				m_fields.erase( it );
		}

		//! Remove field by id.
		void
		remove_field( http_field_t field_id )
		{
			if( http_field_t::field_unspecified != field_id )
			{
				const auto it = find( field_id );

				if( m_fields.end() != it )
					m_fields.erase( it );
			}
		}

		auto
		begin() const
		{
			return m_fields.cbegin();
		}

		auto
		end() const
		{
			return m_fields.cend();
		}

		auto
		fields_count() const
		{
			return m_fields.size();
		}

	private:
		//! Appends last added field.
		/*!
			This is function is used by http-parser when
			field value is created by 2 separate
			invocation of on-header-field-value callback

			Function doesn't check if at least one field exists,
			so it is not in the public interface.
		*/
		void
		append_last_field( const std::string & field_value )
		{
			m_fields.back().m_value.append( field_value );
		}

		fields_container_t::iterator
		find( const std::string & field_name )
		{
			return std::find_if(
				m_fields.begin(),
				m_fields.end(),
				[&]( const auto & f ){
					return caseless_cmp( f.m_name, field_name );
				} );
		}

		fields_container_t::const_iterator
		cfind( const std::string & field_name ) const
		{
			return std::find_if(
				m_fields.cbegin(),
				m_fields.cend(),
				[&]( const auto & f ){
					return caseless_cmp( f.m_name, field_name );
				} );
		}

		fields_container_t::iterator
		find( http_field_t field_id )
		{
			return std::find_if(
				m_fields.begin(),
				m_fields.end(),
				[&]( const auto & f ){
					return f.m_field_id == field_id;
				} );
		}

		fields_container_t::const_iterator
		cfind( http_field_t field_id ) const
		{
			return std::find_if(
				m_fields.cbegin(),
				m_fields.cend(),
				[&]( const auto & f ){
					return f.m_field_id == field_id;
				} );
		}

		fields_container_t m_fields;
};

//
// http_header_common_t
//

//! Req/Resp headers common data.
struct http_header_common_t
	:	public http_header_fields_t
{
	public:
		//! Http version.
		//! \{
		std::uint16_t
		http_major() const
		{ return m_http_major; }

		void
		http_major( std::uint16_t v )
		{ m_http_major = v; }

		std::uint16_t
		http_minor() const
		{ return m_http_minor; }

		void
		http_minor( std::uint16_t v )
		{ m_http_minor = v; }
		//! \}

		//! Length of body of an http-message.
		std::uint64_t
		content_length() const
		{ return m_content_length; }

		void
		content_length( std::uint64_t l )
		{ m_content_length = l; }

		bool
		should_keep_alive() const
		{
			return m_should_keep_alive;
		}

		void
		should_keep_alive( bool keep_alive )
		{ m_should_keep_alive = keep_alive; }

	private:
		//! Http version.
		//! \{
		std::uint16_t m_http_major{1};
		std::uint16_t m_http_minor{1};
		//! \}

		//! Length of body of an http-message.
		std::uint64_t m_content_length{ 0 };

		bool m_should_keep_alive{ false };
};

//! HTTP methods mapping with nodejs http methods
#define RESTINIO_HTTP_METHOD_MAP(RESTINIO_GEN)         \
	RESTINIO_GEN( http_delete,      http_method_delete,       DELETE )       \
	RESTINIO_GEN( http_get,         http_method_get,          GET )          \
	RESTINIO_GEN( http_head,        http_method_head,         HEAD )         \
	RESTINIO_GEN( http_post,        http_method_post,         POST )         \
	RESTINIO_GEN( http_put,         http_method_put,          PUT )          \
  /* pathological */                \
	RESTINIO_GEN( http_connect,     http_method_connect,      CONNECT )      \
	RESTINIO_GEN( http_options,     http_method_options,      OPTIONS )      \
	RESTINIO_GEN( http_trace,       http_method_trace,        TRACE )        \
  /* WebDAV */                      \
	RESTINIO_GEN( http_copy,        http_method_copy,         COPY )         \
	RESTINIO_GEN( http_lock,        http_method_lock,         LOCK )         \
	RESTINIO_GEN( http_mkcol,       http_method_mkcol,        MKCOL )        \
	RESTINIO_GEN( http_move,        http_method_move,         MOVE )         \
	RESTINIO_GEN( http_propfind,    http_method_propfind,     PROPFIND )     \
	RESTINIO_GEN( http_proppatch,   http_method_proppatch,    PROPPATCH )    \
	RESTINIO_GEN( http_search,      http_method_search,       SEARCH )       \
	RESTINIO_GEN( http_unlock,      http_method_unlock,       UNLOCK )       \
	RESTINIO_GEN( http_bind,        http_method_bind, BIND )         \
	RESTINIO_GEN( http_rebind,      http_method_rebind,       REBIND )       \
	RESTINIO_GEN( http_unbind,      http_method_unbind,       UNBIND )       \
	RESTINIO_GEN( http_acl,         http_method_acl,          ACL )          \
  /* subversion */                  \
	RESTINIO_GEN( http_report,      http_method_report,       REPORT )       \
	RESTINIO_GEN( http_mkactivity,  http_method_mkactivity,   MKACTIVITY )   \
	RESTINIO_GEN( http_checkout,    http_method_checkout,     CHECKOUT )     \
	RESTINIO_GEN( http_merge,       http_method_merge,        MERGE )        \
  /* upnp */                        \
	RESTINIO_GEN( http_msearch,     http_method_msearch,      M-SEARCH)      \
	RESTINIO_GEN( http_notify,      http_method_notify,       NOTIFY )       \
	RESTINIO_GEN( http_subscribe,   http_method_subscribe,    SUBSCRIBE )    \
	RESTINIO_GEN( http_unsubscribe, http_method_unsubscribe,  UNSUBSCRIBE )  \
  /* RFC-5789 */                    \
	RESTINIO_GEN( http_patch,       http_method_patch,        PATCH )        \
	RESTINIO_GEN( http_purge,       http_method_purge,        PURGE )        \
  /* CalDAV */                      \
	RESTINIO_GEN( http_mkcalendar,  http_method_mkcalendar,   MKCALENDAR )   \
  /* RFC-2068, section 19.6.1.2 */  \
	RESTINIO_GEN( http_link,        http_method_link,         LINK )         \
	RESTINIO_GEN( http_unlink,      http_method_unlink,       UNLINK )       \

//
// http_method_t
//

//! C++ enum that repeats nodejs c-style enum.
enum class http_method_t : std::uint8_t
{
#define RESTINIO_HTTP_METHOD_GEN( name, ignored1, ignored2 ) name,
	RESTINIO_HTTP_METHOD_MAP( RESTINIO_HTTP_METHOD_GEN )
#undef RESTINIO_HTTP_METHOD_GEN
	// Unknown method.
	http_unknown
};

// Generate helper funcs.
#define RESTINIO_HTTP_METHOD_FUNC_GEN( name, func_name, ignored ) \
	constexpr auto func_name(){ return http_method_t::name; }

	RESTINIO_HTTP_METHOD_MAP( RESTINIO_HTTP_METHOD_FUNC_GEN )
#undef RESTINIO_HTTP_METHOD_FUNC_GEN

constexpr auto
http_method_unknown()
{
	return http_method_t::http_unknown;
}

//
// http_method_from_nodejs()
//

//! Map nodejs http method to http_method_t.
inline http_method_t
http_method_from_nodejs( int m )
{
	auto method = http_method_t::http_unknown;

	constexpr http_method_t method_maping[] =
	{
		#define RESTINIO_HTTP_METHOD_BY_ID_GEN( restinio_name, ignored1, ignored2 ) \
			http_method_t::restinio_name,

			RESTINIO_HTTP_METHOD_MAP( RESTINIO_HTTP_METHOD_BY_ID_GEN )
		#undef RESTINIO_HTTP_METHOD_BY_ID_GEN
		http_method_t::http_unknown
	};

	if( m >= 0 &&
		std::distance(std::begin(method_maping), std::end(method_maping)) > m)
	{
		method = method_maping[ m ];
	}

	return method;
}

//
// method_to_string()
//

//! Helper sunction to get method string name.
inline const char *
method_to_string( http_method_t m )
{
	const char * result = "<unknown>";
	switch( m )
	{
		#define RESTINIO_HTTP_METHOD_STR_GEN( name, ignored2, str ) \
			case http_method_t::name: result = #str; break;

			RESTINIO_HTTP_METHOD_MAP( RESTINIO_HTTP_METHOD_STR_GEN )
		#undef RESTINIO_HTTP_METHOD_STR_GEN

		case http_method_t::http_unknown: break; // Ignore.
	};

	return result;
}

//
// http_request_header
//

//! Req header.
struct http_request_header_t final
	:	public http_header_common_t
{
	public:
		http_request_header_t() = default;

		http_request_header_t(
			http_method_t method,
			std::string request_target )
			:	m_method{ method }
			,	m_request_target{ std::move( request_target ) }
		{}

		http_method_t
		method() const
		{ return m_method; }

		void
		method( http_method_t m )
		{ m_method = m; }

		const std::string &
		request_target() const
		{ return m_request_target; }

		void
		request_target( std::string t )
		{ m_request_target.assign( std::move( t ) ); }

		//! Helpfull function for using in parser callback.
		void
		append_request_target( const char * at, size_t length )
		{ m_request_target.append( at, length ); }

	private:
		http_method_t m_method{ http_method_get() };
		std::string m_request_target;
};

//
// http_response_header_t
//

//! Resp header.
struct http_response_header_t final
	:	public http_header_common_t
{
	public:
		http_response_header_t()
		{}

		http_response_header_t(
			std::uint16_t status_code,
			std::string reason_phrase )
			:	m_status_code{ status_code }
			,	m_reason_phrase{ std::move( reason_phrase ) }
		{}

		std::uint16_t
		status_code() const
		{ return m_status_code; }

		void
		status_code( std::uint16_t c )
		{ m_status_code = c; }

		const std::string &
		reason_phrase() const
		{ return m_reason_phrase; }

		void
		reason_phrase( std::string r )
		{ m_reason_phrase.assign( std::move( r ) ); }

	private:
		std::uint16_t m_status_code{ 200 };
		std::string m_reason_phrase;
};

} /* namespace restinio */
