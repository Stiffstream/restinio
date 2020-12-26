/*
	restinio
*/

/*!
	HTTP server traits.
*/

#pragma once

#include <restinio/request_handler.hpp>
#include <restinio/asio_timer_manager.hpp>
#include <restinio/null_logger.hpp>
#include <restinio/connection_state_listener.hpp>
#include <restinio/ip_blocker.hpp>
#include <restinio/default_strands.hpp>
#include <restinio/connection_count_limiter.hpp>

#include <restinio/utils/metaprogramming.hpp>

namespace restinio
{

namespace details
{

namespace valid_request_handler_type_check
{

template< typename, typename, typename = restinio::utils::metaprogramming::void_t<> >
struct valid_handler_type : public std::false_type {};

template< typename Handler, typename Extra_Data_Factory >
struct valid_handler_type<
		Handler,
		Extra_Data_Factory,
		restinio::utils::metaprogramming::void_t<
			std::enable_if_t<
				std::is_same<
					request_handling_status_t,
					decltype(std::declval<Handler>()(
							std::declval<
									generic_request_handle_t<
											typename Extra_Data_Factory::data_t
									>
							>()))
				>::value,
				bool
			>
		>
	> : public std::true_type
{};

} /* namespace valid_request_handler_type_check */

//
// autodetect_request_handler_type
//
/*!
 * @brief A special type to be used as indicator that the type of
 * a request handler should be automatically detected.
 *
 * In versions prior to 0.6.13 request-handlers in RESTinio have the
 * same format. But since v.0.6.13 the actual type of request-handler
 * is dependent on extra-data-factory type. It means that if a user
 * defines own extra-data-factory for server's traits then user also
 * has to define own request-handler type:
 *
 * @code
 * struct my_extra_data_factory {...};
 *
 * struct my_traits : public restinio::default_traits_t {
 * 	using extra_data_factory_t = my_extra_data_factory;
 * 	using request_handler_t = std::function<
 * 		restinio::request_handling_status_t(
 * 			restinio::generic_request_handle_t<
 * 				my_extra_data_factory::data_t>)
 * 	>;
 * };
 * @endcode
 *
 * But this is a boring and error-prone task. So RESTinio allows a user
 * to specify only `extra_data_factory_t` type and skip the definition
 * of `request_handler_t`. That definition will be performed automatically.
 *
 * The actual detection of request-handler type is performed by
 * using specialization of actual_request_handler_type_detector for
 * autodetect_request_handler_type.
 *
 * @since v.0.6.13
 */
struct autodetect_request_handler_type {};

//
// actual_request_handler_type_detector
//
/*!
 * @brief A metafunction for the detection of type of a request-handler.
 *
 * @since v.0.6.13
 */
template<
	typename Request_Handler,
	typename Extra_Data_Factory >
struct actual_request_handler_type_detector
{
	static_assert(
			valid_request_handler_type_check::valid_handler_type<
					Request_Handler,
					Extra_Data_Factory
				>::value,
			"Request_Handler should be invocable with "
			"generic_request_handle_t<Extra_Data_Factory::data_t>" );

	using request_handler_t = Request_Handler;
};

/*!
 * @brief Special version of metafunction %actual_request_handler_type_detector
 * for the case of %autodetect_request_handler_type.
 *
 * @since v.0.6.13
 */
template< typename Extra_Data_Factory >
struct actual_request_handler_type_detector<
		autodetect_request_handler_type,
		Extra_Data_Factory >
{
	using request_handler_t = std::function<
			request_handling_status_t(
					generic_request_handle_t<typename Extra_Data_Factory::data_t>) >;
};

} /* namespace details */

//
// traits_t
//

template <
		typename Timer_Manager,
		typename Logger,
		typename Request_Handler = details::autodetect_request_handler_type,
		typename Strand = default_strand_t,
		typename Socket = asio_ns::ip::tcp::socket >
struct traits_t
{
	/*!
	 * @brief A type for HTTP methods mapping.
	 *
	 * If RESTinio is used with vanila version of http_parser then
	 * the default value of http_methods_mapper_t is enough. But if a user
	 * uses modified version of http_parser with support of additional,
	 * not-standard HTTP methods then the user should provide its
	 * http_methods_mapper. For example:
	 * \code
	 * // Definition for non-standard HTTP methods.
	 * // Note: values of HTTP_ENCODE and HTTP_DECODE are from modified
	 * // http_parser version.
	 * constexpr const restinio::http_method_id_t http_encode(HTTP_ENCODE, "ENCODE");
	 * constexpr const restinio::http_method_id_t http_decode(HTTP_DECODE, "DECODE");
	 *
	 * // Definition of non-standard http_method_mapper.
	 * struct my_http_method_mapper {
	 * 	inline constexpr restinio::http_method_id_t
	 * 	from_nodejs(int value) noexcept {
	 * 		switch(value) {
	 * 			case HTTP_ENCODE: return http_encode;
	 * 			case HTTP_DECODE: return http_decode;
	 * 			default: return restinio::default_http_methods_t::from_nodejs(value);
	 * 		}
	 * 	}
	 * };
	 * ...
	 * // Make a custom traits with non-standard http_method_mapper.
	 * struct my_server_traits : public restinio::default_traits_t {
	 * 	using http_methods_mapper_t = my_http_method_mapper;
	 * };
	 * \endcode
	 *
	 * @since v.0.5.0
	 */
	using http_methods_mapper_t = default_http_methods_t;

