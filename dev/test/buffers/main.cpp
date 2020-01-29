/*
	restinio
*/

/*!
	Tests for settings parameters that have default constructor.
*/

// #define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <restinio/buffers.hpp>

using namespace restinio;

std::size_t
size( const restinio::asio_ns::const_buffer & b )
{
	return b.size();
}

const void *
address( const restinio::asio_ns::const_buffer & b )
{
	return b.data();
}

TEST_CASE( "buffers on c-string" , "[buffers][c-string]" )
{
	const char * s1 = "0123456789";

	writable_item_t bs{ const_buffer( s1 ) };
	REQUIRE( writable_item_type_t::trivial_write_operation == bs.write_type() );

	auto buf = bs.buf();

	REQUIRE( address( buf ) == (void*)s1 );
	REQUIRE( size( buf ) == 10 );

	static const char s2[] = "012345678901234567890123456789";
	bs = writable_item_t{ const_buffer( s2 ) };
	REQUIRE( writable_item_type_t::trivial_write_operation == bs.write_type() );

	buf = bs.buf();
	REQUIRE( address( buf ) == (void*)s2 );
	REQUIRE( size( buf ) == 30 );

	const char * s3 = "qweasdzxcrtyfghvbnuiojklm,.";
	bs = writable_item_t{ const_buffer( s3, 8 ) };
	REQUIRE( writable_item_type_t::trivial_write_operation == bs.write_type() );

	buf = bs.buf();
	REQUIRE( address( buf ) == (void*)s3 );
	REQUIRE( size( buf ) == 8 );
}

TEST_CASE( "buffers on std::string" , "[buffers][std::string]" )
{
	const char * s1 = "0123456789";
	std::string str1{ s1 };

	writable_item_t bs{ std::move( str1 ) };
	REQUIRE( writable_item_type_t::trivial_write_operation == bs.write_type() );

	auto buf = bs.buf();

	REQUIRE( size( buf ) == 10 );
	REQUIRE( 0 == memcmp( address( buf ), s1, size( buf ) ) );

	const char * s2 = "012345678901234567890123456789";
	std::string str2{ s2 };

	bs = writable_item_t{ std::move( str2 ) };
	REQUIRE( writable_item_type_t::trivial_write_operation == bs.write_type() );

	buf = bs.buf();
	REQUIRE( size( buf ) == 30 );
	REQUIRE( 0 == memcmp( address( buf ), s2, size( buf ) ) );


	const char * s3 = "\0x00\0x00\0x00" "012345678901234567890123456789 012345678901234567890123456789";
	std::string str3{ s3, 64};

	bs = writable_item_t{ std::move( str3 ) };
	REQUIRE( writable_item_type_t::trivial_write_operation == bs.write_type() );

	buf = bs.buf();
	REQUIRE( size( buf ) == 64 );
	REQUIRE( 0 == memcmp( address( buf ), s3, size( buf ) ) );
}

TEST_CASE( "buffers on fmt::basic_memory_buffer<char,1>" ,
		"[buffers][fmt::basic_memory_buffer]" )
{
	fmt::basic_memory_buffer<char, 1u> fmt_buf;
	fmt::format_to( fmt_buf, "Hello, {}", "World!" );

	writable_item_t bs{ std::move( fmt_buf ) };
	REQUIRE( writable_item_type_t::trivial_write_operation == bs.write_type() );

	auto buf = bs.buf();

	REQUIRE( size( buf ) == 13 );
	REQUIRE( 0 == memcmp( address( buf ), "Hello, World!", size( buf ) ) );
}

TEST_CASE( "buffers on shared pointer" , "[buffers][std::shared_ptr]" )
{
	auto str1 = std::make_shared< std::string >( "01234567890123456789xy");

	writable_item_t bs{ str1 };
	REQUIRE( writable_item_type_t::trivial_write_operation == bs.write_type() );

	auto buf = bs.buf();

	REQUIRE( size( buf ) == str1->size() );
	REQUIRE( address( buf ) == (void*)str1->data() );

	auto str2 = std::make_shared< std::string >( "0123456789 0123456789 0123456789 0123456789" );
	bs = writable_item_t{ str2 };
	REQUIRE( writable_item_type_t::trivial_write_operation == bs.write_type() );

	buf = bs.buf();
	REQUIRE( size( buf ) == str2->size() );
	REQUIRE( address( buf ) == (void*)str2->data() );
}

