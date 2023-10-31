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

//FIXME: document this!
template<
	std::size_t Size,
	typename Extra_Data_Factory = no_extra_data_factory_t >
class fixed_size_chain_t
{
	using unique_controller_t = unique_async_handling_controller_t< Extra_Data_Factory >;

	using handler_holder_t = generic_async_request_handler_t< Extra_Data_Factory >;

	using handlers_array_t = std::array< handler_holder_t, Size >;

	using actual_request_handle_t =
			typename async_handling_controller_t< Extra_Data_Factory >::actual_request_handle_t;

	using actual_on_next_result_t =
			typename async_handling_controller_t< Extra_Data_Factory >::actual_on_next_result_t;

	class actual_controller_t final
		: public async_handling_controller_t< Extra_Data_Factory >
	{
		const actual_request_handle_t m_request;
		handlers_array_t m_handlers;
		std::size_t m_current{};

	public:
		explicit actual_controller_t(
			actual_request_handle_t request,
			const handlers_array_t & handlers )
			:	m_request{ request }
			,	m_handlers{ handlers }
		{}

		[[nodiscard]]
		const actual_request_handle_t &
		request_handle() const noexcept override { return m_request; }

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

	handlers_array_t m_handlers;

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

	//FIXME: constructor has to be defined here!

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

