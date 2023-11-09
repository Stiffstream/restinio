/*!
 * @file
 * @brief Common stuff for different types of async handlers chains.
 * @since v.0.7.0
 */

#pragma once

#include <restinio/request_handler.hpp>

namespace restinio::async_chain
{

/*!
 * @brief Type for return value of a scheduler in a chain.
 *
 * A scheduler should schedule the actual processing of a request and should
 * tell whether this scheduling was successful or not. If it was successful,
 * schedule_result_t::ok must be returned, otherwise the
 * schedule_result_t::failure must be returned.
 *
 * @since v.0.7.0
 */
enum class schedule_result_t
{
	//! The scheduling of the actual processing was successful.
	ok,
	//! The scheduling of the actual processing failed. Note, that
	//! there is no additional information about the failure.
	failure
};

/*!
 * @brief Helper function to be used if scheduling was successful.
 *
 * Usage example:
 * @code
 * restinio::async_chain::growable_size_chain_t<>::builder_t builder;
 * builder.add([this](auto controller) {
 *    ... // Actual scheduling.
 *    return restinio::async_chain::ok();
 * });
 * @endcode
 *
 * @since v.0.7.0
 */
[[nodiscard]]
inline constexpr schedule_result_t
ok() noexcept { return schedule_result_t::ok; }

/*!
 * @brief Helper function to be used if scheduling failed.
 *
 * Usage example:
 * @code
 * restinio::async_chain::growable_size_chain_t<>::builder_t builder;
 * builder.add([this](auto controller) {
 *    try {
 *       ... // Actual scheduling.
 *       return restinio::async_chain::ok(); // OK, no problems.
 *    }
 *    catch( ... ) {
 *       return restinio::async_chain::failure();
 *    }
 * });
 * @endcode
 *
 * @since v.0.7.0
 */
[[nodiscard]]
inline constexpr schedule_result_t
failure() noexcept { return schedule_result_t::failure; }

// Just a forward declaration.
template< typename Extra_Data_Factory = no_extra_data_factory_t >
class async_handling_controller_t;

/*!
 * @brief Short alias for unique_ptr to async_handling_controller.
 *
 * @since v.0.7.0
 */
template< typename Extra_Data_Factory = no_extra_data_factory_t >
using unique_async_handling_controller_t =
	std::unique_ptr< async_handling_controller_t< Extra_Data_Factory > >;

/*!
 * @brief Short alias for a type of a scheduler to be used in async chains.
 *
 * @since v.0.7.0
 */
template< typename Extra_Data_Factory = no_extra_data_factory_t >
using generic_async_request_scheduler_t =
	std::function<
		schedule_result_t(unique_async_handling_controller_t<Extra_Data_Factory>)
	>;

/*!
 * @brief Special type to be used as an indicator that there are no more
 * schedulers in an async chain.
 *
 * This type will be used in on_next_result_t variant.
 *
 * @since v.0.7.0
 */
struct no_more_schedulers_t {};

/*!
 * @brief Special type to be used as result of async_handling_controller's
 * on_next method.
 *
 * The async_handling_controller_t::on_next may return an actual scheduler to
 * be called or (if there are no more handlers left) a special no_more_handler
 * value. This is described by on_next_result_t variant type.
 *
 * @since v.0.7.0
 */
template< typename Extra_Data_Factory = no_extra_data_factory_t >
using on_next_result_t = std::variant<
		generic_async_request_scheduler_t< Extra_Data_Factory >,
		no_more_schedulers_t
	>;

// Just a forward declaration.
template< typename Extra_Data_Factory >
void
next( unique_async_handling_controller_t< Extra_Data_Factory > controller );

/*!
 * @brief Interface of a controller of an async chan.
 *
 * All actual controllers have to implement this interface.
 *
 * It's assumed that implementation of that interface won't be thread
 * safe. It means that methods like request_handle() and on_next() must
 * not be called in parallel.
 *
 * @tparam Extra_Data_Factory Type of factory for creation of extra data
 * objects for every incoming request. Should be no_extra_data_factory_t
 * if extra data for incoming requests is not needed.
 *
 * @since v.0.7.0
 */
template< typename Extra_Data_Factory /*= no_extra_data_factory_t*/ >
class async_handling_controller_t
{
	// The next() function should call on_next() method.
	template< typename Extra_Data_Factory_For_Next >
	friend void
	next( unique_async_handling_controller_t< Extra_Data_Factory_For_Next > controller );

public:
	//! Short alias for request_handle type.
	using actual_request_handle_t =
			generic_request_handle_t< typename Extra_Data_Factory::data_t >;

	//! Short alias for async_request_scheduler type.
	using actual_async_request_scheduler_t =
			generic_async_request_scheduler_t< typename Extra_Data_Factory::data_t >;

