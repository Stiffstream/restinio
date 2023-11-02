#include <iostream>

#include <restinio/all.hpp>

#include <restinio/async_chain/fixed_size.hpp>

#include <restinio/helpers/http_field_parsers/basic_auth.hpp>
#include <restinio/helpers/http_field_parsers/try_parse_field.hpp>

#include <so_5/all.hpp>

#include <random>
#include <tuple>

using express_router_t = restinio::router::express_router_t<>;

// Message for transfer a request from RESTinio's thread to processing thread.
struct do_processing
{
	restinio::async_chain::unique_async_handling_controller_t<> m_controller;
};

// Helper function to set common fields in a response object.
template < typename RESP >
RESP
init_resp( RESP resp )
{
	resp.append_header( restinio::http_field::server, "RESTinio sample server /v.0.7" );
	resp.append_header_date_field();

	return resp;
}

// Helper class to incapsulate actual "processing" of requests that
// require authentification.
class auth_performer
{
	// Stuff for generate random delays.
	std::mt19937 m_generator{ std::random_device{}() };
	std::uniform_int_distribution<> m_pause_generator{ 75, 750 };

	// Chain for sending messages related to authentification processing.
	const so_5::mchain_t m_processing_ch;

public:
	// Message to be send to perform "authentification".
	// Will be sent to processing_ch when auth credentials are extracted
	// from an incoming request.
	struct do_auth
	{
		std::string m_username;
		std::string m_password;

		restinio::async_chain::unique_async_handling_controller_t<> m_controller;
	};

	// Message to be send to generate a response with "unauthorized status".
	// This message may be a delayed message (in case if the current credentials
	// are not valid).
	struct ask_for_credentials
	{
		restinio::async_chain::unique_async_handling_controller_t<> m_controller;
	};

	auth_performer( so_5::mchain_t processing_ch )
		: m_processing_ch{ std::move(processing_ch) }
	{}

	// Reaction to an incoming do_processing
	void on_do_processing( do_processing cmd )
	{
		const auto req = cmd.m_controller->request_handle();

		if( restinio::http_method_get() == req->header().method() )
		{
			// Handle cases like /admin and /admin/
			std::string_view target = req->header().request_target();
			if( target.size() > 1u && '/' == target.back() )
				target.remove_suffix( 1u );

			if( "/admin" == target || "/stats" == target )
			{
				// Try to parse Authorization header.
				using namespace restinio::http_field_parsers::basic_auth;

				const auto result = try_extract_params( *req,
						restinio::http_field::authorization );
				if( result )
				{
					const std::chrono::milliseconds pause{
							m_pause_generator( m_generator )
						};
					// Imitate a delay in credential check.
					so_5::send_delayed< so_5::mutable_msg< do_auth > >(
							m_processing_ch,
							pause,
							result->username,
							result->password,
							std::move(cmd.m_controller) );
				}
				else
				{
					// Ask for credentials without a pause.
					so_5::send< so_5::mutable_msg<ask_for_credentials> >(
							m_processing_ch,
							std::move(cmd.m_controller) );
				}
			}
		}

		// If the controller still has a value then the next
		// handler in the chain can be activated.
		if( cmd.m_controller )
			next( std::move(cmd.m_controller) );
	}

	// Reaction to an incoming do_auth message.
	void on_do_auth( do_auth cmd )
	{
		if( "user" == cmd.m_username && "12345" == cmd.m_password )
		{
			// Switch to the next handler in the chain.
			next( std::move(cmd.m_controller) );
		}
		else
		{
			// Take a pause before asking for new credentials.
			so_5::send_delayed< so_5::mutable_msg<ask_for_credentials> >(
					m_processing_ch,
					std::chrono::milliseconds{ 1750 },
					std::move(cmd.m_controller) );
		}
	}

