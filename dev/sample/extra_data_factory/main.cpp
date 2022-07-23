#include <iostream>

#include <restinio/all.hpp>

#include <restinio/sync_chain/growable_size.hpp>

#include <restinio/helpers/http_field_parsers/basic_auth.hpp>
#include <restinio/helpers/http_field_parsers/try_parse_field.hpp>

// Information about user permissions.
struct permissions_t
{
	bool m_admin_view{ false };
	bool m_stats_view{ false };
};

// Identification information for a user.
struct identity_t
{
	static constexpr int invalid_user = -1;

	int m_id{ invalid_user };
	permissions_t m_permissions;
};

// IDs for several users.
constexpr int user_guest = 1;
constexpr int user_junior = 2;
constexpr int user_senior = 3;

// Information about a user after the authentification attempt.
struct auth_result_t
{
	bool m_credentials_provided{ false };

	identity_t m_identity;
};

// Data to be incorporated to a request object.
struct per_request_data_t
{
	auth_result_t m_auth_result;
};

// User-data-factory for this example.
using per_request_data_factory_t = restinio::simple_extra_data_factory_t<
	per_request_data_t >;

// Express router should be tuned to be used with extra-data.
using express_router_t = restinio::router::generic_express_router_t<
		restinio::router::std_regex_engine_t,
		per_request_data_factory_t >;

template < typename RESP >
RESP
init_resp( RESP resp )
{
	resp.append_header( restinio::http_field::server, "RESTinio sample server /v.0.6" );
	resp.append_header_date_field();

	return resp;
}

restinio::request_handling_status_t credentials_handler(
	const restinio::generic_request_handle_t< per_request_data_t > & req )
{
	// Get a reference to additional data.
	auto & ud = req->extra_data();

	// If Authorization header is present we should try to handle it.
	if( req->header().has_field( restinio::http_field::authorization ) )
	{
		ud.m_auth_result.m_credentials_provided = true;

		using namespace restinio::http_field_parsers::basic_auth;

		const auto result = try_extract_params( *req,
				restinio::http_field::authorization );
		if( result )
		{

			if( result->username == "junior" && result->password == "1234" )
			{
				ud.m_auth_result.m_identity = identity_t{
					user_junior,
					permissions_t{ false, true }
				};
			}
			else if( result->username == "senior" && result->password == "4321" )
			{
				ud.m_auth_result.m_identity = identity_t{
					user_senior,
					permissions_t{ true, true }
				};
			}
			else
			{
				// If user provides credentials then the credentials
				// should be valid.
				init_resp( req->create_response( restinio::status_unauthorized() ) )
					.append_header(
						restinio::http_field::content_type,
						"text/plain; charset=utf-8" )
					.append_header(
						restinio::http_field::www_authenticate,
							R"(Basic realm="Valid Username/Password should be provided", charset="utf-8")" )
					.set_body( "Invalid credentials")
					.done();

				// Mark the request as processed.
				return restinio::request_accepted();
			}
		}
		else
		{
			// Invalid format of credentials.
			init_resp( req->create_response( restinio::status_bad_request() ) )
				.append_header(
					restinio::http_field::content_type,
					"text/plain; charset=utf-8" )
				.set_body( "Unable to handle Authorization field")
				.done();

			// Mark the request as processed.
			return restinio::request_accepted();
		}
	}
	else
	{
		ud.m_auth_result.m_credentials_provided = false;
		ud.m_auth_result.m_identity = identity_t{
			user_guest,
			permissions_t{ false, false }
		};
	}

	// Force the invocation of the next handler.
	return restinio::request_not_handled();
}