	/*!
	 * @brief A type for connection state listener.
	 *
	 * By default RESTinio doesn't inform about changes with connection state.
	 * But if a user specify its type of connection state listener then
	 * RESTinio will call this listener object when the state of connection
	 * changes.
	 *
	 * An example:
	 * @code
	 * // Definition of user's state listener.
	 * class my_state_listener {
	 * 	...
	 * public:
	 * 	...
	 * 	void state_changed(const restinio::connection_state::notice_t & notice) noexcept {
	 * 		... // some reaction to state change.
	 * 	}
	 * };
	 *
	 * // Definition of custom traits for HTTP server.
	 * struct my_server_traits : public restinio::default_traits_t {
	 * 	using connection_state_listener_t = my_state_listener;
	 * };
	 * @endcode
	 *
	 * @since v.0.5.1
	 */
	using connection_state_listener_t = connection_state::noop_listener_t;

	/*!
	 * @brief A type for IP-blocker.
	 *
	 * By default RESTinio's accepts all incoming connections.
	 * But since v.0.5.1 a user can specify IP-blocker object that
	 * will be called for every new connection. This IP-blocker can
	 * deny or allow a new connection.
	 *
	 * Type of that IP-blocker object is specified by typedef
	 * ip_blocker_t.
	 *
	 * An example:
	 * @code
	 * // Definition of user's IP-blocker.
	 * class my_ip_blocker {
	 * 	...
	 * public:
	 * 	...
	 * 	restinio::ip_blocker::inspection_result_t
	 * 	state_changed(const restinio::ip_blocker::incoming_info_t & info) noexcept {
	 * 		... // some checking for new connection.
	 * 	}
	 * };
	 *
	 * // Definition of custom traits for HTTP server.
	 * struct my_server_traits : public restinio::default_traits_t {
	 * 	using ip_blocker_t = my_ip_blocker;
	 * };
	 * @endcode
	 *
	 * @since v.0.5.1
	 */
	using ip_blocker_t = ip_blocker::noop_ip_blocker_t;

	using timer_manager_t = Timer_Manager;
	using logger_t = Logger;
	using request_handler_t = Request_Handler;
	using strand_t = Strand;
	using stream_socket_t = Socket;

	/*!
	 * @brief A flag that enables or disables the usage of connection count
	 * limiter.
	 *
	 * Since v.0.6.12 RESTinio allows to limit the number of active
	 * parallel connections to a server. But the usage of this limit
	 * should be turned on explicitly. For example:
	 * @code
	 * struct my_traits : public restinio::default_traits_t {
	 * 	static constexpr bool use_connection_count_limiter = true;
	 * };
	 * @endcode
	 * In that case there will be `max_parallel_connections` method
	 * in server_settings_t type. That method should be explicitly
	 * called to set a specific limit (by the default there is no
	 * limit at all):
	 * @code
	 * restinio::server_settings_t<my_traits> settings;
	 * settings.max_parallel_connections(1000u);
	 * @endcode
	 *
	 * @since v.0.6.12
	 */
	static constexpr bool use_connection_count_limiter = false;

