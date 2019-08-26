/*
	restinio
*/

#include <fstream>

#include <restinio/all.hpp>

#include <clara.hpp>
#include <fmt/format.h>

#include "route_parser.hpp"

struct app_args_t
{
	bool m_help{ false };

	std::string m_address{ "loalhost" };
	std::uint16_t m_port{ 8080 };
	std::size_t m_pool_size{ 1 };

	static app_args_t
	parse( int argc, const char * argv[] )
	{
		using namespace clara;

		app_args_t result;

		const auto make_opt =
			[]( auto & val, const char * name, const char * short_name,
				const char * long_name, const char * description) {

				return Opt( val, name )[ short_name ][ long_name ]
					( fmt::format( description, val ) );
		};

		auto cli = make_opt(
					result.m_address, "address",
					"-a", "--address",
					"address to listen (default: {})" )
			| make_opt(
					result.m_port, "port",
					"-p", "--port",
					"port to listen (default: {})" )
			| make_opt(
					result.m_pool_size, "thread-pool size",
					"-n", "--thread-pool-size",
					"The size of a thread pool to run server (default: {})" )
			| Help(result.m_help);


		auto parse_result = cli.parse( Args(argc, argv) );
		if( !parse_result )
		{
			throw std::runtime_error{
				fmt::format(
					"Invalid command-line arguments: {}",
					parse_result.errorMessage() ) };
		}

		if( result.m_help )
		{
			std::cout << cli << std::endl;
		}

		return result;
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
			.address( args.m_address )
			.port( args.m_port )
			.buffer_size( 1024 )
			.max_pipelined_requests( 4 ) );
}

int
main( int argc, const char *argv[] )
{
	try
	{
		const auto args = app_args_t::parse( argc, argv );

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

