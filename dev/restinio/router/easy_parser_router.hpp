/*
 * RESTinio
 */

/*!
 * @file
 * @brief A router based on easy_parser.
 *
 * @since v.0.6.6
 */

#pragma once

#include <restinio/router/impl/target_path_holder.hpp>
#include <restinio/router/non_matched_request_handler.hpp>

#include <restinio/helpers/easy_parser.hpp>

#include <vector>

namespace restinio
{

namespace router
{

namespace easy_parser_router
{

namespace impl
{

using target_path_holder_t = restinio::router::impl::target_path_holder_t;

/*!
 * @brief Helper type to indicate a negative match attempt.
 *
 * @since v.0.6.6
 */
struct no_match_t {};

/*!
 * @brief An interface for one entry of easy_parser-based router.
 *
 * @since v.0.6.6
 */
class router_entry_t
{
public:
	virtual ~router_entry_t() = default;

	//! An attempt to match a request against the route.
	/*!
	 * If match successed the corresponding request handler is called
	 * and its return value is returned in form of
	 * request_handling_status_t value.
	 *
	 * If match failed then an instance of no_match_t is returned.
	 */
	RESTINIO_NODISCARD
	virtual expected_t< request_handling_status_t, no_match_t >
	try_handle(
		const request_handle_t & req,
		target_path_holder_t & target_path ) const = 0;
};

/*!
 * @brief An alias for unique_ptr of router_entry.
 *
 * @since v.0.6.6
 */
using router_entry_unique_ptr_t = std::unique_ptr< router_entry_t >;

//FIXME: document this!
template< typename Producer, typename Handler >
class actual_router_entry_t : public router_entry_t
{
	http_method_id_t m_method;
	Producer m_producer;
	Handler m_handler;

public:
	template< typename Producer_Arg, typename Handler_Arg >
	actual_router_entry_t(
		http_method_id_t method,
		Producer_Arg && producer,
		Handler_Arg && handler )
		:	m_method{ method }
		,	m_producer{ std::forward<Producer_Arg>(producer) }
		,	m_handler{ std::forward<Handler_Arg>(handler) }
	{}