struct custom_buffer_t
{
	custom_buffer_t( int & counter, std::string str )
		:	m_str{ std::move( str ) }
		,	m_counter{ counter }
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

		writable_item_t bs{ std::move( custom ) };
		REQUIRE( writable_item_type_t::trivial_write_operation == bs.write_type() );
		REQUIRE( 1 == bufs_count );

		auto buf = bs.buf();
		REQUIRE( size( buf ) == 22 );
		REQUIRE( 0 == memcmp( address( buf ), s1, size( buf ) ) );

		{
			writable_item_t bs2{ std::move( bs ) };
			REQUIRE( writable_item_type_t::trivial_write_operation == bs2.write_type() );

			REQUIRE( 1 == bufs_count );

			buf = bs2.buf();
			REQUIRE( size( buf ) == 22 );
			REQUIRE( 0 == memcmp( address( buf ), s1, size( buf ) ) );

			bs = std::move( bs2 );
			REQUIRE( writable_item_type_t::trivial_write_operation == bs.write_type() );
		}


		const char * s2 = "01234567890123456789xy01234567890123456789xy01234567890123456789xy";

		custom = std::make_shared< custom_buffer_t >( bufs_count, s2 );
		REQUIRE( 2 == bufs_count );

		bs = writable_item_t{ custom };
		REQUIRE( writable_item_type_t::trivial_write_operation == bs.write_type() );
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
		writable_item_t bs1;
		writable_item_t bs2;

		auto s1 = "01234567890123456789xy";
		const char * s2 = "abc";
		std::string str2 = { s2 };

		bs1 = writable_item_t{ const_buffer( s1 ) };
		bs2 = writable_item_t{ std::move( str2 ) };
		REQUIRE( writable_item_type_t::trivial_write_operation == bs1.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs2.write_type() );

