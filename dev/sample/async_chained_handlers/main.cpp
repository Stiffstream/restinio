#include <iostream>

#include <restinio/all.hpp>

#include <restinio/async_chain/fixed_size.hpp>

#include <restinio/helpers/http_field_parsers/basic_auth.hpp>
#include <restinio/helpers/http_field_parsers/try_parse_field.hpp>

#include <so_5/all.hpp>

#include <random>
#include <tuple>

using express_router_t = restinio::router::express_router_t<>;

// Helper function to set common fields in a response object.
template < typename RESP >
RESP
init_resp( RESP resp )
{
	resp.append_header( restinio::http_field::server, "RESTinio sample server /v.0.7" );
	resp.append_header_date_field();

	return resp;
}

class auth_performer
{
	// Stuff for generate random delays.
	std::mt19937 m_generator{ std::random_device{}() };
	std::uniform_int_distribution<> m_pause_generator{ 350, 3500 };

	// Chain for sending messages related to authentification processing.
	const so_5::mchain_t m_processing_ch;

public:
	//FIXME: document this!
	struct do_auth
	{
		std::string m_username;
		std::string m_password;

		restinio::async_chain::unique_async_handling_controller_t<> m_controller;
	};

	auth_performer( so_5::mchain_t processing_ch )
		: m_processing_ch{ std::move(processing_ch) }
	{}

	void on_incoming_request(
		restinio::async_chain::unique_async_handling_controller_t<> controller )
	{
		const auto req = controller->request_handle();

		bool should_call_next{ true };

		if( restinio::http_method_get() == req->header().method() )
		{
			// Handle cases like /admin and /admin/
			std::string_view target = req->header().request_target();
			if( target.size() > 1u && '/' == target.back() )
				target.remove_suffix( 1u );

			if( "/admin" == target || "/stats" == target )
			{
				// Authentification has to be processed first.
				should_call_next = false;

				// Try to parse Authorization header.
				using namespace restinio::http_field_parsers::basic_auth;

				const auto result = try_extract_params( *req,
						restinio::http_field::authorization );
				if( result )
				{
					// Imitate a delay in credential check.
					so_5::send_delayed< so_5::mutable_msg< do_auth > >(
							m_processing_ch,
							std::chrono::milliseconds{ m_pause_generator( m_generator ) },
							result->username,
							result->password,
							std::move(controller) );
				}
				else
				{
					generate_unauthorized_response( req );
				}
			}
		}

		if( should_call_next )
			// Switch to the next handler in the chain.
			next( std::move(controller) );
	}

	void on_do_auth( do_auth & cmd )
	{
		if( "user" == cmd.m_username && "12345" == cmd.m_password )
		{
			// Switch to the next handler in the chain.
			next( std::move(cmd.m_controller) );
		}
		else
		{
			//FIXME: a pause has to be used before returning the response.
			generate_unauthorized_response( cmd.m_controller->request_handle() );
		}
	}

private:
	void generate_unauthorized_response(
		const restinio::request_handle_t & req )
	{
		// Just ask for credentials.
		init_resp( req->create_response( restinio::status_unauthorized() ) )
			.append_header( restinio::http_field::content_type,
					"text/plain; charset=utf-8" )
			.append_header( restinio::http_field::www_authenticate,
					R"(Basic realm="Username/Password required", charset="utf-8")" )
			.set_body( "Unauthorized access forbidden")
			.done();
	}
};

auto create_request_handler()
{
	auto router = std::make_shared< express_router_t >();

	router->http_get(
		"/",
		[]( const auto & req, const auto & ) {
			init_resp( req->create_response() )
				.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
				.set_body( "Hello world!")
				.done();

			return restinio::request_accepted();
		} );

	router->http_get(
		"/json",
		[]( const auto & req, const auto & ) {
			init_resp( req->create_response() )
				.append_header( restinio::http_field::content_type, "application/json" )
				.set_body( R"-({"message" : "Hello world!"})-")
				.done();

			return restinio::request_accepted();
		} );

	router->http_get(
		"/html",
		[]( const auto & req, const auto & ) {
			init_resp( req->create_response() )
				.append_header( restinio::http_field::content_type, "text/html; charset=utf-8" )
				.set_body(
					"<html>\r\n"
					"  <head><title>Hello from RESTinio!</title></head>\r\n"
					"  <body>\r\n"
					"    <center><h1>Hello world</h1></center>\r\n"
					"  </body>\r\n"
					"</html>\r\n" )
				.done();

			return restinio::request_accepted();
		} );

	router->http_get(
		"/stats",
		[]( const auto & req, const auto & ) {
			init_resp( req->create_response() )
				.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
				.set_body( "Statistics for that site is not available now")
				.done();

			return restinio::request_accepted();
		} );

	router->http_get(
		"/admin",
		[]( const auto & req, const auto & ) {
			init_resp( req->create_response() )
				.append_header( restinio::http_field::content_type, "text/html; charset=utf-8" )
				.set_body(
					"<html>\r\n"
					"  <head><title>Admin panel</title></head>\r\n"
					"  <body>\r\n"
					"    <center><h1>NOT IMPLEMENTED YET</h1></center>\r\n"
					"  </body>\r\n"
					"</html>\r\n" )
				.done();

			return restinio::request_accepted();
		} );

	return router;
}

