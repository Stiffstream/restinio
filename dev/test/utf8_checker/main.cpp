/*
	restinio
*/

/*!
	Tests for utf8_checker.
*/

#include <catch2/catch.hpp>

#include <restinio/utils/utf8_checker.hpp>

#include <initializer_list>
#include <string>

RESTINIO_NODISCARD
std::string
make_from( std::initializer_list<int> values )
{
	std::string result;
	result.reserve( values.size() );

	for( auto ch : values ) result += static_cast<char>( ch );

	return result;
}

bool
is_valid( const std::string & what )
{
	restinio::utils::utf8_checker_t checker;

	for( auto ch : what )
	{
		if( !checker.process_byte( static_cast<std::uint8_t>( ch ) ) )
			return false;
	}

	return checker.finalized();
}

TEST_CASE( "Basic checks", "[utf-8][basic]" )
{
	{
		const std::string str{ make_from({
				0x48,
				0x65,
				0x6c,
				0x6c,
				0x6f,
				0x2d,
				0xc2,
				0xb5,
				0x40,
				0xc3,
				0x9f,
				0xc3,
				0xb6,
				0xc3,
				0xa4,
				0xc3,
				0xbc,
				0xc3,
				0xa0,
				0xc3,
				0xa1,
				0x2d,
				0x55,
				0x54,
				0x46,
				0x2d,
				0x38,
				0x21,
				0x21
			}) };

		REQUIRE( is_valid(str) );
	}
	{
		const std::string str{ make_from({
				0xce,
				0xba,
				0xe1,
				0xbd,
				0xb9,
				0xcf,
				0x83,
				0xce,
				0xbc,
				0xce,
				0xb5,
				0xed,
				0xa0,
				0x80,
				0x65,
				0x64,
				0x69,
				0x74,
				0x65,
				0x64
			}) };

		REQUIRE( !is_valid(str) );
	}
	{
		const std::string str{ make_from({
				0xf8, 0x88, 0x80, 0x80, 0x80
			}) };

		REQUIRE( !is_valid(str) );
	}
	{
		const std::string str{ make_from({
				0xed, 0x9f, 0xbf
			}) };

		REQUIRE( is_valid(str) );
	}
	{
		const std::string str{ make_from({
				0xf0, 0x90, 0x80, 0x80
			}) };

		REQUIRE( is_valid(str) );
	}
}

