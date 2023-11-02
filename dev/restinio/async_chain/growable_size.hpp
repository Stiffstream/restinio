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
//FIXME: document this!
template< typename Extra_Data_Factory = no_extra_data_factory_t >
class growable_size_chain_t
{
	// Helper class to allow the creation of growable_size_chain_t only
	// for the friends of growable_size_chain_t.
	struct creation_token_t {};

	using unique_controller_t = unique_async_handling_controller_t< Extra_Data_Factory >;

	using handler_holder_t = generic_async_request_handler_t< Extra_Data_Factory >;

	using handlers_vector_t = std::vector< handler_holder_t >;

	using actual_request_handle_t =
			typename async_handling_controller_t< Extra_Data_Factory >::actual_request_handle_t;

	using actual_on_next_result_t =
			typename async_handling_controller_t< Extra_Data_Factory >::actual_on_next_result_t;

	//FIXME: document this!
	class actual_controller_t final
		: public async_handling_controller_t< Extra_Data_Factory >
	{
		const actual_request_handle_t m_request;
		handlers_vector_t m_handlers;
		std::size_t m_current{};

	public:
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

	[[nodiscard]]
	request_handling_status_t
	operator()( const actual_request_handle_t & req ) const
	{
		unique_async_handling_controller_t< Extra_Data_Factory > controller =
				std::make_unique< actual_controller_t >(
						req,
						m_handlers );
		next( std::move(controller) );

		return request_accepted();
	}
};

} /* namespace restinio::async_chain */

