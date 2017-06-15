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
// http_header_field_t
//

//! A single header field.
struct http_header_field_t
{
	http_header_field_t()
	{}

	http_header_field_t(
		std::string name,
		std::string value )
		:	m_name{ std::move( name ) }
		,	m_value{ std::move( value ) }
	{}

	std::string m_name;
	std::string m_value;
};

//
// caseless_cmp()
//

//! Comparator for fields names.
inline bool
caseless_cmp( const std::string & a, const std::string & b )
{
	if( a.size() == b.size() )
	{
		for( std::size_t i = 0; i < a.size(); ++i )
			if( std::tolower( a[ i ] ) != std::tolower( b[ i ] ) )
				return false;

		return true;
	}

	return false;
}

//
// http_header_fields_t
//

//! Header fields map.
class http_header_fields_t
{
	public:
		using fields_container_t = std::vector< http_header_field_t >;

		http_header_fields_t() = default;
		http_header_fields_t(const http_header_fields_t &) = default;
		http_header_fields_t(http_header_fields_t &&) = default;
		virtual ~http_header_fields_t() {}

		http_header_fields_t & operator=(const http_header_fields_t &) = default;
		http_header_fields_t & operator=(http_header_fields_t &&) = default;

		bool
		has_field( const std::string & field_name ) const
		{
			return m_fields.cend() != cfind( field_name );
		}

		void
		set_field(
			std::string field_name,
			std::string field_value )
		{
			auto it = find( field_name );

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

		void
		append_field(
			const std::string & field_name,
			const std::string & field_value )
		{
			auto it = find( field_name );

			if( m_fields.end() != it )
			{
				it->m_value.append( field_value );
			}
			else
			{
				m_fields.emplace_back( field_name, field_value );
			}
		}

		void
		append_last_field( const std::string & field_value )
		{
			m_fields.back().m_value.append( field_value );
		}

		const std::string &
		get_field(
			const std::string & field_name ) const
		{
			auto it = cfind( field_name );

			if( m_fields.end() == it )
				throw exception_t(
					fmt::format( "field '{}' doesn't exist", field_name ) );

			return it->m_value;
		}

		const std::string &
		get_field(
			const std::string & field_name,
			const std::string & default_value ) const
		{
			auto it = cfind( field_name );

			if( m_fields.end() == it )
				return default_value;

			return it->m_value;
		}

		void
		remove_field( const std::string & field_name )
		{
			auto it = find( field_name );

			if( m_fields.end() != it )
				m_fields.erase( it );
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
		{ return m_should_keep_alive; }

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
constexpr inline const char *
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