		auto buf1 = bs1.buf();
		auto buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == std::strlen( s1 ) );
		REQUIRE( address( buf1 ) == (void*)s1 );

		REQUIRE( size( buf2 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf2 ), s2, size( buf2 ) )  );

		writable_item_t bs3;
		REQUIRE( writable_item_type_t::trivial_write_operation == bs3.write_type() );

		bs3 = std::move( bs1 );
		bs1 = std::move( bs2 );
		bs2 = std::move( bs3 );

		REQUIRE( writable_item_type_t::trivial_write_operation == bs1.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs2.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs3.write_type() );

		buf1 = bs1.buf();
		buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf1 ), s2, size( buf1 ) )  );

		REQUIRE( size( buf2 ) == std::strlen( s1 ) );
		REQUIRE( address( buf2 ) == (void*)s1 );
	}
	{
		writable_item_t bs1;
		writable_item_t bs2;

		REQUIRE( writable_item_type_t::trivial_write_operation == bs1.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs2.write_type() );

		auto s1 = "01234567890123456789xy 01234567890123456789xy 01234567890123456789xy";
		const char * s2 = "abcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdef";
		std::string str2 = { s2 };

		bs1 = writable_item_t{ const_buffer( s1 ) };
		bs2 = writable_item_t{ std::move( str2 ) };

		REQUIRE( writable_item_type_t::trivial_write_operation == bs1.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs2.write_type() );

		auto buf1 = bs1.buf();
		auto buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == std::strlen( s1 ) );
		REQUIRE( address( buf1 ) == (void*)s1 );

		REQUIRE( size( buf2 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf2 ), s2, size( buf2 ) )  );

		writable_item_t bs3;
		REQUIRE( writable_item_type_t::trivial_write_operation == bs3.write_type() );

		bs3 = std::move( bs1 );
		bs1 = std::move( bs2 );
		bs2 = std::move( bs3 );

		REQUIRE( writable_item_type_t::trivial_write_operation == bs1.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs2.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs3.write_type() );

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
		writable_item_t bs1;
		writable_item_t bs2;
		REQUIRE( writable_item_type_t::trivial_write_operation == bs1.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs2.write_type() );

		auto str1 = std::make_shared< std::string >( "01234567890123456789xy" );
		const char * s2 = "abc";
		std::string str2 = { s2 };

		bs1 = writable_item_t{ str1 };
		bs2 = writable_item_t{ std::move( str2 ) };

		auto buf1 = bs1.buf();
		auto buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == str1->size() );
		REQUIRE( address( buf1 ) == (void*)str1->data() );

		REQUIRE( size( buf2 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf2 ), s2, size( buf2 ) )  );

		writable_item_t bs3;
		REQUIRE( writable_item_type_t::trivial_write_operation == bs3.write_type() );

		bs3 = std::move( bs1 );
		bs1 = std::move( bs2 );
		bs2 = std::move( bs3 );

		REQUIRE( writable_item_type_t::trivial_write_operation == bs1.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs2.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs3.write_type() );

		buf1 = bs1.buf();
		buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf1 ), s2, size( buf1 ) )  );

		REQUIRE( size( buf2 ) == str1->size() );
		REQUIRE( address( buf2 ) == (void*)str1->data() );
	}

	{
		writable_item_t bs1;
		writable_item_t bs2;
		REQUIRE( writable_item_type_t::trivial_write_operation == bs1.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs2.write_type() );

		auto str1 = std::make_shared< std::string >(
			"01234567890123456789xy01234567890123456789xy01234567890123456789xy" );

		const char * s2 = "abcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdef";
		std::string str2 = { s2 };

		bs1 = writable_item_t{ str1 };
		bs2 = writable_item_t{ std::move( str2 ) };

		auto buf1 = bs1.buf();
		auto buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == str1->size() );
		REQUIRE( address( buf1 ) == (void*)str1->data() );

		REQUIRE( size( buf2 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf2 ), s2, size( buf2 ) )  );

		writable_item_t bs3;
		REQUIRE( writable_item_type_t::trivial_write_operation == bs3.write_type() );

		bs3 = std::move( bs1 );
		bs1 = std::move( bs2 );
		bs2 = std::move( bs3 );

		REQUIRE( writable_item_type_t::trivial_write_operation == bs1.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs2.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs3.write_type() );

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
		writable_item_t bs1;
		writable_item_t bs2;
		REQUIRE( writable_item_type_t::trivial_write_operation == bs1.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs2.write_type() );

		auto str1 = std::make_shared< custom_buffer_t >( bufs_count, "custom" );
		REQUIRE( 1 == bufs_count );

		const char * s2 = "abc";
		std::string str2 = { s2 };

		bs1 = writable_item_t{ str1 };
		REQUIRE( 1 == bufs_count );

		bs2 = writable_item_t{ std::move( str2 ) };

		REQUIRE( writable_item_type_t::trivial_write_operation == bs1.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs2.write_type() );

		auto buf1 = bs1.buf();
		auto buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == str1->size() );
		REQUIRE( address( buf1 ) == (void*)str1->data() );

		REQUIRE( size( buf2 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf2 ), s2, size( buf2 ) )  );

		writable_item_t bs3;
		REQUIRE( writable_item_type_t::trivial_write_operation == bs3.write_type() );

		bs3 = std::move( bs1 );
		REQUIRE( 1 == bufs_count );
		bs1 = std::move( bs2 );
		REQUIRE( 1 == bufs_count );
		bs2 = std::move( bs3 );
		REQUIRE( 1 == bufs_count );

		REQUIRE( writable_item_type_t::trivial_write_operation == bs1.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs2.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs3.write_type() );

		buf1 = bs1.buf();
		buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf1 ), s2, size( buf1 ) )  );

		REQUIRE( size( buf2 ) == str1->size() );
		REQUIRE( address( buf2 ) == (void*)str1->data() );

	}
	REQUIRE( 0 == bufs_count );

	{
		writable_item_t bs1;
		writable_item_t bs2;
		REQUIRE( writable_item_type_t::trivial_write_operation == bs1.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs2.write_type() );

		auto str1 = std::make_shared< custom_buffer_t >(
			bufs_count, "customcustomcustomcustomcustomcustomcustomcustomcustom" );
		REQUIRE( 1 == bufs_count );

		const char * s2 = "abcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdef";
		std::string str2 = { s2 };

		bs1 = writable_item_t{ str1 };
		REQUIRE( 1 == bufs_count );

		bs2 = writable_item_t{ std::move( str2 ) };

		REQUIRE( writable_item_type_t::trivial_write_operation == bs1.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs2.write_type() );

		auto buf1 = bs1.buf();
		auto buf2 = bs2.buf();

		REQUIRE( size( buf1 ) == str1->size() );
		REQUIRE( address( buf1 ) == (void*)str1->data() );

		REQUIRE( size( buf2 ) == std::strlen( s2 ) );
		REQUIRE( 0 == memcmp( address( buf2 ), s2, size( buf2 ) )  );

		writable_item_t bs3;
		REQUIRE( writable_item_type_t::trivial_write_operation == bs3.write_type() );

		bs3 = std::move( bs1 );
		REQUIRE( 1 == bufs_count );
		bs1 = std::move( bs2 );
		REQUIRE( 1 == bufs_count );
		bs2 = std::move( bs3 );
		REQUIRE( 1 == bufs_count );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs1.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs2.write_type() );
		REQUIRE( writable_item_type_t::trivial_write_operation == bs3.write_type() );

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
		std::vector< writable_item_t > v;

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
			std::vector< writable_item_t > w{ std::move( v ) };

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
	writable_item_t x;
	std::shared_ptr< std::string > b1;
	std::shared_ptr< custom_buffer_t > b2;

	REQUIRE_THROWS( x = writable_item_t{ b1 } );
	REQUIRE_THROWS( x = writable_item_t{ b2 } );
}

