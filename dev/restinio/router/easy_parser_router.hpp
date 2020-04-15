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
#include <restinio/router/method_matcher.hpp>

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

namespace meta = restinio::utils::metaprogramming;
namespace ep = restinio::easy_parser;

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

//
// actual_router_entry_t
//
/*!
 * @brief An actual implementation of router_entry interface.
 *
 * @tparam Producer A type of producer that parses a route and produces
 * a value to be used as argument(s) for request handler.
 *
 * @tparam Handle A type of request handler.
 *
 * @since v.0.6.6
 */
template< typename Producer, typename Handler >
class actual_router_entry_t : public router_entry_t
{
	//! HTTP method to match.
	restinio::router::impl::buffered_matcher_holder_t m_method_matcher;

	//! Parser of a route and producer of argument(s) for request handler.
	Producer m_producer;

	//! Request handler to be used.
	Handler m_handler;

public:
	template<
		typename Method_Matcher,
		typename Producer_Arg,
		typename Handler_Arg >
	actual_router_entry_t(
		Method_Matcher && method_matcher,
		Producer_Arg && producer,
		Handler_Arg && handler )
		:	m_producer{ std::forward<Producer_Arg>(producer) }
		,	m_handler{ std::forward<Handler_Arg>(handler) }
	{
		assign( m_method_matcher, std::forward<Method_Matcher>(method_matcher) );
	}

