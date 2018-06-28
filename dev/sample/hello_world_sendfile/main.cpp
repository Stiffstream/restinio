/*
	Simple example using sendfile facility.
*/

#include <iostream>

#include <args.hxx>

#include <restinio/all.hpp>

//
// app_args_t
//

struct app_args_t
{
	bool m_help{ false };
	std::uint16_t m_port{ 8080 };
	std::string m_address{ "localhost" };
	std::size_t m_pool_size{ 1 };
	std::string m_file;
	restinio::file_offset_t m_data_offset{ 0 };
	restinio::file_size_t m_data_size{ std::numeric_limits< restinio::file_size_t >::max() };
	std::string m_content_type{ "text/plain" };
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

		args::ValueFlag< std::string > arg_file(
				parser, "path", "path to a file that will be served as response",
				{ 'f', "file" } );
		args::ValueFlag< restinio::file_offset_t > arg_data_offset(
				parser, "N", "offset of the data portion in file",
				{ 'o', "data-offset" } );
		args::ValueFlag< restinio::file_size_t > arg_data_size(
				parser, "N", "size of the data portion in file",
				{ 's', "data-size" } );

		args::ValueFlag< std::string > arg_content_type(
				parser, "T", "a value of 'Content-Type' header field"
							"default value is: '" + m_content_type + "'",
				{ "content-type" } );

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

		if( arg_pool_size )
			m_pool_size = args::get( arg_pool_size );

		if( arg_file )
		{
			m_file = args::get( arg_file );
		}
		else
		{
			throw std::runtime_error{ "file is mandatory" };
		}

		if( arg_data_offset )
		{
			m_data_offset = args::get( arg_data_offset );
		}

		if( arg_data_size )
		{
			m_data_size = args::get( arg_data_size );
		}

		if( arg_content_type )
		{
			m_content_type = args::get( arg_content_type );
		}

		m_trace_server = arg_trace_server;
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
							return req->create_response( 404, "Not Found" )
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
		const app_args_t args{ argc, argv };

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
