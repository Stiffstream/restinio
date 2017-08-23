/*
	restinio
*/

/*!
	Benchmark for to_lower approaches.
*/

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <stdexcept>

inline bool
std_caseless_cmp(
	const char * a,
	const char * b,
	std::size_t size )
{
	for( std::size_t i = 0; i < size; ++i )
		if( std::tolower( a[ i ] ) != std::tolower( b[ i ] ) )
			return false;

	return true;
}

inline char
tolower( char c )
{
	if( 'A' <= c && c <= 'Z' )
		c |= 0x20;
	return c;
}

inline bool
by_char_caseless_cmp(
	const char * a,
	const char * b,
	std::size_t size )
{
	for( std::size_t i = 0; i < size; ++i )
		if( tolower( a[ i ] ) != tolower( b[ i ] ) )
			return false;

	return true;
}

inline void
tolower_array( char * c )
{
	if( 'A' <= c[ 0 ] && c[ 0 ] <= 'Z' )
		c[ 0 ] |= 0x20;

	if( 'A' <= c[ 1 ] && c[ 1 ] <= 'Z' )
		c[ 1 ] |= 0x20;

	if( 'A' <= c[ 2 ] && c[ 2 ] <= 'Z' )
		c[ 2 ] |= 0x20;

	if( 'A' <= c[ 3 ] && c[ 3 ] <= 'Z' )
		c[ 3 ] |= 0x20;

	// if( 'A' <= c[ 4 ] && c[ 4 ] <= 'Z' )
	// 	c[ 4 ] |= 0x20;

	// if( 'A' <= c[ 5 ] && c[ 5 ] <= 'Z' )
	// 	c[ 5 ] |= 0x20;

	// if( 'A' <= c[ 6 ] && c[ 6 ] <= 'Z' )
	// 	c[ 6 ] |= 0x20;

	// if( 'A' <= c[ 7 ] && c[ 7 ] <= 'Z' )
	// 	c[ 7 ] |= 0x20;
}

inline bool
by_4char_caseless_cmp(
	const char * a,
	const char * b,
	std::size_t size )
{
	auto n = size & (0xFFFFFFFF - 0x1 );

	std::size_t i = 0;
	for( ; i < n; i+=4 )
	{
		alignas( 16 )
		char c[] = { a[ i ], a[ i+1 ], b[ i ], b[ i+1 ] };

		tolower_array( c );

		if( *(std::uint16_t*)c != *(std::uint16_t*)(c + 2) )
			return false;
	}

	for( ; i < size; ++i )
	{
		if( tolower( a[ i ] ) != tolower( b[ i ] ) )
			return false;
	}

	return true;
}

inline bool
by_char_group_2x_caseless_cmp(
	const char * a,
	const char * b,
	std::size_t size )
{
	auto n = size & ( 0xFFFFFFFF - 0x1 );

	std::size_t i = 0;
	while( i < n )
	{
		char c1,c2;

		c1 = a[0]; c2 = b[0]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[1]; c2 = b[1]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		i+=2;
	}

	for( ; i < size; ++i )
	{
		if( tolower( a[ i ] ) != tolower( b[ i ] ) )
			return false;
	}

	return true;
}

inline bool
by_char_group_4x_caseless_cmp(
	const char * a,
	const char * b,
	std::size_t size )
{
	auto n = size & ( 0xFFFFFFFF - 0x3 );

	std::size_t i = 0;
	while( i < n )
	{
		char c1,c2;

		c1 = a[0]; c2 = b[0]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[1]; c2 = b[1]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[2]; c2 = b[2]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[3]; c2 = b[3]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		i+=4;
	}

	for( ; i < size; ++i )
	{
		if( tolower( a[ i ] ) != tolower( b[ i ] ) )
			return false;
	}

	return true;
}

inline bool
by_char_group_8x_caseless_cmp(
	const char * a,
	const char * b,
	std::size_t size )
{
	auto n = size & (0xFFFFFFFF - 0x7);

	std::size_t i = 0;
	while( i < n )
	{
		char c1,c2;

		c1 = a[0]; c2 = b[0]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[1]; c2 = b[1]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[2]; c2 = b[2]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[3]; c2 = b[3]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[4]; c2 = b[4]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[5]; c2 = b[5]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[6]; c2 = b[6]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[7]; c2 = b[7]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		i+=8;
	}

	for( ; i < size; ++i )
	{
		if( tolower( a[ i ] ) != tolower( b[ i ] ) )
			return false;
	}

	return true;
}

