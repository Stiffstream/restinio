/*
	restinio
*/

/*!
	Tests for specific websocket protocol validations.
*/

#define CATCH_CONFIG_MAIN

#include <bitset>

#include <catch/catch.hpp>

#include <restinio/websocket/impl/utf8.hpp>
#include <restinio/websocket/impl/ws_protocol_validator.hpp>

#include <test/common/pub.hpp>
#include <test/websocket/common/pub.hpp>

using namespace restinio;
using namespace restinio::websocket;
using namespace restinio::websocket::impl;

TEST_CASE(
	"UTF-8 check" ,
	"[validators][utf-8]" )
{
	{
		// Hello-µ@ßöäüàá-UTF-8!!
		std::string str{ to_char_each({
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

		REQUIRE( check_utf8_is_correct( str ) == true );
	}
	{
		std::string str{ to_char_each({
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

		REQUIRE( check_utf8_is_correct( str ) == false );
	}
	{
		std::string str{ to_char_each({
				0xf8, 0x88, 0x80, 0x80, 0x80
			}) };

		REQUIRE( check_utf8_is_correct( str ) == false );
	}
	{
		std::string str{ to_char_each({
				0xed, 0x9f, 0xbf
			}) };

		REQUIRE( check_utf8_is_correct( str ) == true );
	}
	{
		std::string str{ to_char_each({
				0xf0, 0x90, 0x80, 0x80
			}) };

		// for( auto ch: str )
		// {
		// 	std::cout << std::bitset<8>(ch) << std::endl;
		// }

		REQUIRE( check_utf8_is_correct( str ) == true );
	}
}


TEST_CASE(
	"websocket protocol validator" ,
	"[validators][ws_protocol_validator]" )
{
	SECTION( "Throw exceptrion on double call process_new_frame()" )
	{
		ws_protocol_validator_t validator;

		message_details_t pong{
			true, opcode_t::pong_frame, 5, 0xFFFFFFFF};

		REQUIRE_NOTHROW( validator.process_new_frame(pong) );
		REQUIRE_THROWS_AS( validator.process_new_frame(pong), std::runtime_error );
	}
	SECTION( "Throw exceptrion on invalid opcode" )
	{
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x03), 125, 0xFFFFFFFF};
			REQUIRE_THROWS_AS(
				validator.process_new_frame(frame), std::runtime_error );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x04), 125, 0xFFFFFFFF};
			REQUIRE_THROWS_AS(
				validator.process_new_frame(frame), std::runtime_error );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x05), 125, 0xFFFFFFFF};
			REQUIRE_THROWS_AS(
				validator.process_new_frame(frame), std::runtime_error );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x06), 125, 0xFFFFFFFF};
			REQUIRE_THROWS_AS(
				validator.process_new_frame(frame), std::runtime_error );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x07), 125, 0xFFFFFFFF};
			REQUIRE_THROWS_AS(
				validator.process_new_frame(frame), std::runtime_error );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x0B), 125, 0xFFFFFFFF};
			REQUIRE_THROWS_AS(
				validator.process_new_frame(frame), std::runtime_error );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x0C), 125, 0xFFFFFFFF};
			REQUIRE_THROWS_AS(
				validator.process_new_frame(frame), std::runtime_error );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x0D), 125, 0xFFFFFFFF};
			REQUIRE_THROWS_AS(
				validator.process_new_frame(frame), std::runtime_error );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x0E), 125, 0xFFFFFFFF};
			REQUIRE_THROWS_AS(
				validator.process_new_frame(frame), std::runtime_error );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x0F), 125, 0xFFFFFFFF};
			REQUIRE_THROWS_AS(
				validator.process_new_frame(frame), std::runtime_error );
		}

	}
	SECTION( "Throw exceptrion on non-final control frame" )
	{
		ws_protocol_validator_t validator;

		message_details_t close{
			false, opcode_t::connection_close_frame, 125, 0xFFFFFFFF};
		message_details_t ping{
			false, opcode_t::ping_frame, 125, 0xFFFFFFFF};
			message_details_t pong{
			false, opcode_t::pong_frame, 125, 0xFFFFFFFF};

		REQUIRE_THROWS_AS( validator.process_new_frame(close), std::runtime_error );
		REQUIRE_THROWS_AS( validator.process_new_frame(ping), std::runtime_error );
		REQUIRE_THROWS_AS( validator.process_new_frame(pong), std::runtime_error );
	}
	SECTION( "Throw exceptrion on set rsv flags" )
	{
		ws_protocol_validator_t validator;

		message_details_t frame1{true, opcode_t::binary_frame, 126, 0xFFFFFFFF};
		frame1.m_rsv1_flag = true;
		message_details_t frame2{true, opcode_t::binary_frame, 126, 0xFFFFFFFF};
		frame2.m_rsv2_flag = true;
		message_details_t frame3{true, opcode_t::binary_frame, 126, 0xFFFFFFFF};
		frame3.m_rsv3_flag = true;

		REQUIRE_THROWS_AS( validator.process_new_frame(frame1), std::runtime_error );
		REQUIRE_THROWS_AS( validator.process_new_frame(frame2), std::runtime_error );
		REQUIRE_THROWS_AS( validator.process_new_frame(frame3), std::runtime_error );
	}
	SECTION( "Throw exceptrion on empty masking key" )
	{
		ws_protocol_validator_t validator;

		message_details_t frame1{true, opcode_t::binary_frame, 126};

		REQUIRE_THROWS_AS( validator.process_new_frame(frame1), std::runtime_error );
	}
	SECTION( "Throw exceptrion on payload len 126 bytes in control frame" )
	{
		ws_protocol_validator_t validator;

		message_details_t close{
			true, opcode_t::connection_close_frame, 126, 0xFFFFFFFF};
		message_details_t ping{
			true, opcode_t::ping_frame, 126, 0xFFFFFFFF};
		message_details_t pong{
			true, opcode_t::pong_frame, 126, 0xFFFFFFFF};

		REQUIRE_THROWS_AS( validator.process_new_frame(close), std::runtime_error );
		REQUIRE_THROWS_AS( validator.process_new_frame(ping), std::runtime_error );
		REQUIRE_THROWS_AS( validator.process_new_frame(pong), std::runtime_error );
	}
	SECTION(
		"Throw exceptrion on continuation frame without previous data frame (fin=0)" )
	{
		ws_protocol_validator_t validator;

		message_details_t frame{
			true, opcode_t::continuation_frame, 5, 0xFFFFFFFF};

		REQUIRE_THROWS_AS( validator.process_new_frame(frame), std::runtime_error );
	}
	SECTION(
		"Throw exceptrion on data frame without finishing of previous data frame (fin=1)" )
	{
		ws_protocol_validator_t validator;

		message_details_t frame1{
			false, opcode_t::text_frame, 5, 0xFFFFFFFF};

		REQUIRE_NOTHROW( validator.process_new_frame(frame1) );

		validator.finish_frame();

		message_details_t frame2{
			false, opcode_t::text_frame, 5, 0xFFFFFFFF};

		REQUIRE_THROWS_AS( validator.process_new_frame(frame2), std::runtime_error );

	}
	SECTION(
		"Normal work in case of data frame then continuation frame" )
	{
		ws_protocol_validator_t validator;

		message_details_t frame1{
			false, opcode_t::text_frame, 5, 0xFFFFFFFF};

		REQUIRE_NOTHROW( validator.process_new_frame(frame1) );

		validator.finish_frame();

		message_details_t frame2{
			true, opcode_t::continuation_frame, 5, 0xFFFFFFFF};

		REQUIRE_NOTHROW( validator.process_new_frame(frame2) );
	}
	SECTION(
		"Normal work in case of data frame then control frame then continuation frame" )
	{
		ws_protocol_validator_t validator;

		message_details_t frame1{
			false, opcode_t::text_frame, 5, 0xFFFFFFFF};

		REQUIRE_NOTHROW( validator.process_new_frame(frame1) );

		validator.finish_frame();

		message_details_t frame2{
			true, opcode_t::ping_frame, 5, 0xFFFFFFFF};

		REQUIRE_NOTHROW( validator.process_new_frame(frame2) );

		validator.finish_frame();

		message_details_t frame3{
			true, opcode_t::continuation_frame, 5, 0xFFFFFFFF};

		REQUIRE_NOTHROW( validator.process_new_frame(frame3) );
	}
	SECTION(
		"Check payload is correct utf-8 sequence in text frame" )
	{
		ws_protocol_validator_t validator;

		std::string payload{ to_char_each({
			0xed, 0x9f, 0xbf }) };

		message_details_t frame{
			false, opcode_t::text_frame, payload.size(), 0xFFFFFFFF};

		REQUIRE_NOTHROW( validator.process_new_frame(frame) );

		REQUIRE_NOTHROW( validator.process_next_payload_part(
			payload.data(), payload.size() ) );
	}
	SECTION(
		"Check payload is incorrect utf-8 sequence in text frame" )
	{
		ws_protocol_validator_t validator;

		std::string payload{ to_char_each({
			0xf8, 0x88, 0x80, 0x80, 0x80 }) };

		message_details_t frame{
			false, opcode_t::text_frame, payload.size(), 0xFFFFFFFF};

		REQUIRE_NOTHROW( validator.process_new_frame(frame) );

		REQUIRE_THROWS_AS( validator.process_next_payload_part(
			payload.data(), payload.size() ) );
	}
}