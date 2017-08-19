/*
	restinio
*/

/*!
	Echo server.
*/

#define CATCH_CONFIG_MAIN

#include <bitset>

#include <catch/catch.hpp>

#include <restinio/all.hpp>
#include <restinio/impl/ws_parser.hpp>
#include <restinio/websocket.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

using namespace restinio;
using namespace restinio::impl;

char
to_char( int val )
{
	return static_cast<char>(val);
};

raw_data_t
to_char_each( std::vector< int > source )
{
	raw_data_t result;
	result.reserve( source.size() );

	for( const auto & val : source )
	{
		result.push_back( to_char(val) );
	}

	return result;
}

void
print_as_hex( const raw_data_t & data )
{
	for( const auto & byte : data )
	{
		std::cout << std::hex << static_cast<std::int16_t>(byte) << " ";
	}

	std::cout << std::endl;
}


TEST_CASE( "Validate parser implementation details" , "[websocket][parser][impl]" )
{
	expected_data_t exp_data(2);

	REQUIRE_FALSE( exp_data.add_byte_and_check_size(0x81) );
	REQUIRE( exp_data.add_byte_and_check_size(0x05) );
	REQUIRE_THROWS( exp_data.add_byte_and_check_size(0xF1) );

	exp_data.reset(1);

	REQUIRE( exp_data.add_byte_and_check_size(0x81) );
	REQUIRE_THROWS( exp_data.add_byte_and_check_size(0x05) );
}

TEST_CASE( "Validate websocket message details constructing" , "[websocket][parser][message]" )
{
	ws_message_details_t m0;
	REQUIRE( m0.m_final_flag == true );
	REQUIRE( m0.m_rsv1_flag == false );
	REQUIRE( m0.m_rsv2_flag == false );
	REQUIRE( m0.m_rsv3_flag == false );
	REQUIRE( m0.m_opcode == opcode_t::continuation_frame );
	REQUIRE( m0.m_mask_flag == false );
	REQUIRE( m0.m_payload_len == 0 );
	REQUIRE( m0.m_ext_payload_len == 0 );
	REQUIRE( m0.m_masking_key == 0 );

	ws_message_details_t m1{false, opcode_t::text_frame, 125};
	REQUIRE( m1.m_final_flag == false );
	REQUIRE( m1.m_rsv1_flag == false );
	REQUIRE( m1.m_rsv2_flag == false );
	REQUIRE( m1.m_rsv3_flag == false );
	REQUIRE( m1.m_opcode == opcode_t::text_frame );
	REQUIRE( m1.m_mask_flag == false );
	REQUIRE( m1.m_payload_len == 125 );
	REQUIRE( m1.m_ext_payload_len == 0 );
	REQUIRE( m1.m_masking_key == 0 );

	ws_message_details_t m2{true, opcode_t::binary_frame, 126};
	REQUIRE( m2.m_final_flag == true );
	REQUIRE( m2.m_rsv1_flag == false );
	REQUIRE( m2.m_rsv2_flag == false );
	REQUIRE( m2.m_rsv3_flag == false );
	REQUIRE( m2.m_opcode == opcode_t::binary_frame );
	REQUIRE( m2.m_mask_flag == false );
	REQUIRE( m2.m_payload_len == 126 );
	REQUIRE( m2.m_ext_payload_len == 126 );
	REQUIRE( m2.m_masking_key == 0 );

	ws_message_details_t m3{true, opcode_t::binary_frame, 65536};
	REQUIRE( m3.m_final_flag == true );
	REQUIRE( m3.m_rsv1_flag == false );
	REQUIRE( m3.m_rsv2_flag == false );
	REQUIRE( m3.m_rsv3_flag == false );
	REQUIRE( m3.m_opcode == opcode_t::binary_frame );
	REQUIRE( m3.m_mask_flag == false );
	REQUIRE( m3.m_payload_len == 127 );
	REQUIRE( m3.m_ext_payload_len == 65536 );
	REQUIRE( m3.m_masking_key == 0 );
}

