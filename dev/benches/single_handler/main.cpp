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

template< typename Settings >
void
setup_common_values(
	const app_args_t & args,
	Settings & settings )
{
	using namespace std::chrono;

	settings
		.address( "localhost" )
		.port( args.m_port )
		.buffer_size( 1024u )
		.read_next_http_message_timelimit( 5s )
		.write_http_response_timelimit( 5s )
		.handle_request_timeout( 5s )
		.max_pipelined_requests( 4u );
}

template< bool Use_Connection_Limits >
struct settings_tunner;

template<>
struct settings_tunner< false >
{
	template< typename Settings >
	static void
	tune( const app_args_t & args, Settings & settings )
	{
		setup_common_values( args, settings );
	}
};

template<>
struct settings_tunner< true >
{
	template< typename Settings >
	static void
	tune( const app_args_t & args, Settings & settings )
	{
		setup_common_values( args, settings );
		settings.max_parallel_connections( args.m_max_parallel_connections );

		std::cout << "connection_count_limit: " <<
				args.m_max_parallel_connections << std::endl;
	}
};

template < typename Traits >
void run_app( const app_args_t args )
{
	auto settings = restinio::on_thread_pool< Traits >( args.m_pool_size );
	settings_tunner< Traits::use_connection_count_limiter >::tune(
			args, settings );

	restinio::run( std::move(settings) );
}

struct multi_thread_no_limit_traits_t : public restinio::traits_t<
		restinio::asio_timer_manager_t,
		restinio::null_logger_t,
		req_handler_t >
{};

struct multi_thread_with_limit_traits_t : public restinio::traits_t<
		restinio::asio_timer_manager_t,
		restinio::null_logger_t,
		req_handler_t >
{
	static constexpr bool use_connection_count_limiter = true;
};

struct single_thread_no_limit_traits_t : public restinio::single_thread_traits_t<
		restinio::asio_timer_manager_t,
		restinio::null_logger_t,
		req_handler_t >
{};

struct single_thread_with_limit_traits_t : public restinio::single_thread_traits_t<
		restinio::asio_timer_manager_t,
		restinio::null_logger_t,
		req_handler_t >
{
	static constexpr bool use_connection_count_limiter = true;
};

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
				if( 0u == args.m_max_parallel_connections )
				{
					run_app< multi_thread_no_limit_traits_t >( args );
				}
				else
				{
					run_app< multi_thread_with_limit_traits_t >( args );
				}
			}
			else if( 1 == args.m_pool_size )
			{
				if( 0u == args.m_max_parallel_connections )
				{
					run_app< single_thread_no_limit_traits_t >( args );
				}
				else
				{
					run_app< single_thread_with_limit_traits_t >( args );
				}
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
