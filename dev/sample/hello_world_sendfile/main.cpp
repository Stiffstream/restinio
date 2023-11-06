/*
	Simple example using sendfile facility.
*/

#include <iostream>

#include <restinio/core.hpp>

#include <fmt/format.h>

#include <restinio-helpers/cmd_line_args_helpers.hpp>

//
// app_args_t
//

struct app_args_t
{
	bool m_help{ false };
	std::string m_address{ "localhost" };
	std::uint16_t m_port{ 8080 };
	std::size_t m_pool_size{ 1 };
	std::string m_file;
	restinio::file_offset_t m_data_offset{ 0 };
	restinio::file_size_t m_data_size{ std::numeric_limits< restinio::file_size_t >::max() };
	std::string m_content_type{ "text/plain" };
	bool m_trace_server{ false };

	static app_args_t
	parse( int argc, const char * argv[] )
	{
		using namespace restinio_helpers;

		app_args_t result;

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
						result.m_file,
						"-f", "--file",
						"path to a file that will be served as response"
					},
				cmd_line_arg_t{
						result.m_data_offset,
						"-o", "--data-offset",
						"offset of the data portion in file (default: {})"
					},
				cmd_line_arg_t{
						result.m_data_size,
						"-s", "--data-size",
						"size of the data portion in file (default: to the end of file)"
					},
				cmd_line_arg_t{
						result.m_content_type,
						"-c", "--content-type",
						"A value of 'Content-Type' header field (default: {})"
					},
				cmd_line_arg_t{
						result.m_trace_server,
						"-t", "--trace",
						"enable trace server"
					} );

		if( result.m_file.empty() )
			throw std::runtime_error{
					"Name of file to be used as a response "
					"has to be specified in command line"
				};

		return result;
	}
};

template < typename Server_Traits >
void run_server( const app_args_t & args )
{
	restinio::run(
		restinio::on_thread_pool< Server_Traits >( args.m_pool_size )
			.port( args.m_port )
			.address( args.m_address )
			.concurrent_accepts_count( args.m_pool_size )
			.request_handler(
				[&]( auto req ){
					if( restinio::http_method_get() == req->header().method() &&
						req->header().request_target() == "/" )
					{
						try
						{
							auto sf = restinio::sendfile( args.m_file );
							sf.offset_and_size(
								args.m_data_offset,
								args.m_data_size );

							return
								req->create_response()
									.append_header( restinio::http_field::server, "RESTinio hello world server" )
									.append_header_date_field()
									.append_header(
										restinio::http_field::content_type,
										args.m_content_type )
									.set_body( std::move( sf ) )
									.done();
						}
						catch( const std::exception & )
						{
							return
								req->create_response(
										restinio::status_not_found() )
									.connection_close()
									.append_header_date_field()
									.done();
						}
					}

					return restinio::request_rejected();
			} ) );
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
						restinio::shared_ostream_logger_t >;

				run_server< traits_t >( args );
			}
			else
			{
				run_server< restinio::default_traits_t >( args );
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
