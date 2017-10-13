/*
	restinio
*/

/*!
	Express.js style router.
*/

#pragma once

#include <restinio/path2regex/path2regex.hpp>
#include <restinio/request_handler.hpp>

#include <map>
#include <vector>

namespace restinio
{

namespace router
{

//
// route_params_t
//

//! Parameters extracted from route.
class route_params_t
{
	public:
		route_params_t() = default;
		route_params_t( route_params_t && ) = default;

		route_params_t &
		operator = ( route_params_t && ) = default;

		//! Prefix and suffix of the matched route.
		//! \{
		const auto &
		match() const { return m_match; }
		const auto &
		prefix() const { return m_prefix; }
		const auto &
		suffix() const { return m_suffix; }

		void
		match( std::string value ){ m_match = std::move( value ); }
		void
		prefix( std::string value ) { m_prefix = std::move( value ); }
		void
		suffix( std::string value ) { m_suffix = std::move( value ); }
		//! \}

		const auto &
		named_parameters()
		{
			return m_named_parameters;
		}

		const std::string &
		operator [] ( const std::string & key ) const
		{
			return m_named_parameters.at( key );
		}

		const auto &
		indexed_parameters()
		{
			return m_indexed_parameters;
		}

		const std::string &
		operator [] ( std::size_t i ) const
		{
			return m_indexed_parameters.at( i );
		}

		void
		add_indexed_param( std::string value )
		{
			m_indexed_parameters.emplace_back( std::move( value ) );
		}

		void
		add_named_param( std::string key, std::string value )
		{
			m_named_parameters[ std::move( key ) ] = std::move( value );
		}

		void
		reset()
		{
			m_match.clear();
			m_prefix.clear();
			m_suffix.clear();
			m_named_parameters.clear();
			m_indexed_parameters.clear();
		}

	private:
		std::string m_match;
		std::string m_prefix;
		std::string m_suffix;

		std::map< std::string, std::string > m_named_parameters;
		std::vector< std::string > m_indexed_parameters;
};

namespace impl
{

using param_appender_sequence_t = path2regex::param_appender_sequence_t< route_params_t >;

//
// route_matcher_t
//

//! A matcher for a given path.
class route_matcher_t
{
	public:
		route_matcher_t(
			http_method_t method,
			std::regex route_regex,
			param_appender_sequence_t param_appender_sequence )
			:	m_method{ method }
			,	m_route_regex{ std::move( route_regex ) }
			,	m_param_appender_sequence{ std::move( param_appender_sequence ) }
		{}

		route_matcher_t() = default;
		route_matcher_t( route_matcher_t && ) = default;

		bool
		match_route(
			const std::string & request_target,
			route_params_t & parameters ) const
		{
			std::smatch matches;
			if( std::regex_search( request_target, matches, m_route_regex ) )
			{
				assert( m_param_appender_sequence.size() + 1 == matches.size() );

				parameters.match( matches[0].str() );
				parameters.prefix( matches.prefix() );
				parameters.suffix( matches.suffix() );

				for( std::size_t i = 1; i < matches.size(); ++i )
				{
					m_param_appender_sequence[ i - 1]( parameters, matches[ i ] );
				}

				return true;
			}

			return false;
		}

		inline bool
		operator () (
			const http_request_header_t & h,
			route_params_t & parameters ) const
		{
			return m_method == h.method() &&
					match_route( h.request_target(), parameters );
		}

	private:
		http_method_t m_method;
		std::regex m_route_regex;
		param_appender_sequence_t m_param_appender_sequence;
};

} /* namespace impl */

//
// express_request_handler_t
//

using express_request_handler_t =
		std::function< request_handling_status_t( request_handle_t, route_params_t ) >;

//
// express_route_entry_t
//

//! A single express route entry.
/*!
	Might be helpful for use without express_router_t,
	if only a single route is needed.
	It gives the same help with route parameters.
*/
class express_route_entry_t
{
		express_route_entry_t(
			http_method_t method,
			path2regex::impl::route_regex_matcher_data_t< route_params_t > matcher_data,
			express_request_handler_t handler )
			:	m_matcher{
					method,
					std::move( matcher_data.m_regex ),
					std::move( matcher_data.m_param_appender_sequence ) }
			,	m_handler{ std::move( handler ) }
		{}

