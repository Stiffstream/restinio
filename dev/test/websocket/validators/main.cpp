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

std::string
status_code_to_bin( uint16_t code)
{
	using namespace ::restinio::utils::impl::bitops;

	return {
		n_bits_from<char, 8>( code ),
		n_bits_from<char, 0>( code )
	};
}

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
	"testing functionality of payload unmasker class" ,
	"[validators][unmasker]" )
{
	std::string masked_payload{
		to_char_each({0x7F, 0x9F, 0x4D, 0x51, 0x58}) };

	unmasker_t unmasker{ 0x37FA213D };

	std::string result;

	for( auto byte: masked_payload )
	{
		result.push_back( static_cast<char>( unmasker.unmask_byte(byte) ) );
	}

	REQUIRE( result == std::string{"Hello"} );
}

TEST_CASE(
	"websocket protocol validator" ,
	"[validators][ws_protocol_validator]" )
{
	SECTION( "Set invalid validation state on double call process_new_frame()" )
	{
		ws_protocol_validator_t validator;

		message_details_t pong{
			true, opcode_t::pong_frame, 5, 0xFFFFFFFF};

		REQUIRE_NOTHROW( validator.process_new_frame(pong) );
		REQUIRE_THROWS_AS( validator.process_new_frame(pong), std::runtime_error );
	}
	SECTION( "Set invalid validation state on invalid opcode" )
	{
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x03), 125, 0xFFFFFFFF};
			REQUIRE( validator.process_new_frame(frame) ==
				validation_state_t::invalid_opcode );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x04), 125, 0xFFFFFFFF};
			REQUIRE( validator.process_new_frame(frame) ==
				validation_state_t::invalid_opcode );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x05), 125, 0xFFFFFFFF};
			REQUIRE( validator.process_new_frame(frame) ==
				validation_state_t::invalid_opcode );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x06), 125, 0xFFFFFFFF};
			REQUIRE( validator.process_new_frame(frame) ==
				validation_state_t::invalid_opcode );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x07), 125, 0xFFFFFFFF};
			REQUIRE( validator.process_new_frame(frame) ==
				validation_state_t::invalid_opcode );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x0B), 125, 0xFFFFFFFF};
			REQUIRE( validator.process_new_frame(frame) ==
				validation_state_t::invalid_opcode );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x0C), 125, 0xFFFFFFFF};
			REQUIRE( validator.process_new_frame(frame) ==
				validation_state_t::invalid_opcode );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x0D), 125, 0xFFFFFFFF};
			REQUIRE( validator.process_new_frame(frame) ==
				validation_state_t::invalid_opcode );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x0E), 125, 0xFFFFFFFF};
			REQUIRE( validator.process_new_frame(frame) ==
				validation_state_t::invalid_opcode );
		}
		{
			ws_protocol_validator_t validator;
			message_details_t frame{
				true, static_cast<opcode_t>(0x0F), 125, 0xFFFFFFFF};
			REQUIRE( validator.process_new_frame(frame) ==
				validation_state_t::invalid_opcode );
		}

	}
	SECTION( "Set invalid validation state on non-final control frame" )
	{
		ws_protocol_validator_t validator;

		message_details_t close{
			false, opcode_t::connection_close_frame, 125, 0xFFFFFFFF};
		message_details_t ping{
			false, opcode_t::ping_frame, 125, 0xFFFFFFFF};
			message_details_t pong{
			false, opcode_t::pong_frame, 125, 0xFFFFFFFF};

		REQUIRE( validator.process_new_frame(close) ==
			validation_state_t::non_final_control_frame );
		REQUIRE( validator.process_new_frame(ping) ==
			validation_state_t::non_final_control_frame );
		REQUIRE( validator.process_new_frame(pong) ==
			validation_state_t::non_final_control_frame );
	}
	SECTION( "Set invalid validation state on set rsv flags" )
	{
		ws_protocol_validator_t validator;

		message_details_t frame1{true, opcode_t::binary_frame, 126, 0xFFFFFFFF};
		frame1.m_rsv1_flag = true;
		message_details_t frame2{true, opcode_t::binary_frame, 126, 0xFFFFFFFF};
		frame2.m_rsv2_flag = true;
		message_details_t frame3{true, opcode_t::binary_frame, 126, 0xFFFFFFFF};
		frame3.m_rsv3_flag = true;

		REQUIRE( validator.process_new_frame(frame1) ==
			validation_state_t::non_zero_rsv_flags );
		REQUIRE( validator.process_new_frame(frame2) ==
			validation_state_t::non_zero_rsv_flags );
		REQUIRE( validator.process_new_frame(frame3) ==
			validation_state_t::non_zero_rsv_flags );
	}
	SECTION( "Set invalid validation state on empty masking key" )
	{
		ws_protocol_validator_t validator;

		message_details_t frame1{true, opcode_t::binary_frame, 126};

		REQUIRE( validator.process_new_frame(frame1) ==
			validation_state_t::empty_mask_from_client_side );
	}
	SECTION( "Set invalid validation state on payload len 126 bytes in control frame" )
	{
		ws_protocol_validator_t validator;

		message_details_t close{
			true, opcode_t::connection_close_frame, 126, 0xFFFFFFFF};
		message_details_t ping{
			true, opcode_t::ping_frame, 126, 0xFFFFFFFF};
		message_details_t pong{
			true, opcode_t::pong_frame, 126, 0xFFFFFFFF};

		REQUIRE( validator.process_new_frame(close) ==
			validation_state_t::payload_len_is_too_big );
		REQUIRE( validator.process_new_frame(ping) ==
			validation_state_t::payload_len_is_too_big );
		REQUIRE( validator.process_new_frame(pong) ==
			validation_state_t::payload_len_is_too_big );
	}
	SECTION(
		"Set invalid validation state on continuation frame without previous data frame (fin=0)" )
	{
		ws_protocol_validator_t validator;

		message_details_t frame{
			true, opcode_t::continuation_frame, 5, 0xFFFFFFFF};

		REQUIRE( validator.process_new_frame(frame) ==
			validation_state_t::continuation_frame_without_data_frame );
	}
	SECTION(
		"Set invalid validation state on data frame without finishing of previous data frame (fin=1)" )
	{
		ws_protocol_validator_t validator;

		message_details_t frame1{
			false, opcode_t::text_frame, 5, 0xFFFFFFFF};

		REQUIRE_NOTHROW( validator.process_new_frame(frame1) );

		validator.finish_frame();

		message_details_t frame2{
			false, opcode_t::text_frame, 5, 0xFFFFFFFF};

		REQUIRE( validator.process_new_frame(frame2) ==
			validation_state_t::new_data_frame_without_finishing_previous );

	}
	SECTION(
		"Normal work in case of receive data frame then continuation frame" )
	{
		ws_protocol_validator_t validator;

		message_details_t frame1{
			false, opcode_t::text_frame, 5, 0xFFFFFFFF};

		REQUIRE( validator.process_new_frame(frame1) ==
			validation_state_t::frame_header_is_valid );

		REQUIRE( validator.finish_frame() ==
			validation_state_t::frame_is_valid );

		message_details_t frame2{
			true, opcode_t::continuation_frame, 5, 0xFFFFFFFF};

		REQUIRE( validator.process_new_frame(frame2) ==
			validation_state_t::frame_header_is_valid );

		REQUIRE( validator.finish_frame() ==
			validation_state_t::frame_is_valid );
	}
	SECTION(
		"Normal work in case of receive data frame then control frame then continuation frame" )
	{
		ws_protocol_validator_t validator;

		message_details_t frame1{
			false, opcode_t::text_frame, 5, 0xFFFFFFFF};

		REQUIRE( validator.process_new_frame(frame1) ==
			validation_state_t::frame_header_is_valid );

		REQUIRE( validator.finish_frame() ==
			validation_state_t::frame_is_valid );

		message_details_t frame2{
			true, opcode_t::ping_frame, 5, 0xFFFFFFFF};

		REQUIRE( validator.process_new_frame(frame2) ==
			validation_state_t::frame_header_is_valid );

		REQUIRE( validator.finish_frame() ==
			validation_state_t::frame_is_valid );

		message_details_t frame3{
			true, opcode_t::continuation_frame, 5, 0xFFFFFFFF};

		REQUIRE( validator.process_new_frame(frame3) ==
			validation_state_t::frame_header_is_valid );
	}
	SECTION(
		"Check payload is correct utf-8 sequence in text frame" )
	{
		ws_protocol_validator_t validator;

		std::string payload{ to_char_each({
			0xed, 0x9f, 0xbf }) };

		message_details_t frame{
			false, opcode_t::text_frame, payload.size(), 0xFFFFFFFF};

		REQUIRE( validator.process_new_frame(frame) ==
			validation_state_t::frame_header_is_valid );

		REQUIRE( validator.process_next_payload_part(
			payload.data(), payload.size() ) ==
			validation_state_t::payload_part_is_valid );

		REQUIRE( validator.finish_frame() ==
			validation_state_t::frame_is_valid );
	}
	SECTION(
		"Check payload is correct utf-8 sequence in text frame (masked)" )
	{
		ws_protocol_validator_t validator{ true };

		std::string payload{ to_char_each({
			0x7F, 0x9F, 0x4D, 0x51, 0x58 }) };

		message_details_t frame{
			false, opcode_t::text_frame, payload.size(), 0x37FA213D };

		REQUIRE( validator.process_new_frame(frame) ==
			validation_state_t::frame_header_is_valid );

		REQUIRE( validator.process_next_payload_part(
			payload.data(), payload.size() ) ==
			validation_state_t::payload_part_is_valid );

		REQUIRE( validator.finish_frame() ==
			validation_state_t::frame_is_valid );
	}
	SECTION(
		"Check payload is incorrect utf-8 sequence in text frame" )
	{
		ws_protocol_validator_t validator;

		std::string payload{ to_char_each({
			0xf8, 0x88, 0x80, 0x80, 0x80 }) };

		message_details_t frame{
			false, opcode_t::text_frame, payload.size(), 0xFFFFFFFF};

		REQUIRE( validator.process_new_frame(frame) ==
			validation_state_t::frame_header_is_valid );

		REQUIRE( validator.process_next_payload_part(
			payload.data(), payload.size() ) ==
			validation_state_t::incorrect_utf8_data );
	}
	SECTION(
		"Check payload is correct utf-8 sequence in text frame (1 byte chunks)" )
	{
		ws_protocol_validator_t validator;

		std::string payload{ to_char_each({
			0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2d, 0xc2, 0xb5, 0x40, 0xc3,
			0x9f, 0xc3, 0xb6, 0xc3, 0xa4, 0xc3, 0xbc, 0xc3, 0xa0, 0xc3,
			0xa1, 0x2d, 0x55, 0x54, 0x46, 0x2d, 0x38, 0x21, 0x21 }) };

		message_details_t frame{
			true, opcode_t::text_frame, payload.size(), 0xFFFFFFFF};

		REQUIRE( validator.process_new_frame(frame) ==
			validation_state_t::frame_header_is_valid );

		for( const auto & byte: payload )
		{
			REQUIRE( validator.process_next_payload_part(
				&byte, 1 ) ==
				validation_state_t::payload_part_is_valid );
		}

		REQUIRE( validator.finish_frame() ==
			validation_state_t::frame_is_valid );
	}
	SECTION(
		"Check payload is correct utf-8 sequence in text and continuation frames" )
	{
		ws_protocol_validator_t validator;

		std::string payload{ to_char_each({
			0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2d, 0xc2, 0xb5, 0x40, 0xc3 }) };

		std::string payload1{ to_char_each({
			0x9f, 0xc3, 0xb6, 0xc3, 0xa4, 0xc3, 0xbc, 0xc3, 0xa0, 0xc3 }) };

		std::string payload2{ to_char_each({
			0xa1, 0x2d, 0x55, 0x54, 0x46, 0x2d, 0x38, 0x21, 0x21 }) };

		message_details_t text_frame{
			false, opcode_t::text_frame, payload.size(), 0xFFFFFFFF};

		message_details_t continuation_frame1{
			false, opcode_t::continuation_frame, payload1.size(), 0xFFFFFFFF};

		message_details_t continuation_frame2{
			true, opcode_t::continuation_frame, payload2.size(), 0xFFFFFFFF};

		// TEXT
		REQUIRE( validator.process_new_frame(text_frame) ==
			validation_state_t::frame_header_is_valid );

		REQUIRE( validator.process_next_payload_part(
			payload.data(), payload.size() ) ==
			validation_state_t::payload_part_is_valid );

		REQUIRE( validator.finish_frame() ==
			validation_state_t::frame_is_valid );

		// CONTINUATION 1
		REQUIRE( validator.process_new_frame(continuation_frame1) ==
			validation_state_t::frame_header_is_valid );

		REQUIRE( validator.process_next_payload_part(
			payload1.data(), payload1.size() ) ==
			validation_state_t::payload_part_is_valid );

		REQUIRE( validator.finish_frame() ==
			validation_state_t::frame_is_valid );

		// CONTINUATION 2
		REQUIRE( validator.process_new_frame(continuation_frame2) ==
			validation_state_t::frame_header_is_valid );

		REQUIRE( validator.process_next_payload_part(
			payload2.data(), payload2.size() ) ==
			validation_state_t::payload_part_is_valid );

		REQUIRE( validator.finish_frame() ==
			validation_state_t::frame_is_valid );
	}
	SECTION(
		"Check payload is incorrect utf-8 sequence in text and continuation frames" )
	{
		ws_protocol_validator_t validator;

		std::string payload{ to_char_each({
			0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2d, 0xc2, 0xb5, 0x40, 0xc3 }) };

		std::string payload1{ to_char_each({
			0x9f, 0xc3, 0xb6, 0xc3, 0xa4, 0xc3, 0xbc, 0xc3, 0xa0, 0xc3 }) };

		std::string payload2{ to_char_each({
			0xa1, 0x2d, 0xFF, 0x54, 0xFF, 0x2d, 0x38, 0xFF, 0x21 }) };

		message_details_t text_frame{
			false, opcode_t::text_frame, payload.size(), 0xFFFFFFFF};

		message_details_t continuation_frame1{
			false, opcode_t::continuation_frame, payload1.size(), 0xFFFFFFFF};

		message_details_t continuation_frame2{
			true, opcode_t::continuation_frame, payload2.size(), 0xFFFFFFFF};

		// TEXT
		REQUIRE( validator.process_new_frame(text_frame) ==
			validation_state_t::frame_header_is_valid );

		REQUIRE( validator.process_next_payload_part(
			payload.data(), payload.size() ) ==
			validation_state_t::payload_part_is_valid );

		REQUIRE( validator.finish_frame() ==
			validation_state_t::frame_is_valid );

		// CONTINUATION 1
		REQUIRE( validator.process_new_frame(continuation_frame1) ==
			validation_state_t::frame_header_is_valid );

		REQUIRE( validator.process_next_payload_part(
			payload1.data(), payload1.size() ) ==
			validation_state_t::payload_part_is_valid );

		REQUIRE( validator.finish_frame() ==
			validation_state_t::frame_is_valid );

		// CONTINUATION 2
		REQUIRE( validator.process_new_frame(continuation_frame2) ==
			validation_state_t::frame_header_is_valid );

		REQUIRE( validator.process_next_payload_part(
			payload2.data(), payload2.size() ) ==
			validation_state_t::incorrect_utf8_data );
	}
	SECTION(
		"Check close frame with correct close code" )
	{
		ws_protocol_validator_t validator;

		std::string payload = status_code_to_bin(1000);

		message_details_t close_frame{
			true, opcode_t::connection_close_frame, payload.size(), 0xFFFFFFFF};

		REQUIRE( validator.process_new_frame( close_frame ) ==
			validation_state_t::frame_header_is_valid );
		REQUIRE(
			validator.process_next_payload_part( payload.data(), payload.size() ) ==
				validation_state_t::payload_part_is_valid );
		REQUIRE( validator.finish_frame() ==
			validation_state_t::frame_is_valid );
	}
	SECTION(
		"Check close frame with incorrect close code" )
	{
		{
			ws_protocol_validator_t validator;

			std::string payload = status_code_to_bin(999);

			message_details_t close_frame{
				true, opcode_t::connection_close_frame, payload.size(), 0xFFFFFFFF};

			REQUIRE( validator.process_new_frame( close_frame ) ==
				validation_state_t::frame_header_is_valid );
			REQUIRE(
				validator.process_next_payload_part( payload.data(), payload.size() ) ==
					validation_state_t::invalid_close_code );
		}
		{
			ws_protocol_validator_t validator;

			std::string payload = status_code_to_bin(1012);

			message_details_t close_frame{
				true, opcode_t::connection_close_frame, payload.size(), 0xFFFFFFFF};

			REQUIRE( validator.process_new_frame( close_frame ) ==
				validation_state_t::frame_header_is_valid );
			REQUIRE(
				validator.process_next_payload_part( payload.data(), payload.size() ) ==
					validation_state_t::invalid_close_code );
		}
		{
			ws_protocol_validator_t validator;

			std::string payload = status_code_to_bin(1013);

			message_details_t close_frame{
				true, opcode_t::connection_close_frame, payload.size(), 0xFFFFFFFF};

			REQUIRE( validator.process_new_frame( close_frame ) ==
				validation_state_t::frame_header_is_valid );
			REQUIRE(
				validator.process_next_payload_part( payload.data(), payload.size() ) ==
					validation_state_t::invalid_close_code );
		}
		{
			ws_protocol_validator_t validator;

			std::string payload = status_code_to_bin(1014);

			message_details_t close_frame{
				true, opcode_t::connection_close_frame, payload.size(), 0xFFFFFFFF};

			REQUIRE( validator.process_new_frame( close_frame ) ==
				validation_state_t::frame_header_is_valid );
			REQUIRE(
				validator.process_next_payload_part( payload.data(), payload.size() ) ==
					validation_state_t::invalid_close_code );
		}
		{
			ws_protocol_validator_t validator;

			std::string payload = status_code_to_bin(1015);

			message_details_t close_frame{
				true, opcode_t::connection_close_frame, payload.size(), 0xFFFFFFFF};

			REQUIRE( validator.process_new_frame( close_frame ) ==
				validation_state_t::frame_header_is_valid );
			REQUIRE(
				validator.process_next_payload_part( payload.data(), payload.size() ) ==
					validation_state_t::invalid_close_code );
		}
		{
			ws_protocol_validator_t validator;

			std::string payload = status_code_to_bin(1016);

			message_details_t close_frame{
				true, opcode_t::connection_close_frame, payload.size(), 0xFFFFFFFF};

			REQUIRE( validator.process_new_frame( close_frame ) ==
				validation_state_t::frame_header_is_valid );
			REQUIRE(
				validator.process_next_payload_part( payload.data(), payload.size() ) ==
					validation_state_t::invalid_close_code );
		}
	}
	SECTION(
		"Check close frame with correct close code and text " )
	{
		ws_protocol_validator_t validator;

		std::string payload = status_code_to_bin(1000) + "Hello";

		message_details_t close_frame{
			true, opcode_t::connection_close_frame, payload.size(), 0xFFFFFFFF};

		REQUIRE( validator.process_new_frame( close_frame ) ==
			validation_state_t::frame_header_is_valid );
		REQUIRE(
			validator.process_next_payload_part( payload.data(), payload.size() ) ==
				validation_state_t::payload_part_is_valid );
		REQUIRE( validator.finish_frame() ==
			validation_state_t::frame_is_valid );
	}
}
