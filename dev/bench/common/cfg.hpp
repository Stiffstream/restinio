/*
	restinio bench common
*/

#pragma once

#include <stdexcept>

#include <json_dto/pub.hpp>
#include <json_dto/validators.hpp>

#include <restinio/settings.hpp>

namespace json_dto
{

template <>
void
read_json_value(
	const rapidjson::Value & object,
	std::chrono::steady_clock::duration & v )
{
	try
	{
		std::int64_t representation;
		read_json_value( object, representation );

		v = std::chrono::milliseconds{ representation };
	}
	catch( const std::exception & ex )
	{
		throw std::invalid_argument{
			std::string{ "unable to read std::chrono::steady_clock::duration: " } +
			ex.what() };
	}
}

template <>
void
write_json_value(
	const std::chrono::steady_clock::duration & v,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	std::int64_t representation =
		std::chrono::duration_cast< std::chrono::milliseconds >( v ).count();

	write_json_value( representation, object, allocator );
}

template <>
void
read_json_value(
	const rapidjson::Value & object,
	asio::ip::tcp & v )
{
	try
	{
		std::string representation;
		read_json_value( object, representation );

		if( representation == "ipv4" )
			v = asio::ip::tcp::v4();
		else if( representation == "ipv6" )
			v = asio::ip::tcp::v6();
		else
		{
			throw std::invalid_argument{ "invalid protocol, must be 'ipv4' or 'ipv6'" };
		}
	}
	catch( const std::exception & ex )
	{
		throw std::invalid_argument{
			std::string{ "unable to read asio::ip::tcp: " } +
			ex.what() };
	}
}

template <>
void
write_json_value(
	const asio::ip::tcp & v,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	if( asio::ip::tcp::v4() == v )
		write_json_value( std::string{ "ipv4" }, object, allocator );
	else
		write_json_value( std::string{ "ipv6" }, object, allocator );
}

template < typename TRAITS >
void
json_io(
	json_dto::json_input_t & input,
	restinio::server_settings_t< TRAITS > & p )
{
	std::uint16_t port;
	std::size_t buffer_size;
	asio::ip::tcp protocol{ asio::ip::tcp::v4() };
	std::chrono::steady_clock::duration
		read_next_http_message_timelimit,
		write_http_response_timelimit,
		handle_request_timeout;
	std::size_t max_pipelined_requests;

	input
		& json_dto::optional( "m_port", port, 8080 )
		& json_dto::optional(
			"protocol",
			protocol,
			asio::ip::tcp::v4() )
		& json_dto::optional( "buffer_size", buffer_size, 4 * 1024 )
		& json_dto::optional(
			"read_next_http_message_timelimit_ms",
			read_next_http_message_timelimit,
			std::chrono::milliseconds( 60 * 1000 ) )
		& json_dto::optional(
			"write_http_response_timelimit_ms",
			write_http_response_timelimit,
			std::chrono::milliseconds( 5 * 1000 ) )
		& json_dto::optional(
			"handle_request_timeout_ms",
			handle_request_timeout,
			std::chrono::milliseconds( 10 * 1000 ) )
		& json_dto::optional(
			"max_pipelined_requests",
			max_pipelined_requests,
			32 );

	p
		.port( port )
		.protocol( protocol )
		.buffer_size( buffer_size )
		.read_next_http_message_timelimit( read_next_http_message_timelimit )
		.write_http_response_timelimit( write_http_response_timelimit )
		.handle_request_timeout( handle_request_timeout )
		.max_pipelined_requests( max_pipelined_requests );
}

} /* namespace json_dto */
