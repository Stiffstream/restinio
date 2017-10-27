#pragma once

#include <iostream>

#include <args/args.hxx>

//
// app_args_t
//

struct app_args_t
{
	bool m_help{ false };

	std::uint16_t m_port{ 8080 };
	std::size_t m_pool_size{ 1 };

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
	}
};
