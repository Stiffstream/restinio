/*
	restinio
*/

/*!
	Express.js style router.
*/

#pragma once

#include <restinio/router/impl/target_path_holder.hpp>
#include <restinio/router/non_matched_request_handler.hpp>

#include <restinio/optional.hpp>

#include <restinio/path2regex/path2regex.hpp>

#include <restinio/router/std_regex_engine.hpp>
#include <restinio/router/method_matcher.hpp>

#include <restinio/utils/from_string.hpp>
#include <restinio/utils/percent_encoding.hpp>

#include <map>
#include <vector>

namespace restinio
{

namespace router
{

namespace impl
{

struct route_params_accessor_t;

} /* namespace impl */

//
// route_params_t
//

//! Parameters extracted from route.
/*!
	Holds values of parameters extracted from route.

	All values are stored as string views,
	route_params_t instance owns bufers used by these views.
	And that leads to following limittions:
	once a copy of a string view is created it is
	important to use it with respect to life time of route_params_t
	instance to which this parameter bind belongs.
	String view is valid during route_params_t
	instance life time.
*/
class route_params_t final
{
	public:
		using named_parameters_container_t =
			std::vector< std::pair< string_view_t, string_view_t > >;
		using indexed_parameters_container_t =
			std::vector< string_view_t >;

	private:
		friend struct impl::route_params_accessor_t;

		void
		match(
			std::unique_ptr< char[] > request_target,
			std::shared_ptr< std::string > key_names_buffer,
			string_view_t match,
			named_parameters_container_t named_parameters,
			indexed_parameters_container_t indexed_parameters )
		{
			m_request_target = std::move( request_target );
			m_key_names_buffer = std::move( key_names_buffer );
			m_match = match;
			m_named_parameters = std::move( named_parameters );
			m_indexed_parameters = std::move( indexed_parameters );
		}

	public:
		route_params_t() = default;

		route_params_t( route_params_t && ) = default;
		route_params_t & operator = ( route_params_t && ) = default;

		route_params_t( const route_params_t & ) = delete;
		route_params_t & operator = ( const route_params_t & ) = delete;

		//! Matched route.
		string_view_t match() const noexcept { return m_match; }

		//! Get named parameter.
		string_view_t
		operator [] ( string_view_t key ) const
		{
			return find_named_parameter_with_check( key ).second;
		}

		//! Check parameter.
		bool
		has( string_view_t key ) const noexcept
		{
			return m_named_parameters.end() != find_named_parameter( key );
		}

		//! Get the value of a parameter if it exists.
		//! @since v.0.4.4
		optional_t< string_view_t >
		get_param( string_view_t key ) const noexcept
		{
			const auto it = find_named_parameter( key );

			return m_named_parameters.end() != it ?
				optional_t< string_view_t >{ it->second } :
				optional_t< string_view_t >{ nullopt };
		}

		//! Get indexed parameter.
		string_view_t
		operator [] ( std::size_t i ) const
		{
			if( i >= m_indexed_parameters.size() )
				throw exception_t{ fmt::format( "invalid parameter index: {}", i ) };

			return m_indexed_parameters.at( i );
		}

		//! Get number of parameters.
		//! \{
		auto named_parameters_size() const noexcept { return m_named_parameters.size(); }
		auto indexed_parameters_size() const noexcept { return m_indexed_parameters.size(); }
		//! \}

	private:
		named_parameters_container_t::const_iterator
		find_named_parameter( string_view_t key ) const noexcept
		{
			return
				std::find_if(
					m_named_parameters.begin(),
					m_named_parameters.end(),
					[&]( const auto p ){
						return key == p.first;
					} );
		}

		named_parameters_container_t::const_reference
		find_named_parameter_with_check( string_view_t key ) const
		{
			auto it = find_named_parameter( key );

			if( m_named_parameters.end() == it )
				throw exception_t{
					fmt::format(
						"invalid parameter name: {}",
						std::string{ key.data(), key.size() } ) };

			return *it;
		}

		//! A raw request target.
		/*!
			All parameters values are defined as string views
			refering parts of this beffer.

			\note `std::unique_ptr< char[] >` is used here on purpose,
			because if we consider std::string, then it has an issue
			when SSO is applied. It is important that
			parameters that refering buffer are valid after move operations with
			the buffer. And std::strings with SSO applied cannot guarantee this.
			Vector on the other hand gives this guarantee.
		*/
		std::unique_ptr< char[] > m_request_target;

		//! Shared buffer for string_view of named parameterts names.
		std::shared_ptr< std::string > m_key_names_buffer;

