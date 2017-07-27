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

class websocket_t;

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
	template < typename TRAITS, typename WS_MESSAGE_HANDLER >
	friend std::unique_ptr< websocket_t >
	upgrade_to_websocket(			/*TODO params*/
		request_t & ,
		WS_MESSAGE_HANDLER ws_message_handler );

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

		template < typename RESPONSE_BUILDER_OUTPUT_TYPE =
					restinio_controlled_output_t >
		auto
		create_response(
			std::uint16_t status_code = 200,
			std::string reason_phrase = "OK" )
		{
			check_connection();

			return response_builder_t< RESPONSE_BUILDER_OUTPUT_TYPE >{
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

} /* namespace restinio */
