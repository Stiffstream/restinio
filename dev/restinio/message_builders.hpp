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

		//! Add header `Date` field.
		RESPONSE_BUILDER &
		append_header_date_field(
			std::time_t t = std::time( nullptr ) )
		{
			const auto tpoint = make_gmtime( t );

			std::array< char, 64 > buf;
			strftime(
				buf.data(),
				buf.size(),
				":%a, %d %b %Y %T GMT",
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
		using base_type_t =
			base_response_builder_t< response_builder_t< restinio_controlled_output_t > >;
	public:
		response_builder_t( response_builder_t && ) = default;

		// Reuse construstors from base.
		using base_type_t::base_type_t;

		//! Set body.
		auto &
		set_body( std::string body )
		{
			m_body.assign( std::move( body ) );
			return *this;
		}
		//! Append body.
		auto &
		append_body( const std::string & body_part )
		{
			m_body.append( body_part );
			return *this;
		}

		//! Complete response.
		void
		done()
		{
			if( m_connection )
			{
				auto conn = std::move( m_connection );

				const response_output_flags_t
					response_output_flags{
						response_parts_attr_t::final_parts,
						response_connection_attr( m_header.should_keep_alive() ) };

				m_header.content_length( m_body.size() );

				conn->write_response_parts(
					m_request_id,
					response_output_flags,
					{
						impl::create_header_string( m_header ),
						std::move( m_body )
					} );
			}
		}

	private:
		std::string m_body;
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
		using base_type_t =
			base_response_builder_t< response_builder_t< user_controlled_output_t > >;
	public:
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
		auto &
		set_body( std::string body )
		{
			m_body.assign( std::move( body ) );
			return *this;
		}
		//! Append body (part).
		auto &
		append_body( const std::string & body_part )
		{
			m_body.append( body_part );
			return *this;
		}

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
		void
		done()
		{
			if( m_connection )
			{
				send_ready_data(
					std::move( m_connection ),
					response_parts_attr_t::final_parts );
			}
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

				if( !m_body.empty() )
				{
					conn->write_response_parts(
						m_request_id,
						response_output_flags,
						{
							impl::create_header_string( m_header ),
							std::move( m_body )
						} );
				}
				else
				{
					conn->write_response_parts(
						m_request_id,
						response_output_flags,
						{
							impl::create_header_string( m_header )
						} );
				}

				m_header_was_sent = true;
			}
			else
			{
				const response_output_flags_t
					response_output_flags{
						response_parts_attr,
						response_connection_attr( m_should_keep_alive_when_header_was_sent ) };

				if( !m_body.empty() )
				{
					conn->write_response_parts(
						m_request_id,
						response_output_flags,
						{
							std::move( m_body )
						} );
				}
				else
					conn->write_response_parts(
						m_request_id,
						response_output_flags,
						{
							/*
								Pass nothing
								just to mark response as finished.
							*/
						} );
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
		std::string m_body;
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
			m_chunks.reserve( 16 );
		}

		//! Append current chunk.
		auto &
		start_chunk( std::string part )
		{
			start_new_chunk( std::move( part ) );
			return *this;
		}

		//! Append current chunk.
		auto &
		append_chunk( const std::string & part )
		{
			current_chunk().append( part );
			return *this;
		}

		//! Resets the value of current chunk.
		auto &
		reset_chunk( std::string chunk )
		{
			current_chunk().assign( std::move( chunk ) );
			return *this;
		}

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
		void
		done()
		{
			if( m_connection )
			{
				send_ready_data(
					std::move( m_connection ),
					response_parts_attr_t::final_parts );
			}
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

		std::vector< std::string >
		create_bufs( bool add_zero_chunk )
		{
			std::vector< std::string > bufs;

			std::size_t reserve_size = 2 * m_chunks.size();

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

			for( auto & chunk : m_chunks )
			{
				if( !chunk.empty() )
				{
					bufs.emplace_back( fmt::format( "{:X}\r\n", chunk.size() ) );

					chunk.append( "\r\n" );
					bufs.emplace_back( std::move( chunk ) );
				}
			}

			if( add_zero_chunk )
			{
				constexpr const char zero_chunk[] = "0\r\n\r\n";
				bufs.emplace_back(
					zero_chunk,
					impl::ct_string_len( zero_chunk ) );
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

		//! Response parts accumulator.
		std::vector< std::string > m_chunks;

		std::string * m_current_chunk{ nullptr };

		void
		start_new_chunk()
		{
			m_chunks.resize( m_chunks.size() + 1 );
			m_current_chunk = &m_chunks.back();
		}

		std::string &
		current_chunk()
		{
			if( !m_current_chunk )
			{
				start_new_chunk();
			}

			return *m_current_chunk;
		}

		void
		start_new_chunk( std::string value )
		{
			start_new_chunk();
			current_chunk().assign( std::move( value ) );
		}
};

} /* namespace restinio */