	//! Short alias for the result type of %on_next method.
	using actual_on_next_result_t =
			on_next_result_t< Extra_Data_Factory >;

	virtual ~async_handling_controller_t() = default;

	/*!
	 * @brief Get reference to the source request.
	 *
	 * Usage example:
	 * @code
	 * restinio::async_chain::schedule_result_t my_handler(
	 * 	restinio::async_chain::unique_async_handling_controller_t<> controller )
	 * {
	 * 	// Get access to the source request.
	 * 	const auto req = controller->request_handle();
	 * 	if( restinio::http_method_get() == req->header().method() )
	 * 	{
	 * 		...
	 * 	}
	 * 	return restinio::async_chain::ok();
	 * }
	 * @endcode
	 */
	[[nodiscard]]
	virtual const actual_request_handle_t &
	request_handle() const noexcept = 0;

private:
	/*!
	 * @brief Command to try find a next scheduler to be invoked.
	 *
	 * Implementation of async_handling_controller_t should switch to the
	 * next scheduler in the chain and return the scheduler to be called next.
	 * If there are no such schedulers, no_more_schedulers_t must be returned.
	 *
	 * @note
	 * This method is intended to be called by next() function.
	 */
	[[nodiscard]]
	virtual actual_on_next_result_t
	on_next() = 0;
};

namespace impl
{

/*!
 * @brief Helper to make a negative response with "Not Implemented" status.
 *
 * This helper will be used if there is no more schedulers to call, but
 * the request is still not handled.
 *
 * @tparam Request_Handle Type of request handle that holds the source request.
 *
 * @since v.0.7.0
 */
template< typename Request_Handle >
void
make_not_implemented_response( const Request_Handle & req )
{
	req->create_response( status_not_found() ).done();
}

/*!
 * @brief Helper to make a negative response with "Internal Server Error" status.
 *
 * This helper will be used if the current scheduler returns
 * schedule_result_t::failure.
 *
 * @tparam Request_Handle Type of request handle that holds the source request.
 *
 * @since v.0.7.0
 */
template< typename Request_Handle >
void
make_internal_server_error_response( const Request_Handle & req )
{
	req->create_response( status_internal_server_error() ).done();
}

/*!
 * @brief Helper type to be used as handler of variant values in std::visit.
 *
 * If there is the next async scheduler to be called it will be called.
 * If it returns schedule_result_t::failure, then negative response will
 * be generated and processing will be stopped.
 *
 * If no_more_schedulers_t is here, then negative response will be generated.
 *
 * @since v.0.7.0
 */
template< typename Extra_Data_Factory >
struct on_next_result_visitor_t
{
	unique_async_handling_controller_t< Extra_Data_Factory > & m_controller;

	void
	operator()(
		const generic_async_request_scheduler_t< Extra_Data_Factory > & handler ) const
	{
		// We have to store request_handle before further processing because
		// m_controller becomes empty after passing to the `handler`.
		const auto req = m_controller->request_handle();
		const auto schedule_result = handler( std::move(m_controller) );
		switch( schedule_result )
		{
		case schedule_result_t::ok:
			/* nothing to do */
			// It's assumed that handler will call next() when it'll be
			// appropriate.
		break;

		case schedule_result_t::failure:
			make_internal_server_error_response( req );
		break;
		}
	}

	void
	operator()(
		const no_more_schedulers_t & ) const
	{
		make_not_implemented_response( m_controller->request_handle() );
	}
};

} /* namespace impl */

/*!
 * @brief Command to try to switch to the next handler in an async chain.
 *
 * If an intermediate handler in an async chain doesn't complete processing
 * of the request it should call next() function to activate the next
 * handler in the chain.
 *
 * If there are no more handlers in the chain the processing of the request
 * will be finished just inside the next() call by generating negative
 * response.
 *
 * @note
 * The handler must not call next() if it generates the response for the
 * request:
 * @code
 * void actual_processor(restinio::async_chain::unique_async_handling_controller_t<> controller) {
 * 	const auto req = controller->request_handle();
 * 	if(request_can_be_handled(req)) {
 * 		processing_of_the_request(req);
 * 		req->create_response(...)
 * 			...
 * 			.done(); // Request processing completed.
 * 	}
 * 	else {
 * 		// This handler can't complete processing of the request.
 * 		// Have to switch to the next handler.
 * 		next(std::move(controller));
 * 	}
 *
 * 	// NOTE: it's not safe to use `controller` here!
 * }
 * @endcode
 *
 * @since v.0.7.0
 */
template< typename Extra_Data_Factory >
void
next( unique_async_handling_controller_t< Extra_Data_Factory > controller )
{
	std::visit(
			impl::on_next_result_visitor_t< Extra_Data_Factory >{ controller },
			controller->on_next() );
}

} /* namespace restinio::async_chain */

