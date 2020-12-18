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

namespace chained_handlers
{

//
// fixed_size_chain_t
//
//FIXME: document this!
template<
	std::size_t Size,
	typename User_Data_Factory = no_user_data_factory_t >
class fixed_size_chain_t
{
	using actual_request_handle_t =
			incoming_request_handle_t< typename User_Data_Factory::data_t >;

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
	//FIXME: document this!
	fixed_size_chain_t() = delete;

	template< typename... Handlers >
	fixed_size_chain_t( Handlers && ...handlers )
	{
		static_assert( Size == sizeof...(handlers),
				"Wrong number of parameters for the constructor of "
				"fixed_size_chain_t<Size>. Exact `Size` parameters expected" );

		store_to< 0u >( std::forward<Handlers>(handlers)... );
	}

	//FIXME: can this method be a const method?
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

