/*
	restinio
*/

/*!
	HTTP-request handlers routine.
*/

#pragma once

#include <restinio/compiler_features.hpp>

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

//
// extra_data_buffer_t
//
/*!
 * @brief Helper for holding a pointer to a buffer where a new
 * object of type Extra_Data should be constructed.
 *
 * This class is intended to make the construction of new objects
 * of type Extra_Data inside a preallocated buffer more type-safe.
 *
 * An instance of Extra_Data is incorporated into a request object
 * by holding a buffer of necessary capacity and alignment inside
 * request object. The `make_within` method of extra-data-factory
 * is called for the construction of new instance of Extra_Data
 * in that buffer. If raw void pointer will be passed to
 * `make_within` method then it would make possible a case when
 * wrong extra-data-factory can be used.
 *
 * But if a pointer to the buffer for new instance will be wrapped
 * into extra_data_buffer_t then it allows additional type checks
 * from the compiler. That is why a extra-data-factory receives
 * extra_data_buffer_t<Extra_Data> as a parameter to `make_within`
 * instead of raw pointers.
 *
 * @since v.0.6.13
 */
template< typename Extra_Data >
class extra_data_buffer_t
{
	void * m_buffer;

public:
	extra_data_buffer_t( void * buffer ) : m_buffer{ buffer } {}

	RESTINIO_NODISCARD
	void *
	get() const noexcept { return m_buffer; }
};

//
// no_extra_data_factory_t
//
/*!
 * @brief The default extra-data-factory to be used in server's traits if
 * a user doesn't specify own one.
 *
 * This factory doesn't nothing. And holds an empty struct as `data_t` member.
 *
 * @since v.0.6.13
 */
struct no_extra_data_factory_t
{
	/*!
	 * @brief A type of extra-data to be incorporated into a request object
	 * by the default.
	 */
	struct data_t {};

	void
	make_within( extra_data_buffer_t<data_t> buffer ) noexcept
	{
		new(buffer.get()) data_t{};
	}
};

//
// simple_extra_data_factory_t
//
/*!
 * @brief A helper template class for cases when extra-data-factory is
 * just a simple stateless object.
 *
 * Usage example:
 * @code
 * struct first_stage_data { ... };
 * struct second_stage_data { ... };
 * struct third_stage_data { ... };
 *
 * using my_extra_data_factory = restinio::simple_extra_data_factory_t<
 * 	std::tuple<first_stage_data, second_stage_data, third_stage_data> >;
 *
 * struct my_traits : public restinio::default_traits_t
 * {
 * 	using extra_data_factory_t = my_extra_data_factory;
 * };
 * @endcode
 *
 * @tparam Extra_Data Type of extra-data to be incorporated into request-objects.
 *
 * @since v.0.6.13
 */
template< typename Extra_Data >
struct simple_extra_data_factory_t
{
	using data_t = Extra_Data;

	void
	make_within( extra_data_buffer_t<data_t> buffer )
		noexcept( noexcept(new(buffer.get()) data_t{}) )
	{
		new(buffer.get()) data_t{};
	}
};

template< typename Extra_Data >
class generic_request_t;

namespace impl
{

template< typename Extra_Data >
connection_handle_t &
access_req_connection( generic_request_t<Extra_Data> & ) noexcept;

//
// generic_request_extra_data_holder_t
//
/*!
 * @brief Helper class for holding a buffer for extra-data object to
 * be incorporated into a request object.
 *
 * It constructs a new object inside internal buffer @a m_data in
 * the constructor and correctly destroys extra-data object in the
 * destructor.
 *
 * @since v.0.6.13
 */
template< typename Extra_Data >
class generic_request_extra_data_holder_t
{
	alignas(Extra_Data) std::array<char, sizeof(Extra_Data)> m_data;

public:
	template< typename Factory >
	generic_request_extra_data_holder_t(
		Factory & factory )
	{
		factory.make_within( extra_data_buffer_t<Extra_Data>{ m_data.data() } );
	}

	~generic_request_extra_data_holder_t() noexcept
	{
		get_ptr()->~Extra_Data();
	}

	RESTINIO_NODISCARD
	Extra_Data *
	get_ptr() noexcept
	{
		// Because the content of m_data.data() is rewritten by
		// placement new we have to use std::launder to avoid UB.
		return RESTINIO_STD_LAUNDER(
				reinterpret_cast<Extra_Data *>(m_data.data()) );
	}

	RESTINIO_NODISCARD
	const Extra_Data *
	get_ptr() const noexcept
	{
		// Because the content of m_data.data() is rewritten by
		// placement new we have to use std::launder to avoid UB.
		return RESTINIO_STD_LAUNDER(
				reinterpret_cast<const Extra_Data *>(m_data.data()) );
	}
};

} /* namespace impl */

//
// generic_request_t
//