		//! Matched pattern.
		string_view_t m_match;

		//! Named params.
		named_parameters_container_t m_named_parameters;

		//! Indexed params.
		indexed_parameters_container_t m_indexed_parameters;
};

namespace impl
{

//
// route_params_accessor_t
//

//! Route params private internals accessor.
struct route_params_accessor_t
{
	//! Init parameters with a matched route params.
	static void
	match(
		route_params_t & rp,
		std::unique_ptr< char[] > request_target,
		std::shared_ptr< std::string > key_names_buffer,
		string_view_t match_,
		route_params_t::named_parameters_container_t named_parameters,
		route_params_t::indexed_parameters_container_t indexed_parameters )
	{
		rp.match(
			std::move( request_target ),
			std::move( key_names_buffer ),
			match_,
			std::move( named_parameters ),
			std::move( indexed_parameters ) );
	}

	//! Get values containers for all parameters (used in unit tests).
	//! \{
	static const auto &
	named_parameters( const route_params_t & rp ) noexcept
	{
		return rp.m_named_parameters;
	}

	static const auto &
	indexed_parameters( const route_params_t & rp ) noexcept
	{
		return rp.m_indexed_parameters;
	}
	//! \}
};

//
// route_params_appender_t
//

//! Helper class for gthering parameters from route.
class route_params_appender_t
{
	public:
		route_params_appender_t(
			route_params_t::named_parameters_container_t & named_parameters,
			route_params_t::indexed_parameters_container_t & indexed_parameters )
			:	m_named_parameters{ named_parameters }
			,	m_indexed_parameters{ indexed_parameters }
		{}

		route_params_appender_t( const route_params_appender_t & ) = delete;
		route_params_appender_t( route_params_appender_t && ) = delete;
		route_params_appender_t & operator = ( const route_params_appender_t & ) = delete;
		route_params_appender_t & operator = ( route_params_appender_t && ) = delete;

		void
		add_named_param( string_view_t key, string_view_t value )
		{
			m_named_parameters.emplace_back( key, value );
		}

		void
		add_indexed_param( string_view_t value )
		{
			m_indexed_parameters.emplace_back( value );
		}

	private:
		route_params_t::named_parameters_container_t & m_named_parameters;
		route_params_t::indexed_parameters_container_t & m_indexed_parameters;
};

using param_appender_sequence_t =
	path2regex::param_appender_sequence_t< route_params_appender_t >;

//
// route_matcher_t
//

//! A matcher for a given path.
template < typename Regex_Engine = std_regex_engine_t >
class route_matcher_t
{
	public:
		using regex_t = typename Regex_Engine::compiled_regex_t;
		using match_results_t = typename Regex_Engine::match_results_t;

		//! Creates matcher with a given parameters.
		route_matcher_t(
			http_method_id_t method,
			regex_t route_regex,
			std::shared_ptr< std::string > named_params_buffer,
			param_appender_sequence_t param_appender_sequence )
			:	m_route_regex{ std::move( route_regex ) }
			,	m_named_params_buffer{ std::move( named_params_buffer ) }
			,	m_param_appender_sequence{ std::move( param_appender_sequence ) }
		{
			assign( m_method_matcher, std::move(method) );
		}

		/*!
		 * Creates matcher with a given parameters.
		 *
		 * This constructor is intended for cases where method_matcher is
		 * specified as object of class derived from method_matcher_t.
		 *
		 * @since v.0.6.6
		 */
		template< typename Method_Matcher >
		route_matcher_t(
			Method_Matcher && method_matcher,
			regex_t route_regex,
			std::shared_ptr< std::string > named_params_buffer,
			param_appender_sequence_t param_appender_sequence )
			:	m_route_regex{ std::move( route_regex ) }
			,	m_named_params_buffer{ std::move( named_params_buffer ) }
			,	m_param_appender_sequence{ std::move( param_appender_sequence ) }
		{
			assign(
					m_method_matcher,
					std::forward<Method_Matcher>(method_matcher) );
		}

		route_matcher_t() = default;
		route_matcher_t( route_matcher_t && ) = default;