TEST_CASE(
	"empty (default constructed) buffers" ,
	"[buffers][empty]" )
{
	writable_item_t x;
	restinio::asio_ns::const_buffer b;
	REQUIRE_NOTHROW( b = x.buf() );
	REQUIRE( 0 == size( b ) );
	REQUIRE( nullptr == address( b ) );

	writable_item_t y;
	y = std::move( x );

	REQUIRE_NOTHROW( b = y.buf() );
	REQUIRE( 0 == size( b ) );
	REQUIRE( nullptr == address( b ) );
}

TEST_CASE( "write_group_t" , "[write_group][ctor/move]" )
{
	{
		writable_items_container_t wic;
		write_group_t wg{ std::move( wic ) };

		REQUIRE( 0 == wg.status_line_size() );
		REQUIRE_FALSE( wg.has_after_write_notificator() );
		REQUIRE( 0 == wg.items_count() );
	}

	{
		writable_items_container_t wic;
		wic.emplace_back( const_buffer( "0123456789" ) );
		wic.emplace_back( const_buffer( "9876543210" ) );

		write_group_t wg1{ std::move( wic ) };
		write_group_t wg2{ std::move( wic ) };

		wg1.status_line_size( 4 );
		wg1.after_write_notificator( []( const auto & ){} );

		REQUIRE( 4 == wg1.status_line_size() );
		REQUIRE( wg1.has_after_write_notificator() );
		REQUIRE( 2 == wg1.items_count() );
		REQUIRE( 0 == wg2.status_line_size() );
		REQUIRE_FALSE( wg2.has_after_write_notificator() );
		REQUIRE( 0 == wg2.items_count() );

		wg2 = std::move( wg1 );
		REQUIRE( 0 == wg1.status_line_size() );
		REQUIRE_FALSE( wg1.has_after_write_notificator() );
		REQUIRE( 0 == wg1.items_count() );
		REQUIRE( 4 == wg2.status_line_size() );
		REQUIRE( wg2.has_after_write_notificator() );
		REQUIRE( 2 == wg2.items_count() );

		write_group_t wg3{ std::move( wg2 ) };
		REQUIRE( 0 == wg2.status_line_size() );
		REQUIRE_FALSE( wg2.has_after_write_notificator() );
		REQUIRE( 0 == wg2.items_count() );
		REQUIRE( 4 == wg3.status_line_size() );
		REQUIRE( wg3.has_after_write_notificator() );
		REQUIRE( 2 == wg3.items_count() );

		wg3.reset();
		REQUIRE( 0 == wg3.status_line_size() );
		REQUIRE_FALSE( wg3.has_after_write_notificator() );
		REQUIRE( 0 == wg3.items_count() );
	}
}

