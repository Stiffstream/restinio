/*
	restinio
*/

/*!
	First match reouter.
*/

#pragma once

#include <vector>

#include <restinio/request_handler.hpp>

namespace restinio
{

namespace router
{

//
// first_match_router_t
//

//! Router selects the first handler that matches with request header.
template < typename MATCHER >
class first_match_router_t
{
	public:
		first_match_router_t() = default;
		first_match_router_t( first_match_router_t && ) = default;

		request_handling_status_t
		operator () ( request_handle_t req ) const
		{
			for( const auto & entry : m_handlers )
			{
				if( entry.m_matcher( req->header() ) )
				{
					return entry.m_handler( std::move( req ) );
				}
			}

			return request_rejected();
		}

	private:
		struct handler_entry_t
		{
			handler_entry_t() = default;
			handler_entry_t( handler_entry_t && ) = default;
			handler_entry_t( MATCHER matcher, default_request_handler_t handler )
				:	m_matcher{ std::move( matcher ) }
				,	m_handler{ std::move( handler ) }
			{}

			MATCHER m_matcher;
			default_request_handler_t m_handler;
		};

		std::vector< handler_entry_t > m_handlers;
};

//
// exact_target_matcher_t
//

//! Matches request target exactly.
struct exact_target_matcher_t
{
		exact_target_matcher_t() = default;
		exact_target_matcher_t( exact_target_matcher_t && ) = default;

		bool
		operator () ( const http_request_header_t & h ) const
		{
			return m_request_target == h.request_target();
		}

	private:
		std::string m_request_target;
};

//
// begins_with_target_matcher_t
//

//! Matches request target beginning.
struct begins_with_target_matcher_t
{
		begins_with_target_matcher_t() = default;
		begins_with_target_matcher_t( begins_with_target_matcher_t && ) = default;

		bool
		operator () ( const http_request_header_t & h ) const
		{
			const auto & target = h.request_target();

			return m_request_target_starter.size() <= target.size() &&
				m_request_target_starter ==
					target.substr( 0, m_request_target_starter.size() );
		}

	private:
		std::string m_request_target_starter;
};

} /* namespace router */

} /* namespace restinio */