	// Reaction to an incoming ask_for_credentials message.
	void on_ask_for_credentials( ask_for_credentials cmd )
	{
		init_resp( cmd.m_controller->request_handle()->create_response(
				restinio::status_unauthorized() ) )
			.append_header( restinio::http_field::content_type,
					"text/plain; charset=utf-8" )
			.append_header( restinio::http_field::www_authenticate,
					R"(Basic realm="Username/Password required", charset="utf-8")" )
			.set_body( "Unauthorized access forbidden")
			.done();
	}
};

// Helper function for creation of a router for ordinary request processing.
[[nodiscard]]
std::shared_ptr< express_router_t > create_request_handler()
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

// Thread for authentification operations.
void auth_thread_func(const so_5::mchain_t& req_ch)
{
	// Channel to be used for messages generated by auth_performer.
	const auto auth_ch = so_5::create_mchain( req_ch->environment() );

	// The performer of authentification operations.
	auth_performer authorither{ auth_ch };

	// This flag will be set to 'true' when some of channels will be closed.
	bool stop = false;
	select(
		so_5::from_all().handle_all()
			// If some channel become closed we should set out 'stop' flag.
			.on_close( [&stop](const auto &) { stop = true; } )
			// A predicate for stopping select() function.
			.stop_on( [&stop]{ return stop; } ),

		so_5::receive_case( req_ch,
			[&]( so_5::mutable_mhood_t<do_processing> cmd ) {
				authorither.on_do_processing( std::move(*cmd) );
			} ),

		so_5::receive_case( auth_ch,
			[&]( so_5::mutable_mhood_t<auth_performer::do_auth> cmd ) {
				authorither.on_do_auth( std::move(*cmd) );
			},
			[&]( so_5::mutable_mhood_t<auth_performer::ask_for_credentials> cmd ) {
				authorither.on_ask_for_credentials( std::move(*cmd) );
			} )
	);
}

// Thread for final request processing operations.
void processing_thread_func( const so_5::mchain_t& req_ch )
{
	// This flag will be set to 'true' when some of channels will be closed.
	bool stop = false;
	receive(
		so_5::from( req_ch ).handle_all()
			// If some channel become closed we should set out 'stop' flag.
			.on_close( [&stop](const auto &) { stop = true; } )
			// A predicate for stopping receive() function.
			.stop_on( [&stop]{ return stop; } ),

		// The handler of incoming request.
		[router = create_request_handler()]
		( so_5::mutable_mhood_t<do_processing> cmd ) {
			// Just call an actual router for request processing.
			// All futher processing will be performed synchronously.
			std::ignore = (*router)(cmd->m_controller->request_handle());
		});
}

int main()
{
	// Type of traits to be used for http-server.
	using traits_t =
		restinio::traits_t<
			restinio::asio_timer_manager_t,
			restinio::single_threaded_ostream_logger_t,
			// fixed_size_chain request handler from async_chain as
			// the type of request handler for the http-server.
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
		auto threads_joiner = so_5::auto_join( auth_thread, processing_thread );

		// Channels for sending requests from RESTinio's thread to
		// separate processing threads.
		auto auth_ch = so_5::create_mchain( sobj );
		auto processing_ch = so_5::create_mchain( sobj );
		// All channel should be automatically closed at scope exit
		// (this is necessary for exception safety).
		auto ch_closer = so_5::auto_close_drop_content( auth_ch, processing_ch );

		// Now we can start our threads.
		// If some exception will be thrown somewhere later the threads
		// will be automatically stopped and joined.
		auth_thread = std::thread{ auth_thread_func, auth_ch };
		processing_thread = std::thread{ processing_thread_func, processing_ch };

		// Run an instance of http-server in single-threaded-mode on
		// the current thread.
		restinio::run(
			restinio::on_this_thread<traits_t>()
				.port( 8080 )
				.address( "localhost" )
				.request_handler(
					// We have to enumerate async handlers.
					// Every handler just delegates the actual processing
					// to a separate thread by sending a message to it.
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