TEST_CASE( "Additional sequences", "[utf-8][additional]" )
{
	REQUIRE( is_valid( make_from(
			{ 0xce, 0xba, 0xe1, 0xbd, 0xb9, 0xcf, 0x83, 0xce, 0xbc, 0xce, 0xb5 }
	) ) );

	// 1-byte. U-00000000
	REQUIRE( is_valid( make_from(
			{ 0x00 }
	) ) );

	// 2-bytes. U-00000080
	REQUIRE( is_valid( make_from(
			{ 0xc2, 0x80 }
	) ) );

	// 3-bytes. U-00000800
	REQUIRE( is_valid( make_from(
			{ 0xe0, 0xa0, 0x80 }
	) ) );

	// 4-bytes. U-00010000
	REQUIRE( is_valid( make_from(
			{ 0xf0, 0x90, 0x80, 0x80 }
	) ) );

	// 5-bytes. U-00200000
	// NOTE: it's out of valid range 0x0000-0x10FFFF.
	REQUIRE( !is_valid( make_from(
			{ 0xf8, 0x88, 0x80, 0x80, 0x80 }
	) ) );

	// 6-bytes. U-04000000
	// NOTE: it's out of valid range 0x0000-0x10FFFF.
	REQUIRE( !is_valid( make_from(
			{ 0xfc, 0x84, 0x80, 0x80, 0x80, 0x80 }
	) ) );

	// 1-byte. U-0000007F
	REQUIRE( is_valid( make_from(
			{ 0x7f }
	) ) );

	// 2-bytes. U-000007FF
	REQUIRE( is_valid( make_from(
			{ 0xdf, 0xbf }
	) ) );

	// 3-bytes. U-0000FFFF
	REQUIRE( is_valid( make_from(
			{ 0xef, 0xbf, 0xbf }
	) ) );

	// 4-bytes. U-001FFFFF
	// NOTE: it's out of valid range 0x0000-0x10FFFF.
	REQUIRE( !is_valid( make_from(
			{ 0xf7, 0xbf, 0xbf, 0xbf }
	) ) );

	// 5-bytes. U-03FFFFFF
	// NOTE: it's out of valid range 0x0000-0x10FFFF.
	REQUIRE( !is_valid( make_from(
			{ 0xfb, 0xbf, 0xbf, 0xbf, 0xbf }
	) ) );

	// 6-bytes. U-7FFFFFFF
	// NOTE: it's out of valid range 0x0000-0x10FFFF.
	REQUIRE( !is_valid( make_from(
			{ 0xfd, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf }
	) ) );

	// U-0000D7FF
	REQUIRE( is_valid( make_from(
			{ 0xed, 0x9f, 0xbf }
	) ) );

	// U-0000E000
	REQUIRE( is_valid( make_from(
			{ 0xee, 0x80, 0x80 }
	) ) );

	// U-0000FFFD
	REQUIRE( is_valid( make_from(
			{ 0xef, 0xbf, 0xbd }
	) ) );

	// U-0010FFFE
	REQUIRE( is_valid( make_from(
			{ 0xf4, 0x8f, 0xbf, 0xbf }
	) ) );

	// U-00110000
	// NOTE: it's out of valid range 0x0000-0x10FFFF.
	REQUIRE( !is_valid( make_from(
			{ 0xf4, 0x90, 0x80, 0x80 }
	) ) );

	// First continuation byte.
	REQUIRE( !is_valid( make_from(
			{ 0x80 }
	) ) );

	// Last continuation byte.
	REQUIRE( !is_valid( make_from(
			{ 0xbf }
	) ) );

	// Combinations of continuation bytes.
	REQUIRE( !is_valid( make_from(
			{ 0x80, 0xbf }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0x80, 0xbf, 0x80 }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0x80, 0xbf, 0x80, 0xbf }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0x80, 0xbf, 0x80, 0xbf, 0x80 }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0x80, 0xbf, 0x80, 0xbf, 0x80, 0xbf }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0x80, 0xbf, 0x80, 0xbf, 0x80, 0xbf, 0x80 }
	) ) );

	// Checks for all possible continuation bytes from 0x80 to 0xbf.
	{
		for( int ch = 0x80; ch <= 0xbf; ++ch )
			REQUIRE( !is_valid( make_from( { ch } ) ) );
	}

	// All 32 first bytes of 2-byte sequences (0xc0-0xdf) followed by space.
	{
		for( int ch = 0xc0; ch <= 0xdf; ++ch )
			REQUIRE( !is_valid( make_from( { ch, ' ' } ) ) );
	}

	// All 16 first bytes of 3-byte sequences (0xe0-0xef) followed by space.
	{
		for( int ch = 0xe0; ch <= 0xef; ++ch )
			REQUIRE( !is_valid( make_from( { ch, ' ' } ) ) );
	}

	// All 8 first bytes of 4-byte sequences (0xf0-0xf7) followed by space.
	{
		for( int ch = 0xf0; ch <= 0xf7; ++ch )
			REQUIRE( !is_valid( make_from( { ch, ' ' } ) ) );
	}

	// All 4 first bytes of 5-byte sequences (0xf8-0xfb) followed by space.
	{
		for( int ch = 0xf8; ch <= 0xfb; ++ch )
			REQUIRE( !is_valid( make_from( { ch, ' ' } ) ) );
	}

	// All 2 first bytes of 5-byte sequences (0xfc-0xfd) followed by space.
	{
		for( int ch = 0xfc; ch <= 0xfd; ++ch )
			REQUIRE( !is_valid( make_from( { ch, ' ' } ) ) );
	}

	// Sequences with the last byte missing.
	REQUIRE( !is_valid( make_from(
			{ 0xc0 }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xe0, 0x80 }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xf0, 0x80, 0x80 }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xf8, 0x80, 0x80, 0x80 }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xfc, 0x80, 0x80, 0x80, 0x80 }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xdf }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xef, 0xbf }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xf7, 0xbf, 0xbf }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xfb, 0xbf, 0xbf, 0xbf }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xfd, 0xbf, 0xbf, 0xbf, 0xbf }
	) ) );

	// All 10 invalid sequences from above concatenated.
	REQUIRE( !is_valid( make_from(
			{ 0xc0,
			  0xe0, 0x80,
			  0xf0, 0x80, 0x80,
			  0xf8, 0x80, 0x80, 0x80,
			  0xfc, 0x80, 0x80, 0x80, 0x80,
			  0xdf,
			  0xef, 0xbf,
			  0xf7, 0xbf, 0xbf,
			  0xfb, 0xbf, 0xbf, 0xbf,
			  0xfd, 0xbf, 0xbf, 0xbf, 0xbf }
	) ) );

	// Impossible bytes.
	REQUIRE( !is_valid( make_from(
			{ ' ', 0xfe, ' ' }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xc2, 0xfe }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ ' ', 0xff, ' ' }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xc2, 0xff }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ ' ', 0xfe, 0xfe, 0xff, 0xff, ' ' }
	) ) );
}