		//! Try to match a given request target with this route.
		bool
		match_route(
			target_path_holder_t & target_path,
			route_params_t & parameters ) const
		{
			match_results_t matches;
			if( Regex_Engine::try_match(
					target_path.view(),
					m_route_regex,
					matches ) )
			{
				assert( m_param_appender_sequence.size() + 1 >= matches.size() );

				// Data for route_params_t initialization.

				auto captured_params = target_path.giveout_data();

				const string_view_t match{
					captured_params.get() + Regex_Engine::submatch_begin_pos( matches[0] ),
					Regex_Engine::submatch_end_pos( matches[0] ) -
						Regex_Engine::submatch_begin_pos( matches[0] ) } ;

				route_params_t::named_parameters_container_t named_parameters;
				route_params_t::indexed_parameters_container_t indexed_parameters;

				route_params_appender_t param_appender{ named_parameters, indexed_parameters };

				// Std regex and pcre engines handle
				// trailing groups with empty values differently.
				// Std despite they are empty includes them in the list of match results;
				// Pcre on the other hand does not.
				// So the second for is for pushing empty values
				std::size_t i = 1;
				for( ; i < matches.size(); ++i )
				{
					const auto & m = matches[ i ];
					m_param_appender_sequence[ i - 1](
						param_appender,
						string_view_t{
							captured_params.get() + Regex_Engine::submatch_begin_pos( m ),
							Regex_Engine::submatch_end_pos( m ) -
								Regex_Engine::submatch_begin_pos( m ) } );
				}

				for( ; i < m_param_appender_sequence.size() + 1; ++i )
				{
					m_param_appender_sequence[ i - 1 ](
						param_appender,
						string_view_t{ captured_params.get(), 0 } );
				}

				// Init route parameters.
				route_params_accessor_t::match(
						parameters,
						std::move( captured_params ),
						m_named_params_buffer, // Do not move (it is used on each match).
						std::move( match ),
						std::move( named_parameters ),
						std::move( indexed_parameters ) );

				return true;
			}

			return false;
		}

		inline bool
		operator () (
			const http_request_header_t & h,
			target_path_holder_t & target_path,
			route_params_t & parameters ) const
		{
			return m_method_matcher->match( h.method() ) &&
					match_route( target_path, parameters );
		}

	private:
		//! HTTP method to match.
		buffered_matcher_holder_t m_method_matcher;

		//! Regex of a given route.
		regex_t m_route_regex;

		//! Buffer for named parameters names string views.
		std::shared_ptr< std::string > m_named_params_buffer;

		//! Parameters values.
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
template < typename Regex_Engine = std_regex_engine_t>
class express_route_entry_t
{
		using matcher_init_data_t =
			path2regex::impl::route_regex_matcher_data_t<
					impl::route_params_appender_t,
					Regex_Engine >;

		template< typename Method_Matcher >
		express_route_entry_t(
			Method_Matcher && method_matcher,
			matcher_init_data_t matcher_data,
			express_request_handler_t handler )
			:	m_matcher{
					std::forward<Method_Matcher>( method_matcher ),
					std::move( matcher_data.m_regex ),
					std::move( matcher_data.m_named_params_buffer ),
					std::move( matcher_data.m_param_appender_sequence ) }
			,	m_handler{ std::move( handler ) }
		{}

	public:
		express_route_entry_t( const express_route_entry_t & ) = delete;
		express_route_entry_t & operator = ( const express_route_entry_t & ) = delete;

		express_route_entry_t() = default;
		express_route_entry_t( express_route_entry_t && ) = default;
		express_route_entry_t &
		operator = ( express_route_entry_t && ) = default;

		template< typename Method_Matcher >
		express_route_entry_t(
			Method_Matcher && method_matcher,
			string_view_t route_path,
			const path2regex::options_t & options,
			express_request_handler_t handler )
			:	express_route_entry_t{
					std::forward<Method_Matcher>( method_matcher ),
					path2regex::path2regex< impl::route_params_appender_t, Regex_Engine >(
						route_path,
						options ),
					std::move( handler ) }
		{}

		template< typename Method_Matcher >
		express_route_entry_t(
			Method_Matcher && method_matcher,
			string_view_t route_path,
			express_request_handler_t handler )
			:	express_route_entry_t{
					std::forward<Method_Matcher>( method_matcher ),
					route_path,
					path2regex::options_t{},
					std::move( handler ) }
		{}

		//! Checks if request header matches entry,
		//! and if so, set route params.
		RESTINIO_NODISCARD
		bool
		match(
			const http_request_header_t & h,
			impl::target_path_holder_t & target_path,
			route_params_t & params ) const
		{
			return m_matcher( h, target_path, params );
		}

		//! Calls a handler of given request with given params.
		RESTINIO_NODISCARD
		request_handling_status_t
		handle( request_handle_t rh, route_params_t rp ) const
		{
			return m_handler( std::move( rh ), std::move( rp ) );
		}

