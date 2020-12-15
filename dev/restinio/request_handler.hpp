/*
	restinio
*/

/*!
	HTTP-request handlers routine.
*/

#pragma once

#include <restinio/exception.hpp>
#include <restinio/http_headers.hpp>
#include <restinio/message_builders.hpp>
#include <restinio/chunked_input_info.hpp>
#include <restinio/impl/connection_base.hpp>

#include <array>
#include <functional>
#include <iosfwd>

namespace restinio
{

//FIXME: document this!
struct no_user_data_factory_t
{
	//FIXME: document this!
	struct data_t {};

	void
	make_within( void * buffer ) noexcept
	{
		new(buffer) data_t{};
	}
};

template< typename User_Data >
class incoming_request_t;

namespace impl
{

template< typename User_Data >
connection_handle_t &
access_req_connection( incoming_request_t<User_Data> & ) noexcept;

//FIXME: document this!
template< typename User_Data >
class incoming_request_user_data_holder_t
{
	alignas(User_Data) std::array<char, sizeof(User_Data)> m_data;

public:
	template< typename Factory >
	incoming_request_user_data_holder_t(
		Factory & factory )
	{
		factory.make_within( m_data.data() );
	}

	~incoming_request_user_data_holder_t() noexcept
	{
		get_ptr()->~User_Data();
	}

	RESTINIO_NODISCARD
	User_Data *
	get_ptr() noexcept
	{
		return reinterpret_cast<User_Data *>(m_data.data());
	}

	RESTINIO_NODISCARD
	const User_Data *
	get_ptr() const noexcept
	{
		return reinterpret_cast<const User_Data *>(m_data.data());
	}
};

} /* namespace impl */

//
// incoming_request_t
//

//FIXME: document User_Data template parameter!
//! HTTP Request data.
/*!
	Provides acces to header and body, and creates response builder
	for a given request.
*/
template< typename User_Data >
class incoming_request_t final
	:	public std::enable_shared_from_this< incoming_request_t< User_Data > >
{
	template< typename UD >
	friend impl::connection_handle_t &
	impl::access_req_connection( incoming_request_t<UD> & ) noexcept;

	public:
		//! Old-format initializing constructor.
		/*!
		 * Can be used in cases where chunked_input_info_t is not
		 * available (or needed).
		 */
		template< typename User_Data_Factory >
		incoming_request_t(
			request_id_t request_id,
			http_request_header_t header,
			std::string body,
			impl::connection_handle_t connection,
			endpoint_t remote_endpoint,
			User_Data_Factory & user_data_factory )
			:	incoming_request_t{
					request_id,
					std::move( header ),
					std::move( body ),
					chunked_input_info_unique_ptr_t{},
					std::move( connection ),
					std::move( remote_endpoint ),
					user_data_factory
				}
		{}

		//! New-format initializing constructor.
		/*!
		 * @since v.0.6.9
		 */
		template< typename User_Data_Factory >
		incoming_request_t(
			request_id_t request_id,
			http_request_header_t header,
			std::string body,
			chunked_input_info_unique_ptr_t chunked_input_info,
			impl::connection_handle_t connection,
			endpoint_t remote_endpoint,
			User_Data_Factory & user_data_factory )
			:	m_request_id{ request_id }
			,	m_header{ std::move( header ) }
			,	m_body{ std::move( body ) }
			,	m_chunked_input_info{ std::move( chunked_input_info ) }
			,	m_connection{ std::move( connection ) }
			,	m_connection_id{ m_connection->connection_id() }
			,	m_remote_endpoint{ std::move( remote_endpoint ) }
			,	m_user_data_holder{ user_data_factory }
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

		//! Get optional info about chunked input.
		/*!
		 * @note
		 * nullptr will be returned if chunked-encoding wasn't used in
		 * the incoming request.
		 *
		 * @since v.0.6.9
		 */
		nullable_pointer_t< const chunked_input_info_t >
		chunked_input_info() const noexcept
		{
			return m_chunked_input_info.get();
		}

		//FIXME: document this!
		RESTINIO_NODISCARD
		User_Data &
		user_data() noexcept
		{
			return *m_user_data_holder.get_ptr();
		}

		//FIXME: document this!
		RESTINIO_NODISCARD
		const User_Data &
		user_data() const noexcept
		{
			return *m_user_data_holder.get_ptr();
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

		//! Optional description for chunked-encoding.
		/*!
		 * It is present only if chunked-encoded body is found in the
		 * incoming request.
		 *
		 * @since v.0.6.9
		 */
		const chunked_input_info_unique_ptr_t m_chunked_input_info;

		impl::connection_handle_t m_connection;
		const connection_id_t m_connection_id;

		//! Remote endpoint for underlying connection.
		const endpoint_t m_remote_endpoint;

		//FIXME: document this!
		impl::incoming_request_user_data_holder_t< User_Data > m_user_data_holder;
};

template< typename User_Data >
std::ostream &
operator<<(
	std::ostream & o,
	const incoming_request_t< User_Data > & req )
{
	o << "{req_id: " << req.request_id() << ", "
		"conn_id: " << req.connection_id() << ", "
		"path: " << req.header().path() << ", "
		"query: " << req.header().query() << "}";

	return o;
}

//! An alias for shared-pointer to incoming request.
template< typename User_Data >
using incoming_request_handle_t =
		std::shared_ptr< incoming_request_t< User_Data > >;

//! An alias for incoming request without additional user-data.
/*!
 * For compatibility with previous versions.
 *
 * @since v.0.6.13
 */
using request_t = incoming_request_t< no_user_data_factory_t::data_t >;

//! An alias for handle for incoming request without additional user-data.
/*!
 * For compatibility with previous versions.
 *
 * @since v.0.6.13
 */
using request_handle_t = std::shared_ptr< request_t >;

//
// default_request_handler_t
//

using default_request_handler_t =
		std::function< request_handling_status_t ( request_handle_t ) >;

namespace impl
{

template< typename User_Data >
connection_handle_t &
access_req_connection( incoming_request_t<User_Data> & req ) noexcept
{
	return req.m_connection;
}

} /* namespace impl */


} /* namespace restinio */
