/*
	restinio
*/

#include <fstream>

#include <restinio/all.hpp>
#include <restinio/router/easy_parser_router.hpp>

#include <clara.hpp>
#include <fmt/format.h>

struct app_args_t
{
	bool m_help{ false };

	std::string m_address{ "localhost" };
	std::uint16_t m_port{ 8080 };
	std::size_t m_pool_size{ 1 };

	static app_args_t
	parse( int argc, const char * argv[] )
	{
		using namespace clara;

		app_args_t result;

		const auto make_opt =
			[]( auto & val, const char * name, const char * short_name,
				const char * long_name, const char * description) {

				return Opt( val, name )[ short_name ][ long_name ]
					( fmt::format(
							restinio::fmtlib_tools::runtime_format_string(description),
							val ) );
		};

		auto cli = make_opt(
					result.m_address, "address",
					"-a", "--address",
					"address to listen (default: {})" )
			| make_opt(
					result.m_port, "port",
					"-p", "--port",
					"port to listen (default: {})" )
			| make_opt(
					result.m_pool_size, "thread-pool size",
					"-n", "--thread-pool-size",
					"The size of a thread pool to run server (default: {})" )
			| Help(result.m_help);


		auto parse_result = cli.parse( Args(argc, argv) );
		if( !parse_result )
		{
			throw std::runtime_error{
				fmt::format(
					RESTINIO_FMT_FORMAT_STRING(
						"Invalid command-line arguments: {}" ),
					parse_result.errorMessage() ) };
		}

		if( result.m_help )
		{
			std::cout << cli << std::endl;
		}

		return result;
	}
};

using router_t = restinio::router::easy_parser_router_t;

using user_id_t = std::uint32_t;
using location_id_t = std::uint32_t;
using visit_id_t = std::uint32_t;

struct make_new_user_t {};
struct make_new_location_t {};
struct make_new_visit_t {};

class request_processor_t
{
	auto
	make_response(
		const restinio::request_handle_t & req,
		const restinio::string_view_t body )
	{
		return req->create_response()
				.append_header( "Server", "RESTinio Benchmark" )
				// .append_header_date_field()
				.append_header( "Content-Type", "text/plain; charset=utf-8" )
				.set_body( body )
				.done();
	}

public:
	auto
	on_get_user(
		const restinio::request_handle_t & req,
		const user_id_t )
	{
		return make_response( req, "on_get_user" );
	}

	auto
	on_get_user_visits(
		const restinio::request_handle_t & req,
		const user_id_t )
	{
		return make_response( req, "on_get_user_visits" );
	}

	auto
	on_post_user(
		const restinio::request_handle_t & req,
		const user_id_t )
	{
		return make_response( req, "on_post_user" );
	}

	auto
	on_make_new_user(
		const restinio::request_handle_t & req )
	{
		return make_response( req, "on_make_new_user" );
	}

	auto
	on_get_location(
		const restinio::request_handle_t & req,
		const location_id_t )
	{
		return make_response( req, "on_get_location" );
	}

	auto
	on_get_location_avg(
		const restinio::request_handle_t & req,
		const location_id_t )
	{
		return make_response( req, "on_get_location_avg" );
	}

	auto
	on_post_location(
		const restinio::request_handle_t & req,
		const location_id_t )
	{
		return make_response( req, "on_post_location" );
	}

	auto
	on_make_new_location(
		const restinio::request_handle_t & req )
	{
		return make_response( req, "on_make_new_location" );
	}

	auto
	on_get_visit(
		const restinio::request_handle_t & req,
		const visit_id_t )
	{
		return make_response( req, "on_get_visit" );
	}

	auto
	on_post_visit(
		const restinio::request_handle_t & req,
		const visit_id_t )
	{
		return make_response( req, "on_post_visit" );
	}

	auto
	on_make_new_visit(
		const restinio::request_handle_t & req )
	{
		return make_response( req, "on_make_new_visit" );
	}
};

auto
create_server_handler()
{
	namespace epr = restinio::router::easy_parser_router;

	auto router = std::make_unique< router_t >();
	auto processor = std::make_shared< request_processor_t >();

	auto user_id = epr::non_negative_decimal_number_p< user_id_t >();
	auto location_id = epr::non_negative_decimal_number_p< location_id_t >();
	auto visit_id = epr::non_negative_decimal_number_p< visit_id_t >();

	auto by = [&]( auto method ) {
		using namespace std::placeholders;
		return std::bind( method, processor, _1, _2 );
	};
	auto by0 = [&]( auto method ) {
		using namespace std::placeholders;
		return std::bind( method, processor, _1 );
	};

	const char t_users[]{ "/users/" };
	const char t_locations[]{ "/locations/" };
	const char t_visits[]{ "/visits/" };

	router->add_handler(
			restinio::http_method_get(),
			epr::path_to_params( t_users, user_id ),
			by( &request_processor_t::on_get_user ) );

	router->add_handler(
			restinio::http_method_get(),
			epr::path_to_params( t_users, user_id, "/visits" ),
			by( &request_processor_t::on_get_user_visits ) );

	router->add_handler(
			restinio::http_method_post(),
			epr::path_to_params( t_users, user_id ),
			by( &request_processor_t::on_post_user ) );

	router->add_handler(
			restinio::http_method_post(),
			epr::path_to_params( t_users, "new" ),
			by0( &request_processor_t::on_make_new_user ) );

	router->add_handler(
			restinio::http_method_get(),
			epr::path_to_params( t_locations, location_id ),
			by( &request_processor_t::on_get_location ) );

	router->add_handler(
			restinio::http_method_get(),
			epr::path_to_params( t_locations, location_id, "/avg" ),
			by( &request_processor_t::on_get_location_avg ) );

	router->add_handler(
			restinio::http_method_post(),
			epr::path_to_params( t_locations, location_id ),
			by( &request_processor_t::on_post_location ) );

	router->add_handler(
			restinio::http_method_post(),
			epr::path_to_params( t_locations, "new" ),
			by0( &request_processor_t::on_make_new_location ) );

	router->add_handler(
			restinio::http_method_get(),
			epr::path_to_params( t_visits, visit_id ),
			by( &request_processor_t::on_get_location ) );

	router->add_handler(
			restinio::http_method_post(),
			epr::path_to_params( t_visits, visit_id ),
			by( &request_processor_t::on_post_visit ) );

	router->add_handler(
			restinio::http_method_post(),
			epr::path_to_params( t_visits, "new" ),
			by0( &request_processor_t::on_make_new_visit ) );

	return router;
}

template < typename TRAITS >
void run_app( const app_args_t args )
{
	restinio::run(
		restinio::on_thread_pool< TRAITS >( args.m_pool_size )
			.address( args.m_address )
			.port( args.m_port )
			.request_handler( create_server_handler() )
			.buffer_size( 1024 )
			.max_pipelined_requests( 4 ) );
}

int
main( int argc, const char *argv[] )
{
	try
	{
		const auto args = app_args_t::parse( argc, argv );

		if( !args.m_help )
		{
			if( 1 < args.m_pool_size )
			{
				using traits_t =
					restinio::traits_t<
						restinio::asio_timer_manager_t,
						restinio::null_logger_t,
						router_t >;

				run_app< traits_t >( args );
			}
			else if( 1 == args.m_pool_size )
			{
				using traits_t =
					restinio::single_thread_traits_t<
						restinio::asio_timer_manager_t,
						restinio::null_logger_t,
						router_t >;

				run_app< traits_t >( args );
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