	private:
		impl::route_matcher_t< Regex_Engine > m_matcher;
		express_request_handler_t m_handler;
};

//
// express_router_t
//

//! Express.js style router.
/*
	Express routers acts as a request handler (it means it is a function-object
	that can be called as a restinio request handler).
	It aggregates several endpoint-handlers and picks one or none of them to handle the request.
	The choice of the handler to execute depends on request target and HTTP method.
	If router finds no handler matching the request then request is considered unmatched.
	It is possible to set a handler for unmatched requests, otherwise router rejects the request and
	RESTinio takes care of it.

	There is a difference between ordinary restinio request handler
	and the one that is used with experss router: express_request_handler_t.
	The signature of a handlers that can be put in router
	has an additional parameter -- a container with parameters extracted from URI (request target).
*/
template < typename Regex_Engine = std_regex_engine_t>
class express_router_t
{
	public:
		express_router_t() = default;
		express_router_t( express_router_t && ) = default;

		RESTINIO_NODISCARD
		request_handling_status_t
		operator () ( request_handle_t req ) const
		{
			impl::target_path_holder_t target_path{ req->header().path() };
			route_params_t params;
			for( const auto & entry : m_handlers )
			{
				if( entry.match( req->header(), target_path, params ) )
				{
					return entry.handle( std::move( req ), std::move( params ) );
				}
			}

			// Here: none of the routes matches this handler.

			if( m_non_matched_request_handler )
			{
				// If non matched request handler is set
				// then call it.
				return m_non_matched_request_handler( std::move( req ) );
			}

			return request_rejected();
		}

		//! Add handlers.
		//! \{
		template< typename Method_Matcher >
		void
		add_handler(
			Method_Matcher && method_matcher,
			string_view_t route_path,
			express_request_handler_t handler )
		{
			add_handler(
				std::forward<Method_Matcher>(method_matcher),
				route_path,
				path2regex::options_t{},
				std::move( handler ) );
		}

		template< typename Method_Matcher >
		void
		add_handler(
			Method_Matcher && method_matcher,
			string_view_t route_path,
			const path2regex::options_t & options,
			express_request_handler_t handler )
		{
			m_handlers.emplace_back(
					std::forward<Method_Matcher>(method_matcher),
					route_path,
					options,
					std::move( handler ) );
		}

		void
		http_delete(
			string_view_t route_path,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_delete(),
				route_path,
				std::move( handler ) );
		}

		void
		http_delete(
			string_view_t route_path,
			const path2regex::options_t & options,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_delete(),
				route_path,
				options,
				std::move( handler ) );
		}

		void
		http_get(
			string_view_t route_path,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_get(),
				route_path,
				std::move( handler ) );
		}

		void
		http_get(
			string_view_t route_path,
			const path2regex::options_t & options,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_get(),
				route_path,
				options,
				std::move( handler ) );
		}

		void
		http_head(
			string_view_t route_path,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_head(),
				route_path,
				std::move( handler ) );
		}

		void
		http_head(
			string_view_t route_path,
			const path2regex::options_t & options,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_head(),
				route_path,
				options,
				std::move( handler ) );
		}

		void
		http_post(
			string_view_t route_path,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_post(),
				route_path,
				std::move( handler ) );
		}

		void
		http_post(
			string_view_t route_path,
			const path2regex::options_t & options,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_post(),
				route_path,
				options,
				std::move( handler ) );
		}

		void
		http_put(
			string_view_t route_path,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_put(),
				route_path,
				std::move( handler ) );
		}

		void
		http_put(
			string_view_t route_path,
			const path2regex::options_t & options,
			express_request_handler_t handler )
		{
			add_handler(
				http_method_put(),
				route_path,
				options,
				std::move( handler ) );
		}
		//! \}

		//! Set handler for requests that don't match any route.
		void
		non_matched_request_handler( non_matched_request_handler_t nmrh )
		{
			m_non_matched_request_handler= std::move( nmrh );
		}

	private:
		using route_entry_t = express_route_entry_t< Regex_Engine >;

		//! A list of existing routes.
		std::vector< route_entry_t > m_handlers;

		//! Handler that is called for requests that don't match any route.
		non_matched_request_handler_t m_non_matched_request_handler;
};

} /* namespace router */

//! Cast named parameter value to a given type.
template < typename Value_Type >
Value_Type
get( const router::route_params_t & params, string_view_t key )
{
	return get< Value_Type >( params[ key ] );
}

//! Cast indexed parameter value to a given type.
template < typename Value_Type >
Value_Type
get( const router::route_params_t & params, std::size_t index )
{
	return get< Value_Type >( params[ index ] );
}

} /* namespace restinio */