// Message for transfer a request from RESTinio's thread to processing thread.
struct do_processing
{
	restinio::async_chain::unique_async_handling_controller_t<> m_controller;
};

// Thread for authentification operations.
void auth_thread_func(const so_5::mchain_t& req_ch)
{
	//FIXME: document this!
	auto auth_ch = so_5::create_mchain( req_ch->environment() );

	//FIXME: document this!
	auth_performer authorither{ auth_ch };

	// This flag will be set to 'true' when some of channels will be closed.
	bool stop = false;
	select(
		so_5::from_all().handle_all()
			// If some channel become closed we should set out 'stop' flag.
			.on_close( [&stop](const auto &) { stop = true; } )
			// A predicate for stopping select() function.
			.stop_on( [&stop]{ return stop; } ),

		// Read and handle handle_request messages from req_ch.
		so_5::receive_case( req_ch,
			[&]( so_5::mutable_mhood_t<do_processing> cmd ) {
				authorither.on_incoming_request( std::move(cmd->m_controller) );
			} ),

		// Read and handle timeout_elapsed messages from delayed_ch.
		so_5::receive_case( auth_ch,
			[&]( so_5::mutable_mhood_t<auth_performer::do_auth> cmd ) {
				authorither.on_do_auth( *cmd );
			})
	);
}

// Thread for final request processing operations.
void processing_thread_func(
	const so_5::mchain_t& req_ch,
	std::shared_ptr<express_router_t> actual_router)
{
	// This flag will be set to 'true' when some of channels will be closed.
	bool stop = false;
	receive(
		so_5::from(req_ch).handle_all()
			// If some channel become closed we should set out 'stop' flag.
			.on_close([&stop](const auto &) { stop = true; })
			// A predicate for stopping select() function.
			.stop_on([&stop]{ return stop; }),
		[&](so_5::mutable_mhood_t<do_processing> cmd) {
			// Just call an actual router for request processing.
			// All futher processing will be performed synchronously.
			std::ignore = (*actual_router)(cmd->m_controller->request_handle());
		});
}

int main()
{
	using traits_t =
		restinio::traits_t<
			restinio::asio_timer_manager_t,
			restinio::single_threaded_ostream_logger_t,
			restinio::async_chain::fixed_size_chain_t<2> >;

	try
	{
		// Launching SObjectizer on a separate thread.
		// There is no need to start and shutdown SObjectize:
		// the wrapped_env_t instance does it automatically.
		so_5::wrapped_env_t sobj;

		// Thread objects for our threads.
		std::thread auth_thread, processing_thread;
		// Our threads should be automatically joined at exit
		// (this is necessary for exception safety).
		auto threads_joiner = so_5::auto_join(auth_thread, processing_thread);

		// Channels for sending requests from RESTinio's thread to
		// separate processing threads.
		auto auth_ch = so_5::create_mchain(sobj);
		auto processing_ch = so_5::create_mchain(sobj);
		// All channel should be automatically closed at scope exit
		// (this is necessary for exception safety).
		auto ch_closer = so_5::auto_close_drop_content(auth_ch, processing_ch);

		// Now we can start our threads.
		// If some exception will be thrown somewhere later the threads
		// will be automatically stopped and joined.
		auth_thread = std::thread{auth_thread_func, auth_ch};
		processing_thread = std::thread{
				processing_thread_func, processing_ch, create_request_handler()
			};

		//FIXME: document this!
		restinio::run(
			restinio::on_this_thread<traits_t>()
				.port( 8080 )
				.address( "localhost" )
				.request_handler(
					[auth_ch]( auto controller ) {
						so_5::send< so_5::mutable_msg<do_processing> >(
								auth_ch,
								std::move(controller) );
						return restinio::async_chain::ok();
					},
					[processing_ch]( auto controller ) {
						so_5::send< so_5::mutable_msg<do_processing> >(
								processing_ch,
								std::move(controller) );
						return restinio::async_chain::ok();
					} ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

