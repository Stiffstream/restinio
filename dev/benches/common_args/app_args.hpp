#pragma once

#include <iostream>

#include <clara.hpp>
#include <fmt/format.h>

//
// app_args_t
//

struct app_args_t
{
	bool m_help{ false };
	std::string m_address{ "localhost" };
	std::uint16_t m_port{ 8080 };
	std::size_t m_pool_size{ 1 };

	std::size_t m_max_parallel_connections{ 0u };

	static app_args_t
	parse( int argc, const char * argv[] )
	{
		using namespace clara;

		app_args_t result;

		auto cli =
			Opt( result.m_address, "address" )
					["-a"]["--address"]
					( fmt::format(
							RESTINIO_FMT_FORMAT_STRING( "address to listen (default: {})" ),
							result.m_address ) )
			| Opt( result.m_port, "port" )
					["-p"]["--port"]
					( fmt::format(
							RESTINIO_FMT_FORMAT_STRING( "port to listen (default: {})" ),
							result.m_port ) )
			| Opt( result.m_pool_size, "thread-pool size" )
					[ "-n" ][ "--thread-pool-size" ]
					( fmt::format(
							RESTINIO_FMT_FORMAT_STRING(
								"The size of a thread pool to run server (default: {})" ),
						result.m_pool_size ) )
			| Opt( result.m_max_parallel_connections, "max parallel connections" )
					[ "-m" ][ "--max-parallel-connections" ]
					( fmt::format(
							RESTINIO_FMT_FORMAT_STRING(
								"The max count of parallel connections. "
								"Zero means that connection count limits is not used. "
								"(default: {})" ),
						result.m_max_parallel_connections )
					)
			| Help(result.m_help);

		auto parse_result = cli.parse( Args(argc, argv) );
		if( !parse_result )
		{
			throw std::runtime_error{
				fmt::format(
					RESTINIO_FMT_FORMAT_STRING( "Invalid command-line arguments: {}" ),
					parse_result.errorMessage() ) };
		}

		if( result.m_help )
		{
			std::cout << cli << std::endl;
		}

		return result;
	}
};
