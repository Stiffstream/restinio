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
#include <cctype>

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
}

inline bool
by_4char_caseless_cmp(
	const char * a,
	const char * b,
	std::size_t size )
{
	auto n = size & (0xFFFFFFFF - 0x1 );

	std::size_t i = 0;
	for( ; i < n; i+=2 )
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

		c1 = a[i]; c2 = b[i]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[i + 1]; c2 = b[i + 1]; if( tolower( c1 ) != tolower( c2 ) ) return false;
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

		c1 = a[i + 0]; c2 = b[ i + 0]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[i + 1]; c2 = b[ i + 1]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[i + 2]; c2 = b[ i + 2]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[i + 3]; c2 = b[ i + 3]; if( tolower( c1 ) != tolower( c2 ) ) return false;
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

		c1 = a[ i + 0 ]; c2 = b[ i + 0]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[ i + 1 ]; c2 = b[ i + 1]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[ i + 2 ]; c2 = b[ i + 2]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[ i + 3 ]; c2 = b[ i + 3]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[ i + 4 ]; c2 = b[ i + 4]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[ i + 5 ]; c2 = b[ i + 5]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[ i + 6 ]; c2 = b[ i + 6]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		c1 = a[ i + 7 ]; c2 = b[ i + 7]; if( tolower( c1 ) != tolower( c2 ) ) return false;
		i+=8;
	}

	for( ; i < size; ++i )
	{
		if( tolower( a[ i ] ) != tolower( b[ i ] ) )
			return false;
	}

	return true;
}

constexpr unsigned char to_lower_lut[] = {
0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
0x40,
	  0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A,
																  0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

bool
by_lut1_caseless_cmp(
	const char * a,
	const char * b,
	std::size_t size )
{
	for( std::size_t i = 0; i < size; ++i )
		if( to_lower_lut[ (unsigned char)a[ i ] ] != to_lower_lut[ (unsigned char)b[ i ] ] )
			return false;

	return true;
}

template< typename C >
const C * to_lower_lut_items()
{
	static constexpr C table[] = {
0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
0x40,
	  0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A,
																  0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };
	return table;
}

constexpr auto
uchar_at( const char * const from, const std::size_t at )
{
	return static_cast< unsigned char >( from[at] );
};

bool
by_lut1_caseless_cmp2(
	const char * a,
	const char * b,
	std::size_t size )
{
	const unsigned char * const table = to_lower_lut_items< unsigned char >();

	for( std::size_t i = 0; i < size; ++i )
		if( table[uchar_at( a, i )] != table[uchar_at( b, i )] )
			return false;

	return true;
}

bool
by_lut2_caseless_cmp(
	const char * a,
	const char * b,
	std::size_t size )
{
	auto n = size & ( 0xFFFFFFFF - 0x1 );

	std::size_t i = 0;
	while( i < n )
	{
		char c1,c2;

		c1 = to_lower_lut[ (unsigned char)a[i]];
		c2 = to_lower_lut[ (unsigned char)b[i]];
		if( c1 != c2 ) return false;

		c1 = to_lower_lut[ (unsigned char)a[i + 1]];
		c2 = to_lower_lut[ (unsigned char)b[i + 1]];
		if( c1 != c2 ) return false;

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
by_lut3_caseless_cmp(
	const char * a,
	const char * b,
	std::size_t size )
{
	auto n = size & (0xFFFFFFFF - 0x3 );

	std::size_t i = 0;
	for( ; i < n; i+=4 )
	{
		alignas( alignof(std::uint32_t) )
		unsigned char c[] = {
			to_lower_lut[ (unsigned char)a[ i ] ],
			to_lower_lut[ (unsigned char)a[ i + 1 ] ],
			to_lower_lut[ (unsigned char)a[ i + 2 ] ],
			to_lower_lut[ (unsigned char)a[ i + 3 ] ],
			to_lower_lut[ (unsigned char)b[ i ] ],
			to_lower_lut[ (unsigned char)b[ i + 1 ] ],
			to_lower_lut[ (unsigned char)b[ i + 2 ] ],
			to_lower_lut[ (unsigned char)b[ i + 3 ] ] };

		if( *(std::uint32_t*)c != *(std::uint32_t*)(c + 4) )
			return false;
	}

	for( ; i < size; ++i )
	{
		if( to_lower_lut[ (unsigned char)a[ i ] ] != to_lower_lut[ (unsigned char)b[ i ] ] )
			return false;
	}

	return true;
}

const std::size_t iterations_count = 1000 * 1000;

#define TOLOWER_BENCH( bench_func, cmp_func ) \
	void bench_func ( const std::vector< std::string > & s1, const std::vector< std::string > & s2 ){ \
		for( std::size_t i = 0; i < iterations_count; ++i ) { \
			for( std::size_t n = 0; n < s1.size(); ++n ) { \
				if( !cmp_func( s1[ n ].data(), s2[ n ].data(), s1[ n ].size() ) ) \
					throw std::runtime_error{ "MUST NEVER HAPPEN" }; \
			} \
		} \
	}

TOLOWER_BENCH( std_bench, std_caseless_cmp )
TOLOWER_BENCH( by_char_bench, by_char_caseless_cmp )
TOLOWER_BENCH( by_4char_bench, by_4char_caseless_cmp )
TOLOWER_BENCH( by_char_group_2x_bench, by_char_group_2x_caseless_cmp )
TOLOWER_BENCH( by_char_group_4x_bench, by_char_group_4x_caseless_cmp )
TOLOWER_BENCH( by_char_group_8x_bench, by_char_group_8x_caseless_cmp )
TOLOWER_BENCH( by_lut1_bench, by_lut1_caseless_cmp )
TOLOWER_BENCH( by_lut1_bench2, by_lut1_caseless_cmp2 )
TOLOWER_BENCH( by_lut2_bench, by_lut2_caseless_cmp )
TOLOWER_BENCH( by_lut3_bench, by_lut3_caseless_cmp )

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

#define RUN_TOLOWER_BENCH( bench_func, tag ) \
	run_bench( \
		tag, \
		[&]{ bench_func( str_set_1, str_set_2 ); } );

int
main()
{
	std::srand( static_cast<unsigned int>(std::time( nullptr )) );

	const auto str_set_1 = create_test_strings();
	const auto str_set_2 = create_test_strings();

	RUN_TOLOWER_BENCH( by_char_bench, "by_char" );

	RUN_TOLOWER_BENCH( by_4char_bench, "by_4char" );

	RUN_TOLOWER_BENCH( by_char_group_2x_bench, "by_char_group_2x" );

	RUN_TOLOWER_BENCH( by_char_group_4x_bench, "by_char_group_4x" );

	RUN_TOLOWER_BENCH( by_char_group_8x_bench, "by_char_group_8x" );

	RUN_TOLOWER_BENCH( by_lut1_bench, "by_lut1" );
	RUN_TOLOWER_BENCH( by_lut1_bench2, "by_lut1(2)" );

	RUN_TOLOWER_BENCH( by_lut2_bench, "by_lut2" );

	RUN_TOLOWER_BENCH( by_lut3_bench, "by_lut3" );

	RUN_TOLOWER_BENCH( std_bench, "std_tolower" );


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
