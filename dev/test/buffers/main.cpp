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

	buffer_storage_t bs{ const_buffer( s1 ) };

	auto buf = bs.buf();

	REQUIRE( address( buf ) == (void*)s1 );
	REQUIRE( size( buf ) == 10 );

	static const char s2[] = "012345678901234567890123456789";
	bs = buffer_storage_t{ const_buffer( s2 ) };

	buf = bs.buf();
	REQUIRE( address( buf ) == (void*)s2 );
	REQUIRE( size( buf ) == 30 );

	const char * s3 = "qweasdzxcrtyfghvbnuiojklm,.";
	bs = buffer_storage_t{ const_buffer( s3, 8 ) };

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

TEST_CASE(
	"buffer conversion c-string std::string" ,
	"[buffers][std::string][c-string]" )
{
	{
		buffer_storage_t bs1;
		buffer_storage_t bs2;

		auto s1 = "01234567890123456789xy";
		const char * s2 = "abc";
		std::string str2 = { s2 };

		bs1 = buffer_storage_t{ const_buffer( s1 ) };
		bs2 = buffer_storage_t{ std::move( str2 ) };

		auto buf1 = bs1.buf();
		auto buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == std::strlen( s1 ) );
		REQUIRE( address( buf1 ) == (void*)s1 );

		REQUIRE( size( buf2 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf2 ), s2, size( buf2 ) )  );

		buffer_storage_t bs3;

		bs3 = std::move( bs1 );
		bs1 = std::move( bs2 );
		bs2 = std::move( bs3 );

		buf1 = bs1.buf();
		buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf1 ), s2, size( buf1 ) )  );

		REQUIRE( size( buf2 ) == std::strlen( s1 ) );
		REQUIRE( address( buf2 ) == (void*)s1 );
	}
	{
		buffer_storage_t bs1;
		buffer_storage_t bs2;

		auto s1 = "01234567890123456789xy 01234567890123456789xy 01234567890123456789xy";
		const char * s2 = "abcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdef";
		std::string str2 = { s2 };

		bs1 = buffer_storage_t{ const_buffer( s1 ) };
		bs2 = buffer_storage_t{ std::move( str2 ) };

		auto buf1 = bs1.buf();
		auto buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == std::strlen( s1 ) );
		REQUIRE( address( buf1 ) == (void*)s1 );

		REQUIRE( size( buf2 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf2 ), s2, size( buf2 ) )  );

		buffer_storage_t bs3;

		bs3 = std::move( bs1 );
		bs1 = std::move( bs2 );
		bs2 = std::move( bs3 );

		buf1 = bs1.buf();
		buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf1 ), s2, size( buf1 ) )  );

		REQUIRE( size( buf2 ) == std::strlen( s1 ) );
		REQUIRE( address( buf2 ) == (void*)s1 );
	}
}

TEST_CASE(
	"buffer conversion std::string and shared_ptr" ,
	"[buffers][std::shared_ptr][std::string]" )
{
	{
		buffer_storage_t bs1;
		buffer_storage_t bs2;

		auto str1 = std::make_shared< std::string >( "01234567890123456789xy" );
		const char * s2 = "abc";
		std::string str2 = { s2 };

		bs1 = buffer_storage_t{ str1 };
		bs2 = buffer_storage_t{ std::move( str2 ) };

		auto buf1 = bs1.buf();
		auto buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == str1->size() );
		REQUIRE( address( buf1 ) == (void*)str1->data() );

		REQUIRE( size( buf2 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf2 ), s2, size( buf2 ) )  );

		buffer_storage_t bs3;

		bs3 = std::move( bs1 );
		bs1 = std::move( bs2 );
		bs2 = std::move( bs3 );

		buf1 = bs1.buf();
		buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf1 ), s2, size( buf1 ) )  );

		REQUIRE( size( buf2 ) == str1->size() );
		REQUIRE( address( buf2 ) == (void*)str1->data() );
	}

	{
		buffer_storage_t bs1;
		buffer_storage_t bs2;

		auto str1 = std::make_shared< std::string >(
			"01234567890123456789xy01234567890123456789xy01234567890123456789xy" );

		const char * s2 = "abcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdef";
		std::string str2 = { s2 };

		bs1 = buffer_storage_t{ str1 };
		bs2 = buffer_storage_t{ std::move( str2 ) };

		auto buf1 = bs1.buf();
		auto buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == str1->size() );
		REQUIRE( address( buf1 ) == (void*)str1->data() );

		REQUIRE( size( buf2 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf2 ), s2, size( buf2 ) )  );

		buffer_storage_t bs3;

		bs3 = std::move( bs1 );
		bs1 = std::move( bs2 );
		bs2 = std::move( bs3 );

		buf1 = bs1.buf();
		buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf1 ), s2, size( buf1 ) )  );

		REQUIRE( size( buf2 ) == str1->size() );
		REQUIRE( address( buf2 ) == (void*)str1->data() );
	}
}

