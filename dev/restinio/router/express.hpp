/*
	restinio
*/

/*!
	Express.js style router.
*/

#pragma once

#include <restinio/path2regex/path2regex.hpp>
#include <restinio/request_handler.hpp>

#include <restinio/router/std_regex_engine.hpp>

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

		//! Matched route.
		//! \{
		const auto &
		match() const { return m_match; }

		void
		match( const char * str, std::size_t size )
		{
			m_match.assign( str, size );
		}
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
		add_indexed_param( const char * str, std::size_t size )
		{
			m_indexed_parameters.emplace_back( str, size );
		}

		void
		add_named_param( std::string key, const char * str, std::size_t size )
		{
			m_named_parameters[ std::move( key ) ] = std::string( str, size );
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

using param_appender_sequence_t =
	path2regex::param_appender_sequence_t< route_params_t >;

//
// route_matcher_t
//

//! A matcher for a given path.
template <typename Regex_Engine = std_regex_engine_t>
class route_matcher_t
{
	public:
		using regex_t = typename Regex_Engine::compiled_regex_t;
		using match_results_t = typename Regex_Engine::match_results_t;

		route_matcher_t(
			http_method_t method,
			regex_t route_regex,
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
			match_results_t matches;
			if( Regex_Engine::try_match(
					request_target,
					m_route_regex,
					matches ) )
			{
				assert( m_param_appender_sequence.size() + 1 == matches.size() );

				auto get_size = []( const auto & m ){ return m.second - m.first; };

				parameters.match(
					Regex_Engine::start_str_piece( matches[0] ),
					Regex_Engine::size_str_piece( matches[0] ) );

				for( std::size_t i = 1; i < matches.size(); ++i )
				{
					const auto & m = matches[ i ];
					m_param_appender_sequence[ i - 1](
						parameters,
						Regex_Engine::start_str_piece( m ),
						Regex_Engine::size_str_piece( m ) );
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
		regex_t m_route_regex;
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
template <typename Regex_Engine = std_regex_engine_t>
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
		impl::route_matcher_t< Regex_Engine > m_matcher;
		express_request_handler_t m_handler;
};

//
// express_router_t
//

//! Express.js style router.
template <typename Regex_Engine = std_regex_engine_t>
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

		//! Add handlers.
		//! \{
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
		//! \}

	private:
		using route_entry_t = express_route_entry_t< Regex_Engine >;

		//! A list of existing routes.
		std::vector< route_entry_t > m_handlers;
};

} /* namespace router */

} /* namespace restinio */
