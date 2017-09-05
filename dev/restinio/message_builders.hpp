/*
	restinio
*/

/*!
	Builders for messages.
*/

#pragma once

#include <time.h>

#include <fmt/format.h>
#include <fmt/time.h>

#include <restinio/common_types.hpp>
#include <restinio/http_headers.hpp>
#include <restinio/connection_handle.hpp>
#include <restinio/os.hpp>

#include <restinio/impl/header_helpers.hpp>

namespace restinio
{

//
// base_response_builder_t
//

template < typename RESPONSE_BUILDER >
class base_response_builder_t
{
	public:
		base_response_builder_t( const base_response_builder_t & ) = delete;
		void
		operator = ( const base_response_builder_t & ) = delete;

		base_response_builder_t( base_response_builder_t && ) = default;

		virtual ~base_response_builder_t()
		{}

		base_response_builder_t(
			std::uint16_t status_code,
			std::string reason_phrase,
			connection_handle_t connection,
			request_id_t request_id,
			bool should_keep_alive )
			:	m_header{ status_code, std::move( reason_phrase ) }
			,	m_connection{ std::move( connection ) }
			,	m_request_id{ request_id }
		{
			m_header.should_keep_alive( should_keep_alive );
		}

		//! Accessors for header.
		//! \{
		http_response_header_t &
		header()
		{
			return m_header;
		}

		const http_response_header_t &
		header() const
		{
			return m_header;
		}
		//! \}

		//! Add header field.
		RESPONSE_BUILDER &
		append_header(
			std::string field_name,
			std::string field_value )
		{
			m_header.set_field(
				std::move( field_name ),
				std::move( field_value ) );
			return upcast_reference();
		}

		//! Add header field.
		RESPONSE_BUILDER &
		append_header(
			http_field_t field_id,
			std::string field_value )
		{
			m_header.set_field(
				field_id,
				std::move( field_value ) );
			return upcast_reference();
		}

		//! Add header `Date` field.
		RESPONSE_BUILDER &
		append_header_date_field(
			std::time_t t = std::time( nullptr ) )
		{
			const auto tpoint = make_gmtime( t );

			std::array< char, 64 > buf;
			// TODO: is there a faster way to get time string?
			strftime(
				buf.data(),
				buf.size(),
				"%a, %d %b %Y %H:%M:%S GMT",
				&tpoint );

			m_header.set_field(
				std::string{ "Date" },
				buf.data() );

			return upcast_reference();
		}

		RESPONSE_BUILDER &
		connection_close()
		{
			m_header.should_keep_alive( false );
			return upcast_reference();
		}

		RESPONSE_BUILDER &
		connection_keep_alive()
		{
			m_header.should_keep_alive();
			return upcast_reference();
		}

	protected:
		http_response_header_t m_header;

		connection_handle_t m_connection;
		const request_id_t m_request_id;

	private:
		RESPONSE_BUILDER &
		upcast_reference()
		{
			return static_cast< RESPONSE_BUILDER & >( *this );
		}
};

//
// response_builder_t
//

template < typename RESP_OUTPUT_STRATEGY >
class response_builder_t
{
	response_builder_t() = delete;
};

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
		set_body( buffer_storage_t body )
		{
			auto size = asio::buffer_size( body.buf() );
			return set_body_impl( body, size );
		}

		//! Append body.
		//! \{
		self_type_t &
		append_body( buffer_storage_t body_part )
		{
			auto size = asio::buffer_size( body_part.buf() );
			return append_body_impl( body_part, size );
		}
		//! \}

		//! Complete response.
		request_handling_status_t
		done()
		{
			if( m_connection )
			{
				auto conn = std::move( m_connection );

				const response_output_flags_t
					response_output_flags{
						response_parts_attr_t::final_parts,
						response_connection_attr( m_header.should_keep_alive() ) };

				m_header.content_length( m_body_size );

				if_neccessary_reserve_first_element_for_header();

				m_response_parts[ 0 ] =
					buffer_storage_t{ impl::create_header_string( m_header ) };

				conn->write_response_parts(
					m_request_id,
					response_output_flags,
					std::move( m_response_parts ) );
			}

			return restinio::request_accepted();
		}

	private:
		self_type_t &
		set_body_impl( buffer_storage_t & body, std::size_t body_size )
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
		append_body_impl( buffer_storage_t & body_part, std::size_t append_size )
		{
			if_neccessary_reserve_first_element_for_header();
			m_response_parts.emplace_back( std::move( body_part ) );
			m_body_size += append_size;
			return *this;
		}

		void
		if_neccessary_reserve_first_element_for_header()
		{
			if( 0 == m_response_parts.size() )
			{
				m_response_parts.reserve( 2 );
				m_response_parts.emplace_back();
			}
		}

