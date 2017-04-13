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
#include <restinio/message_builders.hpp>

namespace restinio
{

//
// request_t
//

//! HTTP Request data.
/*!
	Provides acces to header and body, and creates response builder
	for a given request.
*/
class request_t final
	:	public std::enable_shared_from_this< request_t >
{
	public:
		request_t(
			request_id_t request_id,
			http_request_header_t header,
			std::string body,
			connection_handle_t connection )
			:	m_request_id{ request_id }
			,	m_header{ std::move( header ) }
			,	m_body{ std::move( body ) }
			,	m_connection{ std::move( connection ) }
		{}

		const http_request_header_t &
		header() const
		{
			return m_header;
		}

		const std::string &
		body() const
		{
			return m_body;
		}

		template < typename RESPONSE_BUILDER =
					resp_builder_t< restinio_controlled_output_t > >
		auto
		create_response(
			std::uint16_t status_code = 200,
			std::string reason_phrase = "OK" )
		{
			return RESPONSE_BUILDER{
				status_code,
				reason_phrase,
				std::move( m_connection ),
				m_request_id,
				m_header.should_keep_alive() };
		}

	private:
		const request_id_t m_request_id;
		const http_request_header_t m_header;
		const std::string m_body;

		connection_handle_t m_connection;
};

//! Request handler, that is the type for calling request handlers.
using request_handle_t = std::shared_ptr< request_t >;

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
