/*
	restinio bench single handler.
*/
#include <stdexcept>
#include <iostream>
#include <fstream>

#include <restinio/all.hpp>

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

template < typename TRAITS >
void run_app( const app_args_t args )
{
	using namespace std::chrono;
	restinio::run(
		restinio::on_thread_pool< TRAITS >( args.m_pool_size )
			.address( "localhost" )
			.port( args.m_port )
			.buffer_size( 1024 )
			.read_next_http_message_timelimit( 5s )
			.write_http_response_timelimit( 5s )
			.handle_request_timeout( 5s )
			.max_pipelined_requests( 4 ) );
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
						restinio::asio_timer_manager_t,
						restinio::null_logger_t,
						req_handler_t >;

				run_app< traits_t >( args );
			}
			else if( 1 == args.m_pool_size )
			{
				using traits_t =
					restinio::single_thread_traits_t<
						restinio::asio_timer_manager_t,
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