		std::size_t m_body_size{ 0 };
		buffers_container_t m_response_parts;
};

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

		// Reuse construstors from base.
		using base_type_t::base_type_t;

		//! Manualy set content length.
		auto &
		set_content_length( std::size_t content_length )
		{
			m_header.content_length( content_length );
			return *this;
		}

		//! Set body (part).
		//! \{
		self_type_t &
		set_body( buffer_storage_t body )
		{
			auto size = asio::buffer_size( body.buf() );
			return set_body_impl( body, size );
		}
		//! \}

		//! Append body.
		//! \{
		self_type_t &
		append_body( buffer_storage_t body_part )
		{
			auto size = asio::buffer_size( body_part.buf() );

			if( 0 == size )
				return *this;

			return append_body_impl( body_part );
		}
		//! \}

		//! Flush ready outgoing data.
		/*!
			Schedules for sending currently ready data.
		*/
		void
		flush()
		{
			if( m_connection )
			{
				send_ready_data(
					m_connection,
					response_parts_attr_t::not_final_parts );
			}
		}

		//! Complete response.
		request_handling_status_t
		done()
		{
			if( m_connection )
			{
				send_ready_data(
					std::move( m_connection ),
					response_parts_attr_t::final_parts );
			}
			return restinio::request_accepted();
		}

	private:
		void
		send_ready_data(
			connection_handle_t conn,
			response_parts_attr_t response_parts_attr )
		{
			if( !m_header_was_sent )
			{
				m_should_keep_alive_when_header_was_sent =
					m_header.should_keep_alive();

				const response_output_flags_t
					response_output_flags{
						response_parts_attr,
						response_connection_attr( m_should_keep_alive_when_header_was_sent ) };

				if_neccessary_reserve_first_element_for_header();

				m_response_parts[ 0 ] =
					buffer_storage_t{ impl::create_header_string( m_header ) };

				conn->write_response_parts(
					m_request_id,
					response_output_flags,
					std::move( m_response_parts ) );

				m_header_was_sent = true;
			}
			else
			{
				const response_output_flags_t
					response_output_flags{
						response_parts_attr,
						response_connection_attr( m_should_keep_alive_when_header_was_sent ) };

				conn->write_response_parts(
					m_request_id,
					response_output_flags,
					std::move( m_response_parts ) );
			}
		}

		self_type_t &
		set_body_impl( buffer_storage_t & body, std::size_t body_size )
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
		append_body_impl( buffer_storage_t & body_part )
		{
			if_neccessary_reserve_first_element_for_header();

			m_response_parts.emplace_back( std::move( body_part ) );
			return *this;
		}

		void
		if_neccessary_reserve_first_element_for_header()
		{
			if( !m_header_was_sent && 0 == m_response_parts.size() )
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
		buffers_container_t m_response_parts;
};

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
		using base_type_t =
			base_response_builder_t< response_builder_t< chunked_output_t > >;
	public:
		response_builder_t(
			std::uint16_t status_code,
			std::string reason_phrase,
			connection_handle_t connection,
			request_id_t request_id,
			bool should_keep_alive )
			:	base_type_t{
					status_code,
					std::move( reason_phrase ),
					std::move( connection ),
					request_id,
					should_keep_alive }
		{
			m_chunks.reserve( 4 );
		}

		//! Append current chunk.
		//! \{
		auto &
		append_chunk( buffer_storage_t chunk )
		{
			auto size = asio::buffer_size( chunk.buf() );

			if( 0 != size )
				m_chunks.emplace_back( std::move( chunk ) );

			return *this;
		}

		//! \}

		//! Flush ready outgoing data.
		/*!
			Schedules for sending currently ready data.
		*/
		void
		flush()
		{
			if( m_connection )
			{
				send_ready_data(
					m_connection,
					response_parts_attr_t::not_final_parts );
			}
		}

		//! Complete response.
		request_handling_status_t
		done()
		{
			if( m_connection )
			{
				send_ready_data(
					std::move( m_connection ),
					response_parts_attr_t::final_parts );
			}
			return restinio::request_accepted();
		}

	private:
		void
		send_ready_data(
			connection_handle_t conn,
			response_parts_attr_t response_parts_attr )
		{
			if( !m_header_was_sent )
			{
				prepare_header_for_sending();
			}

			auto bufs = create_bufs( response_parts_attr_t::final_parts == response_parts_attr );
			m_header_was_sent = true;

			const response_output_flags_t
				response_output_flags{
					response_parts_attr,
					response_connection_attr( m_should_keep_alive_when_header_was_sent ) };

			if( !bufs.empty() )
			{
				conn->write_response_parts(
					m_request_id,
					response_output_flags,
					std::move( bufs ) );
			}
		}

		void
		prepare_header_for_sending()
		{
			m_should_keep_alive_when_header_was_sent =
				m_header.should_keep_alive();

			constexpr const char transfer_encoding[] = "Transfer-Encoding";
			std::string
				transfer_encoding_field_name{
					transfer_encoding,
					impl::ct_string_len( transfer_encoding ) };

			constexpr const char value[] = "chunked";
			if( !m_header.has_field( transfer_encoding ) )
			{
				m_header.set_field(
					std::move( transfer_encoding_field_name ),
					std::string{ value, impl::ct_string_len( value ) } );
			}
			else
			{
				auto & current_value = m_header.get_field( transfer_encoding );
				if( std::string::npos == current_value.find( value ) )
				{
					constexpr const char comma_value[] = ",chunked";
					m_header.append_field(
						transfer_encoding,
						std::string{
							comma_value,
							impl::ct_string_len( comma_value ) } );
				}
			}
		}

		buffers_container_t
		create_bufs( bool add_zero_chunk )
		{
			buffers_container_t bufs;

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
						asio::buffer_size( chunk.buf() ) ) );

				// Now include "\r\n"-ending for a previous chunk to format string.
				format_string = "\r\n{:X}\r\n";

				bufs.emplace_back( std::move( chunk ) );

			}

			if( !m_chunks.empty() )
			{
				// Add "\r\n"-ending for the last part (if any).
				const char * rn_ending = "\r\n";
				bufs.emplace_back( const_buffer( rn_ending, 2 ) );
			}

			if( add_zero_chunk )
			{
				const char * zero_chunk = "0\r\n\r\n";
				bufs.emplace_back( const_buffer( zero_chunk, 5 ) );
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
		buffers_container_t m_chunks;
};

} /* namespace restinio */