restinio::request_handling_status_t authorization_handler(
	const restinio::generic_request_handle_t< per_request_data_t > & req )
{
	// Authorization required if user asks for '/admin' or '/stats'.
	if( "/admin" == req->header().path() || "/stats" == req->header().path() )
	{
		const auto & ud = req->extra_data();
		if( !ud.m_auth_result.m_credentials_provided )
		{
			// User should provide credentials.
			init_resp( req->create_response( restinio::status_unauthorized() ) )
				.append_header(
					restinio::http_field::content_type,
					"text/plain; charset=utf-8" )
				.append_header(
					restinio::http_field::www_authenticate,
						R"(Basic realm="Username/Password required", charset="utf-8")" )
				.set_body( "Unauthorized access forbidden")
				.done();

			// Mark the request as processed.
			return restinio::request_accepted();
		}
		
		bool has_permission = [&req]( const auto & permissions ) {
			if( "/admin" == req->header().path() )
				return permissions.m_admin_view;
			if( "/stats" == req->header().path() )
				return permissions.m_stats_view;
			return false;
		}( ud.m_auth_result.m_identity.m_permissions );

		if( !has_permission )
		{
			init_resp( req->create_response( restinio::status_unauthorized() ) )
				.append_header(
					restinio::http_field::content_type,
					"text/plain; charset=utf-8" )
				.set_body( "Access without permissions prohibited")
				.done();

			// Mark the request as processed.
			return restinio::request_accepted();
		}
	}

	return restinio::request_not_handled();
}

auto create_request_handler()
{
	auto router = std::make_shared< express_router_t >();

	router->http_get(
		"/",
		[]( const auto & req, const auto & ) {
			init_resp( req->create_response() )
				.append_header(
					restinio::http_field::content_type,
					"text/plain; charset=utf-8" )
				.set_body( "Hello world!")
				.done();

			return restinio::request_accepted();
		} );

	router->http_get(
		"/json",
		[]( const auto & req, const auto & ) {
			init_resp( req->create_response() )
				.append_header(
					restinio::http_field::content_type,
					"application/json" )
				.set_body( R"-({"message" : "Hello world!"})-")
				.done();

			return restinio::request_accepted();
		} );

	router->http_get(
		"/html",
		[]( const auto & req, const auto & ) {
			init_resp( req->create_response() )
				.append_header(
					restinio::http_field::content_type,
					"text/html; charset=utf-8" )
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
				.append_header(
					restinio::http_field::content_type,
					"text/plain; charset=utf-8" )
				.set_body(
						fmt::format(
							RESTINIO_FMT_FORMAT_STRING( "Stats data for user #{}" ),
							req->extra_data().m_auth_result.m_identity.m_id ) )
				.done();

			return restinio::request_accepted();
		} );

	router->http_get(
		"/admin",
		[]( const auto & req, const auto & ) {
			init_resp( req->create_response() )
				.append_header(
					restinio::http_field::content_type,
					"text/html; charset=utf-8" )
				.set_body(
					fmt::format(
						RESTINIO_FMT_FORMAT_STRING(
							"<html>\r\n"
							"  <head><title>Admin panel for user #{}</title></head>\r\n"
							"  <body>\r\n"
							"    <center><h1>NOT IMPLEMENTED YET</h1></center>\r\n"
							"  </body>\r\n"
							"</html>\r\n" ),
						req->extra_data().m_auth_result.m_identity.m_id ) )
				.done();

			return restinio::request_accepted();
		} );

	return [handler = std::move(router)]( const auto & req ) {
		return (*handler)( req );
	};
}

struct server_traits_t : public restinio::default_single_thread_traits_t
{
	using logger_t = restinio::single_threaded_ostream_logger_t;
	using extra_data_factory_t = per_request_data_factory_t;
	using request_handler_t = restinio::sync_chain::growable_size_chain_t<
		per_request_data_factory_t >;
};

int main()
{
	using namespace std::chrono;

	try
	{
		// Build the chain of handlers.
		restinio::sync_chain::growable_size_chain_t<
			per_request_data_factory_t >::builder_t chain_builder;

		chain_builder.add( credentials_handler );
		chain_builder.add( authorization_handler );
		chain_builder.add( create_request_handler() );

		// Run the server.
		restinio::run(
			restinio::on_this_thread<server_traits_t>()
				.port( 8080 )
				.address( "localhost" )
				.request_handler( chain_builder.release() ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