TEST_CASE(
	"buffer conversion std::string custom" ,
	"[buffers][std::string][custom]" )
{
	int bufs_count = 0;
	{
		buffer_storage_t bs1;
		buffer_storage_t bs2;

		auto str1 = std::make_shared< custom_buffer_t >( bufs_count, "custom" );
		REQUIRE( 1 == bufs_count );

		const char * s2 = "abc";
		std::string str2 = { s2 };

		bs1 = buffer_storage_t{ str1 };
		REQUIRE( 1 == bufs_count );

		bs2 = buffer_storage_t{ std::move( str2 ) };

		auto buf1 = bs1.buf();
		auto buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == str1->size() );
		REQUIRE( address( buf1 ) == (void*)str1->data() );

		REQUIRE( size( buf2 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf2 ), s2, size( buf2 ) )  );

		buffer_storage_t bs3;

		bs3 = std::move( bs1 );
		REQUIRE( 1 == bufs_count );
		bs1 = std::move( bs2 );
		REQUIRE( 1 == bufs_count );
		bs2 = std::move( bs3 );
		REQUIRE( 1 == bufs_count );

		buf1 = bs1.buf();
		buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf1 ), s2, size( buf1 ) )  );

		REQUIRE( size( buf2 ) == str1->size() );
		REQUIRE( address( buf2 ) == (void*)str1->data() );

	}
	REQUIRE( 0 == bufs_count );

	{
		buffer_storage_t bs1;
		buffer_storage_t bs2;

		auto str1 = std::make_shared< custom_buffer_t >(
			bufs_count, "customcustomcustomcustomcustomcustomcustomcustomcustom" );
		REQUIRE( 1 == bufs_count );

		const char * s2 = "abcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdef";
		std::string str2 = { s2 };

		bs1 = buffer_storage_t{ str1 };
		REQUIRE( 1 == bufs_count );

		bs2 = buffer_storage_t{ std::move( str2 ) };

		auto buf1 = bs1.buf();
		auto buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == str1->size() );
		REQUIRE( address( buf1 ) == (void*)str1->data() );

		REQUIRE( size( buf2 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf2 ), s2, size( buf2 ) )  );

		buffer_storage_t bs3;

		bs3 = std::move( bs1 );
		REQUIRE( 1 == bufs_count );
		bs1 = std::move( bs2 );
		REQUIRE( 1 == bufs_count );
		bs2 = std::move( bs3 );
		REQUIRE( 1 == bufs_count );

		buf1 = bs1.buf();
		buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf1 ), s2, size( buf1 ) )  );

		REQUIRE( size( buf2 ) == str1->size() );
		REQUIRE( address( buf2 ) == (void*)str1->data() );

	}
	REQUIRE( 0 == bufs_count );
}

