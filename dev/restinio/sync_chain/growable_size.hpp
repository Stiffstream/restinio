/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to growable-size chain of request-headers.
 *
 * @since v.0.6.13
 */

#pragma once

#include <restinio/request_handler.hpp>

#include <vector>

namespace restinio
{

namespace sync_chain
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
 * Usage example for the case when there is no extra-data in a request object.
 * @code
 * struct my_traits : public restinio::default_traits_t {
 * 	using request_handler_t = restinio::sync_chain::growable_size_chain_t;
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
 * 	using request_handler_t = restinio::sync_chain::growable_size_chain_t<
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
 * 	auto & field_values = std::get<
 * 		std::optional<my_extra_data_factory::request_specific_fields_t>>(req->extra_data());
 * 	auto & user_info = std::get<
 * 		std::optional<my_extra_data_factory::user_info_t>>(req->extra_data());
 * 	... // Actual processing.
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
 * @since v.0.6.13
 */
template< typename Extra_Data_Factory = no_extra_data_factory_t >
class growable_size_chain_t
{
	// Helper class to allow the creation of growable_size_chain_t only
	// for the friends of growable_size_chain_t.
	struct creation_token_t {};

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
	 * @since v.0.6.13
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
		RESTINIO_NODISCARD
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
	using actual_request_handle_t =
			generic_request_handle_t< typename Extra_Data_Factory::data_t >;

	using handler_holder_t = std::function<
			request_handling_status_t(const actual_request_handle_t &)
	>;

	std::vector< handler_holder_t > m_handlers;

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

