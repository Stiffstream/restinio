/*
	restinio
*/

/*!
	Restinio common types.
*/

#pragma once

namespace restinio
{

//! Request handling status.
/*!
	If handler handles request it must return accepted.

	If handler refuses to handle request
	it must return rejected.
*/
enum class request_handling_status_t
{
	//! Request accepted for handling.
	accepted,

	//! Request wasn't accepted for handling.
	rejected
};

//! Helper funcs for working with request_handling_status_t
// \{
constexpr request_handling_status_t
request_accepted()
{
	return request_handling_status_t::accepted;
}

constexpr request_handling_status_t
request_rejected()
{
	return request_handling_status_t::rejected;
}
// \}

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

inline std::ostream &
operator << ( std::ostream & o, response_parts_attr_t attr )
{
	if( response_parts_attr_t::not_final_parts == attr )
		o << "not_final_parts";
	else
		o << "final_parts";

	return o;
}

//! Attribute for parts.
enum class response_connection_attr_t
{
	//! This response says to keep connection.
	connection_keepalive,
	//! This response says to close connection.
	connection_close
};

inline std::ostream &
operator << ( std::ostream & o, response_connection_attr_t attr )
{
	if( response_connection_attr_t::connection_keepalive == attr )
		o << "connection_keepalive";
	else
		o << "connection_close";

	return o;
}

inline response_connection_attr_t
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

inline std::ostream &
operator << ( std::ostream & o, const response_output_flags_t & flags )
{
	return o << "{ " << flags.m_response_parts << ", "
		<< flags.m_response_connection << " }";
}

} /* namespace restinio */