const std::size_t iterations_count = 1000 * 1000;

void
std_bench(
	const std::vector< std::string > & s1,
	const std::vector< std::string > & s2 )
{
	for( std::size_t i = 0; i < iterations_count; ++i )
	{
		for( std::size_t n = 0; n < s1.size(); ++n )
		{
			if( !std_caseless_cmp( s1[ n ].data(), s2[ n ].data(), s1[ n ].size() ) )
				throw std::runtime_error{ "MUST NEVER HAPPEN" };
		}
	}
}

void
by_char_bench(
	const std::vector< std::string > & s1,
	const std::vector< std::string > & s2 )
{
	for( std::size_t i = 0; i < iterations_count; ++i )
	{
		for( std::size_t n = 0; n < s1.size(); ++n )
		{
			if( !by_char_caseless_cmp( s1[ n ].data(), s2[ n ].data(), s1[ n ].size() ) )
				throw std::runtime_error{ "MUST NEVER HAPPEN" };
		}
	}
}

void
by_4char_bench(
	const std::vector< std::string > & s1,
	const std::vector< std::string > & s2 )
{
	for( std::size_t i = 0; i < iterations_count; ++i )
	{
		for( std::size_t n = 0; n < s1.size(); ++n )
		{
			if( !by_4char_caseless_cmp( s1[ n ].data(), s2[ n ].data(), s1[ n ].size() ) )
				throw std::runtime_error{ "MUST NEVER HAPPEN" };
		}
	}
}

void
by_char_group_2x_bench(
	const std::vector< std::string > & s1,
	const std::vector< std::string > & s2 )
{
	for( std::size_t i = 0; i < iterations_count; ++i )
	{
		for( std::size_t n = 0; n < s1.size(); ++n )
		{
			if( !by_char_group_2x_caseless_cmp( s1[ n ].data(), s2[ n ].data(), s1[ n ].size() ) )
				throw std::runtime_error{ "MUST NEVER HAPPEN" };
		}
	}
}

void
by_char_group_4x_bench(
	const std::vector< std::string > & s1,
	const std::vector< std::string > & s2 )
{
	for( std::size_t i = 0; i < iterations_count; ++i )
	{
		for( std::size_t n = 0; n < s1.size(); ++n )
		{
			if( !by_char_group_4x_caseless_cmp( s1[ n ].data(), s2[ n ].data(), s1[ n ].size() ) )
				throw std::runtime_error{ "MUST NEVER HAPPEN" };
		}
	}
}

void
by_char_group_8x_bench(
	const std::vector< std::string > & s1,
	const std::vector< std::string > & s2 )
{
	for( std::size_t i = 0; i < iterations_count; ++i )
	{
		for( std::size_t n = 0; n < s1.size(); ++n )
		{
			if( !by_char_group_8x_caseless_cmp( s1[ n ].data(), s2[ n ].data(), s1[ n ].size() ) )
				throw std::runtime_error{ "MUST NEVER HAPPEN" };
		}
	}
}

std::vector< std::string >
create_test_strings();

template < typename LAMBDA >
void
run_bench( const std::string & tag, LAMBDA lambda )
{
	try
	{
		auto started_at = std::chrono::high_resolution_clock::now();
		lambda();
		auto finished_at = std::chrono::high_resolution_clock::now();
		const double duration =
			std::chrono::duration_cast< std::chrono::microseconds >(
				finished_at - started_at ).count() / 1000.0;

		std::cout << "Done '" << tag << "': " << duration << " ms" << std::endl;
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Failed to run '" << tag << "': " << ex.what() << std::endl;
	}
}

int
main()
{
	std::srand( std::time( nullptr ) );

	const auto str_set_1 = create_test_strings();
	const auto str_set_2 = create_test_strings();

	run_bench(
		"std_tolower",
		[&]{
			std_bench( str_set_1, str_set_2 );
		} );

	run_bench(
		"by_char",
		[&]{
			by_char_bench( str_set_1, str_set_2 );
		} );

	run_bench(
		"by_4char",
		[&]{
			by_4char_bench( str_set_1, str_set_2 );
		} );

	run_bench(
		"by_char_group_2x",
		[&]{
			by_char_group_2x_bench( str_set_1, str_set_2 );
		} );

	run_bench(
		"by_char_group_4x",
		[&]{
			by_char_group_4x_bench( str_set_1, str_set_2 );
		} );


	run_bench(
		"by_char_group_8x",
		[&]{
			by_char_group_8x_bench( str_set_1, str_set_2 );
		} );


	return 0;
}

