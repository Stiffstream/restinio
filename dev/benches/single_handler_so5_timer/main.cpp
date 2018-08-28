/*
	restinio bench single handler.
*/
#include <stdexcept>
#include <iostream>
#include <fstream>

#include <restinio/all.hpp>
#include <restinio/so5/so_timer_manager.hpp>

#include <benches/common_args/app_args.hpp>

const std::string resp_body{ "Hello world!" };

struct req_handler_t
{
	auto operator () ( restinio::request_handle_t req ) const
	{
		if( restinio::http_method_get() == req->header().method() &&
			req->header().request_target() == "/" )
		{
			return
				req->create_response()
					.append_header( "Server", "RESTinio Benchmark" )
					// .append_header_date_field()
					.append_header( "Content-Type", "text/plain; charset=utf-8" )
					.set_body( resp_body )
					.done();
		}

		return restinio::request_rejected();
	}
};

// Agent that starts/stops a server.
template < typename Traits >
class a_http_server_t :	public so_5::agent_t
{
	public:
		a_http_server_t(
			context_t ctx,
			std::size_t pool_size,
			restinio::server_settings_t< Traits > settings )
			:	so_5::agent_t{ std::move( ctx ) }
			,	m_ioctx_pool{ pool_size }
			,	m_server{
					restinio::external_io_context( m_ioctx_pool.io_context() ),
					std::move( settings ) }
		{}

		virtual void so_evt_start() override
		{
			m_server.open_sync();
			m_ioctx_pool.start();
		}

		virtual void so_evt_finish() override
		{
			m_ioctx_pool.stop();
			m_ioctx_pool.wait();
		}

	private:
		restinio::impl::ioctx_on_thread_pool_t<
				restinio::impl::own_io_context_for_thread_pool_t > m_ioctx_pool;
		restinio::http_server_t< Traits > m_server;
};

template < typename Traits >
void run_app( const app_args_t & args )
{
	using namespace std::chrono;

	so_5::wrapped_env_t sobj{ [&]( auto & env ){
		env.introduce_coop(
			[ & ]( so_5::coop_t & coop ) {
				coop.make_agent< a_http_server_t< Traits > >(
					args.m_pool_size,
					restinio::server_settings_t< Traits >{}
						.address( "localhost" )
						.port( args.m_port )
						.buffer_size( 1024 )
						.read_next_http_message_timelimit( 5s )
						.write_http_response_timelimit( 5s )
						.handle_request_timeout( 5s )
						.timer_manager(
							coop.environment(),
							coop.make_agent< restinio::so5::a_timeout_handler_t >()->so_direct_mbox() )
						.max_pipelined_requests( 4 ) );
			} );
	} };

	std::cout << "Press ENTER to exit." << std::endl;
	std::string line;
	std::getline( std::cin, line );
}

int main(int argc, const char *argv[])
{
	try
	{
		const auto args = app_args_t::parse( argc, argv );

		if( !args.m_help )
		{
			std::cout << "pool size: " << args.m_pool_size << std::endl;

			if( 1 < args.m_pool_size )
			{
				using traits_t =
					restinio::traits_t<
						restinio::so5::so_timer_manager_t,
						restinio::null_logger_t,
						req_handler_t >;

				run_app< traits_t >( args );
			}
			else if( 1 == args.m_pool_size )
			{
				using traits_t =
					restinio::single_thread_traits_t<
						restinio::so5::so_timer_manager_t,
						restinio::null_logger_t,
						req_handler_t >;

				run_app< traits_t >( args );
			}
			else
			{
				throw std::runtime_error{ "invalid asio pool size" };
			}
		}
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
