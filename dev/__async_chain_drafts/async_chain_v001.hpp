enum class schedule_result_t
{
	ok,
	failure
};

template< typename Extra_Data_Factory = no_extra_data_factory_t >
class async_handling_controller_t
	: public std::enable_shared_from_this< async_handling_controller_t< Extra_Data_Factory > >
{
public:
	using actual_request_handle_t =
			generic_request_handle_t< typename Extra_Data_Factory::data_t >;

	virtual ~async_handling_controller_t() = default;

	//FIXME: const?
	[[nodiscard]]
	virtual const actual_request_handle_t &
	request_handle() const noexcept = 0;

	virtual void
	next() = 0;
};

template< typename Extra_Data_Factory >
using async_handling_controller_handle_t =
	std::shared_ptr< async_handling_controller_t< Extra_Data_Factory > >;

template< typename Extra_Data_Factory >
using generic_async_request_handler_t =
	std::function<
		schedule_result_t(async_handling_controller_handle_t<Extra_Data_Factory>)
	>;

namespace impl
{

template< typename Request_Handle >
void
make_not_implemented_response( const Request_Handle & req )
{
	req->create_response( status_not_found() ).done();
}

template< typename Request_Handle >
void
make_internal_server_error_response( const Request_Handle & req )
{
	req->create_response( status_internal_server_error() ).done();
}

} /* namespace impl */

template<
	std::size_t Size,
	typename Extra_Data_Factory = no_extra_data_factory_t >
class fixed_size_chain_t
{
	using handler_holder_t = generic_async_request_handler_t< Extra_Data_Factory >;

	using handlers_array_t = std::array< handler_holder_t, Size >;

	using actual_request_handle_t =
			typename async_handling_controller_t< Extra_Data_Factory >::actual_request_handle_t;

	handlers_array_t m_handlers;

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

		void
		next() override
		{
			const auto index_to_use = m_current;
			++m_current;

			if( index_to_use >= m_handlers.size() )
			{
				impl::make_not_implemented_response( m_request );
			}
			else
			{
				const schedule_result_t r =
						m_handlers[ index_to_use ]( shared_from_this() );
				switch( r )
				{
				case schedule_result_t::ok: /* nothing to do */ break;

				case schedule_result_t::failure:
					make_internal_server_error_response( m_request );
				break;
				}
			}
		}
	};

public:
	//FIXME: constructor has to be defined here!

	[[nodiscard]]
	request_handling_status_t
	operator()( const actual_request_handle_t & req ) const
	{
		auto controller = std::make_shared< actual_controller_t >(
				req,
				m_handlers );
		controller->next();

		return request_accepted();
	}
};