	/*!
	 * @brief The type of extra-data-factory.
	 *
	 * By the default RESTinio doesn't hold any additional data for a
	 * request object. But if a user has to store some user-specific
	 * data inside a request object the user has to do the following
	 * steps:
	 *
	 * The first one is the definition of factory type that should
	 * look like:
	 * @code
	 * class some_extra_data_factory {
	 * public:
	 * 	using data_t = ...;
	 *
	 * 	void make_within(restinio::extra_data_buffer_t<data_t> buf);
	 * };
	 * @endcode
	 * Where the name `data_t` should define a name of type to incorporated
	 * into request object.
	 * And the method `make_within` should call a placement new for
	 * type `data_t` to construct a new object of `data_t` inside the
	 * buffer `buf`:
	 * @code
	 * void some_extra_data_factory::make_within(
	 * 		restinio::extra_data_buffer_t<data_t> buf) {
	 * 	new(buf.get()) data_t{...};
	 * }
	 * @endcode
	 *
	 * The second step is the definition of extra-data-factory in server's traits:
	 * @code
	 * struct my_traits : public restinio::default_traits_t {
	 * 	using extra_data_factory_t = some_extra_data_factory;
	 * };
	 * @endcode
	 *
	 * The third step is the creation of the extra-data-factory instance and
	 * passing it to server settings:
	 * @code
	 * restino::run(on_thread_pool<my_traits>(16)
	 * 	.port(...)
	 * 	.address(...)
	 * 	.extra_data_factory(std::make_shared<some_extra_data_factory>(..))
	 * 	.request_handler(...)
	 * 	...
	 * );
	 * @endcode
	 * Please note that the third step is not necessary if extra-data-factory
	 * type is DefaultConstructible. In that case an instance of
	 * extra-data-factory will be created automatically.
	 *
	 * Please note that if RESTinio's server is used with express-like or
	 * easy_parser-based routers then `request_handler_t` should be
	 * defined with the respect to extra-data-factory type:
	 * @code
	 * struct my_extra_data_factory {
	 * 	struct data_t {...};
	 *
	 * 	void make_within(restinio::extra_data_buffer_t<data_t> buf) {
	 * 		new(buf.get()) data_t{};
	 * 	}
	 * };
	 *
	 * using my_router = restinio::router::generic_express_router_t<
	 * 		restinio::router::std_regex_engine_t,
	 * 		my_extra_data_factory
	 * >;
	 *
	 * struct my_traits : public restinio::default_traits_t {
	 * 	using extra_data_factory_t = my_extra_data_factory;
	 * 	using request_handler_t = my_router;
	 * };
	 * @endcode
	 *
	 * @since v.0.6.13
	 */
	using extra_data_factory_t = no_extra_data_factory_t;
};

//
// request_handler_type_from_traits_t
//
/*!
 * @brief A metafunction for extraction a request-handler type from
 * server's traits.
 *
 * This metafunction is necessary because `request_handler_t` in
 * Traits can be an alias for details::autodetect_request_handler_type.
 * Because of that details::actual_request_handler_type_detector metafunction
 * is invoked for the detection of request-handler type.
 *
 * @since v.0.6.13
 */
template< typename Traits >
using request_handler_type_from_traits_t =
	typename details::actual_request_handler_type_detector<
			typename Traits::request_handler_t,
			typename Traits::extra_data_factory_t
		>::request_handler_t;

//
// generic_request_type_from_traits_t
//
/*!
 * @brief A metafunction for the detection of actual type of request-object
 * from server's traits.
 *
 * The actual type of request-object depends from extra-data-factory.
 * This metafunction detect the actual type with the respect to the
 * definition of `extra_data_factory_t` inside Traits.
 *
 * @since v.0.6.13
 */
template< typename Traits >
using generic_request_type_from_traits_t =
	generic_request_t< typename Traits::extra_data_factory_t::data_t >;

//
// single_thread_traits_t
//

template <
		typename Timer_Manager,
		typename Logger,
		typename Request_Handler = details::autodetect_request_handler_type >
using single_thread_traits_t =
	traits_t< Timer_Manager, Logger, Request_Handler, noop_strand_t >;

//
// default_traits_t
//

using default_traits_t = traits_t< asio_timer_manager_t, null_logger_t >;

/*!
 * \brief Default traits for single-threaded HTTP-server.
 *
 * Uses default timer manager. And null logger.
 *
 * Usage example:
 * \code
 * struct my_traits : public restinio::default_single_thread_traits_t {
 * 	using logger_t = my_special_single_threaded_logger_type;
 * };
 * \endcode
 *
 * \since
 * v.0.4.0
 */
using default_single_thread_traits_t = single_thread_traits_t<
		asio_timer_manager_t,
		null_logger_t >;

} /* namespace restinio */