	RESTINIO_NODISCARD
	expected_t< request_handling_status_t, no_match_t >
	try_handle(
		const request_handle_t & req,
		target_path_holder_t & target_path ) const override
	{
		if( m_method == req->header().method() )
		{
			auto parse_result = easy_parser::try_parse(
					target_path.view(),
					m_producer );
			if( parse_result )
			{
				return m_handler( req, *parse_result );
			}
		}

		return make_unexpected( no_match_t{} );
	}
};

//
// unescape_transformer_t
//
/*!
 * @brief A transformer that performs percent-unescaping of
 * an input string.
 *
 * @since v.0.6.6
 */
template< typename Unescape_Traits >
struct unescape_transformer_t
	:	public restinio::easy_parser::impl::transformer_tag< std::string >
{
	using input_type = std::string;

	RESTINIO_NODISCARD
	result_type
	transform( input_type && input ) const
	{
		return restinio::utils::unescape_percent_encoding< Unescape_Traits >(
				input );
	}
};

} /* namespace impl */

using namespace restinio::easy_parser;

//
// root_t
//
/*!
 * @brief A special type that indicates the root path.
 *
 * It should be used in the cases when a request-handler for the root
 * path (aka '/') has be defined:
 * @code
 * namespace epr = restinio::router::easy_parser_router;
 * ...
 * router->add_handler(retinio::http_method_get(),
 * 	epr::root_p(),
 * 	[](const restinio::request_handle_t & req, const epr::root_t ) {
 * 		...
 * 	});
 * @endcode
 *
 * @since v.0.6.6
 */
struct root_t {};

//
// root_p
//
/*!
 * @brief A factory for the creation of special root-producer.
 *
 * It should be used in the cases when a request-handler for the root
 * path (aka '/') has be defined:
 * @code
 * namespace epr = restinio::router::easy_parser_router;
 * ...
 * router->add_handler(retinio::http_method_get(),
 * 	epr::root_p(),
 * 	[](const restinio::request_handle_t & req, const epr::root_t ) {
 * 		...
 * 	});
 * @endcode
 *
 * @since v.0.6.6
 */
RESTINIO_NODISCARD
inline auto
root_p()
{
	return symbol_p( '/' ) >> just( root_t{} );
}

//
// slash
//
/*!
 * @brief A factory for a clause that matches '/' in the route path.
 *
 * Usage example:
 * @code
 * namespace epr = restinio::router::easy_parser_router;
 *
 * struct load_params { std::uint32_t count{1}, timeout_sec{1}; };
 *
 * router->add_handler(restinio::http_method_get(),
 * 	epr::produce<load_params>(
 * 		epr::slash(), // '/'
 * 		epr::non_negative_decimal_number_p() >> &load_params::count,
 * 		epr::slash(), // '/'
 * 		epr::non_negative_decimal_number_p() >> &load_params::timeout_sec
 * 	),
 * 	[](const restinio::request_handle_t & req, const load_params params) {
 * 		...
 * 	});
 * @endcode
 *
 * @since v.0.6.6
 */
RESTINIO_NODISCARD
inline auto
slash()
{
	return symbol( '/' );
}

/*!
 * @brief A factory that creates a string-producer that extracts a
 * sequence on symbols until the separator will be found.
 *
 * Usage example:
 * @code
 * namespace epr = restinio::router::easy_parser_router;
 *
 * struct film_by_athor_and_title { std::string author, title };
 * router->add_handler(http_method_get(),
 * 	// Route for '/film/:author/:title'
 * 	produce<film_by_athor_and_title>(
 * 		epr::exact("/film/"),
 * 		epr::path_fragment_p() >> &film_by_athor_and_title::author,
 * 		epr::slash(),
 * 		epr::path_fragment_p() >> &film_by_athor_and_title::title
 * 	),
 * 	[](const restinio::request_handle_t & req,
 * 		const film_by_athor_and_title & params) {...});
 * @endcode
 *
 * By default the separator is '/', by it can be changed by @a separator
 * argument:
 * @code
 * namespace epr = restinio::router::easy_parser_router;
 *
 * struct user_group { std::string user, group };
 * router->add_handler(http_method_get(),
 * 	// Route for '/user-group'
 * 	produce<user_group>(
 * 		epr::slash(),
 * 		epr::path_fragment_p('-') >> &user_group::user,
 * 		epr::path_fragment_p() >> &user_group::group
 * 	),
 * 	[](const restinio::request_handle_t & req,
 * 		const user_group & params) {...});
 * @endcode
 *
 * @since v.0.6.6
 */
RESTINIO_NODISCARD
inline auto
path_fragment_p( char separator = '/' )
{
	return produce< std::string >(
			repeat( 1, N,
					any_symbol_if_not_p( separator ) >> to_container() ) );
}

/*!
 * @brief A factory for unescape_transformer.
 *
 * The unescape_transformer performs unescaping of percent-encoded
 * string.
 *
 * Usage example:
 * @code
 * namespace epr = restinio::router::easy_parser_router;
 *
 * struct film_by_athor_and_title { std::string author, title };
 * router->add_handler(http_method_get(),
 * 	// Route for '/film/:author/:title'
 * 	produce<film_by_athor_and_title>(
 * 		epr::exact("/film/"),
 * 		epr::path_fragment_p() >> epr::unescape() >> &film_by_athor_and_title::author,
 * 		epr::slash(),
 * 		epr::path_fragment_p() >> epr::unescape() >> &film_by_athor_and_title::title
 * 	),
 * 	[](const restinio::request_handle_t & req,
 * 		const film_by_athor_and_title & params) {...});
 * @endcode
 *
 * @note
 * This function can be parametrized by Unescape_Traits type:
 * @code
 * namespace epr = restinio::router::easy_parser_router;
 *
 * struct film_by_athor_and_title { std::string author, title };
 * router->add_handler(http_method_get(),
 * 	// Route for '/film/:author/:title'
 * 	produce<film_by_athor_and_title>(
 * 		epr::exact("/film/"),
 * 		epr::path_fragment_p()
 * 			>> epr::unescape<restinio::utils::javascript_compatible_unescape_traits>()
 * 			>> &film_by_athor_and_title::author,
 * 		epr::slash(),
 * 		epr::path_fragment_p()
 * 			>> epr::unescape<restinio::utils::javascript_compatible_unescape_traits>()
 * 			>> &film_by_athor_and_title::title
 * 	),
 * 	[](const restinio::request_handle_t & req,
 * 		const film_by_athor_and_title & params) {...});
 * @endcode
 *
 * @since v.0.6.6
 */
template< typename Unescape_Traits =
		restinio::utils::restinio_default_unescape_traits >
RESTINIO_NODISCARD
auto
unescape()
{
	return impl::unescape_transformer_t< Unescape_Traits >{};
}

} /* namespace easy_parser_router */

//
// easy_parser_router_t
//
//FIXME: document this!
class easy_parser_router_t
{
public:
	easy_parser_router_t() = default;

	easy_parser_router_t( const easy_parser_router_t & ) = delete;
	easy_parser_router_t &
	operator=( const easy_parser_router_t & ) = delete;

	easy_parser_router_t( easy_parser_router_t && ) = default;
	easy_parser_router_t &
	operator=( easy_parser_router_t && ) = default;

	RESTINIO_NODISCARD
	request_handling_status_t
	operator()( request_handle_t req ) const
	{
		using namespace easy_parser_router::impl;

		// Take care of an optional trailing slash.
		string_view_t path_to_inspect{ req->header().path() };
		if( path_to_inspect.size() > 1u && '/' == path_to_inspect.back() )
			path_to_inspect.remove_suffix( 1u );

		target_path_holder_t target_path{ path_to_inspect };
		for( const auto & entry : m_entries )
		{
			const auto r = entry->try_handle( req, target_path );
			if( r )
			{
				return *r;
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

	template< typename Producer, typename Handler >
	void
	add_handler(
		http_method_id_t method,
		Producer && producer,
		Handler && handler )
	{
		using namespace easy_parser_router::impl;

		using producer_type = std::decay_t< Producer >;
		using handler_type = std::decay_t< Handler >;

		using actual_entry_type = actual_router_entry_t<
				producer_type, handler_type >;

		auto entry = std::make_unique< actual_entry_type >(
				method,
				std::forward<Producer>(producer),
				std::forward<Handler>(handler) );

		m_entries.push_back( std::move(entry) );
	}

	//! Set handler for requests that don't match any route.
	void
	non_matched_request_handler( non_matched_request_handler_t nmrh )
	{
		m_non_matched_request_handler= std::move( nmrh );
	}

private:
	using entries_container_t =
			std::vector< easy_parser_router::impl::router_entry_unique_ptr_t >;

	entries_container_t m_entries;

	//! Handler that is called for requests that don't match any route.
	non_matched_request_handler_t m_non_matched_request_handler;
};

} /* namespace router */

} /* namespace restinio */

