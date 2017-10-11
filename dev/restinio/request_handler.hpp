/*
	restinio
*/

/*!
	HTTP-request handlers routine.
*/

#pragma once

#include <functional>

#include <restinio/exception.hpp>
#include <restinio/http_headers.hpp>
#include <restinio/connection_handle.hpp>
#include <restinio/message_builders.hpp>

namespace restinio
{

class request_t;

namespace impl
{

connection_handle_t &
access_req_connection( request_t & );

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
	friend connection_handle_t &
	impl::access_req_connection( request_t & );

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

		//! Get request header.
		const http_request_header_t &
		header() const
		{
			return m_header;
		}

		//! Get request body.
		const std::string &
		body() const
		{
			return m_body;
		}

		//! Create response.
		/*!
			Creates response object if is called for the first time
			all further calls will throw exception.
		*/
		template < typename Output = restinio_controlled_output_t >
		auto
		create_response(
			std::uint16_t status_code = 200,
			std::string reason_phrase = "OK" )
		{
			check_connection();

			return response_builder_t< Output >{
				status_code,
				reason_phrase,
				std::move( m_connection ),
				m_request_id,
				m_header.should_keep_alive() };
		}

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

		connection_handle_t m_connection;
};

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
access_req_connection( request_t & req )
{
	return req.m_connection;
}

} /* namespace impl */


} /* namespace restinio */