TEST_CASE( "Validate mask and unmask operations" , "[websocket][parser][mask]" )
{
	raw_data_t unmasked_bin_data_etalon = "Hello";
	raw_data_t bin_data = unmasked_bin_data_etalon;
	uint32_t mask_key = 0x3D21FA37;
	mask_unmask_payload( mask_key, bin_data );
	raw_data_t masked_bin_data_etalon{
		to_char(0x7F), to_char(0x9F), to_char(0x4D), to_char(0x51), to_char(0x58) };

	REQUIRE( bin_data == masked_bin_data_etalon );
	mask_unmask_payload( mask_key, bin_data );
	REQUIRE( bin_data == unmasked_bin_data_etalon );
}

TEST_CASE( "Reset parser" , "[websocket][parser][reset]" )
{
	raw_data_t bin_data{ to_char_each({0x81, 0x05}) };
	ws_parser_t parser;
	auto nparsed = parser.parser_execute( bin_data.data(), bin_data.size() );

	REQUIRE( nparsed == 2 );
	REQUIRE( parser.header_parsed() == true );
	auto ws_message_details = parser.current_message();

	REQUIRE( ws_message_details.payload_len() == 5 );
	parser.reset();
	REQUIRE( parser.header_parsed() == false );
	REQUIRE( parser.current_message().payload_len() == 0 );
}

TEST_CASE( "Parse header (2 bytes only)" , "[websocket][parser][read]" )
{
	{
		raw_data_t bin_data{ to_char_each({0x81, 0x05}) };
		ws_parser_t parser;
		auto nparsed = parser.parser_execute( bin_data.data(), bin_data.size() );

		REQUIRE( nparsed == 2 );
		REQUIRE( parser.header_parsed() == true );

		auto ws_message_details = parser.current_message();

		REQUIRE( ws_message_details.m_final_flag == true );
		REQUIRE( ws_message_details.m_rsv1_flag == false );
		REQUIRE( ws_message_details.m_rsv2_flag == false );
		REQUIRE( ws_message_details.m_rsv3_flag == false );
		REQUIRE( ws_message_details.m_opcode == opcode_t::text_frame );
		REQUIRE( ws_message_details.payload_len() == 5 );
	}
}

TEST_CASE( "Parse header (2 bytes + 2 bytes ext length)" , "[websocket][parser][read]" )
{
	{
		// 127
		raw_data_t bin_data{ to_char_each({0x82, 0x7E, 0x00, 0x7F}) };
		ws_parser_t parser;
		auto nparsed = parser.parser_execute( bin_data.data(), bin_data.size() );

		REQUIRE( parser.current_message().payload_len() == 127 );
	}
	{
		// 128
		raw_data_t bin_data{ to_char_each({0x82, 0x7E, 0x00, 0x80}) };
		ws_parser_t parser;
		auto nparsed = parser.parser_execute( bin_data.data(), bin_data.size() );

		REQUIRE( parser.current_message().payload_len() == 128 );
	}
	{
		// 1000
		raw_data_t bin_data{ to_char_each({0x82, 0x7E, 0x03, 0xE8}) };
		ws_parser_t parser;
		auto nparsed = parser.parser_execute( bin_data.data(), bin_data.size() );
		REQUIRE( parser.current_message().payload_len() == 1000 );
	}
	{
		// 32783
		raw_data_t bin_data{ to_char_each({0x82, 0x7E, 0x80, 0x0F}) };
		ws_parser_t parser;
		auto nparsed = parser.parser_execute( bin_data.data(), bin_data.size() );
		REQUIRE( parser.current_message().payload_len() == 32783 );
	}
}

TEST_CASE( "Parse header (2 bytes + 8 bytes ext length)" , "[websocket][parser][read]" )
{
	{
		raw_data_t bin_data{ to_char_each(
			{0x82, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F}) };
		ws_parser_t parser;
		auto nparsed = parser.parser_execute( bin_data.data(), bin_data.size() );

		REQUIRE( parser.current_message().payload_len() == 0x7F );
	}
	{
		raw_data_t bin_data{ to_char_each(
			{0x82, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80}) };
		ws_parser_t parser;
		auto nparsed = parser.parser_execute( bin_data.data(), bin_data.size() );

		REQUIRE( parser.current_message().payload_len() == 0x80 );
	}
	{
		raw_data_t bin_data{ to_char_each(
			{0x82, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x7F}) };
		ws_parser_t parser;
		auto nparsed = parser.parser_execute( bin_data.data(), bin_data.size() );

		REQUIRE( parser.current_message().payload_len() == 0x7F7F );
	}
	{
		raw_data_t bin_data{ to_char_each(
			{0x82, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x11}) };
		ws_parser_t parser;
		auto nparsed = parser.parser_execute( bin_data.data(), bin_data.size() );

		REQUIRE( parser.current_message().payload_len() == 0x8011 );
	}
}

