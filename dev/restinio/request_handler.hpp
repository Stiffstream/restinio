/*
	restinio
*/

/*!
	HTTP-request handlers routine.
*/

#pragma once

#include <functional>

#include <restinio/http_headers.hpp>
#include <restinio/connection_handle.hpp>

namespace restinio
{

//
// http_request_t
//

//! Request header and body.
/*!
	Helps to use header and body without copying them.
*/
struct http_request_t final
	:	public std::enable_shared_from_this< http_request_t >
{
	http_request_t()
	{}

	http_request_t(
		http_request_header_t header,
		std::string body )
		:	m_header{ std::move( header ) }
		,	m_body{ std::move( body ) }
	{}

	http_request_header_t m_header;
	std::string m_body;
};

using http_request_handle_t =
	std::shared_ptr< http_request_t >;

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

//
// default_request_handler_t
//

using default_request_handler_t =
		std::function< request_handling_status_t (
			http_request_handle_t,
			connection_handle_t ) >;

} /* namespace restinio */
