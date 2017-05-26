/*
	restinio
*/

/*!
	First match reouter.
*/

#pragma once

#include <regex>
#include <vector>
#include <tuple>

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

		void
		add_handler(
			MATCHER matcher,
			default_request_handler_t handler )
		{
			m_handlers.emplace_back(
				std::move( matcher ),
				std::move( handler ) );
		}

	private:
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

		exact_target_matcher_t(
			http_method_t method,
			std::string request_target )
			:	m_method{ method }
			,	m_request_target{ std::move( request_target ) }
		{}

		bool
		operator () ( const http_request_header_t & h ) const
		{
			return
			m_method == h.method() &&
			m_request_target == h.request_target();
		}

	private:
		http_method_t m_method;
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

		begins_with_target_matcher_t(
			http_method_t method,
			std::string request_target_starter )
			:	m_method{ method }
			,	m_request_target_starter{ std::move( request_target_starter ) }
		{}

		bool
		operator () ( const http_request_header_t & h ) const
		{
			if( m_method != h.method() )
				return false;

			const auto & target = h.request_target();

			return m_request_target_starter.size() <= target.size() &&
				m_request_target_starter ==
					target.substr( 0, m_request_target_starter.size() );
		}

	private:
		http_method_t m_method;
		std::string m_request_target_starter;
};

//
// regex_target_matcher_t
//

//! Matches request target beginning.
struct regex_target_matcher_t
{
		enum algo_t
		{
			match_algo,
			search_algo
		};

		regex_target_matcher_t() = default;
		regex_target_matcher_t( regex_target_matcher_t && ) = default;

		regex_target_matcher_t(
			http_method_t method,
			std::regex regex,
			algo_t algo = match_algo,
			std::regex_constants::match_flag_type flags = std::regex_constants::match_default )
			:	m_method{ method }
			,	m_regex{ std::move( regex ) }
			,	m_algo{ algo }
			,	m_flags{ flags }
		{}

		bool
		operator () ( const http_request_header_t & h ) const
		{
			if( m_method != h.method() )
				return false;

			if( match_algo == m_algo )
			{
				return std::regex_match( h.request_target(), m_regex, m_flags );
			}

			return std::regex_search( h.request_target(), m_regex, m_flags );
		}

	private:
		http_method_t m_method;
		std::regex m_regex;
		algo_t m_algo;
		std::regex_constants::match_flag_type m_flags;
};

} /* namespace router */

} /* namespace restinio */