//! HTTP Request data.
/*!
	Provides acces to header and body, and creates response builder
	for a given request.

	@tparam Extra_Data The type of extra-data to be incorporated into
	a request object.
*/
template< typename Extra_Data >
class generic_request_t final
	:	public std::enable_shared_from_this< generic_request_t< Extra_Data > >
{
	template< typename UD >
	friend impl::connection_handle_t &
	impl::access_req_connection( generic_request_t<UD> & ) noexcept;

	public:
		//! Old-format initializing constructor.
		/*!
		 * Can be used in cases where chunked_input_info_t is not
		 * available (or needed).
		 */
		template< typename Extra_Data_Factory >
		generic_request_t(
			request_id_t request_id,
			http_request_header_t header,
			std::string body,
			impl::connection_handle_t connection,
			endpoint_t remote_endpoint,
			Extra_Data_Factory & extra_data_factory )
			:	generic_request_t{
					request_id,
					std::move( header ),
					std::move( body ),
					chunked_input_info_unique_ptr_t{},
					std::move( connection ),
					std::move( remote_endpoint ),
					extra_data_factory
				}
		{}

		//! New-format initializing constructor.
		/*!
		 * @since v.0.6.9
		 */
		template< typename Extra_Data_Factory >
		generic_request_t(
			request_id_t request_id,
			http_request_header_t header,
			std::string body,
			chunked_input_info_unique_ptr_t chunked_input_info,
			impl::connection_handle_t connection,
			endpoint_t remote_endpoint,
			Extra_Data_Factory & extra_data_factory )
			:	m_request_id{ request_id }
			,	m_header{ std::move( header ) }
			,	m_body{ std::move( body ) }
			,	m_chunked_input_info{ std::move( chunked_input_info ) }
			,	m_connection{ std::move( connection ) }
			,	m_connection_id{ m_connection->connection_id() }
			,	m_remote_endpoint{ std::move( remote_endpoint ) }
			,	m_extra_data_holder{ extra_data_factory }
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

		/*!
		 * @brief Get writeable access to extra-data object incorporated
		 * into a request object.
		 *
		 * @note
		 * This method is present always but it has the sense only if
		 * Extra_Data is not no_extra_data_factory_t::data_t.
		 *
		 * Usage example:
		 * @code
		 * struct my_extra_data_factory {
		 * 	struct data_t {
		 * 		user_identity user_id_;
		 * 		...
		 * 	};
		 *
		 * 	void make_within(restinio::extra_data_buffer_t<data_t> buf) {
		 * 		new(buf.get()) data_t{};
		 * 	}
		 * };
		 *
		 * struct my_traits : public restinio::default_traits_t {
		 * 	using extra_data_factory_t = my_extra_data_factory;
		 * };
		 *
		 * restinio::request_handling_status_t authentificator(
		 * 		const restinio::generic_request_handle_t<my_extra_data_factory::data_t> & req) {
		 * 	auto & ud = req->extra_data();
		 * 	...
		 * 	ud.user_id_ = some_calculated_user_id;
		 * }
		 * @endcode
		 *
		 * @since v.0.6.13
		 */
		RESTINIO_NODISCARD
		Extra_Data &
		extra_data() noexcept
		{
			return *m_extra_data_holder.get_ptr();
		}

		/*!
		 * @brief Get readonly access to extra-data object incorporated
		 * into a request object.
		 *
		 * @note
		 * This method is present always but it has the sense only if
		 * Extra_Data is not no_extra_data_factory_t::data_t.
		 *
		 * Usage example:
		 * @code
		 * struct my_extra_data_factory {
		 * 	struct data_t {
		 * 		user_identity user_id_;
		 * 		...
		 * 	};
		 *
		 * 	void make_within(restinio::extra_data_buffer_t<data_t> buf) {
		 * 		new(buf.get()) data_t{};
		 * 	}
		 * };
		 *
		 * struct my_traits : public restinio::default_traits_t {
		 * 	using extra_data_factory_t = my_extra_data_factory;
		 * };
		 *
		 * restinio::request_handling_status_t actual_handler(
		 * 		const restinio::generic_request_handle_t<my_extra_data_factory::data_t> & req) {
		 * 	const auto & ud = req->extra_data();
		 * 	if(ud.user_id_.valid()) {
		 * 		...
		 * 	}
		 * 	else {
		 * 		...
		 * 	}
		 * }
		 * @endcode
		 *
		 * @since v.0.6.13
		 */
		RESTINIO_NODISCARD
		const Extra_Data &
		extra_data() const noexcept
		{
			return *m_extra_data_holder.get_ptr();
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

		/*!
		 * @brief An instance of extra-data that is incorporated into
		 * a request object.
		 *
		 * @since v.0.6.13
		 */
		impl::generic_request_extra_data_holder_t< Extra_Data > m_extra_data_holder;
};

template< typename Extra_Data >
std::ostream &
operator<<(
	std::ostream & o,
	const generic_request_t< Extra_Data > & req )
{
	o << "{req_id: " << req.request_id() << ", "
		"conn_id: " << req.connection_id() << ", "
		"path: " << req.header().path() << ", "
		"query: " << req.header().query() << "}";

	return o;
}

//! An alias for shared-pointer to incoming request.
template< typename Extra_Data >
using generic_request_handle_t =
		std::shared_ptr< generic_request_t< Extra_Data > >;

//! An alias for incoming request without additional extra-data.
/*!
 * For compatibility with previous versions.
 *
 * @since v.0.6.13
 */
using request_t = generic_request_t< no_extra_data_factory_t::data_t >;

//! An alias for handle for incoming request without additional extra-data.
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

template< typename Extra_Data >
connection_handle_t &
access_req_connection( generic_request_t<Extra_Data> & req ) noexcept
{
	return req.m_connection;
}

} /* namespace impl */


} /* namespace restinio */
