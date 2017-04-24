/*
	restinio
*/

/*!
	HTTP-Connection handler routine.
*/

#pragma once

#include <array>

namespace restinio
{

namespace impl
{

//
// ct_correct_len
//

//! Compile time c-string length.
constexpr std::size_t
ct_correct_len( std::size_t size_with_term )
{
	return size_with_term - 1;
}

enum class content_length_field_presence_t
{
	add_content_length,
	skip_content_length
};

//! Creates a string for http response header.
inline std::string
create_header_string(
	const http_response_header_t & h,
	content_length_field_presence_t content_length_field_presence =
		content_length_field_presence_t::add_content_length,
	std::size_t buffer_size = 1024 )
{
	std::string result;
	result.reserve( buffer_size );

	constexpr const char header_part1[] = "HTTP/";
	result.append( header_part1, ct_correct_len( sizeof( header_part1 ) ) );

	result += static_cast<char>( '0' + h.http_major() );
	result += '.';
	result += static_cast<char>( '0' + h.http_minor() );
	result += ' ';

	auto status_code = h.status_code();

//FIXME: there should be a check for status_code in range 100..999.
//May be a special type like bounded_value_t<100,999> must be used in
//http_response_header_t.
	result += '0' + ( status_code / 100 ) % 10;
	result += '0' + ( status_code / 10 ) % 10;
	result += '0' + ( status_code ) % 10;

	result += ' ';
	result += h.reason_phrase();

	constexpr const char header_rn[] = "\r\n";
	result.append( header_rn, ct_correct_len( sizeof( header_rn ) ) );

	if( h.should_keep_alive() )
	{
		constexpr const char header_part2_1[] = "Connection: keep-alive\r\n";
		result.append( header_part2_1, ct_correct_len( sizeof( header_part2_1 ) ) );
	}
	else
	{
		constexpr const char header_part2_2[] = "Connection: close\r\n";
		result.append( header_part2_2, ct_correct_len( sizeof( header_part2_2 ) ) );
	}

	if( content_length_field_presence_t::add_content_length ==
		content_length_field_presence )
	{
		std::array< char, 64 > buf;
		const auto n =
			std::snprintf(
				buf.data(),
				buf.size(),
				"Content-Length: %llu\r\n",
				static_cast< unsigned long long >( h.content_length() ) );

		result.append( buf.data(), static_cast<std::string::size_type>(n) );
	}


	constexpr const char header_field_sep[] = ": ";
	for( const auto & f : h )
	{
		result += f.m_name;
		result.append( header_field_sep, ct_correct_len( sizeof( header_field_sep ) ) );
		result += f.m_value;
		result.append( header_rn, ct_correct_len( sizeof( header_rn ) ) );
	}

	result.append( header_rn, ct_correct_len( sizeof( header_rn ) ) );

	return result;
}

inline auto
create_error_resp( std::uint16_t status, std::string phrase )
{
	http_response_header_t h{ status, std::move( phrase ) };
	h.should_keep_alive( false );
	h.http_major( 1 );
	h.http_minor( 1 );
	return create_header_string( h );
}

inline auto
create_not_implemented_resp()
{
	return create_error_resp( 501, "Not Implemented" );
}

inline auto
create_timeout_resp()
{
	return create_error_resp( 504, "Gateway Time-out" );
}

} /* namespace impl */

} /* namespace restinio */
