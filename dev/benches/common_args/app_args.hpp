#pragma once

#include <iostream>

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

	std::size_t m_max_parallel_connections{ 0u };

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
						"-n", "--pool-size",
						"size of a thread pool to run server (default: {})"
					},
				cmd_line_arg_t{
						result.m_max_parallel_connections,
						"-m", "--max-parallel-connections",
						"max count of parallel connections. "
						"Zero means that connection count limits is not used. "
						"(default: {})"
					} );

		return result;
	}
};

