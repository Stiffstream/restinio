/*
	restinio
*/

/*!
	helpers for http communication.
*/

#pragma once

#include <restinio/impl/include_fmtlib.hpp>

#include <restinio/impl/string_caseless_compare.hpp>

#include <restinio/exception.hpp>
#include <restinio/string_view.hpp>
#include <restinio/optional.hpp>
#include <restinio/common_types.hpp>

#include <http_parser.h>

#include <iosfwd>
#include <string>
#include <vector>
#include <algorithm>

namespace restinio
{


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
	RESTINIO_GEN( x_frame_options,              X-Frame-Options )             \
\
	RESTINIO_GEN( access_control,               Access-Control )              \
	RESTINIO_GEN( access_control_allow_credentials, Access-Control-Allow-Credentials ) \
	RESTINIO_GEN( access_control_allow_headers, Access-Control-Allow-Headers )\
	RESTINIO_GEN( access_control_allow_methods, Access-Control-Allow-Methods )\
	RESTINIO_GEN( access_control_allow_origin,  Access-Control-Allow-Origin ) \
	RESTINIO_GEN( access_control_max_age,       Access-Control-Max-Age )      \
	RESTINIO_GEN( access_control_request_method,    Access-Control-Request-Method )    \
	RESTINIO_GEN( access_control_request_headers,   Access-Control-Request-Headers )   \
	RESTINIO_GEN( compliance,                   Compliance )                  \
	RESTINIO_GEN( content_transfer_encoding,    Content-Transfer-Encoding )   \
	RESTINIO_GEN( cost,                         Cost )                        \
	RESTINIO_GEN( ediint_features,              EDIINT-Features )             \
	RESTINIO_GEN( message_id,                   Message-ID )                  \
	RESTINIO_GEN( method_check,                 Method-Check )                \
	RESTINIO_GEN( method_check_expires,         Method-Check-Expires )        \
	RESTINIO_GEN( non_compliance,               Non-Compliance )              \
	RESTINIO_GEN( optional,                     Optional )                    \
	RESTINIO_GEN( referer_root,                 Referer-Root )                \
	RESTINIO_GEN( resolution_hint,              Resolution-Hint )             \
	RESTINIO_GEN( resolver_location,            Resolver-Location )           \
	RESTINIO_GEN( subok,                        SubOK )                       \
	RESTINIO_GEN( subst,                        Subst )                       \
	RESTINIO_GEN( title,                        Title )                       \
	RESTINIO_GEN( ua_color,                     UA-Color )                    \
	RESTINIO_GEN( ua_media,                     UA-Media )                    \
	RESTINIO_GEN( ua_pixels,                    UA-Pixels )                   \
	RESTINIO_GEN( ua_resolution,                UA-Resolution )               \
	RESTINIO_GEN( ua_windowpixels,              UA-Windowpixels )             \
	RESTINIO_GEN( version,                      Version )                     \
	RESTINIO_GEN( x_device_accept,              X-Device-Accept )             \
	RESTINIO_GEN( x_device_accept_charset,      X-Device-Accept-Charset )     \
	RESTINIO_GEN( x_device_accept_encoding,     X-Device-Accept-Encoding )    \
	RESTINIO_GEN( x_device_accept_language,     X-Device-Accept-Language )    \
	RESTINIO_GEN( x_device_user_agent,          X-Device-User-Agent )
	// SPECIAL CASE: RESTINIO_GEN( connection,                   Connection )
	// SPECIAL CASE: RESTINIO_GEN( content_length,               Content-Length )

//
// http_field_t
//

//! C++ enum that repeats nodejs c-style enum.
/*!
	\note Fields `Connection` and `Content-Length` are specieal cases,
	thus they are not present in the list.
*/
enum class http_field_t : std::uint8_t //By now 152 + 34 + 1 items fits to uint8_t
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
string_to_field( string_view_t field ) noexcept
{
	const char * field_name = field.data();
	const std::size_t field_name_size = field.size();

#define RESTINIO_HTTP_CHECK_FOR_FIELD( field_id, candidate_field_name ) \
	if( impl::is_equal_caseless(field_name, #candidate_field_name , field_name_size ) ) \
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
			// Known to be more used first:
			RESTINIO_HTTP_CHECK_FOR_FIELD( host,                         Host )

			RESTINIO_HTTP_CHECK_FOR_FIELD( a_im,                         A-IM )
			RESTINIO_HTTP_CHECK_FOR_FIELD( alpn,                         ALPN )
			RESTINIO_HTTP_CHECK_FOR_FIELD( dasl,                         DASL )
			RESTINIO_HTTP_CHECK_FOR_FIELD( date,                         Date )
			RESTINIO_HTTP_CHECK_FOR_FIELD( etag,                         ETag )
			RESTINIO_HTTP_CHECK_FOR_FIELD( from,                         From )
			RESTINIO_HTTP_CHECK_FOR_FIELD( link,                         Link )
			RESTINIO_HTTP_CHECK_FOR_FIELD( safe,                         Safe )
			RESTINIO_HTTP_CHECK_FOR_FIELD( slug,                         SLUG )
			RESTINIO_HTTP_CHECK_FOR_FIELD( vary,                         Vary )
			RESTINIO_HTTP_CHECK_FOR_FIELD( cost,                         Cost )
			break;

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
			RESTINIO_HTTP_CHECK_FOR_FIELD( subok,                        SubOK )
			RESTINIO_HTTP_CHECK_FOR_FIELD( subst,                        Subst )
			RESTINIO_HTTP_CHECK_FOR_FIELD( title,                        Title )
			break;

		case 6:
			// Known to be more used first:
			RESTINIO_HTTP_CHECK_FOR_FIELD( accept,                       Accept )
			RESTINIO_HTTP_CHECK_FOR_FIELD( cookie,                       Cookie )
			RESTINIO_HTTP_CHECK_FOR_FIELD( server,                       Server )

			RESTINIO_HTTP_CHECK_FOR_FIELD( digest,                       Digest )
			RESTINIO_HTTP_CHECK_FOR_FIELD( expect,                       Expect )
			RESTINIO_HTTP_CHECK_FOR_FIELD( origin,                       Origin )
			RESTINIO_HTTP_CHECK_FOR_FIELD( pragma,                       Pragma )
			RESTINIO_HTTP_CHECK_FOR_FIELD( prefer,                       Prefer )
			RESTINIO_HTTP_CHECK_FOR_FIELD( public_,                      Public )
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
			RESTINIO_HTTP_CHECK_FOR_FIELD( version,                      Version )
			break;

		case 8:
			RESTINIO_HTTP_CHECK_FOR_FIELD( alt_used,                     Alt-Used )
			RESTINIO_HTTP_CHECK_FOR_FIELD( if_match,                     If-Match )
			RESTINIO_HTTP_CHECK_FOR_FIELD( if_range,                     If-Range )
			RESTINIO_HTTP_CHECK_FOR_FIELD( location,                     Location )
			RESTINIO_HTTP_CHECK_FOR_FIELD( pep_info,                     Pep-Info )
			RESTINIO_HTTP_CHECK_FOR_FIELD( position,                     Position )
			RESTINIO_HTTP_CHECK_FOR_FIELD( protocol,                     Protocol )
			RESTINIO_HTTP_CHECK_FOR_FIELD( optional,                     Optional )
			RESTINIO_HTTP_CHECK_FOR_FIELD( ua_color,                     UA-Color )
			RESTINIO_HTTP_CHECK_FOR_FIELD( ua_media,                     UA-Media )
			break;

		case 9:
			RESTINIO_HTTP_CHECK_FOR_FIELD( forwarded,                    Forwarded )
			RESTINIO_HTTP_CHECK_FOR_FIELD( negotiate,                    Negotiate )
			RESTINIO_HTTP_CHECK_FOR_FIELD( overwrite,                    Overwrite )
			RESTINIO_HTTP_CHECK_FOR_FIELD( ua_pixels,                    UA-Pixels )
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
			RESTINIO_HTTP_CHECK_FOR_FIELD( compliance,                   Compliance )
			RESTINIO_HTTP_CHECK_FOR_FIELD( message_id,                   Message-ID )
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
			// Known to be more used first:
			RESTINIO_HTTP_CHECK_FOR_FIELD( content_type,                 Content-Type )

			RESTINIO_HTTP_CHECK_FOR_FIELD( accept_patch,                 Accept-Patch )
			RESTINIO_HTTP_CHECK_FOR_FIELD( content_base,                 Content-Base )
			RESTINIO_HTTP_CHECK_FOR_FIELD( derived_from,                 Derived-From )
			RESTINIO_HTTP_CHECK_FOR_FIELD( max_forwards,                 Max-Forwards )
			RESTINIO_HTTP_CHECK_FOR_FIELD( mime_version,                 MIME-Version )
			RESTINIO_HTTP_CHECK_FOR_FIELD( schedule_tag,                 Schedule-Tag )
			RESTINIO_HTTP_CHECK_FOR_FIELD( redirect_ref,                 Redirect-Ref )
			RESTINIO_HTTP_CHECK_FOR_FIELD( variant_vary,                 Variant-Vary )
			RESTINIO_HTTP_CHECK_FOR_FIELD( method_check,                 Method-Check )
			RESTINIO_HTTP_CHECK_FOR_FIELD( referer_root,                 Referer-Root )
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
			RESTINIO_HTTP_CHECK_FOR_FIELD( ua_resolution,                UA-Resolution )
			break;

		case 14:
			RESTINIO_HTTP_CHECK_FOR_FIELD( accept_charset,               Accept-Charset )
			RESTINIO_HTTP_CHECK_FOR_FIELD( http2_settings,               HTTP2-Settings )
			RESTINIO_HTTP_CHECK_FOR_FIELD( protocol_query,               Protocol-Query )
			RESTINIO_HTTP_CHECK_FOR_FIELD( proxy_features,               Proxy-Features )
			RESTINIO_HTTP_CHECK_FOR_FIELD( schedule_reply,               Schedule-Reply )
			RESTINIO_HTTP_CHECK_FOR_FIELD( non_compliance,               Non-Compliance )
			RESTINIO_HTTP_CHECK_FOR_FIELD( access_control,               Access-Control )
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
			RESTINIO_HTTP_CHECK_FOR_FIELD( x_device_accept,              X-Device-Accept )
			RESTINIO_HTTP_CHECK_FOR_FIELD( resolution_hint,              Resolution-Hint )
			RESTINIO_HTTP_CHECK_FOR_FIELD( ediint_features,              EDIINT-Features )
			RESTINIO_HTTP_CHECK_FOR_FIELD( ua_windowpixels,              UA-Windowpixels )
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
			RESTINIO_HTTP_CHECK_FOR_FIELD( resolver_location,            Resolver-Location )
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
			RESTINIO_HTTP_CHECK_FOR_FIELD( x_device_user_agent,          X-Device-User-Agent )
			break;

		case 20:
			RESTINIO_HTTP_CHECK_FOR_FIELD( sec_websocket_accept,         Sec-WebSocket-Accept )
			RESTINIO_HTTP_CHECK_FOR_FIELD( surrogate_capability,         Surrogate-Capability )
			RESTINIO_HTTP_CHECK_FOR_FIELD( method_check_expires,         Method-Check-Expires )
			break;

		case 21:
			RESTINIO_HTTP_CHECK_FOR_FIELD( apply_to_redirect_ref,        Apply-To-Redirect-Ref )
			RESTINIO_HTTP_CHECK_FOR_FIELD( if_schedule_tag_match,        If-Schedule-Tag-Match )
			RESTINIO_HTTP_CHECK_FOR_FIELD( sec_websocket_version,        Sec-WebSocket-Version )
			break;

		case 22:
			RESTINIO_HTTP_CHECK_FOR_FIELD( authentication_control,       Authentication-Control )
			RESTINIO_HTTP_CHECK_FOR_FIELD( sec_websocket_protocol,       Sec-WebSocket-Protocol )
			RESTINIO_HTTP_CHECK_FOR_FIELD( access_control_max_age,       Access-Control-Max-Age )
			break;

		case 23:
			RESTINIO_HTTP_CHECK_FOR_FIELD( x_device_accept_charset,      X-Device-Accept-Charset )
			break;

		case 24:
			RESTINIO_HTTP_CHECK_FOR_FIELD( sec_websocket_extensions,     Sec-WebSocket-Extensions )
			RESTINIO_HTTP_CHECK_FOR_FIELD( x_device_accept_encoding,     X-Device-Accept-Encoding )
			RESTINIO_HTTP_CHECK_FOR_FIELD( x_device_accept_language,     X-Device-Accept-Language )
			break;

		case 25:
			RESTINIO_HTTP_CHECK_FOR_FIELD( optional_www_authenticate,    Optional-WWW-Authenticate )
			RESTINIO_HTTP_CHECK_FOR_FIELD( proxy_authentication_info,    Proxy-Authentication-Info )
			RESTINIO_HTTP_CHECK_FOR_FIELD( strict_transport_security,    Strict-Transport-Security )
			RESTINIO_HTTP_CHECK_FOR_FIELD( content_transfer_encoding,    Content-Transfer-Encoding )
			break;

		case 27:
			RESTINIO_HTTP_CHECK_FOR_FIELD( public_key_pins_report_only,  Public-Key-Pins-Report-Only )
			RESTINIO_HTTP_CHECK_FOR_FIELD( access_control_allow_origin,  Access-Control-Allow-Origin )
			break;

		case 28:
			RESTINIO_HTTP_CHECK_FOR_FIELD( access_control_allow_headers, Access-Control-Allow-Headers )
			RESTINIO_HTTP_CHECK_FOR_FIELD( access_control_allow_methods, Access-Control-Allow-Methods )
			break;

		case 29:
			RESTINIO_HTTP_CHECK_FOR_FIELD( access_control_request_method,    Access-Control-Request-Method )
			break;

		case 30:
			RESTINIO_HTTP_CHECK_FOR_FIELD( access_control_request_headers,   Access-Control-Request-Headers )
			break;

		case 32:
			RESTINIO_HTTP_CHECK_FOR_FIELD( access_control_allow_credentials, Access-Control-Allow-Credentials )
			break;
	}

#undef RESTINIO_HTTP_CHECK_FOR_FIELD

	return http_field_t::field_unspecified;
}

//
// field_to_string()
//

//! Helper sunction to get method string name.
inline const char *
field_to_string( http_field_t f ) noexcept
{
	const char * result = "";
	switch( f )
	{
		#define RESTINIO_HTTP_FIELD_STR_GEN( name, string_name ) \
			case http_field_t::name: result = #string_name; break;

			RESTINIO_HTTP_FIELDS_MAP( RESTINIO_HTTP_FIELD_STR_GEN )
		#undef RESTINIO_HTTP_FIELD_STR_GEN

		case http_field_t::field_unspecified: break; // Ignore.
	}

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
class http_header_field_t
{
	public:
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
			string_view_t name,
			string_view_t value )
			:	m_name{ name.data(), name.size() }
			,	m_value{ value.data(), value.size() }
			,	m_field_id{ string_to_field( m_name ) }
		{}

		http_header_field_t(
			http_field_t field_id,
			std::string value )
			:	m_name{ field_to_string( field_id ) }
			,	m_value{ std::move( value ) }
			,	m_field_id{ field_id }
		{}

		http_header_field_t(
			http_field_t field_id,
			string_view_t value )
			:	m_name{ field_to_string( field_id ) }
			,	m_value{ std::move( value ) }
			,	m_field_id{ field_id }
		{}

		const std::string & name() const noexcept { return m_name; }
		const std::string & value() const noexcept { return m_value; }
		http_field_t field_id() const noexcept { return m_field_id; }

		void
		name( std::string n )
		{
			m_name = std::move( n );
			m_field_id = string_to_field( m_name );
		}

		void
		value( std::string v )
		{
			m_value = std::move( v );
		}

		void
		append_value( string_view_t v )
		{
			m_value.append( v.data(), v.size() );
		}

		void
		field_id( http_field_t field_id )
		{
			m_field_id = field_id;
			m_name = field_to_string( m_field_id );
		}

	private:
		std::string m_name;
		std::string m_value;
		http_field_t m_field_id;
};

// Make neccessary forward declarations.
class http_header_fields_t;
namespace impl
{

void
append_last_field_accessor( http_header_fields_t &, string_view_t );

} /* namespace impl */

#if !defined( RESTINIO_HEADER_FIELDS_DEFAULT_RESERVE_COUNT )
	#define RESTINIO_HEADER_FIELDS_DEFAULT_RESERVE_COUNT 4
#endif

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

	@par Getting values of fields

	Since v.0.4.9 there are two groups of methods for accessing values of
	fields. The first group returns `std::string` (or references/pointers
	to `std::string`). This group includes the following methods: get_field(),
	get_field_or(), try_get_field().

	The second group returns `string_view_t` or `optional_t<string_view_t>`.
	This group includes the following methods: value_of() and opt_value_of().

	The first group was created in early versions of RESTinio and is present
	here for historical and compatibility reasons. They are not deprecated
	yet but they could be deprecated in newer versions of RESTinio.
	Because of that the usage of value_of() and opt_value_of() is more
	preferable.
*/
class http_header_fields_t
{
		friend void
		impl::append_last_field_accessor( http_header_fields_t &, string_view_t );

	public:
		using fields_container_t = std::vector< http_header_field_t >;

		//! Type of const_iterator for enumeration of fields.
		using const_iterator = fields_container_t::const_iterator;

		http_header_fields_t()
		{
			m_fields.reserve( RESTINIO_HEADER_FIELDS_DEFAULT_RESERVE_COUNT );
		}
		http_header_fields_t(const http_header_fields_t &) = default;
		http_header_fields_t(http_header_fields_t &&) = default;
		virtual ~http_header_fields_t() {}

		http_header_fields_t & operator=(const http_header_fields_t &) = default;
		http_header_fields_t & operator=(http_header_fields_t &&) = default;

		void
		swap_fields( http_header_fields_t & http_header_fields )
		{
			std::swap( m_fields, http_header_fields.m_fields );
		}

		//! Check field by name.
		bool
		has_field( string_view_t field_name ) const noexcept
		{
			return m_fields.cend() != cfind( field_name );
		}

		//! Check field by field-id.
		/*!
			\note If `field_id=http_field_t::field_unspecified`
			then function returns not more than just a fact
			whether there is at least one unspecified field.
		*/
		bool
		has_field( http_field_t field_id ) const noexcept
		{
			return m_fields.cend() != cfind( field_id );
		}

		//! Set header field via http_header_field_t.
		void
		set_field( http_header_field_t http_header_field )
		{
			fields_container_t::iterator it;
			if( http_field_t::field_unspecified != http_header_field.field_id() )
			{
				// Field has a standard name.
				// Search it by id.
				it = find( http_header_field.field_id() );
			}
			else
			{
				// Field has a non standard name.
				// Search it by name.
				it = find( http_header_field.name() );
			}

			if( m_fields.end() != it )
			{
				*it = std::move( http_header_field );
			}
			else
			{
				m_fields.emplace_back( std::move( http_header_field ) );
			}
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
				it->name( std::move( field_name ) );
				it->value( std::move( field_value ) );
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
					it->value( std::move( field_value ) );
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
			string_view_t field_name,
			string_view_t field_value )
		{
			const auto it = find( field_name );

			if( m_fields.end() != it )
			{
				it->append_value( field_value );
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
			string_view_t field_value )
		{
			if( http_field_t::field_unspecified != field_id )
			{
				const auto it = find( field_id );

				if( m_fields.end() != it )
				{
					it->append_value( field_value );
				}
				else
				{
					m_fields.emplace_back( field_id, field_value );
				}
			}
		}

		//! Get field by name.
		const std::string &
		get_field( string_view_t field_name ) const
		{
			const auto it = cfind( field_name );

			if( m_fields.end() == it )
				throw exception_t{
					fmt::format( "field '{}' doesn't exist", field_name ) };

			return it->value();
		}

		//! Try to get the value of a field by field name.
		/*!
			@note
			Returns nullptr if the field is not found.

			Usage example:
			\code
			auto f = headers().try_get_field("Content-Type");
			if(f && *f == "text/plain")
				...
			\endcode
		*/
		nullable_pointer_t<const std::string>
		try_get_field( string_view_t field_name ) const noexcept
		{
			const auto it = cfind( field_name );
			if( m_fields.end() == it )
				return nullptr;
			else
				return std::addressof(it->value());
		}

		//! Get field by id.
		const std::string &
		get_field( http_field_t field_id ) const
		{
			if( http_field_t::field_unspecified == field_id )
			{
				throw exception_t{
					fmt::format(
						"unspecified fields cannot be searched by id" ) };
			}

			const auto it = cfind( field_id );

			if( m_fields.end() == it )
			{
				throw exception_t{
					fmt::format(
						"field '{}' doesn't exist",
						field_to_string( field_id ) ) };
			}

			return it->value();
		}

		//! Try to get the value of a field by field ID.
		/*!
			@note
			Returns nullptr if the field is not found.

			Usage example:
			\code
			auto f = headers().try_get_field(restinio::http_field::content_type);
			if(f && *f == "text/plain")
				...
			\endcode
		*/
		nullable_pointer_t<const std::string>
		try_get_field( http_field_t field_id ) const noexcept
		{
			if( http_field_t::field_unspecified != field_id )
			{
				const auto it = cfind( field_id );
				if( m_fields.end() != it )
					return std::addressof(it->value());
			}

			return nullptr;
		}

		//! Get field value by field name or default value if the field not found.
		/*!
			@note
			This method returns field value as a new std::string instance,
			not a const reference to std::string.
		*/
		std::string
		get_field_or(
			string_view_t field_name,
			string_view_t default_value ) const
		{
			const auto it = cfind( field_name );

			if( m_fields.end() == it )
				return std::string( default_value.data(), default_value.size() );

			return it->value();
		}

		//! Get field value by field name or default value if the field not found.
		/*!
			@note
			This method returns field value as a new std::string instance,
			not a const reference to std::string.
		*/
		std::string
		get_field_or(
			string_view_t field_name,
			std::string && default_value ) const
		{
			const auto it = cfind( field_name );

			if( m_fields.end() == it )
				return std::move(default_value);

			return it->value();
		}

		//! Get field by name or default value if the field not found.
		/*!
			This is just overload for get_field_or(string_view_t,string_view_t);
		*/
		auto
		get_field_or(
			string_view_t field_name,
			const char * default_value ) const
		{
			return this->get_field_or( field_name, string_view_t{ default_value } );
		}

		//! Get field by name or default value if the field not found.
		/*!
			This is just overload for get_field_or(string_view_t,string_view_t);
		*/
		auto
		get_field_or(
			string_view_t field_name,
			const std::string & default_value ) const
		{
			return this->get_field_or( field_name, string_view_t{ default_value } );
		}

		//! Get field by id or default value if the field not found.
		/*!
			@note
			This method returns field value as a new std::string instance,
			not a const reference to std::string.
		*/
		std::string
		get_field_or(
			http_field_t field_id,
			string_view_t default_value ) const
		{
			if( http_field_t::field_unspecified != field_id )
			{
				const auto it = cfind( field_id );

				if( m_fields.end() != it )
					return it->value();
			}

			return std::string( default_value.data(), default_value.size() );
		}

		//! Get field by id or default value if the field not found.
		/*!
			This is just overload for get_field_or(http_field_t,string_view_t);
		*/
		auto
		get_field_or(
			http_field_t field_id,
			const char * default_value ) const
		{
			return this->get_field_or( field_id, string_view_t{ default_value } );
		}

		//! Get field by id or default value if the field not found.
		/*!
			This is just overload for get_field_or(http_field_t,string_view_t);
		*/
		auto
		get_field_or(
			http_field_t field_id,
			const std::string & default_value ) const
		{
			return this->get_field_or( field_id, string_view_t{ default_value } );
		}

		//! Get field by id or default value if the field not found.
		/*!
			@note
			This method returns field value as a new std::string instance,
			not a const reference to std::string.
		*/
		std::string
		get_field_or(
			http_field_t field_id,
			std::string && default_value ) const
		{
			if( http_field_t::field_unspecified != field_id )
			{
				const auto it = cfind( field_id );

				if( m_fields.end() != it )
					return it->value();
			}

			return std::move( default_value );
		}

		//! Remove field by name.
		void
		remove_field( string_view_t field_name )
		{
			const auto it = find( field_name );

			if( m_fields.end() != it )
			{
				m_fields.erase( it );
			}
		}

		//! Remove field by id.
		void
		remove_field( http_field_t field_id )
		{
			if( http_field_t::field_unspecified != field_id )
			{
				const auto it = find( field_id );

				if( m_fields.end() != it )
				{
					m_fields.erase( it );
				}
			}
		}

		/*!
		 * @name Getters of field value which return string_view.
		 * @{
		 */
		//! Get the value of a field or throw if the field not found.
		string_view_t
		value_of(
			//! Name of a field.
			string_view_t name ) const
		{
			return { this->get_field(name) };
		}

		//! Get the value of a field or throw if the field not found.
		string_view_t
		value_of(
			//! ID of a field.
			http_field_t field_id ) const
		{
			return { this->get_field(field_id) };
		}

		//! Get optional value of a field.
		/*!
			Doesn't throw exception if the field is not found. Empty optional
			will be returned instead.

			Usage example:
			\code
			auto f = headers().opt_value_of("Content-Type");
			if(f && *f == "text/plain")
				...
			\endcode
		*/
		optional_t< string_view_t >
		opt_value_of(
			//! Name of a field.
			string_view_t name ) const noexcept
		{
			optional_t< string_view_t > result;

			if( auto * ptr = this->try_get_field(name) )
				result = string_view_t{ *ptr };

			return result;
		}

		//! Get optional value of a field.
		/*!
			Doesn't throw exception if the field is not found. Empty optional
			will be returned instead.

			Usage example:
			\code
			auto f = headers().opt_value_of(restinio::http_field::content_type);
			if(f && *f == "text/plain")
				...
			\endcode
		*/
		optional_t< string_view_t >
		opt_value_of(
			//! ID of a field.
			http_field_t field_id ) const noexcept
		{
			optional_t< string_view_t > result;

			if( auto * ptr = this->try_get_field(field_id) )
				result = string_view_t{ *ptr };

			return result;
		}
		/*!
		 * @}
		 */

		//! Enumeration of fields.
		/*!
			Calls \a lambda for each field in the container.

			Lambda should have one of the following formats:
			\code
			void(const http_header_field_t &);
			void(http_header_field_t);
			\endcode

			This method is `noexcept` if \a lambda is `noexcept`.

			Usage example:
			\code
			headers().for_each_field( [](const auto & f) {
				std::cout << f.name() << ": " << f.value() << std::endl;
			} );
			\endcode
		*/
		template< typename Lambda >
		void
		for_each_field( Lambda && lambda ) const
				noexcept(noexcept(lambda(
						std::declval<const http_header_field_t &>())))
		{
			for( const auto & f : m_fields )
				lambda( f );
		}

		const_iterator
		begin() const noexcept
		{
			return m_fields.cbegin();
		}

		const_iterator
		end() const noexcept
		{
			return m_fields.cend();
		}

		auto fields_count() const noexcept
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
		append_last_field( string_view_t field_value )
		{
			m_fields.back().append_value( field_value );
		}

		fields_container_t::iterator
		find( string_view_t field_name ) noexcept
		{
			return std::find_if(
				m_fields.begin(),
				m_fields.end(),
				[&]( const auto & f ){
					return impl::is_equal_caseless( f.name(), field_name );
				} );
		}

		fields_container_t::const_iterator
		cfind( string_view_t field_name ) const noexcept
		{
			return std::find_if(
				m_fields.cbegin(),
				m_fields.cend(),
				[&]( const auto & f ){
					return impl::is_equal_caseless( f.name(), field_name );
				} );
		}

		fields_container_t::iterator
		find( http_field_t field_id ) noexcept
		{
			return std::find_if(
				m_fields.begin(),
				m_fields.end(),
				[&]( const auto & f ){
					return f.field_id() == field_id;
				} );
		}

		fields_container_t::const_iterator
		cfind( http_field_t field_id ) const noexcept
		{
			return std::find_if(
				m_fields.cbegin(),
				m_fields.cend(),
				[&]( const auto & f ){
					return f.field_id() == field_id;
				} );
		}

		fields_container_t m_fields;
};

//
// http_connection_header_t
//

//! Values for conection header field.
enum class http_connection_header_t : std::uint8_t
{
	keep_alive,
	close,
	upgrade
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
		http_major() const noexcept
		{ return m_http_major; }

		void
		http_major( std::uint16_t v ) noexcept
		{ m_http_major = v; }

		std::uint16_t
		http_minor() const noexcept
		{ return m_http_minor; }

		void
		http_minor( std::uint16_t v ) noexcept
		{ m_http_minor = v; }
		//! \}

		//! Length of body of an http-message.
		std::uint64_t
		content_length() const noexcept
		{ return m_content_length; }

		void
		content_length( std::uint64_t l ) noexcept
		{ m_content_length = l; }

		bool
		should_keep_alive() const noexcept
		{
			return http_connection_header_t::keep_alive == m_http_connection_header_field_value;
		}

		void
		should_keep_alive( bool keep_alive ) noexcept
		{
			connection( keep_alive?
				http_connection_header_t::keep_alive :
				http_connection_header_t::close );
		}

		//! Get the value of 'connection' header field.
		http_connection_header_t
		connection() const
		{
			return m_http_connection_header_field_value;
		}

		//! Set the value of 'connection' header field.
		void
		connection( http_connection_header_t ch ) noexcept
		{
			m_http_connection_header_field_value = ch;
		}

	private:
		//! Http version.
		//! \{
		std::uint16_t m_http_major{1};
		std::uint16_t m_http_minor{1};
		//! \}

		//! Length of body of an http-message.
		std::uint64_t m_content_length{ 0 };

		http_connection_header_t m_http_connection_header_field_value{ http_connection_header_t::close };
};

//! HTTP methods mapping with nodejs http methods
#define RESTINIO_HTTP_METHOD_MAP(RESTINIO_GEN)         \
	RESTINIO_GEN( http_method_delete,       HTTP_DELETE,       DELETE )       \
	RESTINIO_GEN( http_method_get,          HTTP_GET,          GET )          \
	RESTINIO_GEN( http_method_head,         HTTP_HEAD,         HEAD )         \
	RESTINIO_GEN( http_method_post,         HTTP_POST,         POST )         \
	RESTINIO_GEN( http_method_put,          HTTP_PUT,          PUT )          \
  /* pathological */                \
	RESTINIO_GEN( http_method_connect,      HTTP_CONNECT,      CONNECT )      \
	RESTINIO_GEN( http_method_options,      HTTP_OPTIONS,      OPTIONS )      \
	RESTINIO_GEN( http_method_trace,        HTTP_TRACE,        TRACE )        \
  /* WebDAV */                      \
	RESTINIO_GEN( http_method_copy,         HTTP_COPY,         COPY )         \
	RESTINIO_GEN( http_method_lock,         HTTP_LOCK,         LOCK )         \
	RESTINIO_GEN( http_method_mkcol,        HTTP_MKCOL,        MKCOL )        \
	RESTINIO_GEN( http_method_move,         HTTP_MOVE,         MOVE )         \
	RESTINIO_GEN( http_method_propfind,     HTTP_PROPFIND,     PROPFIND )     \
	RESTINIO_GEN( http_method_proppatch,    HTTP_PROPPATCH,    PROPPATCH )    \
	RESTINIO_GEN( http_method_search,       HTTP_SEARCH,       SEARCH )       \
	RESTINIO_GEN( http_method_unlock,       HTTP_UNLOCK,       UNLOCK )       \
	RESTINIO_GEN( http_method_bind,         HTTP_BIND,         BIND )         \
	RESTINIO_GEN( http_method_rebind,       HTTP_REBIND,       REBIND )       \
	RESTINIO_GEN( http_method_unbind,       HTTP_UNBIND,       UNBIND )       \
	RESTINIO_GEN( http_method_acl,          HTTP_ACL,          ACL )          \
  /* subversion */                  \
	RESTINIO_GEN( http_method_report,       HTTP_REPORT,       REPORT )       \
	RESTINIO_GEN( http_method_mkactivity,   HTTP_MKACTIVITY,   MKACTIVITY )   \
	RESTINIO_GEN( http_method_checkout,     HTTP_CHECKOUT,     CHECKOUT )     \
	RESTINIO_GEN( http_method_merge,        HTTP_MERGE,        MERGE )        \
  /* upnp */                        \
	RESTINIO_GEN( http_method_msearch,      HTTP_MSEARCH,      M-SEARCH)      \
	RESTINIO_GEN( http_method_notify,       HTTP_NOTIFY,       NOTIFY )       \
	RESTINIO_GEN( http_method_subscribe,    HTTP_SUBSCRIBE,    SUBSCRIBE )    \
	RESTINIO_GEN( http_method_unsubscribe,  HTTP_UNSUBSCRIBE,  UNSUBSCRIBE )  \
  /* RFC-5789 */                    \
	RESTINIO_GEN( http_method_patch,        HTTP_PATCH,        PATCH )        \
	RESTINIO_GEN( http_method_purge,        HTTP_PURGE,        PURGE )        \
  /* CalDAV */                      \
	RESTINIO_GEN( http_method_mkcalendar,   HTTP_MKCALENDAR,   MKCALENDAR )   \
  /* RFC-2068, section 19.6.1.2 */  \
	RESTINIO_GEN( http_method_link,         HTTP_LINK,         LINK )         \
	RESTINIO_GEN( http_method_unlink,       HTTP_UNLINK,       UNLINK ) 

//
// http_method_id_t
//
/*!
 * @brief A type for representation of HTTP method ID.
 *
 * RESTinio uses http_parser for working with HTTP-protocol.
 * HTTP-methods in http_parser are identified by `int`s like
 * HTTP_GET, HTTP_POST and so on.
 *
 * Usage of plain `int` is error prone. So since v.0.5.0 RESTinio contain
 * type http_method_id_t as type for ID of HTTP method.
 *
 * An instance of http_method_id_t contains two values:
 * * integer identifier from http_parser (like HTTP_GET, HTTP_POST and so on);
 * * a string representation of HTTP method ID (like "GET", "POST", "DELETE"
 * and so on).
 *
 * There is an important requirement for user-defined HTTP method IDs:
 * a pointer to string representation of HTTP method ID must outlive
 * the instance of http_method_id_t. It means that is safe to use string
 * literals or static strings, for example:
 * @code
 * constexpr const restinio::http_method_id_t my_http_method(255, "MY-METHOD");
 * @endcode
 *
 * @note 
 * Instances of http_method_id_t can't be used in switch() operator.
 * For example, you can't write that way:
 * @code
 * const int method_id = ...;
 * switch(method_id) {
 * 	case restinio::http_method_get(): ...; break;
 * 	case restinio::http_method_post(): ...; break;
 * 	case restinio::http_method_delete(): ...; break;
 * }
 * @endcode
 * In that case raw_id() method can be used:
 * @code
 * const int method_id = ...;
 * switch(method_id) {
 * 	case restinio::http_method_get().raw_id(): ...; break;
 * 	case restinio::http_method_post().raw_id(): ...; break;
 * 	case restinio::http_method_delete().raw_id(): ...; break;
 * }
 * @endcode
 *
 * @since v.0.5.0
 */
class http_method_id_t
{
	int m_value;
	const char * m_name;

public:
	static constexpr const int unknown_method = -1;

	constexpr http_method_id_t() noexcept
		:	m_value{ unknown_method }
		,	m_name{ "<undefined>" }
		{}
	constexpr http_method_id_t(
		int value,
		const char * name ) noexcept
		:	m_value{ value }
		,	m_name{ name }
		{}

	constexpr http_method_id_t( const http_method_id_t & ) noexcept = default;
	constexpr http_method_id_t &
	operator=( const http_method_id_t & ) noexcept = default;

	constexpr http_method_id_t( http_method_id_t && ) noexcept = default;
	constexpr http_method_id_t &
	operator=( http_method_id_t && ) noexcept = default;

	constexpr auto
	raw_id() const noexcept { return m_value; }

	constexpr const char *
	c_str() const noexcept { return m_name; }

	friend constexpr bool
	operator==( const http_method_id_t & a, const http_method_id_t & b ) noexcept {
		return a.raw_id() == b.raw_id();
	}

	friend constexpr bool
	operator!=( const http_method_id_t & a, const http_method_id_t & b ) noexcept {
		return a.raw_id() != b.raw_id();
	}

	friend constexpr bool
	operator<( const http_method_id_t & a, const http_method_id_t & b ) noexcept {
		return a.raw_id() < b.raw_id();
	}
};

inline std::ostream &
operator<<( std::ostream & to, const http_method_id_t & m )
{
	return to << m.c_str();
}

// Generate helper funcs.
#define RESTINIO_HTTP_METHOD_FUNC_GEN( func_name, nodejs_code, method_name ) \
	inline constexpr http_method_id_t func_name() { \
		return { nodejs_code, #method_name }; \
	}

	RESTINIO_HTTP_METHOD_MAP( RESTINIO_HTTP_METHOD_FUNC_GEN )
#undef RESTINIO_HTTP_METHOD_FUNC_GEN

inline constexpr http_method_id_t
http_method_unknown()
{
	return http_method_id_t{};
}

//
// default_http_methods_t
//
/*!
 * @brief The default implementation for http_method_mapper.
 *
 * Since v.0.5.0 RESTinio allows to use modified versions of http_parser
 * libraries. Such modified versions can handle non-standard HTTP methods.
 * In that case a user should define its own http_method_mapper-type.
 * That http_method_mapper must contain static method from_nodejs for
 * mapping the http_parser's ID of HTTP method to an instance of
 * http_method_id_t.
 *
 * Class default_http_methods_t is the default implementation of
 * http_method_mapper-type for vanila version of http_parser.
 *
 * @since v.0.5.0
 */
class default_http_methods_t
{
public :
	inline static constexpr http_method_id_t
	from_nodejs( int value ) noexcept 
	{
		http_method_id_t result;
		switch( value )
		{
#define RESTINIO_HTTP_METHOD_FUNC_GEN( func_name, nodejs_code, method_name ) \
			case nodejs_code : result = func_name(); break;

	RESTINIO_HTTP_METHOD_MAP( RESTINIO_HTTP_METHOD_FUNC_GEN )
#undef RESTINIO_HTTP_METHOD_FUNC_GEN
			default : ; // Nothing to do.
		}

		return result;
	}
};

//
// http_request_header
//

//! Req header.
struct http_request_header_t final
	:	public http_header_common_t
{
		static std::size_t
		memchr_helper( int chr , const char * from, std::size_t size )
		{
			const char * result = static_cast< const char * >(
					std::memchr( from, chr, size ) );

			return result ? static_cast< std::size_t >( result - from ) : size;
		}

	public:
		http_request_header_t() = default;

		http_request_header_t(
			http_method_id_t method,
			std::string request_target_ )
			:	m_method{ method }
		{
			request_target( std::move( request_target_ ) );
		}

		http_method_id_t
		method() const noexcept
		{ return m_method; }

		void
		method( http_method_id_t m ) noexcept
		{ m_method = m; }

		const std::string &
		request_target() const noexcept
		{ return m_request_target; }

		void
		request_target( std::string t )
		{
			m_request_target.assign( std::move( t ) );

			m_fragment_separator_pos =
				memchr_helper( '#', m_request_target.data(), m_request_target.size() );

			m_query_separator_pos =
				memchr_helper( '?', m_request_target.data(), m_fragment_separator_pos );
		}

		//! Request URL-structure.
		//! \{

		//! Get the path part of the request URL.
		/*!
			If request target is `/weather/temperature?from=2012-01-01&to=2012-01-10`,
			then function returns string view on '/weather/temperature' part.
		*/
		string_view_t
		path() const noexcept
		{
			return string_view_t{ m_request_target.data(), m_query_separator_pos };
		}

		//! Get the query part of the request URL.
		/*!
			If request target is `/weather/temperature?from=2012-01-01&to=2012-01-10`,
			then function returns string view on 'from=2012-01-01&to=2012-01-10' part.
		*/
		string_view_t
		query() const noexcept
		{
			return
				m_fragment_separator_pos == m_query_separator_pos ?
				string_view_t{ nullptr, 0 } :
				string_view_t{
					m_request_target.data() + m_query_separator_pos + 1,
					m_fragment_separator_pos - m_query_separator_pos - 1 };
		}


		//! Get the fragment part of the request URL.
		/*!
			If request target is `/sobjectizerteam/json_dto-0.2#markdown-header-what-is-json_dto`,
			then function returns string view on 'markdown-header-what-is-json_dto' part.
		*/
		string_view_t
		fragment() const
		{
			return
				m_request_target.size() == m_fragment_separator_pos ?
				string_view_t{ nullptr, 0 } :
				string_view_t{
					m_request_target.data() + m_fragment_separator_pos + 1,
					m_request_target.size() - m_fragment_separator_pos - 1 };
		}
		//! \}

		//! Helpfull function for using in parser callback.
		void
		append_request_target( const char * at, size_t length )
		{
			if( m_request_target.size() == m_fragment_separator_pos )
			{
				// If fragment separator hadn't  already appeared,
				// search for it in a new block.

				const auto fragment_separator_pos_inc =
					memchr_helper( '#', at, length );

				m_fragment_separator_pos += fragment_separator_pos_inc;

				if( m_request_target.size() == m_query_separator_pos )
				{
					// If request separator hadn't already appeared,
					// search for it in a new block.
					m_query_separator_pos +=
						memchr_helper( '?', at, fragment_separator_pos_inc );
				}
			}
			// Else fragment separator appeared
			// (req separator is either already defined or does not exist)

			m_request_target.append( at, length );
		}

	private:
		http_method_id_t m_method{ http_method_get() };
		std::string m_request_target;
		std::size_t m_query_separator_pos{ 0 };
		std::size_t m_fragment_separator_pos{ 0 };
};

//
// http_status_code_t
//

//! A handy wrapper for HTTP response status code.
class http_status_code_t
{
	public:
		constexpr http_status_code_t() noexcept
		{}

		constexpr explicit http_status_code_t( std::uint16_t status_code ) noexcept
			:	m_status_code{ status_code }
		{}

		constexpr auto
		raw_code() const noexcept
		{
			return m_status_code;
		}

		constexpr bool
		operator == ( const http_status_code_t & sc ) const noexcept
		{
			return raw_code() == sc.raw_code();
		}

		constexpr bool
		operator != ( const http_status_code_t & sc ) const noexcept
		{
			return sc.raw_code() != sc.raw_code();
		}

		constexpr bool
		operator < ( const http_status_code_t & sc ) const noexcept
		{
			return sc.raw_code() < sc.raw_code();
		}

	private:
		//! Status code value.
		std::uint16_t m_status_code{ 0 };
};

namespace status_code
{

/** @name RFC 2616 status code list.
 * @brief Codes defined by RFC 2616: https://www.w3.org/Protocols/rfc2616/rfc2616-sec6.html#sec6.1.1.
*/
///@{

// Add '_', because 'continue is reserved word.'
constexpr http_status_code_t continue_{ 100 };

constexpr http_status_code_t switching_protocols{ 101 };
constexpr http_status_code_t ok{ 200 };
constexpr http_status_code_t created{ 201 };
constexpr http_status_code_t accepted{ 202 };
constexpr http_status_code_t non_authoritative_information{ 203 };
constexpr http_status_code_t no_content{ 204 };
constexpr http_status_code_t reset_content{ 205 };
constexpr http_status_code_t partial_content{ 206 };
constexpr http_status_code_t multiple_choices{ 300 };
constexpr http_status_code_t moved_permanently{ 301 };
constexpr http_status_code_t found{ 302 };
constexpr http_status_code_t see_other{ 303 };
constexpr http_status_code_t not_modified{ 304 };
constexpr http_status_code_t use_proxy{ 305 };
constexpr http_status_code_t temporary_redirect{ 307 };
constexpr http_status_code_t bad_request{ 400 };
constexpr http_status_code_t unauthorized{ 401 };
constexpr http_status_code_t payment_required{ 402 };
constexpr http_status_code_t forbidden{ 403 };
constexpr http_status_code_t not_found{ 404 };
constexpr http_status_code_t method_not_allowed{ 405 };
constexpr http_status_code_t not_acceptable{ 406 };
constexpr http_status_code_t proxy_authentication_required{ 407 };
constexpr http_status_code_t request_time_out{ 408 };
constexpr http_status_code_t conflict{ 409 };
constexpr http_status_code_t gone{ 410 };
constexpr http_status_code_t length_required{ 411 };
constexpr http_status_code_t precondition_failed{ 412 };

//413 Payload Too Large (RFC 7231)
// The request is larger than the server is willing or able to process.
// Previously called "Request Entity Too Large".[44]
constexpr http_status_code_t payload_too_large{ 413 };

// 414 URI Too Long (RFC 7231)
// The URI provided was too long for the server to process.
// Often the result of too much data being encoded as a query-string of a GET request,
// in which case it should be converted to a POST request.
// Called "Request-URI Too Long" previously.[46]
constexpr http_status_code_t uri_too_long{ 414 };

constexpr http_status_code_t unsupported_media_type{ 415 };
constexpr http_status_code_t requested_range_not_satisfiable{ 416 };
constexpr http_status_code_t expectation_failed{ 417 };
constexpr http_status_code_t internal_server_error{ 500 };
constexpr http_status_code_t not_implemented{ 501 };
constexpr http_status_code_t bad_gateway{ 502 };
constexpr http_status_code_t service_unavailable{ 503 };
constexpr http_status_code_t gateway_time_out{ 504 };
constexpr http_status_code_t http_version_not_supported{ 505 };
///@}

/** @name Additional status codes.
 * @brief Codes not covered with RFC 2616.
*/
///@{
	// RFC 7538
constexpr http_status_code_t permanent_redirect{ 308 };

	// RFC 2518
constexpr http_status_code_t processing{ 102 };
constexpr http_status_code_t multi_status{ 207 };
constexpr http_status_code_t unprocessable_entity{ 422 };
constexpr http_status_code_t locked{ 423 };
constexpr http_status_code_t failed_dependency{ 424 };
constexpr http_status_code_t insufficient_storage{ 507 };

	// RFC 6585
constexpr http_status_code_t precondition_required{ 428 };
constexpr http_status_code_t too_many_requests{ 429 };
constexpr http_status_code_t request_header_fields_too_large{ 431 };
constexpr http_status_code_t network_authentication_required{ 511 };
///@}

} /* namespace status_code */

//
// http_status_line_t
//

//! HTTP response header status line.
class http_status_line_t
{
	public:
		http_status_line_t()
		{}

		http_status_line_t(
			http_status_code_t sc,
			std::string reason_phrase )
			:	m_status_code{ sc }
			,	m_reason_phrase{ std::move( reason_phrase ) }
		{}

		http_status_code_t
		status_code() const noexcept
		{ return m_status_code; }

		void
		status_code( http_status_code_t c ) noexcept
		{ m_status_code = c; }

		const std::string &
		reason_phrase() const noexcept
		{ return m_reason_phrase; }

		void
		reason_phrase( std::string r )
		{ m_reason_phrase.assign( std::move( r ) ); }

	private:
		http_status_code_t m_status_code{ status_code::ok };
		std::string m_reason_phrase;
};

inline std::ostream &
operator << ( std::ostream & o, const http_status_line_t & status_line )
{
	return o << "{" << status_line.status_code().raw_code() << ", "
			<< status_line.reason_phrase() << "}";
}

/** @name RFC 2616 statuses.
 * @brief Codes defined by RFC 2616: https://www.w3.org/Protocols/rfc2616/rfc2616-sec6.html#sec6.1.1.
*/
///@{

inline http_status_line_t status_continue()
{ return http_status_line_t{ status_code::continue_, "Continue" }; }

inline http_status_line_t status_switching_protocols()
{ return http_status_line_t{ status_code::switching_protocols, "Switching Protocols" }; }

inline http_status_line_t status_ok()
{ return http_status_line_t{ status_code::ok, "OK" }; }

inline http_status_line_t status_created()
{ return http_status_line_t{ status_code::created, "Created" }; }

inline http_status_line_t status_accepted()
{ return http_status_line_t{ status_code::accepted, "Accepted" }; }

inline http_status_line_t status_non_authoritative_information()
{ return http_status_line_t{ status_code::non_authoritative_information, "Non-Authoritative Information" }; }

inline http_status_line_t status_no_content()
{ return http_status_line_t{ status_code::no_content, "No Content" }; }

inline http_status_line_t status_reset_content()
{ return http_status_line_t{ status_code::reset_content, "Reset Content" }; }

inline http_status_line_t status_partial_content()
{ return http_status_line_t{ status_code::partial_content, "Partial Content" }; }

inline http_status_line_t status_multiple_choices()
{ return http_status_line_t{ status_code::multiple_choices, "Multiple Choices" }; }

inline http_status_line_t status_moved_permanently()
{ return http_status_line_t{ status_code::moved_permanently, "Moved Permanently" }; }

inline http_status_line_t status_found()
{ return http_status_line_t{ status_code::found, "Found" }; }

inline http_status_line_t status_see_other()
{ return http_status_line_t{ status_code::see_other, "See Other" }; }

inline http_status_line_t status_not_modified()
{ return http_status_line_t{ status_code::not_modified, "Not Modified" }; }

inline http_status_line_t status_use_proxy()
{ return http_status_line_t{ status_code::use_proxy, "Use Proxy" }; }

inline http_status_line_t status_temporary_redirect()
{ return http_status_line_t{ status_code::temporary_redirect, "Temporary Redirect" }; }

inline http_status_line_t status_bad_request()
{ return http_status_line_t{ status_code::bad_request, "Bad Request" }; }

inline http_status_line_t status_unauthorized()
{ return http_status_line_t{ status_code::unauthorized, "Unauthorized" }; }

inline http_status_line_t status_payment_required()
{ return http_status_line_t{ status_code::payment_required, "Payment Required" }; }

inline http_status_line_t status_forbidden()
{ return http_status_line_t{ status_code::forbidden, "Forbidden" }; }

inline http_status_line_t status_not_found()
{ return http_status_line_t{ status_code::not_found, "Not Found" }; }

inline http_status_line_t status_method_not_allowed()
{ return http_status_line_t{ status_code::method_not_allowed, "Method Not Allowed" }; }

inline http_status_line_t status_not_acceptable()
{ return http_status_line_t{ status_code::not_acceptable, "Not Acceptable" }; }

inline http_status_line_t status_proxy_authentication_required()
{ return http_status_line_t{status_code::proxy_authentication_required, "Proxy Authentication Required" }; }

inline http_status_line_t status_request_time_out()
{ return http_status_line_t{ status_code::request_time_out, "Request Timeout" }; }

inline http_status_line_t status_conflict()
{ return http_status_line_t{ status_code::conflict, "Conflict" }; }

inline http_status_line_t status_gone()
{ return http_status_line_t{ status_code::gone, "Gone" }; }

inline http_status_line_t status_length_required()
{ return http_status_line_t{ status_code::length_required, "Length Required" }; }

inline http_status_line_t status_precondition_failed()
{ return http_status_line_t{ status_code::precondition_failed, "Precondition Failed" }; }

inline http_status_line_t status_payload_too_large()
{ return http_status_line_t{ status_code::payload_too_large, "Payload Too Large" }; }

inline http_status_line_t status_uri_too_long()
{ return http_status_line_t{ status_code::uri_too_long, "URI Too Long" }; }

inline http_status_line_t status_unsupported_media_type()
{ return http_status_line_t{ status_code::unsupported_media_type, "Unsupported Media Type" }; }

inline http_status_line_t status_requested_range_not_satisfiable()
{ return http_status_line_t{ status_code::requested_range_not_satisfiable, "Requested Range Not Satisfiable" }; }

inline http_status_line_t status_expectation_failed()
{ return http_status_line_t{ status_code::expectation_failed, "Expectation Failed" }; }

inline http_status_line_t status_internal_server_error()
{ return http_status_line_t{ status_code::internal_server_error, "Internal Server Error" }; }

inline http_status_line_t status_not_implemented()
{ return http_status_line_t{ status_code::not_implemented, "Not Implemented" }; }

inline http_status_line_t status_bad_gateway()
{ return http_status_line_t{ status_code::bad_gateway, "Bad Gateway" }; }

inline http_status_line_t status_service_unavailable()
{ return http_status_line_t{ status_code::service_unavailable, "Service Unavailable" }; }

inline http_status_line_t status_gateway_time_out()
{ return http_status_line_t{ status_code::gateway_time_out, "Gateway Timeout" }; }

inline http_status_line_t status_http_version_not_supported()
{ return http_status_line_t{ status_code::http_version_not_supported, "HTTP Version not supported" }; }
///@}

/** @name Additional statuses.
 * @brief Not covered with RFC 2616.
*/
///@{
	// RFC 7538
inline http_status_line_t status_permanent_redirect()
{ return http_status_line_t{ status_code::permanent_redirect, "Permanent Redirect" }; }

	// RFC 2518
inline http_status_line_t status_processing()
{ return http_status_line_t{ status_code::processing, "Processing" }; }

inline http_status_line_t status_multi_status()
{ return http_status_line_t{ status_code::multi_status, "Multi-Status" }; }

inline http_status_line_t status_unprocessable_entity()
{ return http_status_line_t{ status_code::unprocessable_entity, "Unprocessable Entity" }; }

inline http_status_line_t status_locked()
{ return http_status_line_t{ status_code::locked, "Locked" }; }

inline http_status_line_t status_failed_dependency()
{ return http_status_line_t{ status_code::failed_dependency, "Failed Dependency" }; }

inline http_status_line_t status_insufficient_storage()
{ return http_status_line_t{ status_code::insufficient_storage, "Insufficient Storage" }; }

	// RFC 6585
inline http_status_line_t status_precondition_required()
{ return http_status_line_t{ status_code::precondition_required, "Precondition Required" }; }

inline http_status_line_t status_too_many_requests()
{ return http_status_line_t{ status_code::too_many_requests, "Too Many Requests" }; }

inline http_status_line_t status_request_header_fields_too_large()
{ return http_status_line_t{ status_code::request_header_fields_too_large, "Request Header Fields Too Large" }; }

inline http_status_line_t status_network_authentication_required()
{ return http_status_line_t{ status_code::network_authentication_required, "Network Authentication Required" }; }
///@}

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

		http_response_header_t( http_status_line_t status_line )
			:	m_status_line{ std::move( status_line ) }
		{}

		http_status_code_t
		status_code() const noexcept
		{ return m_status_line.status_code(); }

		void
		status_code( http_status_code_t c ) noexcept
		{ m_status_line.status_code( c ); }

		const std::string &
		reason_phrase() const noexcept
		{ return m_status_line.reason_phrase(); }

		void
		reason_phrase( std::string r )
		{ m_status_line.reason_phrase( std::move( r ) ); }

		const http_status_line_t &
		status_line() const noexcept
		{
			return m_status_line;
		}

		void
		status_line( http_status_line_t sl )
		{
			m_status_line = std::move( sl );
		}

	private:
		http_status_line_t m_status_line;
};

} /* namespace restinio */
