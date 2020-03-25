/*
	restinio
*/

/*!
	Builders for messages.
*/

#pragma once

#include <ctime>
#include <chrono>

#include <restinio/impl/include_fmtlib.hpp>

#include <restinio/common_types.hpp>
#include <restinio/http_headers.hpp>
#include <restinio/os.hpp>
#include <restinio/sendfile.hpp>
#include <restinio/impl/connection_base.hpp>

#include <restinio/impl/header_helpers.hpp>

namespace restinio
{

//
// make_date_field_value()
//

//! Format a timepoint to a string of a propper format.
inline std::string
make_date_field_value( std::time_t t )
{
	const auto tpoint = make_gmtime( t );

	std::array< char, 64 > buf;
	// TODO: is there a faster way to get time string?
	strftime(
		buf.data(),
		buf.size(),
		"%a, %d %b %Y %H:%M:%S GMT",
		&tpoint );

	return std::string{ buf.data() };
}

inline std::string
make_date_field_value( std::chrono::system_clock::time_point tp )
{
	return make_date_field_value( std::chrono::system_clock::to_time_t( tp ) );
}

//
// base_response_builder_t
//

template < typename Response_Builder >
class base_response_builder_t
{
	public:
		base_response_builder_t( const base_response_builder_t & ) = delete;
		base_response_builder_t & operator = ( const base_response_builder_t & ) = delete;

		base_response_builder_t( base_response_builder_t && ) noexcept = default;
		base_response_builder_t & operator =( base_response_builder_t && ) noexcept = default;

		virtual ~base_response_builder_t() = default;

		base_response_builder_t(
			http_status_line_t status_line,
			impl::connection_handle_t connection,
			request_id_t request_id,
			bool should_keep_alive )
			:	m_header{ std::move( status_line ) }
			,	m_connection{ std::move( connection ) }
			,	m_request_id{ request_id }
		{
			m_header.should_keep_alive( should_keep_alive );
		}

		//! Accessors for header.
		//! \{
		http_response_header_t &
		header() noexcept
		{
			return m_header;
		}

		const http_response_header_t &
		header() const noexcept
		{
			return m_header;
		}
		//! \}

		//! Add header field.
		Response_Builder &
		append_header(
			std::string field_name,
			std::string field_value ) &
		{
			m_header.set_field(
				std::move( field_name ),
				std::move( field_value ) );
			return upcast_reference();
		}

		//! Add header field.
		Response_Builder &&
		append_header(
			std::string field_name,
			std::string field_value ) &&
		{
			return std::move( this->append_header(
										std::move( field_name ),
										std::move( field_value ) ) );
		}

		//! Add header field.
		Response_Builder &
		append_header( http_header_field_t http_header_field ) &
		{
			m_header.set_field( std::move( http_header_field ) );
			return upcast_reference();
		}

		//! Add header field.
		Response_Builder &&
		append_header( http_header_field_t http_header_field ) &&
		{
			return std::move( this->append_header(
										std::move( http_header_field ) ) );
		}

		//! Add header field.
		Response_Builder &
		append_header(
			http_field_t field_id,
			std::string field_value ) &
		{
			m_header.set_field(
				field_id,
				std::move( field_value ) );
			return upcast_reference();
		}

		//! Add header field.
		Response_Builder &&
		append_header(
			http_field_t field_id,
			std::string field_value ) &&
		{
			return std::move( this->append_header(
										field_id,
										std::move( field_value ) ) );
		}


		//! Add header `Date` field.
		Response_Builder &
		append_header_date_field(
			std::chrono::system_clock::time_point tp =
				std::chrono::system_clock::now() ) &
		{
			m_header.set_field( http_field_t::date, make_date_field_value( tp ) );
			return upcast_reference();
		}

		//! Add header `Date` field.
		Response_Builder &&
		append_header_date_field(
			std::chrono::system_clock::time_point tp =
				std::chrono::system_clock::now() ) &&
		{
			return std::move( this->append_header_date_field( tp ) );
		}

		//! Set connection close.
		Response_Builder &
		connection_close() & noexcept
		{
			m_header.should_keep_alive( false );
			return upcast_reference();
		}

