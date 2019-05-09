/*
	restinio
*/

/*!
	HTTP-Connection handler routine.
*/

#pragma once

#include <array>
#include <numeric>

#include <restinio/buffers.hpp>

namespace restinio
{

namespace impl
{

//
// ct_string_len
//

//! Compile time c-string length.
template< std::size_t N >
inline constexpr std::size_t ct_string_len( const char (&)[N] ) noexcept
{
	return N-1;
}

enum class content_length_field_presence_t : std::uint8_t
{
	add_content_length,
	skip_content_length
};

//
// calculate_approx_buffer_size_for_header()
//

//! Calculate buffer size that is enough for serializing the buffer.
inline std::size_t
calculate_approx_buffer_size_for_header(
	const http_response_header_t & h ) noexcept
{
	std::size_t result = 13; // "HTTP/1.1 xxx "
	result += h.reason_phrase().size() + 2; // 2 is for "\r\n".
	result += 26; // "Connection: keep-alive\r\n" is also enough for "Connection: close\r\n" (21).
	result += 20 + 18; // "Content-Length: %llu\r\n" assume size is size_t, and 18 is always ok.

	result += 2; // Final "\r\n\r\n".

	h.for_each_field( [&result](const auto & f) noexcept {
			result += f.name().size() + 2 + f.value().size() + 2;
		} );

	return result;
}

//
// create_header_string()
//

//! Creates a string for http response header.
inline std::string
create_header_string(
	const http_response_header_t & h,
	content_length_field_presence_t content_length_field_presence =
		content_length_field_presence_t::add_content_length,
	std::size_t buffer_size = 0 )
{
	std::string result;

	if( 0 != buffer_size )
		result.reserve( buffer_size );
	else
		result.reserve( calculate_approx_buffer_size_for_header( h ) );

	constexpr const char header_part1[] = "HTTP/";
	result.append( header_part1, ct_string_len( header_part1 ) );

	result += static_cast<char>( '0' + h.http_major() );
	result += '.';
	result += static_cast<char>( '0' + h.http_minor() );
	result += ' ';

	const auto sc = h.status_code().raw_code();

//FIXME: there should be a check for status_code in range 100..999.
//May be a special type like bounded_value_t<100,999> must be used in
//http_response_header_t.
	result += '0' + ( sc / 100 ) % 10;
	result += '0' + ( sc / 10 ) % 10;
	result += '0' + ( sc ) % 10;

	result += ' ';
	result += h.reason_phrase();

	constexpr const char header_rn[] = "\r\n";
	result.append( header_rn, ct_string_len( header_rn ) );

	switch( h.connection() )
	{
		case http_connection_header_t::keep_alive:
		{
			constexpr const char header_part2_1[] = "Connection: keep-alive\r\n";
			result.append( header_part2_1, ct_string_len( header_part2_1 ) );
			break;
		}

		case http_connection_header_t::close:
		{
			constexpr const char header_part2_2[] = "Connection: close\r\n";
			result.append( header_part2_2, ct_string_len( header_part2_2 ) );
			break;
		}

		case http_connection_header_t::upgrade:
		{
			constexpr const char header_part2_3[] = "Connection: Upgrade\r\n";
			result.append( header_part2_3, ct_string_len( header_part2_3 ) );
			break;
		}
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
	h.for_each_field( [&result, header_field_sep, header_rn](const auto & f) {
		result += f.name();
		result.append( header_field_sep, ct_string_len( header_field_sep ) );
		result += f.value();
		result.append( header_rn, ct_string_len( header_rn ) );
	} );

	result.append( header_rn, ct_string_len( header_rn ) );

	return result;
}

inline auto
create_not_implemented_resp()
{
	constexpr const char raw_501_response[] =
		"HTTP/1.1 501 Not Implemented\r\n"
		"Connection: close\r\n"
		"Content-Length: 0\r\n"
		"\r\n";

	writable_items_container_t result;
	result.emplace_back( raw_501_response );
	return result;
}

inline auto
create_timeout_resp()
{
	constexpr const char raw_504_response[] =
		"HTTP/1.1 504 Gateway Time-out\r\n"
		"Connection: close\r\n"
		"Content-Length: 0\r\n"
		"\r\n";

	writable_items_container_t result;
	result.emplace_back( raw_504_response );
	return result;
}

} /* namespace impl */

} /* namespace restinio */
