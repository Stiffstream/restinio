/*
	restinio bench single handler.
*/
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <unordered_map>

#include <restinio/all.hpp>
#include <restinio/timertt_timer_factory.hpp>

#include <args/args.hxx>

enum class timertt_container_type_t
{
	wheel,
	list,
	heap
};

//
// app_args_t
//

struct app_args_t
{
	bool m_help{ false };

	std::uint16_t m_port{ 8080 };
	std::size_t m_pool_size{ 1 };
	timertt_container_type_t m_timertt_container_type{ timertt_container_type_t::heap };

	app_args_t( int argc, const char * argv[] )
	{
		args::ArgumentParser parser( "Single handler benchmark", "" );
		args::HelpFlag help( parser, "Help", "Usage example", { 'h', "help" } );

		args::ValueFlag< std::size_t > arg_port(
				parser, "port",
				"HTTP server port",
				{ 'p', "port" } );

		args::ValueFlag< std::size_t > arg_pool_size(
				parser, "size",
				"The size of a thread pool to run the server",
				{ 'n', "asio-pool-size" } );

		std::unordered_map< std::string, timertt_container_type_t> map{
			{"wheel", timertt_container_type_t::wheel },
			{"list", timertt_container_type_t::list },
			{"heap", timertt_container_type_t::heap } };

		args::MapFlag< std::string, timertt_container_type_t >
			timertt_container_type{ parser, "type", "Use specific container type", {'t', "type"}, map };

		try
		{
			parser.ParseCLI( argc, argv );
		}
		catch( const args::Help & )
		{
			m_help = true;
			std::cout << parser;
		}

		if( arg_port )
			m_port = args::get( arg_port );
		if( arg_pool_size )
			m_pool_size = args::get( arg_pool_size );
		if( timertt_container_type )
			m_timertt_container_type = args::get( timertt_container_type );
	}
};


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

template < typename Traits >
void run_app( const app_args_t & args )
{
	using namespace std::chrono;

	restinio::run(
		restinio::on_thread_pool< Traits >( args.m_pool_size )
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
		const app_args_t args{ argc, argv };

		if( !args.m_help )
		{
			std::cout << "pool size: " << args.m_pool_size << std::endl;

			if( 1 < args.m_pool_size )
			{
				if( timertt_container_type_t::wheel == args.m_timertt_container_type )
				{
					using traits_t =
						restinio::traits_t<
							restinio::mt_timertt_wheel_timer_factory_t,
							restinio::null_logger_t,
							req_handler_t >;

					run_app< traits_t >( args );
				}
				else if( timertt_container_type_t::list == args.m_timertt_container_type )
				{
					using traits_t =
						restinio::traits_t<
							restinio::mt_timertt_list_timer_factory_t,
							restinio::null_logger_t,
							req_handler_t >;

					run_app< traits_t >( args );
				}
				else if( timertt_container_type_t::heap == args.m_timertt_container_type )
				{
					using traits_t =
						restinio::traits_t<
							restinio::mt_timertt_heap_timer_factory_t,
							restinio::null_logger_t,
							req_handler_t >;

					run_app< traits_t >( args );
				}
			}
			else if( 1 == args.m_pool_size )
			{
				if( timertt_container_type_t::wheel == args.m_timertt_container_type )
				{
					using traits_t =
						restinio::single_thread_traits_t<
							restinio::st_timertt_wheel_timer_factory_t,
							restinio::null_logger_t,
							req_handler_t >;

					run_app< traits_t >( args );
				}
				else if( timertt_container_type_t::list == args.m_timertt_container_type )
				{
					using traits_t =
						restinio::single_thread_traits_t<
							restinio::st_timertt_list_timer_factory_t,
							restinio::null_logger_t,
							req_handler_t >;

					run_app< traits_t >( args );
				}
				else if( timertt_container_type_t::heap == args.m_timertt_container_type )
				{
					using traits_t =
						restinio::single_thread_traits_t<
							restinio::st_timertt_heap_timer_factory_t,
							// restinio::single_threaded_ostream_logger_t,
							restinio::null_logger_t,
							req_handler_t >;

					run_app< traits_t >( args );
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
