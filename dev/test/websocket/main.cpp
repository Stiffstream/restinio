/*
	restinio
*/

/*!
	Echo server.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <restinio/all.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

using namespace restinio;

char
to_char( int val )
{
	return static_cast<char>(val);
};

TEST_CASE( "Validate parser implementation details" , "[websocket][parser][impl]" )
{
	impl::expected_data_t exp_data(2);

	REQUIRE_FALSE( exp_data.add_byte_and_check_size(0x81) );
	REQUIRE( exp_data.add_byte_and_check_size(0x05) );
	REQUIRE_THROWS( exp_data.add_byte_and_check_size(0xF1) );

	exp_data.reset(1);

	REQUIRE( exp_data.add_byte_and_check_size(0x81) );
	REQUIRE_THROWS( exp_data.add_byte_and_check_size(0x05) );

}

TEST_CASE( "Parse simple message" , "[websocket][parser]" )
{
	raw_data_t bin_data{ to_char(0x81), to_char(0x05), to_char(0x48), to_char(0x65), to_char(0x6C), to_char(0x6C), to_char(0x6F) };

	ws_parser_t parser;

	auto nparsed = parser.parser_execute( bin_data.data(), bin_data.size() );

	REQUIRE( nparsed == 2 );
	REQUIRE( parser.waiting_for_reset() == true );

	auto ws_message = parser.current_message();
	auto header = ws_message.m_header;

	REQUIRE( header.m_final_flag == true );
	REQUIRE( header.m_rsv1_flag == false );
	REQUIRE( header.m_rsv2_flag == false );
	REQUIRE( header.m_rsv3_flag == false );
	REQUIRE( header.m_opcode == restinio::opcode_t::text_frame );

	REQUIRE( ws_message.payload_len() == 5 );

	parser.reset();

	REQUIRE( parser.waiting_for_reset() == false );
	REQUIRE( parser.current_message().payload_len() == 0 );
}

TEST_CASE( "Parse simple message (chunked)" , "[websocket][parser]" )
{
	raw_data_t bin_data{ to_char(0x81), to_char(0x05), to_char(0x48), to_char(0x65), to_char(0x6C), to_char(0x6C), to_char(0x6F) };

	ws_parser_t parser;

	int shift = 0;

	for( ; shift < bin_data.size() - 1 ; ++shift  )
	{
		if( !parser.waiting_for_reset() )
		{
			auto nparsed = parser.parser_execute( bin_data.data() + shift, 1 );

			REQUIRE( nparsed == 1 );
		}
		else
		{
			break;
		}
	}

	REQUIRE( shift == 2 );

	auto ws_message = parser.current_message();
	auto header = ws_message.m_header;

	REQUIRE( header.m_final_flag == true );
	REQUIRE( header.m_rsv1_flag == false );
	REQUIRE( header.m_rsv2_flag == false );
	REQUIRE( header.m_rsv3_flag == false );
	REQUIRE( header.m_opcode == restinio::opcode_t::text_frame );

	REQUIRE( ws_message.payload_len() == 5 );
}
