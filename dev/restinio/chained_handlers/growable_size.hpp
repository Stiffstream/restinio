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

namespace chained_handlers
{

//
// growable_size_chain_t
//
//FIXME: document this!
template< typename User_Data_Factory = no_user_data_factory_t >
class growable_size_chain_t
{
	struct creation_token_t {};

public:
	friend class builder_t;
	class builder_t
	{
	public:
		builder_t()
			:	m_chain{ new growable_size_chain_t( creation_token_t{} ) }
		{}

		RESTINIO_NODISCARD
		std::unique_ptr< growable_size_chain_t >
		release() noexcept
		{
			return { std::move(m_chain) };
		}

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
			incoming_request_handle_t< typename User_Data_Factory::data_t >;

	using handler_holder_t = std::function<
			request_handling_status_t(const actual_request_handle_t &)
	>;

	std::vector< handler_holder_t > m_handlers;

	//FIXME: document this!
	growable_size_chain_t( creation_token_t ) {}

public:
	//FIXME: document this!
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

} /* namespace chained_handlers */

} /* namespace restinio */

