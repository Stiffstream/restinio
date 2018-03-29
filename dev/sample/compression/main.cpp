 /*
	Simple example using sendfile facility.
*/

#include <iostream>

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

std::string get_random_nums_str( std::size_t count )
{
	std::ostringstream sout;

	while( 0 != count-- )
	{
		sout << std::rand() << "\r\n";
	}

	return sout.str();
}

std::vector< int >
get_random_nums( std::size_t count )
{
	std::vector< int > result;
	result.reserve( count );

	std::generate_n( std::back_inserter( result ), count, [](){ return rand(); } );

	return result;
}

using router_t = restinio::router::express_router_t<>;

auto make_transform_params(
	const restinio::request_t & req,
	const restinio::query_string_params_t & qp )
{
	namespace rtz = restinio::transforms::zlib;

	const auto accept_encoding =
		req.header().get_field(
			restinio::http_field::accept_encoding,
			"deflate" );

	auto copression_level = [&]{
		int level = -1;
		if( qp.has( "level" ) )
		{
			level= restinio::cast_to< int >( qp[ "level" ] );
		}
		return level;
	};

	if( std::string::npos != accept_encoding.find( "deflate" ) )
	{
		return rtz::make_deflate_compress_params( copression_level() );
	}
	else if( std::string::npos != accept_encoding.find( "gzip" ) )
	{
		return rtz::make_gzip_compress_params( copression_level() );
	}

	return rtz::make_identity_params();
};

auto make_router()
{
	auto router = std::make_unique< router_t >();

	router->http_get(
		R"-(/rand/nums)-",
		[ & ]( restinio::request_handle_t req, auto ){

			namespace rtz = restinio::transforms::zlib;

			const auto qp = restinio::parse_query( req->header().query() );
			std::size_t count = 100;

			if( qp.has( "count" ) )
			{
				count = restinio::cast_to< decltype(count) >( qp[ "count" ] );
			}

			if( count < 10000 )
			{
				auto resp = req->create_response();
				resp
					.append_header( "Server", "RESTinio Benchmark" )
					.append_header_date_field()
					.append_header( "Content-Type", "text/plain; charset=utf-8" );


				auto ba = rtz::body_appender( resp, make_transform_params( *req, qp ) );
				ba.append( get_random_nums_str( count ) );
				ba.complete();

				return resp.done();
			}

			// The data is big enough, so use chunked encoding.
			auto resp = req->create_response< restinio::chunked_output_t >();
			resp
				.append_header( "Server", "RESTinio Benchmark" )
				.append_header_date_field()
				.append_header( "Content-Type", "text/plain; charset=utf-8" );

			auto ba = rtz::body_appender( resp, make_transform_params( *req, qp ) );

			while( 0 != count )
			{
				const auto current_portion_size = std::min< std::size_t >( count, 10000 );
				ba.make_chunk( get_random_nums_str( current_portion_size ) );
				ba.flush();

				count -= current_portion_size;
			}

			ba.complete();

			return resp.done();
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
