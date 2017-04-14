/*
	restinio
*/

/*!
	Restinio common types.
*/

#pragma once

namespace restinio
{

using request_id_t = unsigned int;

//! Response output flags for buffers commited to response-coordinator
struct response_output_flags_t
{
	response_output_flags_t(
		bool response_is_complete,
		bool connection_should_keep_alive )
		:	m_response_is_complete{ response_is_complete }
		,	m_connection_should_keep_alive{ connection_should_keep_alive }
	{}

	bool m_response_is_complete;
	bool m_connection_should_keep_alive;
};

} /* namespace restinio */
