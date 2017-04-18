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

	protected:
		http_response_header_t m_header;

		connection_handle_t m_connection;
		const request_id_t m_request_id;
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
			base_response_builder_t<
				response_builder_t<
					restinio_controlled_output_t > >;
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
		{}

		//! Set body
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

template <>
class response_builder_t< user_controlled_output_t > final
	:	public base_response_builder_t< response_builder_t< user_controlled_output_t > >
{
		using base_type_t =
			base_response_builder_t<
				response_builder_t<
					user_controlled_output_t > >;
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
		{}

		//! Set body
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

				conn->write_response_parts(
					m_request_id,
					response_output_flags,
					{
						impl::create_header_string( m_header ),
						std::move( m_body )
					} );
			}
			else if( !m_body.empty() )
			{
				const response_output_flags_t
					response_output_flags{
						response_parts_attr,
						response_connection_attr( m_should_keep_alive_when_header_was_sent ) };

				conn->write_response_parts(
					m_request_id,
					response_output_flags,
					{
						std::move( m_body )
					} );
			}

		}

		//! flag used by flush() function.
		bool m_header_was_sent{ false };
		bool m_should_keep_alive_when_header_was_sent{ true };
		std::string m_body;
};

} /* namespace restinio */
