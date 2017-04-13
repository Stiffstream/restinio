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

		//! Set body
		RESPONSE_BUILDER &
		set_body( std::string body )
		{
			m_body.assign( std::move( body ) );
			return static_cast< RESPONSE_BUILDER & >( *this );
		}

		//! Add header field.
		RESPONSE_BUILDER &
		append_header(
			std::string field_name,
			std::string field_value )
		{
			m_header.set_field(
				std::move( field_name ),
				std::move( field_value ) );
			return static_cast< RESPONSE_BUILDER & >( *this );
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

			return static_cast< RESPONSE_BUILDER & >( *this );
		}

		RESPONSE_BUILDER &
		connection_close()
		{
			m_header.should_keep_alive( false );
			return static_cast< RESPONSE_BUILDER & >( *this );
		}

		RESPONSE_BUILDER &
		connection_keep_alive()
		{
			m_header.should_keep_alive();
			return static_cast< RESPONSE_BUILDER & >( *this );
		}

		//! Append body.
		RESPONSE_BUILDER &
		append_body( const std::string & body_part )
		{
			m_body.append( body_part );
			return static_cast< RESPONSE_BUILDER & >( *this );
		}

	protected:
		http_response_header_t m_header;
		std::string m_body;

		connection_handle_t m_connection;
		const request_id_t m_request_id;
};

//
// response_builder_t
//

// TODO: resp_builder_t is not a final name.
template < typename RESP_OUTPUT_STRATEGY >
class resp_builder_t
{
	resp_builder_t() = delete;
};

struct restinio_controlled_output_t {};

template <>
class resp_builder_t< restinio_controlled_output_t >
	:	public base_response_builder_t< resp_builder_t< restinio_controlled_output_t > >
{
		using base_type_t =
			base_response_builder_t<
				resp_builder_t<
					restinio_controlled_output_t > >;
	public:
		resp_builder_t(
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
		{}


		//! Complete response.
		void
		done()
		{
			if( m_connection )
			{
				auto conn = std::move( m_connection );

				// TODO
			}
		}
};

//
// response_builder_t
//

//! Response builder.
class response_builder_t
{
	public:
		virtual ~response_builder_t()
		{}

		response_builder_t( const response_builder_t & ) = delete;
		void
		operator = ( const response_builder_t & ) = delete;

		response_builder_t( response_builder_t && ) = default;

		response_builder_t(
			std::uint16_t status_code,
			std::string reason_phrase,
			connection_handle_t connection )
			:	m_header{ status_code, std::move( reason_phrase ) }
			,	m_connection{ std::move( connection ) }
		{}

		response_builder_t(
			connection_handle_t connection )
			:	response_builder_t{ 200, "OK", std::move( connection ) }
		{}

		response_builder_t(
			const http_request_header_t & request_header,
			std::uint16_t status_code,
			std::string reason_phrase,
			connection_handle_t connection )
			:	m_header{ status_code, std::move( reason_phrase ) }
			,	m_connection{ std::move( connection ) }
		{
			m_header.http_major( request_header.http_major() );
			m_header.http_minor( request_header.http_minor() );
			m_header.should_keep_alive( request_header.should_keep_alive() );
		}

		response_builder_t(
			http_request_header_t reques_header,
			connection_handle_t connection )
			:	response_builder_t{ reques_header, 200, "OK", std::move( connection ) }
		{}

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

		//! Set body
		response_builder_t &
		set_body( std::string body )
		{
			m_body.assign( std::move( body ) );
			return *this;
		}

		//! Add header field.
		response_builder_t &
		append_header(
			std::string field_name,
			std::string field_value )
		{
			m_header.set_field(
				std::move( field_name ),
				std::move( field_value ) );
			return *this;
		}

		//! Add header `Date` field.
		response_builder_t &
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

			return *this;
		}

		response_builder_t &
		connection_close()
		{
			m_header.should_keep_alive( false );
			return *this;
		}

		response_builder_t &
		connection_keep_alive()
		{
			m_header.should_keep_alive();
			return *this;
		}

		//! Append body.
		response_builder_t &
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

				conn->write_response_message(
					std::move( m_header ),
					std::move( m_body ) );
			}
		}

	private:
		http_response_header_t m_header;
		std::string m_body;

		connection_handle_t m_connection;
};

} /* namespace restinio */
