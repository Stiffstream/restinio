/*!
 * @file
 * @brief Common stuff for different types of async handlers chains.
 * @since v.0.7.0
 */

#pragma once

#include <restinio/request_handler.hpp>

namespace restinio::async_chain
{

//FIXME: document this!
enum class schedule_result_t
{
	ok,
	failure
};

//FIXME: document this!
[[nodiscard]]
inline constexpr schedule_result_t
ok() noexcept { return schedule_result_t::ok; }

//FIXME: document this!
[[nodiscard]]
inline constexpr schedule_result_t
failure() noexcept { return schedule_result_t::failure; }

//FIXME: document this!
template< typename Extra_Data_Factory = no_extra_data_factory_t >
class async_handling_controller_t;

//FIXME: document this!
//FIXME: should Extra_Data_Factory be equal to no_extra_data_factory_t by default?
template< typename Extra_Data_Factory >
using unique_async_handling_controller_t =
	std::unique_ptr< async_handling_controller_t< Extra_Data_Factory > >;

//FIXME: document this!
//FIXME: should Extra_Data_Factory be equal to no_extra_data_factory_t by default?
template< typename Extra_Data_Factory >
//FIXME: should this type be named async_request_handler_t?
using generic_async_request_handler_t =
	std::function<
		schedule_result_t(unique_async_handling_controller_t<Extra_Data_Factory>)
	>;

//FIXME: document this!
struct no_more_handlers_t {};

//FIXME: document this!
template< typename Extra_Data_Factory >
using on_next_result_t = std::variant<
		generic_async_request_handler_t< Extra_Data_Factory >,
		no_more_handlers_t
	>;

//FIXME: document this!
template< typename Extra_Data_Factory /*= no_extra_data_factory_t*/ >
class async_handling_controller_t
{
public:
	using actual_request_handle_t =
			generic_request_handle_t< typename Extra_Data_Factory::data_t >;

	using actual_async_request_handler_t =
			generic_async_request_handler_t< typename Extra_Data_Factory::data_t >;

	using actual_on_next_result_t =
			on_next_result_t< Extra_Data_Factory >;

	virtual ~async_handling_controller_t() = default;

	//FIXME: const?
	[[nodiscard]]
	virtual const actual_request_handle_t &
	request_handle() const noexcept = 0;

	[[nodiscard]]
	virtual actual_on_next_result_t
	on_next() = 0;
};

namespace impl
{

//FIXME: document this!
template< typename Request_Handle >
void
make_not_implemented_response( const Request_Handle & req )
{
	req->create_response( status_not_found() ).done();
}

//FIXME: document this!
template< typename Request_Handle >
void
make_internal_server_error_response( const Request_Handle & req )
{
	req->create_response( status_internal_server_error() ).done();
}

//FIXME: document this!
template< typename Extra_Data_Factory >
struct on_next_result_visitor_t
{
	unique_async_handling_controller_t< Extra_Data_Factory > & m_controller;

	void
	operator()(
		const generic_async_request_handler_t< Extra_Data_Factory > & handler ) const
	{
		//FIXME: document this!
		const auto req = m_controller->request_handle();
		const auto schedule_result = handler( std::move(m_controller) );
		switch( schedule_result )
		{
		case schedule_result_t::ok: /* nothing to do */ break;

		case schedule_result_t::failure:
			make_internal_server_error_response( req );
		break;
		}
	}

	void
	operator()(
		const no_more_handlers_t & ) const
	{
		make_not_implemented_response( m_controller->request_handle() );
	}
};

} /* namespace impl */

//FIXME: document this!
template< typename Extra_Data_Factory >
void
next( unique_async_handling_controller_t< Extra_Data_Factory > controller )
{
	std::visit(
			impl::on_next_result_visitor_t< Extra_Data_Factory >{ controller },
			controller->on_next() );
}

} /* namespace restinio::async_chain */