	public:
		express_route_entry_t( const express_route_entry_t & ) = delete;
		const express_route_entry_t &
		operator = ( const express_route_entry_t & ) = delete;

		express_route_entry_t() = default;
		express_route_entry_t( express_route_entry_t && ) = default;
		express_route_entry_t &
		operator = ( express_route_entry_t && ) = default;

		express_route_entry_t(
			http_method_t method,
			const std::string & route_path,
			const path2regex::options_t & options,
			express_request_handler_t handler )
			:	express_route_entry_t{
					method,
					path2regex::path2regex< route_params_t >(
						route_path,
						options ),
					std::move( handler ) }
		{}

		express_route_entry_t(
			http_method_t method,
			const std::string & route_path,
			express_request_handler_t handler )
			:	express_route_entry_t{
					method,
					route_path,
					path2regex::options_t{},
					std::move( handler ) }
		{}

		//! Checks if request header matches entry,
		//! and if so, set route params.
		bool
		match( const http_request_header_t & h, route_params_t & params ) const
		{
			return m_matcher( h, params );
		}

		//! Calls a handler of given request with given params.
		request_handling_status_t
		handle( request_handle_t rh, route_params_t rp ) const
		{
			return m_handler( std::move( rh ), std::move( rp ) );
		}

		//! Try to match the entry and calls a handler with extracted params.
		request_handling_status_t
		try_to_handle( request_handle_t rh ) const
		{
			route_params_t params;
			if( match( rh->header(), params ) )
				return handle( std::move( rh ), std::move( params ) );

			return request_rejected();
		}

	private:
		impl::route_matcher_t m_matcher;
		express_request_handler_t m_handler;
};

//
// express_router_t
//

//! Express.js style router.
class express_router_t
{
	public:
		express_router_t() = default;
		express_router_t( express_router_t && ) = default;

		request_handling_status_t
		operator () ( request_handle_t req ) const
		{
			route_params_t params;
			for( const auto & entry : m_handlers )
			{
				if( entry.match( req->header(), params ) )
				{
					return entry.handle( std::move( req ), std::move( params ) );
				}
			}

			return request_rejected();
		}

		void
		add_handler(
			http_method_t method,
			const std::string & route_path,
			express_request_handler_t handler )
		{
			add_handler(
				method,
				route_path,
				path2regex::options_t{},
				std::move( handler ) );
		}

		void
		add_handler(
			http_method_t method,
			const std::string & route_path,
			const path2regex::options_t & options,
			express_request_handler_t handler )
		{
			m_handlers.emplace_back( method, route_path, options, std::move( handler ) );
		}

		void
		http_delete(
			const std::string & route_path,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_t::http_delete,
				route_path,
				std::move( handler ) );
		}

		void
		http_delete(
			const std::string & route_path,
			const path2regex::options_t & options,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_t::http_delete,
				route_path,
				options,
				std::move( handler ) );
		}

		void
		http_get(
			const std::string & route_path,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_t::http_get,
				route_path,
				std::move( handler ) );
		}

		void
		http_get(
			const std::string & route_path,
			const path2regex::options_t & options,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_t::http_get,
				route_path,
				options,
				std::move( handler ) );
		}

		void
		http_head(
			const std::string & route_path,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_t::http_head,
				route_path,
				std::move( handler ) );
		}

		void
		http_head(
			const std::string & route_path,
			const path2regex::options_t & options,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_t::http_head,
				route_path,
				options,
				std::move( handler ) );
		}

		void
		http_post(
			const std::string & route_path,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_t::http_post,
				route_path,
				std::move( handler ) );
		}

		void
		http_post(
			const std::string & route_path,
			const path2regex::options_t & options,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_t::http_post,
				route_path,
				options,
				std::move( handler ) );
		}

		void
		http_put(
			const std::string & route_path,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_t::http_put,
				route_path,
				std::move( handler ) );
		}

		void
		http_put(
			const std::string & route_path,
			const path2regex::options_t & options,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_t::http_put,
				route_path,
				options,
				std::move( handler ) );
		}

	private:
		std::vector< express_route_entry_t > m_handlers;
};

} /* namespace router */

} /* namespace restinio */
