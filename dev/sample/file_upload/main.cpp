 /*
	Simple example that shows file upload functionality.
*/

#include <iostream>
#include <random>
#include <fstream>

#include <restinio/all.hpp>

#include <restinio/helpers/file_upload.hpp>

#include <clara.hpp>
#include <fmt/format.h>

//
// app_args_t
//

struct app_args_t
{
	bool m_help{ false };
	std::string m_dest_folder{ "." };
	std::string m_address{ "localhost" };
	std::uint16_t m_port{ 8080 };
	std::size_t m_pool_size{ 1 };
	bool m_trace_server{ false };

	static app_args_t
	parse( int argc, const char * argv[] )
	{
		using namespace clara;

		app_args_t result;

		auto cli =
			Opt( result.m_dest_folder, "destination folder" )
					["-d"]["--dest-folder"]
					( fmt::format( "destination folder for uploaded files"
							" (default: {})", result.m_dest_folder ) )
			| Opt( result.m_address, "address" )
					["-a"]["--address"]
					( fmt::format( "address to listen (default: {})", result.m_address ) )
			| Opt( result.m_port, "port" )
					["-p"]["--port"]
					( fmt::format( "port to listen (default: {})", result.m_port ) )
			| Opt( result.m_pool_size, "thread-pool size" )
					[ "-n" ][ "--thread-pool-size" ]
					( fmt::format(
						"The size of a thread pool to run server (default: {})",
						result.m_pool_size ) )
			| Opt( result.m_trace_server )
					[ "-t" ][ "--trace" ]
					( "Enable trace server" )
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

using router_t = restinio::router::express_router_t<>;

void store_file_to_disk(
	const app_args_t & args,
	restinio::string_view_t file_name,
	restinio::string_view_t raw_content )
{
	std::ofstream dest_file;
	dest_file.exceptions( std::ofstream::failbit );
	dest_file.open(
			fmt::format( "{}/{}", args.m_dest_folder, file_name ),
			std::ios_base::out | std::ios_base::trunc | std::ios_base::binary );
	dest_file.write( raw_content.data(), raw_content.size() );
}

void save_file(
	const app_args_t & args,
	const restinio::request_handle_t & req )
{
	using namespace restinio::file_upload;

	const auto enumeration_result = enumerate_parts_with_files(
			*req,
			[&args]( const part_description_t & part ) {
				if( "file" == part.name )
				{
					// We can handle the name only in 'filename' parameter.
					if( part.filename )
					{
						// NOTE: the validity of filename is not checked.
						// This is just for simplification of the example.
						store_file_to_disk( args, *part.filename, part.body );

						// There is no need to handle other parts.
						return handling_result_t::stop_enumeration;
					}
				}

				// We expect only one part with name 'file'.
				// So if that part is not found yet there is some error
				// and there is no need to continue.
				return handling_result_t::terminate_enumeration;
			} );

	if( !enumeration_result || 1u != *enumeration_result )
		throw std::runtime_error( "file content not found!" );
}

auto make_router( const app_args_t & args )
{
	auto router = std::make_unique< router_t >();

	router->http_get(
		"/",
		[ & ]( const restinio::request_handle_t& req, auto ){
			const auto action_url = fmt::format( "http://{}:{}/upload",
					args.m_address, args.m_port );

			auto resp = req->create_response();
			resp.append_header( "Server", "RESTinio" );
			resp.append_header_date_field();
			resp.append_header(
					restinio::http_field::content_type,
					"text/html; charset=utf-8" );
			resp.set_body(
R"---(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>File Upload!</title>
</head>
<body>
<p>Please select file to be uploaded to server.</p>
<form method="post" action=")---" + action_url + R"---(" enctype="multipart/form-data">
    <p><input type="text" name="comment" id="comment-id" value=""></p>
    <p><input type="file" name="file" id="file-id"></p>
    <p><button type="submit">Submit</button></p>
</form>
</body>
</html>
)---" );
			return resp.done();
		} );

	router->http_post( "/upload",
		[&]( const auto & req, const auto & )
		{
			save_file( args, req );

			auto resp = req->create_response();
			resp.append_header( "Server", "RESTinio" );
			resp.append_header_date_field();
			resp.append_header(
					restinio::http_field::content_type,
					"text/plain; charset=utf-8" );
			resp.set_body( "Ok. Uploaded" );
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
			.request_handler( make_router( args ) ) );
}

int main( int argc, const char * argv[] )
{
	try
	{
		const auto args = app_args_t::parse( argc, argv );

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