// Create string in different cases.
std::vector< std::string >
create_test_strings()
{
	std::vector< std::string >
		result{
			"A-IM",
			"Accept",
			"Accept-Additions",
			"Accept-Charset",
			"Accept-Datetime",
			"Accept-Encoding",
			"Accept-Features",
			"Accept-Language",
			"Accept-Patch",
			"Accept-Post",
			"Accept-Ranges",
			"Age",
			"Allow",
			"ALPN",
			"Alt-Svc",
			"Alt-Used",
			"Alternates",
			"Apply-To-Redirect-Ref",
			"Authentication-Control",
			"Authentication-Info",
			"Authorization",
			"C-Ext",
			"C-Man",
			"C-Opt",
			"C-PEP",
			"C-PEP-Info",
			"Cache-Control",
			"CalDAV-Timezones",
			"Close",
			"Content-Base",
			"Content-Disposition",
			"Content-Encoding",
			"Content-ID",
			"Content-Language",
			"Content-Location",
			"Content-MD5",
			"Content-Range",
			"Content-Script-Type",
			"Content-Style-Type",
			"Content-Type",
			"Content-Version",
			"Cookie",
			"Cookie2",
			"DASL",
			"DAV",
			"Date",
			"Default-Style",
			"Delta-Base",
			"Depth",
			"Derived-From",
			"Destination",
			"Differential-ID",
			"Digest",
			"ETag",
			"Expect",
			"Expires",
			"Ext",
			"Forwarded",
			"From",
			"GetProfile",
			"Hobareg",
			"Host",
			"HTTP2-Settings",
			"IM",
			"If",
			"If-Match",
			"If-Modified-Since",
			"If-None-Match",
			"If-Range",
			"If-Schedule-Tag-Match",
			"If-Unmodified-Since",
			"Keep-Alive",
			"Label",
			"Last-Modified",
			"Link",
			"Location",
			"Lock-Token",
			"Man",
			"Max-Forwards",
			"Memento-Datetime",
			"Meter",
			"MIME-Version",
			"Negotiate",
			"Opt",
			"Optional-WWW-Authenticate",
			"Ordering-Type",
			"Origin",
			"Overwrite",
			"P3P",
			"PEP",
			"PICS-Label",
			"Pep-Info",
			"Position",
			"Pragma",
			"Prefer",
			"Preference-Applied",
			"ProfileObject",
			"Protocol",
			"Protocol-Info",
			"Protocol-Query",
			"Protocol-Request",
			"Proxy-Authenticate",
			"Proxy-Authentication-Info",
			"Proxy-Authorization",
			"Proxy-Features",
			"Proxy-Instruction",
			"Public",
			"Public-Key-Pins",
			"Public-Key-Pins-Report-Only",
			"Range",
			"Redirect-Ref",
			"Referer",
			"Retry-After",
			"Safe",
			"Schedule-Reply",
			"Schedule-Tag",
			"Sec-WebSocket-Accept",
			"Sec-WebSocket-Extensions",
			"Sec-WebSocket-Key",
			"Sec-WebSocket-Protocol",
			"Sec-WebSocket-Version",
			"Security-Scheme",
			"Server",
			"Set-Cookie",
			"Set-Cookie2",
			"SetProfile",
			"SLUG",
			"SoapAction",
			"Status-URI",
			"Strict-Transport-Security",
			"Surrogate-Capability",
			"Surrogate-Control",
			"TCN",
			"TE",
			"Timeout",
			"Topic",
			"Trailer",
			"Transfer-Encoding",
			"TTL",
			"Urgency",
			"URI",
			"Upgrade",
			"User-Agent",
			"Variant-Vary",
			"Vary",
			"Via",
			"WWW-Authenticate",
			"Want-Digest",
			"Warning",
			"X-Frame-Options" };

	for( auto & s : result )
	{
		for( auto & c : s )
		{
			if( 0 == std::rand() % 3 )
				c = std::toupper( c );
			else
				c = std::tolower( c );
		}
	}

	return result;
}
