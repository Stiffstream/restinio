/*
	restinio bench single handler.
*/

#include <iostream>
#include <fstream>

#include <args.hxx>

#include <restinio/all.hpp>
#include <restinio/so5/so_timer_factory.hpp>

#include <bench/common/cfg.hpp>

//
// app_args_t
//

struct app_args_t
{
	bool m_help{ false };
	std::string m_config_file;
	std::size_t m_asio_pool_size{ 1 };

	// Type of SObjectizers timer.
	enum class timer_type_t {
		wheel,
		list,
		heap
	} m_timer_type = { timer_type_t::wheel };


	app_args_t( int argc, const char * argv[] )
	{
		args::ArgumentParser parser( "Single handler RESTinio benchmark", "" );
		args::HelpFlag help( parser, "Help", "Usage example", { 'h', "help" } );

		args::ValueFlag< std::string > config_file(
				parser, "config_file", "Configuration file",
				{ 'c', "config" } );
		args::ValueFlag< std::size_t > asio_pool_size(
				parser, "asio_pool_size",
				"Size of worker thread pool for Asio",
				{ "asio-pool-size" } );
		args::ValueFlag< std::string > timer_type(
				parser, "timer_type", "Timer type: wheel, list, heap",
				{ 't', "timer" } );

		try
		{
			parser.ParseCLI( argc, argv );
		}
		catch( const args::Help & )
		{
			m_help = true;
			std::cout << parser;
		}

		if( config_file )
			m_config_file = args::get( config_file );
		else if( !m_help )
		{
			std::cout << parser;
			throw std::runtime_error( "Config name must be specified" );
		}

		if( asio_pool_size )
			m_asio_pool_size = args::get( asio_pool_size );

		if( timer_type )
		{
			const std::string tt = args::get( timer_type );

			if( tt == "wheel" )
				m_timer_type = timer_type_t::wheel;
			else if( tt == "list" )
				m_timer_type = timer_type_t::list;
			else if( tt == "heap" )
				m_timer_type = timer_type_t::heap;
			else
				throw std::invalid_argument( "unknown type of timer" );
		}
	}
};

const std::string RESP_BODY{ "Hello world!" };

struct req_handler_t
{
	auto
	operator () ( restinio::request_handle_t req ) const
	{
		if( restinio::http_method_get() == req->header().method() &&
			req->header().request_target() == "/" )
		{
			req->create_response()
				.append_header( "Server", "RESTinio Benchmark server /v.0.2" )
				.append_header_date_field()
				.append_header( "Content-Type", "text/plain; charset=utf-8" )
				.set_body( RESP_BODY )
				.done();

			return restinio::request_accepted();
		}

		return restinio::request_rejected();
	}
};

//
// a_http_server_t
//

template < typename TRAITS >
class a_http_server_t
	:	public so_5::agent_t
{
		typedef so_5::agent_t so_base_type_t;

	public:
		using http_server_t = restinio::http_server_t< TRAITS >;
		using server_settings_t = restinio::server_settings_t< TRAITS >;

		a_http_server_t(
			context_t ctx,
			std::size_t asio_pool_size,
			server_settings_t settings )
			:	so_base_type_t{ std::move( ctx ) }
			,	m_server{
					restinio::create_child_io_context( asio_pool_size ),
					std::move( settings ) }
		{}

		virtual void
		so_evt_start() override
		{
			m_server.open();
		}

		virtual void
		so_evt_finish() override
		{
			m_server.close();
		}

	private:
		http_server_t m_server;
};

void
create_main_coop(
	const app_args_t & args,
	so_5::environment_t & env,
	so_5::coop_t & coop )
{
	using traits_t =
		restinio::traits_t<
			restinio::so5::so_timer_factory_t,
			restinio::null_logger_t,
			req_handler_t >;

	restinio::server_settings_t< traits_t > settings{};

	{
		std::ifstream fin{ args.m_config_file, std::ios::binary };
		if( !fin )
		{
			throw std::runtime_error{ "unable to open config: " + args.m_config_file };
		}
		json_dto::from_stream( fin, settings );
		fin.close();
	}

	using restinio::so5::a_timeout_handler_t;
	settings.timer_factory(
		env,
		coop.make_agent< a_timeout_handler_t >()
			->so_direct_mbox() );

	coop.make_agent< a_http_server_t< traits_t > >(
		args.m_asio_pool_size,
		std::move( settings ) );
}

int
main(int argc, const char *argv[] )
{
	try
	{
		const app_args_t args{ argc, argv };

		if( !args.m_help )
		{
			std::cout << "pool size: " << args.m_asio_pool_size << std::endl;

			so_5::launch(
				[&]( auto & env )
				{
					env.introduce_coop(
						[ & ]( so_5::coop_t & coop ) {
							create_main_coop( args, env, coop );
						} );

					// Wait for quit command.
					std::cout << "Type \"quit\" or \"q\" to quit." << std::endl;

					std::string cmd;
					do
					{
						std::cin >> cmd;
					} while( cmd != "quit" && cmd != "q" );

					// Here after user entered "quit" command.
					env.stop();
				},
				[&]( so_5::environment_params_t & params )
				{
					// Appropriate timer thread must be used.
					so_5::timer_thread_factory_t timer
						= so_5::timer_wheel_factory();
					if( args.m_timer_type == app_args_t::timer_type_t::list )
						timer = so_5::timer_list_factory();
					else if( args.m_timer_type == app_args_t::timer_type_t::heap )
						timer = so_5::timer_heap_factory();

					params.timer_thread( timer );
				} );
		}
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
