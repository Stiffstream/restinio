/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to growable-size chain of async request-headers.
 *
 * @since v.0.7.0
 */

#pragma once

#include <restinio/async_chain/common.hpp>

#include <vector>

namespace restinio::async_chain
{

//
// growable_size_chain_t
//
/*!
 * @brief A holder of variable-size chain of synchronous handlers.
 *
 * @note
 * Once a list of handler is filled and an instance of growable_size_chain_t
 * is created that instance can't be changed: a new handler can't be added, and
 * an old handler can be removed. The creation of growable_size_chain_t
 * instance is performed by the help of growable_size_chain_t::builder_t
 * class.
 *
 * Usage example for the case when there is no extra-data in a request object
 * (please note that this is simplified example without actual asynchronous code):
 * @code
 * struct my_traits : public restinio::default_traits_t {
 * 	using request_handler_t = restinio::async_chain::growable_size_chain_t;
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
 * // Building of a chain.
 * restinio::async_chain::growable_size_chain_t<>::builder_t builder;
 * if(config.force_headers_checking())
 * 	builder.add(headers_checker);
 * if(config.force_user_authentification())
 * 	builder.add(authentificator);
 * builder.add(actual_handler);
 *
 * restinio::run(
 * 	on_thread_pool<my_traits>(16)
 * 		.address(...)
 * 		.port(...)
 * 		.request_handler(builder.release())
 * );
 * @endcode
 *
 * Usage example for the case when some extra-data is incorporated into
 * a request object:
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
 * 	using data_t = std::tuple<
 * 			std::optional<request_specific_fields_t>,
 * 			std::optional<user_info_t>>;
 *
 * 	void make_within(restinio::extra_data_buffer_t<data_t> buf) {
 * 		new(buf.get()) data_t{};
 * 	}
 * };
 *
 * struct my_traits : public restinio::default_traits_t {
 * 	using extra_data_factory_t = my_extra_data_factory;
 * 	using request_handler_t = restinio::async_chain::growable_size_chain_t<
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
 * // Building of a chain.
 * restinio::sync_chain::growable_size_chain_t::builder_t builder;
 * if(config.force_headers_checking())
 * 	builder.add(headers_checker);
 * if(config.force_user_authentification())
 * 	builder.add(authentificator);
 * builder.add(actual_handler);
 *
 * restinio::run(
 * 	on_thread_pool<my_traits>(16)
 * 		.address(...)
 * 		.port(...)
 * 		.request_handler(builder.release())
 * );
 * @endcode
 *
 * @tparam Extra_Data_Factory The type of extra-data-factory specified in
 * the server's traits.
 *
 * @since v.0.7.0
 */
template< typename Extra_Data_Factory = no_extra_data_factory_t >
class growable_size_chain_t
{
	// Helper class to allow the creation of growable_size_chain_t only
	// for the friends of growable_size_chain_t.
	struct creation_token_t {};

	//! Short alias for a request handler.
	using unique_controller_t = unique_async_handling_controller_t< Extra_Data_Factory >;

	//! Short alias for a request handler.
	using handler_holder_t = generic_async_request_handler_t< Extra_Data_Factory >;

	//! Short alias for a vector of request handlers.
	using handlers_vector_t = std::vector< handler_holder_t >;

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
	 * Object of this type holds a copy of the source vector of handlers.
	 */
	class actual_controller_t final
		: public async_handling_controller_t< Extra_Data_Factory >
	{
		//! The source request.
		const actual_request_handle_t m_request;
		//! Request handlers.
		handlers_vector_t m_handlers;
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
			const handlers_vector_t & handlers )
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

public:
	friend class builder_t;

	/*!
	 * @brief A builder of an instance of growable_size_chain.
	 *
	 * Creates an empty instance of growable_size_chain_t in the constructor.
	 * That instance can be obtained by release() method.
	 *
	 * @note
	 * New handlers can be added to the chain by add() method until
	 * release() is called.
	 *
	 * @attention
	 * An instance of builder works like an unique_ptr: it will hold
	 * a nullptr after a call of release() method.
	 *
	 * @since v.0.7.0
	 */
	class builder_t
	{
	public:
		builder_t()
			:	m_chain{ new growable_size_chain_t( creation_token_t{} ) }
		{}

		/*!
		 * @brief Stop adding of new handlers and acquire the chain instance.
		 *
		 * @note
		 * The builder object should not be used after the calling of
		 * that method.
		 */
		[[nodiscard]]
		std::unique_ptr< growable_size_chain_t >
		release() noexcept
		{
			return { std::move(m_chain) };
		}

		/*!
		 * @brief Add a new handler to the chain.
		 */
		template< typename Handler >
		void
		add( Handler && handler )
		{
			if( !m_chain )
				throw exception_t{ "an attempt to add a handler to "
						"a growable-size-chain builder that already "
						"released"
				};
			m_chain->m_handlers.push_back(
					growable_size_chain_t::handler_holder_t{
							std::forward<Handler>(handler)
					} );
		}

	private:
		std::unique_ptr< growable_size_chain_t > m_chain;
	};

private:
	//! The vector of request handlers.
	handlers_vector_t m_handlers;

	/*!
	 * @brief The main constructor.
	 *
	 * It has that form because the default constructor is public and
	 * marked as deleted.
	 */
	growable_size_chain_t( creation_token_t ) {}

public:
	/*!
	 * @note
	 * The default constructor is disable because an instance of
	 * that class should be created and filled by using builder_t.
	 */
	growable_size_chain_t() = delete;

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