TEST_CASE( "write_group_t::status_line_size" , "[write_group][status_line_size]" )
{
	{
		writable_items_container_t wic;
		write_group_t wg{ std::move( wic ) };

		REQUIRE_THROWS( wg.status_line_size(42) ); // Empty group.
	}

	{
		writable_items_container_t wic;
		wic.emplace_back( const_buffer( "HTTP 200 OK\r\n" ) );
		wic.emplace_back(
			const_buffer(
				"x-a: 0123456789012345678901234567890123456789\r\n"
				"x-b: 0123456789012345678901234567890123456789\r\n"
				"\r\n" ) );
		write_group_t wg{ std::move( wic ) };

		REQUIRE_THROWS( wg.status_line_size(42) ); // first buffer size is less than 42.
	}

	{
		writable_items_container_t wic;
		wic.emplace_back( restinio::sendfile(
					restinio::null_file_descriptor() /* fake not real */,
					restinio::file_meta_t{1024, std::chrono::system_clock::now() } ) );
		write_group_t wg{ std::move( wic ) };

		REQUIRE_THROWS( wg.status_line_size(42) ); // first buffer is not a trivial one.
	}
}

TEST_CASE( "write_group_t merge" , "[write_group][merge]" )
{
	{
		writable_items_container_t wic;
		wic.emplace_back( const_buffer( "0123456789" ) );

		write_group_t wg1{ std::move( wic ) };

		wic.emplace_back( const_buffer( "9876543210" ) );
		write_group_t wg2{ std::move( wic ) };

		REQUIRE( 1 == wg1.items_count() );
		REQUIRE( 1 == wg2.items_count() );

		wg1.merge( std::move( wg2 ) );
		REQUIRE( 2 == wg1.items_count() );
		REQUIRE( 0 == wg2.items_count() );
	}
}

TEST_CASE( "write_group_t after_write_notificator" , "[write_group][after_write_notificator]" )
{
	{
		writable_items_container_t wic;

		write_group_t wg{ std::move( wic ) };
		REQUIRE_FALSE( wg.has_after_write_notificator() );

		wg.after_write_notificator( []( const auto & ){} );
		REQUIRE( wg.has_after_write_notificator() );

		write_group_t wg_second = std::move( wg );
		REQUIRE_FALSE( wg.has_after_write_notificator() );
		REQUIRE( wg_second.has_after_write_notificator() );
	}

	{
		int counter{ 0 };

		{
			writable_items_container_t wic;

			write_group_t wg{ std::move( wic ) };


			wg.after_write_notificator( [&]( const auto & ){ ++counter; } );

			REQUIRE( wg.has_after_write_notificator() );
			REQUIRE_NOTHROW(
				wg.invoke_after_write_notificator_if_exists( asio_ns::error_code{} ) );

			REQUIRE( 1 == counter );

			REQUIRE_NOTHROW(
				wg.invoke_after_write_notificator_if_exists( asio_ns::error_code{} ) );

			// No second call!
			REQUIRE( 1 == counter );
		}

		// No call from dtor!
		REQUIRE( 1 == counter );
	}

	{
		int counter{ 0 };

		{
			writable_items_container_t wic;

			write_group_t wg{ std::move( wic ) };

			wg.after_write_notificator(
				[&]( const auto & ec ){
					++counter;
					REQUIRE(
						ec ==
						make_asio_compaible_error(
							asio_convertible_error_t::write_group_destroyed_passively ) );
				} );
		}

		// Call from dtor!
		REQUIRE( 1 == counter );
	}
}