TEST_CASE( "Overlong sequences", "[utf-8][overlong]" )
{
	REQUIRE( !is_valid( make_from(
			{ 0xc0, 0xaf }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xe0, 0x80, 0xaf }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xf0, 0x80, 0x80, 0xaf }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xf8, 0x80, 0x80, 0x80, 0xaf }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xfc, 0x80, 0x80, 0x80, 0x80, 0xaf }
	) ) );

	REQUIRE( !is_valid( make_from(
			{ 0xc1, 0xbf }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xe0, 0x9f, 0xbf }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xf0, 0x8f, 0xbf, 0xbf }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xf8, 0x87, 0xbf, 0xbf, 0xbf }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xfc, 0x83, 0xbf, 0xbf, 0xbf, 0xbf }
	) ) );

	REQUIRE( !is_valid( make_from(
			{ 0xc0, 0x80 }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xe0, 0x80, 0x80 }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xf0, 0x80, 0x80, 0x80 }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xf8, 0x80, 0x80, 0x80, 0x80 }
	) ) );
	REQUIRE( !is_valid( make_from(
			{ 0xfc, 0x80, 0x80, 0x80, 0x80, 0x80 }
	) ) );

}

TEST_CASE( "Illegal code positions", "[utf-8][illegal-positions]" )
{
	// Single UTF-16 surrogates.
	//
	// U+D800
	REQUIRE( !is_valid( make_from(
			{ 0xed, 0xa0, 0x80 }
	) ) );
	// U+DB7F
	REQUIRE( !is_valid( make_from(
			{ 0xed, 0xad, 0xbf }
	) ) );
	// U+DB80
	REQUIRE( !is_valid( make_from(
			{ 0xed, 0xae, 0x80 }
	) ) );
	// U+DBFF
	REQUIRE( !is_valid( make_from(
			{ 0xed, 0xaf, 0xbf }
	) ) );
	// U+DC00
	REQUIRE( !is_valid( make_from(
			{ 0xed, 0xb0, 0x80 }
	) ) );
	// U+DF80
	REQUIRE( !is_valid( make_from(
			{ 0xed, 0xbe, 0x80 }
	) ) );
	// U+DFFF
	REQUIRE( !is_valid( make_from(
			{ 0xed, 0xbf, 0xbf }
	) ) );

	// Paired UTF-16 surrogates
	// U+DB800 U+DC00
	REQUIRE( !is_valid( make_from(
			{ 0xed, 0xa0, 0x80, 0xed, 0xb0, 0x80 }
	) ) );
	// U+D800 U+DFFF
	REQUIRE( !is_valid( make_from(
			{ 0xed, 0xa0, 0x80, 0xed, 0xbf, 0xbf }
	) ) );
	// U+DB7F U+DC00
	REQUIRE( !is_valid( make_from(
			{ 0xed, 0xad, 0xbf, 0xed, 0xb0, 0x80 }
	) ) );
	// U+DB7F U+DFFF
	REQUIRE( !is_valid( make_from(
			{ 0xed, 0xad, 0xbf, 0xed, 0xbf, 0xbf }
	) ) );
	// U+DB80 U+DC00
	REQUIRE( !is_valid( make_from(
			{ 0xed, 0xae, 0x80, 0xed, 0xb0, 0x80 }
	) ) );
	// U+DB80 U+DFFF
	REQUIRE( !is_valid( make_from(
			{ 0xed, 0xae, 0x80, 0xed, 0xbf, 0xbf }
	) ) );
	// U+DBFF U+DC00
	REQUIRE( !is_valid( make_from(
			{ 0xed, 0xaf, 0xbf, 0xed, 0xb0, 0x80 }
	) ) );
	// U+DBFF U+DFFF
	REQUIRE( !is_valid( make_from(
			{ 0xed, 0xaf, 0xbf, 0xed, 0xbf, 0xbf }
	) ) );
}

