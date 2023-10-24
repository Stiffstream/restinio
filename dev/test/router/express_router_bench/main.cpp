/*
	restinio
*/

#include <fstream>

#include <restinio/all.hpp>

#include <fmt/format.h>

#include <restinio-helpers/cmd_line_args_helpers.hpp>

#ifndef RESTINIO_EXPRESS_ROUTER_BENCH_APP_TITLE
	#define RESTINIO_EXPRESS_ROUTER_BENCH_APP_TITLE "Express router benchmark"
#endif

#ifndef RESTINIO_EXPRESS_ROUTER_BENCH_REGEX_ENGINE
	#define RESTINIO_EXPRESS_ROUTER_BENCH_REGEX_ENGINE restinio::router::std_regex_engine_t
#endif

using router_t = restinio::router::express_router_t< RESTINIO_EXPRESS_ROUTER_BENCH_REGEX_ENGINE >;

struct app_args_t
{
	bool m_help{ false };

	std::string m_address{ "localhost" };
	std::uint16_t m_port{ 8080 };
	std::size_t m_pool_size{ 1 };
	std::string m_routes_file;

	static app_args_t
	parse( int argc, const char * argv[] )
	{
		app_args_t result;

		using namespace restinio_helpers;

		process_cmd_line_args( argc, argv, result,
				cmd_line_arg_t{
						result.m_address,
						"-a", "--address",
						"address to listen (default: {})"
					},
				cmd_line_arg_t{
						result.m_port,
						"-p", "--port",
						"port to listen (default: {})"
					},
				cmd_line_arg_t{
						result.m_pool_size,
						"-n", "--thread-pool-size",
						"size of a thread pool to run server (default: {})"
					},
				cmd_line_arg_t{
						result.m_pool_size,
						"-r", "--routes-file",
						"path to routes file containing lines "
						"of the following format:\n"
						"(GET|POST|...) (/some/route)"
					} );

		return result;
	}
};

struct route_line_t
{
	restinio::http_method_id_t m_method;
	std::string m_route;
};

using route_lines_container_t = std::vector< route_line_t >;

route_lines_container_t
read_route_lines_from_file( const std::string & file_name )
{
	route_lines_container_t result;

	std::ifstream fin{ file_name.c_str() };

	if( !fin )
		throw std::runtime_error{ "unable to read file '" + file_name + "'" };

	while( fin )
	{
		std::array< char, 1024 > buf;
		if( !fin.getline( buf.data(), buf.size(), ' ' ) )
			break;

		route_line_t route_line;

		const std::string method_str{ buf.data() };
		{

			int m = 0;
			restinio::http_method_id_t mm{ restinio::http_method_unknown() };
			restinio::http_method_id_t method = restinio::http_method_unknown();

			while( ( mm = restinio::default_http_methods_t::from_nodejs( m++ ) ) !=
				restinio::http_method_unknown() )
			{
				if( method_str == mm.c_str() )
				{
					method = mm;
					break;
				}
			}

			if( restinio::http_method_unknown() == method )
			{
				throw std::runtime_error{ "invalid method: " + method_str };
			}

			route_line.m_method = method;
		}

		if( !fin.getline( buf.data(), buf.size() ) )
			break;

		route_line.m_route.assign( buf.data() );
		result.emplace_back( std::move( route_line ) );
	}

	return result;
}

const std::string resp_body{ "ExpressBench" };

auto
create_server_handler( route_lines_container_t routes )
{
	auto router = std::make_unique< router_t >();

	for( auto & r : routes )
	{
		std::cout << "Add route: "
			<< r.m_method.c_str() << " '"
			<< r.m_route << "'" << std::endl;

		router->add_handler(
			r.m_method,
			r.m_route,
			[]( auto req, auto ){
				return
					req->create_response()
						.append_header( "Server", "RESTinio Benchmark" )
						// .append_header_date_field()
						.append_header( "Content-Type", "text/plain; charset=utf-8" )
						.set_body( resp_body )
						.done();
			} );
	}

	return router;
}

template < typename TRAITS >
void run_app( const app_args_t args )
{
	auto routes = read_route_lines_from_file( args.m_routes_file );

	std::cout << "Total routes: " << routes.size() << std::endl;

	using namespace std::chrono;
	restinio::run(
		restinio::on_thread_pool< TRAITS >( args.m_pool_size )
			.address( args.m_address )
			.port( args.m_port )
			.request_handler( create_server_handler( std::move( routes ) ) )
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
						router_t >;

				run_app< traits_t >( args );
			}
			else if( 1 == args.m_pool_size )
			{
				using traits_t =
					restinio::single_thread_traits_t<
						restinio::asio_timer_manager_t,
						restinio::null_logger_t,
						router_t >;

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

