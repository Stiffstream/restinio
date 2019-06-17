/*
	restinio
*/

/*!
	Restinio common types.
*/

#pragma once

#include <cstdint>

#include <restinio/asio_include.hpp>

namespace restinio
{

//! Request handling status.
/*!
	If handler handles request it must return accepted.

	If handler refuses to handle request
	it must return rejected.
*/
enum class request_handling_status_t : std::uint8_t
{
	//! Request accepted for handling.
	accepted,

	//! Request wasn't accepted for handling.
	rejected
};

//! @name Helper funcs for working with request_handling_status_t
//! \see request_handling_status_t.
///@{
constexpr request_handling_status_t
request_accepted() noexcept
{
	return request_handling_status_t::accepted;
}

constexpr request_handling_status_t
request_rejected() noexcept
{
	return request_handling_status_t::rejected;
}
///@}

//! Request id in scope of single connection.
using request_id_t = unsigned int;

//! Attribute for parts.
enum class response_parts_attr_t : std::uint8_t
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
enum class response_connection_attr_t : std::uint8_t
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
		response_connection_attr_t response_connection ) noexcept
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

//
// nullable_pointer_t
//
/*!
 * @brief Type for pointer that can be nullptr.
 *
 * This type is used in methods which return raw pointers. It indicates
 * that returned value should be checked for nullptr.
 *
 * @since v.0.4.9
 */
template< typename T >
using nullable_pointer_t = T*;

//
// not_null_pointer_t
//
/*!
 * @brief Type for pointer that is not null by design.
 *
 * @note
 * There is no any compile-time or run-time checks for a value of the pointer.
 * It is just a flag that we don't expect a nullptr here by design.
 *
 * @since v.0.4.9
 */
template< typename T >
using not_null_pointer_t = T*;

/*!
 * @brief Type for ID of connection.
 */
using connection_id_t = std::uint64_t;

//! An alias for endpoint type from Asio.
using endpoint_t = asio_ns::ip::tcp::endpoint;

} /* namespace restinio */