TEST_CASE( "Parse header (2 bytes + 4 bytes masking key )" ,
	"[websocket][parser][read]" )
{
	{
		raw_data_t bin_data{ to_char_each({
			0x81, 0x85, 0x37, 0xFA, 0x21, 0x3D}) };
		ws_parser_t parser;
		auto nparsed = parser.parser_execute( bin_data.data(), bin_data.size() );

		REQUIRE( parser.current_message().m_mask_flag == true );
		REQUIRE( parser.current_message().m_masking_key == 0x37FA213D );
	}
}

TEST_CASE( "Parse simple message" , "[websocket][parser][read]" )
{
	raw_data_t bin_data{ to_char(0x81), to_char(0x05), to_char(0x48), to_char(0x65), to_char(0x6C), to_char(0x6C), to_char(0x6F) };

	ws_parser_t parser;

	auto nparsed = parser.parser_execute( bin_data.data(), bin_data.size() );

	REQUIRE( nparsed == 2 );
	REQUIRE( parser.header_parsed() == true );

	auto ws_message_details = parser.current_message();

	REQUIRE( ws_message_details.m_final_flag == true );
	REQUIRE( ws_message_details.m_rsv1_flag == false );
	REQUIRE( ws_message_details.m_rsv2_flag == false );
	REQUIRE( ws_message_details.m_rsv3_flag == false );
	REQUIRE( ws_message_details.m_opcode == opcode_t::text_frame );

	REQUIRE( ws_message_details.payload_len() == 5 );
}

TEST_CASE( "Parse simple message (chunked)" , "[websocket][parser][read]" )
{
	raw_data_t bin_data{ to_char(0x81), to_char(0x05), to_char(0x48), to_char(0x65), to_char(0x6C), to_char(0x6C), to_char(0x6F) };

	ws_parser_t parser;

	int shift = 0;

	for( ; shift < bin_data.size() - 1 ; ++shift  )
	{
		if( !parser.header_parsed() )
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

	auto ws_message_details = parser.current_message();

	REQUIRE( ws_message_details.m_final_flag == true );
	REQUIRE( ws_message_details.m_rsv1_flag == false );
	REQUIRE( ws_message_details.m_rsv2_flag == false );
	REQUIRE( ws_message_details.m_rsv3_flag == false );
	REQUIRE( ws_message_details.m_opcode == opcode_t::text_frame );

	REQUIRE( ws_message_details.payload_len() == 5 );
}

TEST_CASE( "Write simple message" , "[websocket][parser][write]" )
{
	{
		ws_message_details_t m;

		raw_data_t bin_data = write_message_details( m );
		raw_data_t etalon{ to_char(0x80), to_char(0x00) };

		REQUIRE( bin_data == etalon );
	}
	{
		std::string payload = "Hello";
		ws_message_details_t m{true, opcode_t::text_frame, payload.size()};

		raw_data_t bin_data = write_message_details( m );
		bin_data.append( payload );

		raw_data_t etalon{
			to_char_each({0x81, 0x05, 0x48, 0x65, 0x6C, 0x6C, 0x6F}) };

		REQUIRE( bin_data == etalon );
	}
	{
		std::string payload = "Hello";
		ws_message_details_t m{true, opcode_t::text_frame, payload.size()};

		std::uint32_t masking_key = 0x3D21FA37;
		m.set_masking_key(masking_key);

		mask_unmask_payload(masking_key, payload);

		raw_data_t bin_data = write_message_details( m );
		bin_data.append( payload );

		raw_data_t etalon{
			to_char_each(
				{0x81, 0x85, 0x37, 0xFA, 0x21, 0x3D, 0x7F, 0x9F, 0x4D, 0x51, 0x58}) };

		REQUIRE( bin_data == etalon );
	}
}