 /*
	Simple example using sendfile facility.
*/

#include <iostream>
#include <random>

#include <args.hxx>

#include <restinio/all.hpp>
#include <restinio/transforms/zlib.hpp>

//
// app_args_t
//

struct app_args_t
{
	bool m_help{ false };
	std::uint16_t m_port{ 8080 };
	std::string m_address{ "localhost" };
	std::size_t m_pool_size{ 1 };
	bool m_trace_server{ false };

	app_args_t( int argc, const char * argv[] )
	{
		args::ArgumentParser parser( "Sendfile hello server", "" );
		args::HelpFlag help( parser, "Help", "Usage example", { 'h', "help" } );

		args::ValueFlag< std::uint16_t > arg_port(
				parser, "port", "tcp port to run server on (default: 8080)",
				{ 'p', "port" } );

		args::ValueFlag< std::string > arg_address(
				parser, "ip", "tcp address of server (default: localhost), "
						"examples: 0.0.0.0, 192.168.1.42",
				{ 'a', "address" } );

		args::ValueFlag< std::size_t > arg_pool_size(
				parser, "size",
				"The size of a thread pool to run the server",
				{ 'n', "thread-pool-size" } );

		args::Flag arg_trace_server(
				parser, "trace server",
				"Enable trace server",
				{'t', "trace"} );

		try
		{
			parser.ParseCLI( argc, argv );
		}
		catch( const args::Help & )
		{
			m_help = true;
			std::cout << parser;
			return;
		}

		if( arg_port )
			m_port = args::get( arg_port );

		if( arg_address )
			m_address = args::get( arg_address );

		m_trace_server = arg_trace_server;
	}
};

using router_t = restinio::router::express_router_t<>;

namespace rtz = restinio::transforms::zlib;

auto make_router()
{
	auto router = std::make_unique< router_t >();

	router->http_post(
		R"-(/)-",
		[ & ]( restinio::request_handle_t req, auto ){

			return req->create_response()
				.append_header( "Server", "RESTinio" )
				.append_header_date_field()
				.append_header(
					// Inherit content-type from request (if it is set).
					restinio::http_field::content_type,
					req->header().get_field(
						restinio::http_field::content_type,
						"text/plain" ) )
				.set_body(
					// Decompress request body if necessary.
					rtz::handle_body(
						*req,
						[]( std::string body_data ){ return body_data; } ) )
				.done();

		} );

	return router;
}

template < typename Server_Traits >
void run_server( const app_args_t & args )
{
	restinio::run(
		restinio::on_thread_pool< Server_Traits >( args.m_pool_size )
			.port( args.m_port )
			.address( args.m_address )
			.concurrent_accepts_count( args.m_pool_size )
			.request_handler( make_router() ) );
}

int main( int argc, const char * argv[] )
{
	try
	{
		const app_args_t args{ argc, argv };

		if( !args.m_help )
		{
			if( args.m_trace_server )
			{
				using traits_t =
					restinio::traits_t<
						restinio::asio_timer_manager_t,
						restinio::shared_ostream_logger_t,
						router_t >;

				run_server< traits_t >( args );
			}
			else
			{
				using traits_t =
					restinio::traits_t<
						restinio::asio_timer_manager_t,
						restinio::null_logger_t,
						router_t >;

				run_server< traits_t >( args );
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
