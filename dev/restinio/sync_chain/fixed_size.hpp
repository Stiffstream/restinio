/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to fixed-size chain of request-headers.
 *
 * @since v.0.6.13
 */

#pragma once

#include <restinio/request_handler.hpp>

#include <array>

namespace restinio
{

namespace sync_chain
{

//
// fixed_size_chain_t
//
/*!
 * @brief A holder of fixed-size chain of synchronous handlers.
 *
 * @note
 * An instance of that type is intended to be filled with actual handlers
 * at the creation time. After that new handlers can't be added to the chain,
 * and old handlers can't be removed from the chain.
 *
 * Usage example for the case when there is no extra-data in a request object.
 * @code
 * struct my_traits : public restinio::default_traits_t {
 * 	using request_handler_t = restinio::sync_chain::fixed_size_chain_t<3>;
 * };
 *
 * // The first handler in the chain.
 * restinio::request_handling_status_t headers_checker(
 * 	const restinio::request_handle_t & req )
 * {
 * 	... // Checks values of HTTP-fields and rejects invalid requests.
 * }
 *
 * // The second handler in the chain.
 * restinio::request_handling_status_t authentificator(
 * 	const restinio::request_handle_t & req )
 * {
 * 	... // Checks user's credentials and rejects requests from
 * 	    // non-authentificated users.
 * }
 *
 * // The last handler in the chain.
 * restinio::request_handling_status_t actual_handler(
 * 	const restinio::request_handle_t & req )
 * {
 * 	... // Actual processing.
 * }
 *
 * restinio::run(
 * 	on_thread_pool<my_traits>(16)
 * 		.address(...)
 * 		.port(...)
 * 		.request_handler(
 * 			// Just enumerate all handlers.
 * 			headers_checker,
 * 			authentificator,
 * 			actual_handler )
 * );
 * @endcode
 *
 * An instance of `fixed_size_chain_t` can also be created manually and
 * passed to server's settings by `unique_ptr`:
 * @code
 * auto chain = std::make_unique<restinio::fixed_size_chain_t<3>>(
 * 	headers_checker, authentificator, actual_handler);
 * ...
 * restinio::run(
 * 	on_thread_pool<my_traits>(16)
 * 		.address(...)
 * 		.port(...)
 * 		.request_handler(std::move(chain))
 * );
 * @endcode
 *
 * Usage example for the case when some extra-data is incorporated into
 * a request object.
 * @code
 * struct my_extra_data_factory {
 * 	// A data formed by checker of HTTP-fields.
 * 	struct request_specific_fields_t {...};
 *
 * 	// A data formed by user-authentificator.
 * 	struct user_info_t {...};
 *
 * 	// A data to be incorporated into a request object.
 * 	using data_t = std::tuple<request_specific_fields_t, user_info_t>;
 *
 * 	void make_within(restinio::extra_data_buffer_t<data_t> buf) {
 * 		new(buf.get()) data_t{};
 * 	}
 * };
 *
 * struct my_traits : public restinio::default_traits_t {
 * 	using extra_data_factory_t = my_extra_data_factory;
 * 	using request_handler_t = restinio::sync_chain::fixed_size_chain_t<
 * 			3,
 * 			extra_data_factory>;
 * };
 *
 * using my_request_handle_t =
 * 	restinio::generic_request_handle_t<my_extra_data_factory::data_t>;
 *
 * // The first handler in the chain.
 * restinio::request_handling_status_t headers_checker(
 * 	const my_request_handle_t & req )
 * {
 * 	... // Checks values of HTTP-fields and rejects invalid requests.
 * }
 *
 * // The second handler in the chain.
 * restinio::request_handling_status_t authentificator(
 * 	const my_request_handle_t & req )
 * {
 * 	... // Checks user's credentials and rejects requests from
 * 	    // non-authentificated users.
 * }
 *
 * // The last handler in the chain.
 * restinio::request_handling_status_t actual_handler(
 * 	const my_request_handle_t & req )
 * {
 * 	auto & field_values = std::get<my_extra_data_factory::request_specific_fields_t>(req->extra_data());
 * 	auto & user_info = std::get<my_extra_data_factory::user_info_t>(req->extra_data());
 * 	... // Actual processing.
 * }
 *
 * restinio::run(
 * 	on_thread_pool<my_traits>(16)
 * 		.address(...)
 * 		.port(...)
 * 		.request_handler(
 * 			// Just enumerate all handlers.
 * 			headers_checker,
 * 			authentificator,
 * 			actual_handler )
 * );
 * @endcode
 *
 * @tparam Size The exact number of handlers in the chain.
 *
 * @tparam Extra_Data_Factory The type of extra-data-factory specified in
 * the server's traits.
 *
 * @since v.0.6.13
 */
template<
	std::size_t Size,
	typename Extra_Data_Factory = no_extra_data_factory_t >
class fixed_size_chain_t
{
	using actual_request_handle_t =
			generic_request_handle_t< typename Extra_Data_Factory::data_t >;

	using handler_holder_t = std::function<
			request_handling_status_t(const actual_request_handle_t &)
	>;

	std::array< handler_holder_t, Size > m_handlers;

	template< std::size_t >
	void
	store_to() noexcept {}

	template<
		std::size_t Index,
		typename Head,
		typename... Tail >
	void
	store_to( Head && head, Tail && ...tail )
	{
		m_handlers[ Index ] =
			[handler = std::move(head)]
			( const actual_request_handle_t & req ) -> request_handling_status_t
			{
				return handler( req );
			};

		store_to< Index + 1u >( std::forward<Tail>(tail)... );
	}

public:
	/*!
	 * @attention
	 * The default constructor is disabled. It's because a chain should
	 * be initialized by handlers at the creation time. Because of that
	 * fixed_size_chain_t isn't a DefaultConstructible type.
	 */
	fixed_size_chain_t() = delete;

	/*!
	 * @brief Initializing constructor.
	 *
	 * @note
	 * The number of parameters should match the value of @a Size
	 * template parameter.
	 */
	template< typename... Handlers >
	fixed_size_chain_t( Handlers && ...handlers )
	{
		static_assert( Size == sizeof...(handlers),
				"Wrong number of parameters for the constructor of "
				"fixed_size_chain_t<Size>. Exact `Size` parameters expected" );

		store_to< 0u >( std::forward<Handlers>(handlers)... );
	}

	RESTINIO_NODISCARD
	request_handling_status_t
	operator()( const actual_request_handle_t & req ) const
	{
		for( auto & h : m_handlers )
		{
			const request_handling_status_t result = h( req );

			switch( result )
			{
				case request_handling_status_t::accepted:
				case request_handling_status_t::rejected:
					// There is no need to try next handler.
					return result;

				case request_handling_status_t::not_handled:
					// Nothing to do. The next handler should be tried.
					break;
			}
		}

		return request_not_handled();
	}
};

} /* namespace sync_chain */

} /* namespace restinio */

