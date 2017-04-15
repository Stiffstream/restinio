/*
	restinio
*/

/*!
	Restinio common types.
*/

#pragma once

namespace restinio
{

//! Request id in scope of single connection.
using request_id_t = unsigned int;

//! Attribute for parts.
enum class response_parts_attr_t
{
	//! Intermediate parts (more parts of response to follow).
	not_final_parts,
	//! Final parts (response ands with these parts).
	final_parts
};

//! Attribute for parts.
enum class response_connection_attr_t
{
	//! This response says to keep connection.
	connection_keepalive,
	//! This response says to close connection.
	connection_close
};

constexpr response_connection_attr_t
response_connection_attr( bool should_keep_alive )
{
	if( should_keep_alive )
		return response_connection_attr_t::connection_keepalive;

	return response_connection_attr_t::connection_close;
}

//! Response output flags for buffers commited to response-coordinator
struct response_output_flags_t
{
	response_output_flags_t(
		response_parts_attr_t response_parts,
		response_connection_attr_t response_connection )
		:	m_response_parts{ response_parts }
		,	m_response_connection{ response_connection }
	{}

	response_parts_attr_t m_response_parts;
	response_connection_attr_t m_response_connection;
};

} /* namespace restinio */