		//! Set connection close.
		Response_Builder &&
		connection_close() && noexcept
		{
			return std::move( this->connection_close() );
		}


		//! Set connection keep-alive.
		Response_Builder &
		connection_keep_alive() & noexcept
		{
			m_header.should_keep_alive();
			return upcast_reference();
		}

		Response_Builder &&
		connection_keep_alive() && noexcept
		{
			return std::move( this->connection_keep_alive() );
		}

	protected:
		std::size_t
		calculate_status_line_size() const noexcept
		{
			// "HTTP/1.1 *** <reason-phrase>"
			return 8 + 1 + 3 + 1 + m_header.status_line().reason_phrase().size();
		}

		http_response_header_t m_header;

		impl::connection_handle_t m_connection;
		const request_id_t m_request_id;

		void
		throw_done_must_be_called_once() const
		{
			throw exception_t{ "done() cannot be called twice" };
		}

	private:
		Response_Builder &
		upcast_reference() noexcept
		{
			return static_cast< Response_Builder & >( *this );
		}
};

//
// response_builder_t
//

//! Forbid arbitrary response_builder_t instantiations.
template < typename Response_Output_Strategy >
class response_builder_t
{
	response_builder_t() = delete;
};

//! Tag type for RESTinio controlled output response builder.
struct restinio_controlled_output_t {};

//! Simple standard response builder.
/*!
	Requires user to set header and body.
	Content length is automatically calculated.
	Once the data is ready, the user calls done() method
	and the resulting response is scheduled for sending.
*/
template <>
class response_builder_t< restinio_controlled_output_t > final
	:	public base_response_builder_t< response_builder_t< restinio_controlled_output_t > >
{
	public:
		using base_type_t =
			base_response_builder_t< response_builder_t< restinio_controlled_output_t > >;
		using self_type_t =
			response_builder_t< restinio_controlled_output_t >;

		response_builder_t( response_builder_t && ) = default;

		// Reuse construstors from base.
		using base_type_t::base_type_t;

		//! Set body.
		self_type_t &
		set_body( writable_item_t body ) &
		{
			auto size = body.size();
			return set_body_impl( body, size );
		}

		//! Set body.
		self_type_t &&
		set_body( writable_item_t body ) &&
		{
			return std::move( this->set_body( std::move( body ) ) );
		}

		//! Append body.
		self_type_t &
		append_body( writable_item_t body_part ) &
		{
			auto size = body_part.size();
			return append_body_impl( body_part, size );
		}

		//! Append body.
		self_type_t &&
		append_body( writable_item_t body_part ) &&
		{
			return std::move( this->append_body( std::move( body_part ) ) );
		}

		//! Complete response.
		request_handling_status_t
		done( write_status_cb_t wscb = write_status_cb_t{} )
		{
			if( m_connection )
			{
				const response_output_flags_t
					response_output_flags{
						response_parts_attr_t::final_parts,
						response_connection_attr( m_header.should_keep_alive() ) };

				m_header.content_length( m_body_size );

				if_neccessary_reserve_first_element_for_header();

				m_response_parts[ 0 ] =
					writable_item_t{ impl::create_header_string( m_header ) };

				write_group_t wg{ std::move( m_response_parts ) };
				wg.status_line_size( calculate_status_line_size() );

				if( wscb )
				{
					wg.after_write_notificator( std::move( wscb ) );
				}

				auto conn = std::move( m_connection );

				conn->write_response_parts(
					m_request_id,
					response_output_flags,
					std::move( wg ) );
			}
			else
			{
				throw_done_must_be_called_once();
			}

			return restinio::request_accepted();
		}

	private:
		self_type_t &
		set_body_impl( writable_item_t & body, std::size_t body_size )
		{
			if_neccessary_reserve_first_element_for_header();

			// Leave only buf that is reserved for header,
			// so forget all the previous data.
			m_response_parts.resize( 1 );

			if( 0 < body_size )
			{
				m_response_parts.emplace_back( std::move( body ) );
			}

			m_body_size = body_size;

			return *this;
		}

		self_type_t &
		append_body_impl( writable_item_t & body_part, std::size_t append_size )
		{
			if_neccessary_reserve_first_element_for_header();

			if( 0 < append_size )
			{
					m_response_parts.emplace_back( std::move( body_part ) );
					m_body_size += append_size;
			}

			return *this;
		}

		void
		if_neccessary_reserve_first_element_for_header()
		{
			if( m_response_parts.empty() )
			{
				m_response_parts.reserve( 2 );
				m_response_parts.emplace_back();
			}
		}

		std::size_t m_body_size{ 0 };
		writable_items_container_t m_response_parts;
};

//! Tag type for user controlled output response builder.
struct user_controlled_output_t {};

//! User controlled response output builder.
/*!
	This type of output allows user
	to send body divided into parts.
	But it is up to user to set the correct
	Content-Length field.
*/
template <>
class response_builder_t< user_controlled_output_t > final
	:	public base_response_builder_t< response_builder_t< user_controlled_output_t > >
{
	public:
		using base_type_t =
			base_response_builder_t< response_builder_t< user_controlled_output_t > >;
		using self_type_t =
			response_builder_t< user_controlled_output_t >;

		response_builder_t( response_builder_t && ) = default;

		// Reuse construstors from base.
		using base_type_t::base_type_t;

		//! Manualy set content length.
		self_type_t &
		set_content_length( std::size_t content_length ) &
		{
			m_header.content_length( content_length );
			return *this;
		}

		//! Manualy set content length.
		self_type_t &&
		set_content_length( std::size_t content_length ) &&
		{
			return std::move( this->set_content_length( content_length ) );
		}

		//! Set body (part).
		self_type_t &
		set_body( writable_item_t body ) &
		{
			auto size = body.size();
			return set_body_impl( body, size );
		}

		//! Set body (part).
		self_type_t &&
		set_body( writable_item_t body ) &&
		{
			return std::move( this->set_body( std::move( body ) ) );
		}

		//! Append body.
		self_type_t &
		append_body( writable_item_t body_part ) &
		{
			auto size = body_part.size();

			if( 0 == size )
				return *this;

			return append_body_impl( body_part );
		}

		//! Append body.
		self_type_t &&
		append_body( writable_item_t body_part ) &&
		{
			return std::move( this->append_body( std::move( body_part ) ) );
		}

		//! Flush ready outgoing data.
		/*!
			Schedules for sending currently ready data.
		*/
		self_type_t &
		flush( write_status_cb_t wscb = write_status_cb_t{} ) &
		{
			if( m_connection )
			{
				send_ready_data(
					m_connection,
					response_parts_attr_t::not_final_parts,
					std::move( wscb ) );
			}

			return *this;
		}

		//! Flush ready outgoing data.
		self_type_t &&
		flush( write_status_cb_t wscb = write_status_cb_t{} ) &&
		{
			return std::move( this->flush( std::move( wscb ) ) );
		}

		//! Complete response.
		request_handling_status_t
		done( write_status_cb_t wscb = write_status_cb_t{} )
		{
			if( m_connection )
			{
				// Note: m_connection should become empty after return
				// from that method.
				impl::connection_handle_t old_conn_handle{
						std::move(m_connection) };
				send_ready_data(
					old_conn_handle,
					response_parts_attr_t::final_parts,
					std::move( wscb ) );
			}
			else
			{
				throw_done_must_be_called_once();
			}

			return restinio::request_accepted();
		}

	private:
		void
		send_ready_data(
			const impl::connection_handle_t & conn,
			response_parts_attr_t response_parts_attr,
			write_status_cb_t wscb )
		{
			std::size_t status_line_size{ 0 };

			if( !m_header_was_sent )
			{
				m_should_keep_alive_when_header_was_sent =
					m_header.should_keep_alive();

				if_neccessary_reserve_first_element_for_header();

				m_response_parts[ 0 ] =
					writable_item_t{ impl::create_header_string( m_header ) };

				m_header_was_sent = true;
				status_line_size = calculate_status_line_size();
			}

			if( !m_response_parts.empty() ||
				wscb ||
				response_parts_attr_t::final_parts == response_parts_attr )
			{
				const response_output_flags_t
					response_output_flags{
						response_parts_attr,
						response_connection_attr( m_should_keep_alive_when_header_was_sent ) };

				write_group_t wg{ std::move( m_response_parts ) };
				wg.status_line_size( status_line_size );

				if( wscb )
				{
					wg.after_write_notificator( std::move( wscb ) );
				}

				conn->write_response_parts(
					m_request_id,
					response_output_flags,
					std::move( wg ) );
			}
		}

		self_type_t &
		set_body_impl( writable_item_t & body, std::size_t body_size )
		{
			if_neccessary_reserve_first_element_for_header();

			// Leave only buf that is reserved for header,
			// so forget all the previous data.
			if( !m_header_was_sent )
				m_response_parts.resize( 1 );
			else
				m_response_parts.resize( 0 );

			if( 0 < body_size )
			{
				// if body is not empty:
				m_response_parts.emplace_back( std::move( body ) );
			}

			return *this;
		}

		self_type_t &
		append_body_impl( writable_item_t & body_part )
		{
			if_neccessary_reserve_first_element_for_header();

			m_response_parts.emplace_back( std::move( body_part ) );
			return *this;
		}

		void
		if_neccessary_reserve_first_element_for_header()
		{
			if( !m_header_was_sent && m_response_parts.empty() )
			{
				m_response_parts.reserve( 2 );
				m_response_parts.emplace_back();
			}
		}


		//! Flag used by flush() function.
		bool m_header_was_sent{ false };

		//! Saved keep_alive attr actual at the point
		//! a header data was sent.
		/*!
			It is neccessary to guarantee that all parts of response
			will have the same response-connection-attr
			(keep-alive or close);
		*/
		bool m_should_keep_alive_when_header_was_sent{ true };

		//! Body accumulator.
		/*!
			For this type of output it contains a part of a body.
			On each flush it is cleared.
		*/
		writable_items_container_t m_response_parts;
};

//! Tag type for chunked output response builder.
struct chunked_output_t {};

//! Chunked transfer encoding output builder.
/*!
	This type of output sets transfer-encoding to chunked
	and expects user to set body using chunks of data.
*/
template <>
class response_builder_t< chunked_output_t > final
	:	public base_response_builder_t< response_builder_t< chunked_output_t > >
{
	public:
		using base_type_t =
			base_response_builder_t< response_builder_t< chunked_output_t > >;
		using self_type_t =
			response_builder_t< chunked_output_t >;

		response_builder_t(
			http_status_line_t status_line,
			impl::connection_handle_t connection,
			request_id_t request_id,
			bool should_keep_alive )
			:	base_type_t{
					std::move( status_line ),
					std::move( connection ),
					request_id,
					should_keep_alive }
		{
			m_chunks.reserve( 4 );
		}

		response_builder_t( response_builder_t && ) = default;

		//! Append current chunk.
		self_type_t &
		append_chunk( writable_item_t chunk ) &
		{
			auto size = chunk.size();

			if( 0 != size )
				m_chunks.emplace_back( std::move( chunk ) );

			return *this;
		}

		//! Append current chunk.
		self_type_t &&
		append_chunk( writable_item_t chunk ) &&
		{
			return std::move( this->append_chunk( std::move( chunk ) ) );
		}

		//! Flush ready outgoing data.
		/*!
			Schedules for sending currently ready data.
		*/
		self_type_t &
		flush( write_status_cb_t wscb = write_status_cb_t{} ) &
		{
			if( m_connection )
			{
				send_ready_data(
					m_connection,
					response_parts_attr_t::not_final_parts,
					std::move( wscb ) );
			}

			return *this;
		}

		//! Flush ready outgoing data.
		self_type_t &&
		flush( write_status_cb_t wscb = write_status_cb_t{} ) &&
		{
			return std::move( this->flush( std::move( wscb ) ) );
		}

		//! Complete response.
		request_handling_status_t
		done( write_status_cb_t wscb = write_status_cb_t{} )
		{
			if( m_connection )
			{
				// Note: m_connection should become empty after return
				// from that method.
				impl::connection_handle_t old_conn_handle{
						std::move(m_connection) };
				send_ready_data(
					old_conn_handle,
					response_parts_attr_t::final_parts,
					std::move( wscb ) );
			}
			else
			{
				throw_done_must_be_called_once();
			}

			return restinio::request_accepted();
		}

	private:
		void
		send_ready_data(
			const impl::connection_handle_t & conn,
			response_parts_attr_t response_parts_attr,
			write_status_cb_t wscb )
		{
			std::size_t status_line_size{ 0 };
			if( !m_header_was_sent )
			{
				status_line_size = calculate_status_line_size();
				prepare_header_for_sending();
			}

			auto bufs = create_bufs( response_parts_attr_t::final_parts == response_parts_attr );
			m_header_was_sent = true;

			const response_output_flags_t
				response_output_flags{
					response_parts_attr,
					response_connection_attr( m_should_keep_alive_when_header_was_sent ) };

			// We have buffers or at least we have after-write notificator.
			if( !bufs.empty() || wscb )
			{
				write_group_t wg{ std::move( bufs ) };
				wg.status_line_size( status_line_size );

				if( wscb )
				{
					wg.after_write_notificator( std::move( wscb ) );
				}

				conn->write_response_parts(
					m_request_id,
					response_output_flags,
					std::move( wg ) );
			}
		}

		void
		prepare_header_for_sending()
		{
			m_should_keep_alive_when_header_was_sent =
				m_header.should_keep_alive();

			constexpr const char value[] = "chunked";
			if( !m_header.has_field( restinio::http_field::transfer_encoding ) )
			{
				m_header.set_field(
					restinio::http_field::transfer_encoding,
					std::string{ value, impl::ct_string_len( value ) } );
			}
			else
			{
				auto & current_value =
					m_header.get_field( restinio::http_field::transfer_encoding );
				if( std::string::npos == current_value.find( value ) )
				{
					constexpr const char comma_value[] = ",chunked";
					m_header.append_field(
						restinio::http_field::transfer_encoding,
						std::string{
							comma_value,
							impl::ct_string_len( comma_value ) } );
				}
			}
		}

		writable_items_container_t
		create_bufs( bool add_zero_chunk )
		{
			writable_items_container_t bufs;

			std::size_t reserve_size = 2 * m_chunks.size() + 1;

			if( !m_header_was_sent )
			{
				++reserve_size;
			}
			if( add_zero_chunk )
			{
				++reserve_size;
			}

			bufs.reserve( reserve_size );

			if( !m_header_was_sent )
			{
				bufs.emplace_back(
					impl::create_header_string(
						m_header,
						impl::content_length_field_presence_t::skip_content_length ) );
			}

			const char * format_string = "{:X}\r\n";
			for( auto & chunk : m_chunks )
			{
				bufs.emplace_back(
					fmt::format(
						format_string,
						asio_ns::buffer_size( chunk.buf() ) ) );

				// Now include "\r\n"-ending for a previous chunk to format string.
				format_string = "\r\n{:X}\r\n";

				bufs.emplace_back( std::move( chunk ) );

			}

			const char * const ending_representation = "\r\n" "0\r\n\r\n";
			const char * appendix_begin = ending_representation + 2;
			const char * appendix_end = appendix_begin;

			if( !m_chunks.empty() )
			{
				// Add "\r\n"part to appendix.
				appendix_begin -= 2;
				// bufs.emplace_back( const_buffer( rn_ending, 2 ) );
			}

			if( add_zero_chunk )
			{
				// Add "0\r\n\r\n"part to appendix.
				appendix_end += 5;
			}

			if( appendix_begin != appendix_end )
			{
				bufs.emplace_back( const_buffer(
						appendix_begin,
						static_cast<std::size_t>(appendix_end - appendix_begin)
					) );
			}

			m_chunks.clear();

			return bufs;
		}

		//! Flag used by flush() function.
		bool m_header_was_sent{ false };

		//! Saved keep_alive attr actual at the point
		//! a header data was sent.
		/*!
			It is neccessary to guarantee that all parts of response
			will have the same response-connection-attr
			(keep-alive or close);
		*/
		bool m_should_keep_alive_when_header_was_sent{ true };

		//! Chunks accumulator.
		writable_items_container_t m_chunks;
};

} /* namespace restinio */

