#include <type_traits>
#include <iostream>
#include <chrono>

#include <restinio/all.hpp>
#include <restinio/so5/so_timer_manager.hpp>

#include <clara/clara.hpp>
#include <fmt/format.h>

// Application agents.
#include "app.hpp"

//
// app_args_t
//
struct app_args_t
{
	bool m_help{ false };
	std::string m_address{ "localhost" };
	std::uint16_t m_port{ 8080 };

	static app_args_t
	parse( int argc, const char * argv[] )
	{
		using namespace clara;

		app_args_t result;

		auto cli =
			Opt( result.m_address, "address" )
					["-a"]["--address"]
					( fmt::format( "address to listen (default: {})", result.m_address ) )
			| Opt( result.m_port, "port" )
					["-p"]["--port"]
					( fmt::format( "port to listen (default: {})", result.m_port ) )
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

//
// a_http_server_t
//

// Agent that starts/stops a server.
template < typename TRAITS >
class a_http_server_t :	public so_5::agent_t
{
	public:
		a_http_server_t(
			context_t ctx,
			restinio::server_settings_t< TRAITS > settings )
			:	so_5::agent_t{ std::move( ctx ) }
			,	m_server{
					restinio::own_io_context(),
					std::move( settings ) }
		{}

		virtual void so_evt_start() override
		{
			m_server_thread = std::thread{ [&] {
				m_server.open_async(
						[]{ /* Ok. */ },
						[]( auto ex ) { std::rethrow_exception( ex ); } );
				m_server.io_context().run();
			} };
		}

		virtual void so_evt_finish() override
		{
			m_server.close_async(
					[&]{ m_server.io_context().stop(); },
					[]( auto ex ) { std::rethrow_exception( ex ); } );
			m_server_thread.join();
		}

	private:
		restinio::http_server_t< TRAITS > m_server;
		std::thread m_server_thread;
};

void create_main_coop(
	const app_args_t & args,
	so_5::coop_t & coop )
{
	using traits_t =
		restinio::traits_t<
			restinio::so5::so_timer_manager_t,
			restinio::single_threaded_ostream_logger_t >;

	restinio::server_settings_t< traits_t > settings{};

	settings
		.port( args.m_port )
		.address( "localhost" )
		.timer_manager(
			coop.environment(),
			coop.make_agent< restinio::so5::a_timeout_handler_t >()->so_direct_mbox() )
		.request_handler(
			create_request_handler(
				// Add application agent to cooperation.
				coop.make_agent< a_main_handler_t >()->so_direct_mbox() ) );

	coop.make_agent< a_http_server_t< traits_t > >( std::move( settings ) );
}

int main( int argc, char const *argv[] )
{
	try
	{
		const auto args = app_args_t::parse( argc, argv );

		if( args.m_help )
			return 0;

		so_5::wrapped_env_t sobj{ [&]( auto & env )
			{
				env.introduce_coop(
					[ & ]( so_5::coop_t & coop ) {
						create_main_coop( args, coop );
					} );
			} };

		std::string cmd;
		do
		{
			// Wait for quit command.
			std::cout << "Type \"quit\" or \"q\" to quit." << std::endl;
			std::cin >> cmd;
		} while( cmd != "quit" && cmd != "q" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