TEST_CASE(
	"buffers in vector" ,
	"[buffers][std::string][c-string][std::shared_ptr][custom]" )
{
	int bufs_count = 0;
	{
		std::vector< buffer_storage_t > v;

		v.reserve( 4 );

		const char * strings[] = {
			"0",
			"01",
			"012",
			"0123",
			"01234",
			"012345",
			"0123456",
			"01234567",
			"012345678",
			"0123456789",
			"abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0",
			"abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ" "01",
			"abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ" "012",
			"abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123",
			"abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ" "01234",
			"abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ" "012345",
			"abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456",
			"abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ" "01234567",
			"abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ" "012345678",
			"abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789",
		};

		v.emplace_back( const_buffer( strings[ 0 ] ) );
		v.emplace_back( const_buffer( strings[ 1 ], std::strlen( strings[ 1 ] ) ) );
		v.emplace_back( std::string{ strings[ 2 ] } );
		v.emplace_back( std::make_shared< std::string >( strings[ 3 ] ) );

		v.emplace_back( std::make_shared< custom_buffer_t >( bufs_count, strings[ 4 ] ) );
		REQUIRE( 1 == bufs_count );
		v.emplace_back( std::make_shared< custom_buffer_t >( bufs_count, strings[ 5 ] ) );
		REQUIRE( 2 == bufs_count );

		v.emplace_back( const_buffer( strings[ 6 ] ) );
		v.emplace_back( const_buffer( strings[ 7 ] ) );
		v.emplace_back( const_buffer( strings[ 8 ] ) );
		v.emplace_back( const_buffer( strings[ 9 ] ) );

		v.emplace_back( std::string{ strings[ 10 ] } );
		v.emplace_back( std::string{ strings[ 11 ] } );
		v.emplace_back( std::string{ strings[ 12 ] } );
		v.emplace_back( std::string{ strings[ 13 ] } );
		v.emplace_back( std::string{ strings[ 14 ] } );
		v.emplace_back( std::string{ strings[ 15 ] } );

		v.emplace_back( std::make_shared< std::string >( strings[ 16 ] ) );
		v.emplace_back( std::make_shared< std::string >( strings[ 17 ] ) );
		v.emplace_back( std::make_shared< std::string >( strings[ 18 ] ) );

		v.emplace_back( std::make_shared< custom_buffer_t >( bufs_count, strings[ 19 ] ) );
		REQUIRE( 3 == bufs_count );

		for( std::size_t i = 0; i < v.size(); ++i )
		{
			auto buf = v[ i ].buf();

			REQUIRE( size( buf ) == std::strlen( strings[ i ] ) );
			REQUIRE( 0 == memcmp( address( buf ), strings[ i ], size( buf ) )  );
		}

		{
			std::vector< buffer_storage_t > w{ std::move( v ) };

			for( std::size_t i = 0; i < w.size(); ++i )
			{
				auto buf = w[ i ].buf();

				REQUIRE( size( buf ) == std::strlen( strings[ i ] ) );
				REQUIRE( 0 == memcmp( address( buf ), strings[ i ], size( buf ) )  );
			}

			v = std::move( w );
		}

		{
			auto lambda = [ &strings, w = std::move( v ) ]() -> bool {
				for( std::size_t i = 0; i < w.size(); ++i )
				{
					auto buf = w[ i ].buf();

					if( size( buf ) != std::strlen( strings[ i ] ) ||
						0 != memcmp( address( buf ), strings[ i ], size( buf ) ) )
						return false;
				}

				return true;
			};

			auto lambda2 = std::move( lambda );

			REQUIRE( lambda2() );
		}

	}

	REQUIRE( 0 == bufs_count );
}

TEST_CASE(
	"buffers in shared_ptr exceptions" ,
	"[buffers][std::shared_ptr][exception]" )
{
	buffer_storage_t x;
	std::shared_ptr< std::string > b1;
	std::shared_ptr< custom_buffer_t > b2;

	REQUIRE_THROWS( x = buffer_storage_t{ b1 } );
	REQUIRE_THROWS( x = buffer_storage_t{ b2 } );
}