	RESTINIO_NODISCARD
	expected_t< request_handling_status_t, no_match_t >
	try_handle(
		const request_handle_t & req,
		target_path_holder_t & target_path ) const override
	{
		if( m_method_matcher->match( req->header().method() ) )
		{
			auto parse_result = easy_parser::try_parse(
					target_path.view(),
					m_producer );
			if( parse_result )
			{
				return Producer::invoke_handler( req, m_handler, *parse_result );
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

//
// special_produce_tuple_item_clause_t
//
/*!
 * @brief A special case of produce-consume clause where the produced
 * value is stored into a tuple.
 *
 * @since v.0.6.6
 */
template< typename Producer, std::size_t Index >
class special_produce_tuple_item_clause_t
	:	public ep::impl::consume_value_clause_t<
				Producer,
				ep::impl::tuple_item_consumer_t<Index> >
{
	using consumer_t = ep::impl::tuple_item_consumer_t<Index>;

	using base_type_t = ep::impl::consume_value_clause_t<
			Producer,
			consumer_t >;

	// NOTE: this is just a workaround for VS2017.
	template< typename Producer_Arg >
	RESTINIO_NODISCARD
	static Producer
	make_producer( Producer_Arg && producer )
	{
		return { std::forward<Producer_Arg>(producer) };
	}

public:
	template< typename Producer_Arg >
	special_produce_tuple_item_clause_t( Producer_Arg && producer )
		:	base_type_t{
				make_producer( std::forward<Producer_Arg>(producer) ),
				consumer_t{} }
	{}
};

//
// special_exact_fixed_size_fragment_clause_t
//
/*!
 * @brief A special clause type for case when
 * exact_fixed_size_fragment_producer should be used without storing
 * its value.
 *
 * This type is an equivalent of `exact_p() >> skip()`, but it can be
 * used where a type is required.
 *
 * @since v.0.6.6
 */
template< std::size_t Size >
class special_exact_fixed_size_fragment_clause_t
	:	public ep::impl::consume_value_clause_t<
			ep::impl::exact_fixed_size_fragment_producer_t<Size>,
			ep::impl::any_value_skipper_t >
{
	using producer_t = ep::impl::exact_fixed_size_fragment_producer_t<Size>;
	using consumer_t = ep::impl::any_value_skipper_t;

	using base_type_t = ep::impl::consume_value_clause_t<
			producer_t,
			consumer_t >;

public:
	special_exact_fixed_size_fragment_clause_t(
		const char (&fragment)[Size] )
		:	base_type_t{ producer_t{ fragment }, consumer_t{} }
	{}
};

//
// special_exact_fragment_clause_t
//
/*!
 * @brief A special clause type for case when
 * exact_fragment_producer should be used without storing
 * its value.
 *
 * This type is an equivalent of `exact_p() >> skip()`, but it can be
 * used where a type is required.
 *
 * @since v.0.6.6
 */
class special_exact_fragment_clause_t
	:	public ep::impl::consume_value_clause_t<
			ep::impl::exact_fragment_producer_t,
			ep::impl::any_value_skipper_t >
{
	using producer_t = ep::impl::exact_fragment_producer_t;
	using consumer_t = ep::impl::any_value_skipper_t;

	using base_type_t = ep::impl::consume_value_clause_t<
			producer_t,
			consumer_t >;

public:
	special_exact_fragment_clause_t( std::string value )
		:	base_type_t{ producer_t{ std::move(value) }, consumer_t{} }
	{}

	special_exact_fragment_clause_t( string_view_t value )
		:	base_type_t{
				producer_t{ std::string{ value.data(), value.size() } },
				consumer_t{} }
	{}
};

namespace dsl_details
{

// Adds type H to type list R if Is_Producer is true.
template< typename H, typename R, bool Is_Producer >
struct add_type_if_necessary_impl;

template<
	typename H,
	template<class...> class To,
	typename... Results >
struct add_type_if_necessary_impl< H, To<Results...>, false >
{
	using type = To<Results...>;
};

template<
	typename H,
	template<class...> class To,
	typename... Results >
struct add_type_if_necessary_impl< H, To<Results...>, true >
{
	using type = To<Results..., typename H::result_type>;
};

// Adds type H to type list R if H is a producer.
template< typename H, typename R >
struct add_type_if_necessary
	: add_type_if_necessary_impl< H, R, ep::impl::is_producer_v<H> >
{};

// Produces a type-list of producers from type-list From.
template< typename From, typename To >
struct result_tuple_detector;

// Adds a type from Sources to Results only if this type is a producer.
template<
	template<class...> class From,
	typename... Sources,
	template<class...> class To,
	typename... Results >
struct result_tuple_detector< From<Sources...>, To<Results...> >
{
	using type = typename result_tuple_detector<
			meta::tail_of_t< Sources... >,
			typename add_type_if_necessary<
					meta::head_of_t< Sources... >,
					To< Results... > >::type
		>::type;
};

template<
	template<class...> class From,
	template<class...> class To,
	typename... Results >
struct result_tuple_detector< From<>, To<Results...> >
{
	using type = To<Results...>;
};

// Produces a type of std::tuple of producers from type-list Args_Type_List.
template< typename Args_Type_List >
struct detect_result_tuple
{
	using type = meta::rename_t<
			typename result_tuple_detector<
					Args_Type_List,
					meta::type_list<> >::type,
			std::tuple >;
};

template< typename Args_Type_List >
using detect_result_tuple_t = typename detect_result_tuple<Args_Type_List>::type;

// Detects an appropriate type of clause for T.
// If T is a producer then there will be a new clause that
// consumes a value T produces.
// In that case Current_Index will also be incremented.
//
// If T is not a producer then Current_Index will be kept.
//
// If T is a string literal, or std::string, or string_view
// then T will be replaced by a special clause.
template< typename T, bool Is_Producer, std::size_t Current_Index >
struct one_clause_type_processor
{
	using clause_type = T;
	static constexpr std::size_t next_index = Current_Index;
};

template< std::size_t Size, std::size_t Current_Index >
struct one_clause_type_processor<const char[Size], false, Current_Index>
{
	using clause_type = special_exact_fixed_size_fragment_clause_t<Size>;
	static constexpr std::size_t next_index = Current_Index;
};

template< std::size_t Current_Index >
struct one_clause_type_processor<std::string, false, Current_Index>
{
	using clause_type = special_exact_fragment_clause_t;
	static constexpr std::size_t next_index = Current_Index;
};

template< std::size_t Current_Index >
struct one_clause_type_processor<string_view_t, false, Current_Index>
{
	using clause_type = special_exact_fragment_clause_t;
	static constexpr std::size_t next_index = Current_Index;
};

template< typename T, std::size_t Current_Index >
struct one_clause_type_processor<T, true, Current_Index>
{
	using clause_type = special_produce_tuple_item_clause_t< T, Current_Index >;
	static constexpr std::size_t next_index = Current_Index + 1u;
};

// Takes a type-list of user-specified types From and produces a
// typelist of actual clauses types To.
//
// The Current_Index should 0 at the first invocation.
template< typename From, typename To, std::size_t Current_Index >
struct clauses_type_maker;

template<
	template<class...> class From,
	typename... Sources,
	template<class...> class To,
	typename... Results,
	std::size_t Current_Index >
struct clauses_type_maker< From<Sources...>, To<Results...>, Current_Index >
{
private:
	using head_type = meta::head_of_t< Sources... >;

	using one_clause_type = one_clause_type_processor<
			head_type,
			ep::impl::is_producer_v<head_type>,
			Current_Index >;

public:
	using type = typename clauses_type_maker<
			meta::tail_of_t< Sources... >,
			To< Results..., typename one_clause_type::clause_type >,
			one_clause_type::next_index >::type;
};

template<
	template<class...> class From,
	template<class...> class To,
	typename... Results,
	std::size_t Current_Index >
struct clauses_type_maker< From<>, To<Results...>, Current_Index >
{
	using type = To< Results... >;
};

// Takes a type-list of user-specified types Args_Type_List and produces a
// typelist of actual clauses types to be used for parsing.
template< typename Args_Type_List >
struct make_clauses_types
{
	using type = meta::rename_t<
			typename clauses_type_maker<
					Args_Type_List,
					meta::type_list<>,
					0u >::type,
			std::tuple >;
};

template< typename Args_Type_List >
using make_clauses_types_t = typename make_clauses_types<Args_Type_List>::type;

//
// special_decay
//
/*!
 * @brief A special analog of std::decay meta-function that is
 * handles array differently.
 *
 * The std::decay converts `char[]` into `char*` and that is not
 * appropriate because `const char[]` is handled by
 * exact_fixed_size_fragment_producer.
 *
 * The special_decay keeps the type of arrays and do not handles
 * function pointers (it's because function pointers is not used
 * by easy-parser).
 *
 * @since v.0.6.6
 */
template< typename T >
struct special_decay
{
private:
	using U = std::remove_reference_t<T>;

public:
	using type = typename std::conditional<
				std::is_array<U>::value,
				U,
				std::remove_cv_t<U>
			>::type;
};

} /* namespace dsl_details */

//
// dsl_processor
//
/*!
 * @brief The main meta-function for processing route DSL.
 *
 * It takes types of user-supplied clauses/produces and makes
 * two types:
 *
 * - result_tuple. This is a type of std::tuple for holding all values produced
 *   by producers from Args. This tuple can be an empty tuple.
 * - clauses_tuple. This a type of std::tuple with clauses for parsing of a
 *   user's route.
 *
 * @since v.0.6.6
 */
template< typename... Args >
struct dsl_processor
{
	static_assert( 0u != sizeof...(Args), "Args can't be an empty list" );

	using arg_types = meta::transform_t<
			dsl_details::special_decay, meta::type_list<Args...> >;

	using result_tuple = dsl_details::detect_result_tuple_t< arg_types >;

	using clauses_tuple = dsl_details::make_clauses_types_t< arg_types >;
};

//
// path_to_tuple_producer_t
//
/*!
 * @brief An implementation of a producer for path_to_tuple case.
 *
 * This implementation produces a tuple and provides a way to call a
 * request-handler with passing that tuple as a single argument.
 *
 * @since v.0.6.6
 */
template<
	typename Target_Type,
	typename Subitems_Tuple >
class path_to_tuple_producer_t
	:	public ep::impl::produce_t< Target_Type, Subitems_Tuple >
{
	using base_type_t = ep::impl::produce_t< Target_Type, Subitems_Tuple >;

public:
	using base_type_t::base_type_t;

	template< typename Handler >
	RESTINIO_NODISCARD
	static auto
	invoke_handler(
		const request_handle_t & req,
		Handler && handler,
		typename base_type_t::result_type & type )
	{
		return handler( req, type );
	}
};

namespace path_to_params_details
{

template<
	typename F,
	typename Tuple,
	std::size_t... Indexes >
decltype(auto)
call_with_tuple_impl(
	F && what,
	const request_handle_t & req,
	Tuple && params,
	std::index_sequence<Indexes...> )
{
	return std::forward<F>(what)(
			req,
			std::get<Indexes>(std::forward<Tuple>(params))... );
}

//
// call_with_tuple
//
/*!
 * @brief A helper function to call a request-handler with a tuple.
 *
 * This helper passes every item of a tuple as a separate parameter.
 *
 * @since v.0.6.6
 */
template< typename F, typename Tuple >
decltype(auto)
call_with_tuple(
	F && what,
	const request_handle_t & req,
	Tuple && params )
{
	return call_with_tuple_impl(
			std::forward<F>(what),
			req,
			std::forward<Tuple>(params),
			std::make_index_sequence<
					std::tuple_size< std::remove_reference_t<Tuple> >::value
			>{} );
}

} /* namespace path_to_params_details */

//
// path_to_params_producer_t
//
/*!
 * @brief An implementation of a producer for path_to_params case.
 *
 * This implementation produces a tuple and provides a way to call a
 * request-handler with passing that tuple as a set of separate
 * parameters.
 *
 * @since v.0.6.6
 */
template<
	typename Target_Type,
	typename Subitems_Tuple >
class path_to_params_producer_t
	:	public ep::impl::produce_t< Target_Type, Subitems_Tuple >
{
	using base_type_t = ep::impl::produce_t< Target_Type, Subitems_Tuple >;

public:
	using base_type_t::base_type_t;

	template< typename Handler >
	RESTINIO_NODISCARD
	static auto
	invoke_handler(
		const request_handle_t & req,
		Handler && handler,
		typename base_type_t::result_type & type )
	{
		return path_to_params_details::call_with_tuple( handler, req, type );
	}
};

} /* namespace impl */

using namespace restinio::easy_parser;

//
// path_to_tuple
// 
/*!
 * @brief Describe a route for a handler that accepts params from the
 * route in form of a tuple.
 *
 * Usage example:
 * @code
 * router->add_handler(http_method_get(),
 * 	path_to_tuple("/api/v1/users/", user_id_parser),
 * 	[](const auto & req, const auto & params) {
 *			auto uid = std::get<0>(params);
 *			...
 * 	});
 *
 * router->add_handler(http_method_get(),
 * 	path_to_tuple(
 * 		"/api/v1/books/", book_id_parser,
 * 		"/versions/", version_id_parser),
 * 	[](const auto & req, const auto & params) {
 * 		auto book_id = std::get<0>(params);
 * 		auto ver_id = std::get<1>(params);
 * 		...
 * 	});
 * @endcode
 *
 * Please note that a route can contain no params at all. In that case
 * an empty tuple will be passed as an argument to the request handler:
 * @code
 * router->add_handler(http_method_get(),
 * 	path_to_tuple("/api/v1/books"),
 * 	[](const auto & req, auto params) {
 * 		static_assert(
 * 			std::is_same<std::tuple<>, decltype(params)>::value,
 * 			"type std::tuple<> is expected");
 * 		...
 * 	});
 * @endcode
 *
 * @since v.0.6.6
 */
template< typename... Args >
RESTINIO_NODISCARD
auto
path_to_tuple( Args && ...args )
{
	using dsl_processor = impl::dsl_processor< Args... >;
	using result_tuple_type = typename dsl_processor::result_tuple;
	using subclauses_tuple_type = typename dsl_processor::clauses_tuple;

	using producer_type = impl::path_to_tuple_producer_t<
			result_tuple_type,
			subclauses_tuple_type >;

	return producer_type{
			subclauses_tuple_type{ std::forward<Args>(args)... }
	};
}

//
// path_to_params
//
/*!
 * @brief Describe a route for a handler that accepts params from the
 * route in form of a list of separate arguments.
 *
 * Usage example:
 * @code
 * router->add_handler(http_method_get(),
 * 	path_to_params("/api/v1/users/", user_id_parser),
 * 	[](const auto & req, const auto & uid) {
 *			...
 * 	});
 *
 * router->add_handler(http_method_get(),
 * 	path_to_params(
 * 		"/api/v1/books/", book_id_parser,
 * 		"/versions/", version_id_parser),
 * 	[](const auto & req, const auto & book_id, const auto & ver_id) {
 * 		...
 * 	});
 * @endcode
 *
 * Please note that a route can contain no params at all. In that case
 * the request handler will receive just one parameter: a requst_handle.
 * @code
 * router->add_handler(http_method_get(),
 * 	path_to_params("/api/v1/books"),
 * 	[](const auto & req) {
 * 		...
 * 	});
 * @endcode
 *
 * @since v.0.6.6
 */
template< typename... Args >
RESTINIO_NODISCARD
auto
path_to_params( Args && ...args )
{
	using dsl_processor = impl::dsl_processor< Args... >;
	using result_tuple_type = typename dsl_processor::result_tuple;
	using subclauses_tuple_type = typename dsl_processor::clauses_tuple;

	using producer_type = impl::path_to_params_producer_t<
			result_tuple_type,
			subclauses_tuple_type >;

	return producer_type{
			subclauses_tuple_type{ std::forward<Args>(args)... }
	};
}

/*!
 * @brief A factory that creates a string-producer that extracts a
 * sequence on symbols until the separator will be found.
 *
 * Usage example:
 * @code
 * namespace epr = restinio::router::easy_parser_router;
 *
 * router->add_handler(http_method_get(),
 * 	// Route for '/film/:author/:title'
 * 	path_to_params(
 * 		"/film/",
 * 		epr::path_fragment_p(),
 * 		"/",
 * 		epr::path_fragment_p()
 * 	),
 * 	[](const restinio::request_handle_t & req,
 * 		const std::string & author,
 * 		const std::string & title) {...});
 * @endcode
 *
 * By default the separator is '/', by it can be changed by @a separator
 * argument:
 * @code
 * namespace epr = restinio::router::easy_parser_router;
 *
 * router->add_handler(http_method_get(),
 * 	// Route for '/user-group'
 * 	path_to_params(
 * 		"/",
 * 		epr::path_fragment_p('-'),
 * 		epr::path_fragment_p()
 * 	),
 * 	[](const restinio::request_handle_t & req,
 * 		const std::string & user,
 * 		const std::string & group) {...});
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
 * router->add_handler(http_method_get(),
 * 	// Route for '/film/:author/:title'
 * 	path_to_params(
 * 		"/film/",
 * 		epr::path_fragment_p() >> epr::unescape(),
 * 		"/",
 * 		epr::path_fragment_p() >> epr::unescape()
 * 	),
 * 	[](const restinio::request_handle_t & req,
 * 		const std::string & author,
 * 		const std::string & title) {...});
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
 * 	path_to_params(
 * 		"/film/",
 * 		epr::path_fragment_p()
 * 			>> epr::unescape<restinio::utils::javascript_compatible_unescape_traits>(),
 * 		"/",
 * 		epr::path_fragment_p()
 * 			>> epr::unescape<restinio::utils::javascript_compatible_unescape_traits>()
 * 	),
 * 	[](const restinio::request_handle_t & req,
 * 		const std::string & author,
 * 		const std::string & title) {...});
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
/*!
 * @brief A request router that uses easy_parser for matching requests
 * with handlers.
 *
 * Usage example:
 * @code
 * using router_t = restinio::router::easy_parser_router_t;
 * namespace epr = restinio::router::easy_parser_router;
 *
 * auto make_router(...) {
 * 	auto router = std::make_unique<router_t>();
 * 	...
 * 	router->http_get(epr::path_to_params(...),
 * 		[](const auto & req, ...) {...});
 * 	router->http_post(epr::path_to_params(...),
 * 		[](const auto & req, ...) {...});
 * 	router->http_delete(epr::path_to_params(...),
 * 		[](const auto & req, ...) {...});
 *
 * 	router->add_handler(
 * 		restinio::http_method_lock(),
 * 		epr::path_to_params(...),
 * 		[](const auto & req, ...) {...});
 *
 * 	router->add_handler(
 * 		restinio::router::any_of_methods(
 * 			restinio::http_method_get(),
 * 			restinio::http_method_delete(),
 * 			restinio::http_method_post()),
 * 		epr::path_to_params(...),
 * 		[](const auto & req, ...) {...});
 *
 * 	router->add_handler(
 * 		restinio::router::none_of_methods(
 * 			restinio::http_method_get(),
 * 			restinio::http_method_delete(),
 * 			restinio::http_method_post()),
 * 		epr::path_to_params(...),
 * 		[](const auto & req, ...) {...});
 *
 * 	return router;
 * }
 * ...
 * struct traits_t : public restinio::default_traits_t {
 * 	using request_handler_t = router_t;
 * }
 * ...
 * restinio::run(
 * 	restinio::on_this_thread<traits_t>()
 * 		.request_handler(make_router)
 * 		...
 * );
 * @endcode
 *
 * @since v.0.6.6
 */
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

	template<
		typename Method_Matcher,
		typename Route_Producer,
		typename Handler >
	void
	add_handler(
		Method_Matcher && method_matcher,
		Route_Producer && route,
		Handler && handler )
	{
		using namespace easy_parser_router::impl;

		using producer_type = std::decay_t< Route_Producer >;
		using handler_type = std::decay_t< Handler >;

		using actual_entry_type = actual_router_entry_t<
				producer_type, handler_type >;

		auto entry = std::make_unique< actual_entry_type >(
				std::forward<Method_Matcher>(method_matcher),
				std::forward<Route_Producer>(route),
				std::forward<Handler>(handler) );

		m_entries.push_back( std::move(entry) );
	}

	//! Set handler for HTTP GET request.
	template< typename Route_Producer, typename Handler >
	void
	http_get(
		Route_Producer && route,
		Handler && handler )
	{
		this->add_handler(
				http_method_get(),
				std::forward<Route_Producer>(route),
				std::forward<Handler>(handler) );
	}

	//! Set handler for HTTP DELETE request.
	template< typename Route_Producer, typename Handler >
	void
	http_delete(
		Route_Producer && route,
		Handler && handler )
	{
		this->add_handler(
				http_method_delete(),
				std::forward<Route_Producer>(route),
				std::forward<Handler>(handler) );
	}

	//! Set handler for HTTP HEAD request.
	template< typename Route_Producer, typename Handler >
	void
	http_head(
		Route_Producer && route,
		Handler && handler )
	{
		this->add_handler(
				http_method_head(),
				std::forward<Route_Producer>(route),
				std::forward<Handler>(handler) );
	}

	//! Set handler for HTTP POST request.
	template< typename Route_Producer, typename Handler >
	void
	http_post(
		Route_Producer && route,
		Handler && handler )
	{
		this->add_handler(
				http_method_post(),
				std::forward<Route_Producer>(route),
				std::forward<Handler>(handler) );
	}

	//! Set handler for HTTP PUT request.
	template< typename Route_Producer, typename Handler >
	void
	http_put(
		Route_Producer && route,
		Handler && handler )
	{
		this->add_handler(
				http_method_put(),
				std::forward<Route_Producer>(route),
				std::forward<Handler>(handler) );
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

