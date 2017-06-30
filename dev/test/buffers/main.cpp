/*
	restinio
*/

/*!
	Tests for settings parameters that have default constructor.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <restinio/buffers.hpp>

using namespace restinio;

std::size_t
size( const asio::const_buffer & b )
{
	return asio::buffer_size( b );
}

const void *
address( const asio::const_buffer & b )
{
	return asio::buffer_cast< const void * >( b );
}

TEST_CASE( "buffers on c-string" , "[buffers][c-string]" )
{
	const char * s1 = "0123456789";

	buffer_storage_t bs{ s1 };

	auto buf = bs.buf();

	REQUIRE( address( buf ) == (void*)s1 );
	REQUIRE( size( buf ) == 10 );

	const char s2[] = "012345678901234567890123456789";
	bs = buffer_storage_t{ s2 };

	buf = bs.buf();
	REQUIRE( address( buf ) == (void*)s2 );
	REQUIRE( size( buf ) == 30 );

	const char * s3 = "qweasdzxcrtyfghvbnuiojklm,.";
	bs = buffer_storage_t{ s3, 8 };

	buf = bs.buf();
	REQUIRE( address( buf ) == (void*)s3 );
	REQUIRE( size( buf ) == 8 );
}

TEST_CASE( "buffers on std::string" , "[buffers][std::string]" )
{
	const char * s1 = "0123456789";
	std::string str1{ s1 };

	buffer_storage_t bs{ std::move( str1 ) };

	auto buf = bs.buf();

	REQUIRE( size( buf ) == 10 );
	REQUIRE( 0 == memcmp( address( buf ), s1, size( buf ) ) );

	const char * s2 = "012345678901234567890123456789";
	std::string str2{ s2 };

	bs = buffer_storage_t{ std::move( str2 ) };

	buf = bs.buf();
	REQUIRE( size( buf ) == 30 );
	REQUIRE( 0 == memcmp( address( buf ), s2, size( buf ) ) );


	const char * s3 = "\0x00\0x00\0x00" "012345678901234567890123456789 012345678901234567890123456789";
	std::string str3{ s3, 64};

	bs = buffer_storage_t{ std::move( str3 ) };

	buf = bs.buf();
	REQUIRE( size( buf ) == 64 );
	REQUIRE( 0 == memcmp( address( buf ), s3, size( buf ) ) );
}


TEST_CASE( "buffers on shared pointer" , "[buffers][std::shared_ptr]" )
{
	auto str1 = std::make_shared< std::string >( "01234567890123456789xy");

	buffer_storage_t bs{ str1 };

	auto buf = bs.buf();

	REQUIRE( size( buf ) == str1->size() );
	REQUIRE( address( buf ) == (void*)str1->data() );

	auto str2 = std::make_shared< std::string >( "0123456789 0123456789 0123456789 0123456789" );
	bs = buffer_storage_t{ str2 };

	buf = bs.buf();
	REQUIRE( size( buf ) == str2->size() );
	REQUIRE( address( buf ) == (void*)str2->data() );
}

struct custom_buffer_t
{
	custom_buffer_t( int & counter, std::string str )
		:	m_counter{ counter }
		,	m_str{ std::move( str ) }
	{
		++m_counter;
	}

	~custom_buffer_t()
	{
		--m_counter;
	}

	const char * data() { return m_str.data(); }
	std::size_t size() { return m_str.size(); }

	std::string m_str;
	int & m_counter;

};

TEST_CASE( "buffers on custom" , "[buffers][custom]" )
{
	int bufs_count = 0;

	{
		const char * s1 =  "01234567890123456789xy";
		auto custom = std::make_shared< custom_buffer_t >( bufs_count, s1 );

		REQUIRE( 1 == bufs_count );

		buffer_storage_t bs{ std::move( custom ) };
		REQUIRE( 1 == bufs_count );

		auto buf = bs.buf();
		REQUIRE( size( buf ) == 22 );
		REQUIRE( 0 == memcmp( address( buf ), s1, size( buf ) ) );

		{
			buffer_storage_t bs2{ std::move( bs ) };

			REQUIRE( 1 == bufs_count );

			buf = bs2.buf();
			REQUIRE( size( buf ) == 22 );
			REQUIRE( 0 == memcmp( address( buf ), s1, size( buf ) ) );

			bs = std::move( bs2 );

			buf = bs2.buf();
			REQUIRE( size( buf ) == 22 );
			REQUIRE( 0 == memcmp( address( buf ), s1, size( buf ) ) );
		}


		const char * s2 = "01234567890123456789xy01234567890123456789xy01234567890123456789xy";

		custom = std::make_shared< custom_buffer_t >( bufs_count, s2 );
		REQUIRE( 2 == bufs_count );

		bs = buffer_storage_t{ custom };
		REQUIRE( 1 == bufs_count );

		buf = bs.buf();
		REQUIRE( size( buf ) == 66 );
		REQUIRE( 0 == memcmp( address( buf ), s2, size( buf ) ) );
	}

	REQUIRE( 0 == bufs_count );
}

// TEST_CASE( "buffer conversion std::string shared_ptr" , "[buffers][std::shared_ptr][std::string][c-string][custom]" )

TEST_CASE(
	"buffer conversion std::string and shared_ptr" ,
	"[buffers][std::shared_ptr][std::string]" )
{
	alignas( std::max_align_t ) std::array< char, sizeof( std::string ) > storage1;
	alignas( std::max_align_t ) std::array< char, sizeof( std::string ) > storage2;

	const char * s = "abc";
	std::string str = { s };

	new( storage1.data() ) std::string{ std::move( str ) };

	// storage2 = storage1;
	memcpy( storage2.data(), storage1.data(), storage2.size() );

	std::string * ss = reinterpret_cast< std::string * >( (void*)storage2.data() );

	std::cout << ss << std::endl;

	using namespace std;
	ss->std::string::~string();

	// buffer_storage_t bs1;
	// // buffer_storage_t bs2;

	// auto str1 = std::make_shared< std::string >( "01234567890123456789xy" );
	// const char * s2 = "abc";
	// std::string str2 = { s2 };
	// buffer_storage_t bs2{ std::move( str2 ) };

	// bs1 = buffer_storage_t{ str1 };
	// bs2 = buffer_storage_t{ std::move( str2 ) };

	// auto buf1 = bs1.buf();
	// auto buf2 = bs2.buf();

	// REQUIRE( size( buf1 ) == str1->size() );
	// REQUIRE( address( buf1 ) == (void*)str1->data() );

	// REQUIRE( size( buf2 ) == std::strlen( s2 ) );
	// REQUIRE( 0 == memcmp( address( buf2 ), s2, size( buf2 ) )  );

}

