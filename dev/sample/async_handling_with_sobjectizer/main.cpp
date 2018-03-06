#include <type_traits>
#include <iostream>
#include <chrono>

#include <args.hxx>

#include <restinio/all.hpp>
#include <restinio/so5/so_timer_manager.hpp>

// Application agents.
#include "app.hpp"

//
// app_args_t
//

struct app_args_t
{
	bool m_help{ false };
	std::uint16_t m_port{ 8080 };

	app_args_t( int argc, const char * argv[] )
	{
		args::ArgumentParser parser( "RESTinio with SObjectizer sample", "" );
		args::HelpFlag help( parser, "Help", "Usage example", { 'h', "help" } );

		args::ValueFlag< std::uint16_t > server_port(
				parser, "port", "tcp port to run server on (default: 8080)",
				{ 'p', "port" } );

		try
		{
			parser.ParseCLI( argc, argv );
		}
		catch( const args::Help & )
		{
			m_help = true;
			std::cout << parser;
		}

		if( server_port )
			m_port = args::get( server_port );
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
		const app_args_t args{ argc, argv };

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
