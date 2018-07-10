/*
	restinio
*/

#include <fstream>

#include <args.hxx>

#include <restinio/all.hpp>

#include "route_parser.hpp"

struct app_args_t
{
	bool m_help{ false };

	std::uint16_t m_port{ 8080 };
	std::size_t m_pool_size{ 1 };

	app_args_t( int argc, const char * argv[] )
	{
		args::ArgumentParser parser( "HardcodedRouter benchmark", "" );
		args::HelpFlag help( parser, "Help", "Usage example", { 'h', "help" } );

		args::ValueFlag< std::uint16_t > arg_port(
				parser, "port",
				"HTTP server port",
				{ 'p', "port" } );

		args::ValueFlag< std::size_t > arg_pool_size(
				parser, "size",
				"The size of a thread pool to run the server",
				{ 'n', "pool-size" } );

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
		{
			m_pool_size = args::get( arg_pool_size );

			if( m_pool_size < 1 )
				throw std::runtime_error{ "invalid pool size" };
		}
	}
};

const std::string resp_body{ "HardcodBench" };

struct request_handler_t
{
	auto
	operator () ( restinio::request_handle_t req ) const
	{
		auto route_parse_result = parse_route( req->header().request_target() );

		if( request_type_t::unknown != route_parse_result.m_request_type )
		{
			return
				req->create_response()
					.append_header( "Server", "RESTinio Benchmark" )
					// .append_header_date_field()
					.append_header( "Content-Type", "text/plain; charset=utf-8" )
					.set_body( resp_body )
					.done();
		}

		return
			req->create_response( restinio::status_not_implemented() )
				.connection_close()
				.done();
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
			.max_pipelined_requests( 4 ) );
}

int
main( int argc, const char *argv[] )
{
	try
	{
		const app_args_t args{ argc, argv };

		if( !args.m_help )
		{
			if( 1 < args.m_pool_size )
			{
				using traits_t =
					restinio::traits_t<
						restinio::asio_timer_manager_t,
						restinio::null_logger_t,
						request_handler_t >;

				run_app< traits_t >( args );
			}
			else if( 1 == args.m_pool_size )
			{
				using traits_t =
					restinio::single_thread_traits_t<
						restinio::asio_timer_manager_t,
						restinio::null_logger_t,
						request_handler_t >;

				run_app< traits_t >( args );
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

