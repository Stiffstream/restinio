/*
	restinio
*/

/*!
	Express.js style router.
*/

#pragma once

#include <restinio/path2regex/path2regex.hpp>
#include <restinio/request_handler.hpp>

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

		const auto &
		indexed_parameters()
		{
			return m_indexed_parameters;
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

		bool
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
// express_router_t
//

//! Express.js style router.
class express_router_t
{
		struct handler_entry_t
		{
			handler_entry_t() = default;
			handler_entry_t( handler_entry_t && ) = default;
			handler_entry_t(
				impl::route_matcher_t matcher,
				express_request_handler_t handler )
				:	m_matcher{ std::move( matcher ) }
				,	m_handler{ std::move( handler ) }
			{}

			impl::route_matcher_t m_matcher;
			express_request_handler_t m_handler;
		};

	public:
		express_router_t() = default;
		express_router_t( express_router_t && ) = default;

		request_handling_status_t
		operator () ( request_handle_t req ) const
		{
			route_params_t params;
			for( const auto & entry : m_handlers )
			{
				if( entry.m_matcher( req->header(), params ) )
				{
					return entry.m_handler( std::move( req ), std::move( params ) );
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
			auto mather_data =
				path2regex::path2regex< route_params_t >(
					route_path,
					options );

			m_handlers.emplace_back(
				impl::route_matcher_t{
					method,
					std::move( mather_data.m_regex ),
					std::move( mather_data.m_param_appender_sequence ) },
				std::move( handler ) );
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
		std::vector< handler_entry_t > m_handlers;
};

} /* namespace router */

} /* namespace restinio */
