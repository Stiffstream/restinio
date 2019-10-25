 /*
	Simple example that shows file upload functionality.
*/

#include <iostream>
#include <random>
#include <fstream>

#include <restinio/all.hpp>

#include <restinio/helpers/http_field_parsers/content-type.hpp>
#include <restinio/helpers/http_field_parsers/content-disposition.hpp>
#include <restinio/helpers/multipart_body.hpp>
#include <restinio/helpers/string_algo.hpp>

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

std::string to_string(
	const restinio::string_view_t & what )
{
	return { what.data(), what.size() };
}

template< typename Container >
restinio::optional_t< std::string >
try_find_parameter(
	const Container & where,
	restinio::string_view_t parameter_name )
{
	using std::begin;
	using std::end;

	const auto it_value = std::find_if(
			begin(where),
			end(where),
			[parameter_name]( const auto & p ) noexcept {
				return p.first == parameter_name;
			} );

	if( it_value != end(where) )
		return it_value->second;
	else
		return restinio::nullopt;
}

std::string get_boundary(
	const restinio::request_handle_t & req )
{
	// There should be content-type field.
	const auto content_type = req->header().value_of(
			restinio::http_field::content_type );

	const auto r = restinio::http_field_parsers::content_type_value_t::
			try_parse( content_type );
	if( !r.first )
		throw std::runtime_error( "unable to parse content-type field: " +
				to_string( content_type ) );

	const auto & media_type = r.second.m_media_type;

	if( "multipart" != media_type.m_type ||
			"form-data" != media_type.m_subtype )
		throw std::runtime_error( "expects `multipart/form-data` content-type, "
				"got: `" + media_type.m_type + "/" + media_type.m_subtype + "`" );

	const auto opt_boundary = try_find_parameter(
			media_type.m_parameters,
			"boundary" );

	if( !opt_boundary )
		throw std::runtime_error(
				"'boundary' is not found in content-type field: " +
				to_string( content_type ) );

	std::string result;
	result.reserve( 2u + opt_boundary->size() );
	result.append( "--" );
	result.append( *opt_boundary );

	return result;
}

void store_file_to_disk(
	const app_args_t & args,
	const std::string & file_name,
	restinio::string_view_t raw_content )
{
	std::ofstream dest_file;
	dest_file.exceptions( std::ofstream::failbit );
	dest_file.open( args.m_dest_folder + "/" + file_name,
			std::ios_base::out | std::ios_base::trunc | std::ios_base::binary );
	dest_file.write( raw_content.data(), raw_content.size() );
}

bool try_handle_body_fragment(
	const app_args_t & args,
	restinio::string_view_t fragment )
{
	const auto parse_result = restinio::multipart_body::try_parse_part( fragment );
	if( parse_result.first )
	{
		const auto disposition_field =
				parse_result.second.m_fields.opt_value_of(
						restinio::http_field::content_disposition );
		if( disposition_field )
		{
			const auto parsed_disposition =
					restinio::http_field_parsers::content_disposition_value_t::
							try_parse( *disposition_field );

			if( !parsed_disposition.first )
				throw std::runtime_error(
						"unable to parse Content-Disposition field: `" +
						to_string( parse_result.second.m_body ) );

			if( "form-data" != parsed_disposition.second.m_value )
				throw std::runtime_error(
						"unexpected value of Content-Disposition field: `" +
						parsed_disposition.second.m_value );

			const auto opt_name = try_find_parameter(
					parsed_disposition.second.m_parameters,
					"name" );
			if( opt_name && *opt_name == "file" )
			{
				auto opt_filename = try_find_parameter(
						parsed_disposition.second.m_parameters,
						"filename*" );
				if( !opt_filename )
					opt_filename = try_find_parameter(
							parsed_disposition.second.m_parameters,
							"filename" );

				if( !opt_filename )
					throw std::runtime_error(
							"file name is not specified in Content-Disposition field: `" +
							to_string( *disposition_field ) );

				store_file_to_disk(
						args,
						to_string( *opt_filename ),
						parse_result.second.m_body );

				return true;
			}
		}
	}

	return false;
}

void save_file(
	const app_args_t & args,
	const restinio::request_handle_t & req )
{
	const auto boundary = get_boundary( req );

	const auto parts = restinio::multipart_body::split_multipart_body(
			req->body(),
			boundary );

	if( parts.empty() )
		throw std::runtime_error( "no valid parts found in multi-part body" );

	for( const auto & p : parts )
		if( try_handle_body_fragment( args, p ) )
			break;
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

