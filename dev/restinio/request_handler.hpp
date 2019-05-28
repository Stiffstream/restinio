/*
	restinio
*/

/*!
	HTTP-request handlers routine.
*/

#pragma once

#include <functional>
#include <iosfwd>

#include <restinio/exception.hpp>
#include <restinio/http_headers.hpp>
#include <restinio/message_builders.hpp>
#include <restinio/impl/connection_base.hpp>

namespace restinio
{

class request_t;

namespace impl
{

connection_handle_t &
access_req_connection( request_t & ) noexcept;

} /* namespace impl */

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
	friend impl::connection_handle_t &
	impl::access_req_connection( request_t & ) noexcept;

	public:
		request_t(
			request_id_t request_id,
			http_request_header_t header,
			std::string body,
			impl::connection_handle_t connection,
			endpoint_t remote_endpoint )
			:	m_request_id{ request_id }
			,	m_header{ std::move( header ) }
			,	m_body{ std::move( body ) }
			,	m_connection{ std::move( connection ) }
			,	m_connection_id{ m_connection->connection_id() }
			,	m_remote_endpoint{ std::move( remote_endpoint ) }
		{}

		//! Get request header.
		const http_request_header_t &
		header() const noexcept
		{
			return m_header;
		}

		//! Get request body.
		const std::string &
		body() const noexcept
		{
			return m_body;
		}

		template < typename Output = restinio_controlled_output_t >
		auto
		create_response( http_status_line_t status_line = status_ok() )
		{
			check_connection();

			return response_builder_t< Output >{
				status_line,
				std::move( m_connection ),
				m_request_id,
				m_header.should_keep_alive() };
		}

		//! Get request id.
		auto request_id() const noexcept { return m_request_id; }

		//! Get connection id.
		connection_id_t connection_id() const noexcept { return m_connection_id; }

		//! Get the remote endpoint of the underlying connection.
		const endpoint_t & remote_endpoint() const noexcept { return m_remote_endpoint; }

	private:
		void
		check_connection()
		{
			if( !m_connection )
			{
				throw exception_t{ "connection already moved" };
			}
		}

		const request_id_t m_request_id;
		const http_request_header_t m_header;
		const std::string m_body;

		impl::connection_handle_t m_connection;
		const connection_id_t m_connection_id;

		//! Remote endpoint for underlying connection.
		const endpoint_t m_remote_endpoint;
};

inline std::ostream &
operator << ( std::ostream & o, const request_t & req )
{
	o << "{req_id: " << req.request_id() << ", "
		"conn_id: " << req.connection_id() << ", "
		"path: " << req.header().path() << ", "
		"query: " << req.header().query() << "}";

	return o;
}

//! Request handler, that is the type for calling request handlers.
using request_handle_t = std::shared_ptr< request_t >;

//
// default_request_handler_t
//

using default_request_handler_t =
		std::function< request_handling_status_t ( request_handle_t ) >;

namespace impl
{

inline connection_handle_t &
access_req_connection( request_t & req ) noexcept
{
	return req.m_connection;
}

} /* namespace impl */


} /* namespace restinio */
