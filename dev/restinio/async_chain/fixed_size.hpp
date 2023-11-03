/*!
 * @file
 * @brief Chain of async handlers of the fixed size.
 * @since v.0.7.0
 */

#pragma once

#include <restinio/async_chain/common.hpp>

#include <array>

namespace restinio::async_chain
{

/*!
 * @brief A holder of fixed-size chain of asynchronous handlers.
 *
 * @note
 * An instance of that type is intended to be filled with actual handlers
 * at the creation time. After that new handlers can't be added to the chain,
 * and old handlers can't be removed from the chain.
 *
 * Usage example for the case when there is no extra-data in a request object
 * (please note that this is simplified example without actual asynchronous code):
 * @code
 * struct my_traits : public restinio::default_traits_t {
 * 	using request_handler_t = restinio::async_chain::fixed_size_chain_t<3>;
 * };
 *
 * // The first handler in the chain.
 * restinio::async_chain::schedule_result_t headers_checker(
 * 	restinio::async_chain::unique_async_handling_controller_t<> controller )
 * {
 * 	... // Checks values of HTTP-fields and rejects invalid requests.
 *
 * 	// Activation of the next handler.
 * 	next( std::move(controller) );
 *
 * 	return restinio::async_chain::ok();
 * }
 *
 * // The second handler in the chain.
 * restinio::async_chain::schedule_result_t authentificator(
 * 	restinio::async_chain::unique_async_handling_controller_t<> controller )
 * {
 * 	... // Checks user's credentials and rejects requests from
 * 	    // non-authentificated users.
 *
 * 	// Activation of the next handler.
 * 	next( std::move(controller) );
 *
 * 	return restinio::async_chain::ok();
 * }
 *
 * // The last handler in the chain.
 * restinio::async_chain::schedule_result_t actual_handler(
 * 	restinio::async_chain::unique_async_handling_controller_t<> controller )
 * {
 * 	... // Actual processing.
 *
 * 	return restinio::async_chain::ok();
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
 * auto chain = std::make_unique<restinio::async_chain::fixed_size_chain_t<3>>(
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
 * a request object
 * (please note that this is simplified example without actual asynchronous code):
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
 * 	using request_handler_t = restinio::async_chain::fixed_size_chain_t<
 * 			3,
 * 			extra_data_factory>;
 * };
 *
 * using my_unique_controller_t =
 * 	restinio::async_chain::unique_async_handling_controller_t<my_extra_data_factory>;
 * using my_request_handle_t = my_unique_controller_t::actual_request_handle_t;
 *
 * // The first handler in the chain.
 * restinio::async_chain::schedule_result_t headers_checker(
 * 	my_unique_controller_t controller )
 * {
 * 	const my_request_handle_t req = acceptor->request_handle();
 * 	... // Checks values of HTTP-fields and rejects invalid requests.
 * 	...
 * 	next( std::move(controller) );
 * 	return restinio::async_chain::ok();
 * }
 *
 * // The second handler in the chain.
 * restinio::async_chain::schedule_result_t authentificator(
 * 	my_unique_controller_t controller )
 * {
 * 	const my_request_handle_t req = acceptor->request_handle();
 * 	... // Checks user's credentials and rejects requests from
 * 	    // non-authentificated users.
 * 	...
 * 	next( std::move(controller) );
 * 	return restinio::async_chain::ok();
 * }
 *
 * // The last handler in the chain.
 * restinio::async_chain::schedule_result_t actual_handler(
 * 	my_unique_controller_t controller )
 * {
 * 	const my_request_handle_t req = acceptor->request_handle();
 * 	auto & field_values = std::get<my_extra_data_factory::request_specific_fields_t>(req->extra_data());
 * 	auto & user_info = std::get<my_extra_data_factory::user_info_t>(req->extra_data());
 * 	... // Actual processing.
 * 	return restinio::async_chain::ok();
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
 * @since v.0.7.0
 */
template<
	std::size_t Size,
	typename Extra_Data_Factory = no_extra_data_factory_t >
class fixed_size_chain_t
{
	//! Short alias for unique controller type.
	using unique_controller_t = unique_async_handling_controller_t< Extra_Data_Factory >;

	//! Short alias for a request handler.
	using handler_holder_t = generic_async_request_handler_t< Extra_Data_Factory >;

	//! Short alias for arrya of request handlers.
	using handlers_array_t = std::array< handler_holder_t, Size >;

	//! Short alias to a smart pointer to the source request.
	using actual_request_handle_t =
			typename async_handling_controller_t< Extra_Data_Factory >::actual_request_handle_t;

	//! Short alias for the result of controller's on_next method.
	using actual_on_next_result_t =
			typename async_handling_controller_t< Extra_Data_Factory >::actual_on_next_result_t;

	/*!
	 * @brief Actual implementation of the controller interface.
	 *
	 * @note
	 * Object of this type holds a copy of the source array of handlers.
	 */
	class actual_controller_t final
		: public async_handling_controller_t< Extra_Data_Factory >
	{
		//! The source request.
		const actual_request_handle_t m_request;
		//! Request handlers.
		handlers_array_t m_handlers;
		//! Index of the current handler to be used.
		/*!
		 * @note
		 * May be equal to or greater than m_handlers.size() in the case
		 * when all handlers are already processed.
		 */
		std::size_t m_current{};

	public:
		//! Initializing constructor.
		explicit actual_controller_t(
			actual_request_handle_t request,
			const handlers_array_t & handlers )
			:	m_request{ request }
			,	m_handlers{ handlers }
		{}

		[[nodiscard]]
		const actual_request_handle_t &
		request_handle() const noexcept override { return m_request; }

	private:
		[[nodiscard]]
		actual_on_next_result_t
		on_next() override
		{
			const auto index_to_use = m_current;
			++m_current;

			if( index_to_use >= m_handlers.size() )
			{
				return { no_more_handlers_t{} };
			}
			else
			{
				return { m_handlers[ index_to_use ] };
			}
		}
	};

	//! The array of request handlers.
	/*!
	 * @note
	 * It's initialized in the constructor and then never changed.
	 */
	handlers_array_t m_handlers;

	//! Helper method to initialize the array of request handlers.
	template<
		typename Head,
		typename... Tail >
	void
	store_to( std::size_t index, Head && head, Tail && ...tail )
	{
		m_handlers[ index ] =
			[handler = std::move(head)]
			( unique_controller_t controller ) -> schedule_result_t
			{
				return handler( std::move(controller) );
			};

		if constexpr( 0u != sizeof...(tail) )
			store_to( index + 1u, std::forward<Tail>(tail)... );
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

		store_to( 0u, std::forward<Handlers>(handlers)... );
	}

	/*!
	 * Initiates execution of the first request handler in the chain.
	 *
	 * @note
	 * Always returns request_handling_status_t::accepted.
	 */
	[[nodiscard]]
	request_handling_status_t
	operator()( const actual_request_handle_t & req ) const
	{
		unique_controller_t controller =
				std::make_unique< actual_controller_t >(
						req,
						m_handlers );
		next( std::move(controller) );

		return request_accepted();
	}
};

} /* namespace restinio::async_chain */

