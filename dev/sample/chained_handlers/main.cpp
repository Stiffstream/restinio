#include <iostream>

#include <restinio/all.hpp>

#include <restinio/sync_chain/fixed_size.hpp>

#include <restinio/helpers/http_field_parsers/basic_auth.hpp>
#include <restinio/helpers/http_field_parsers/try_parse_field.hpp>

using express_router_t = restinio::router::express_router_t<>;

template < typename RESP >
RESP
init_resp( RESP resp )
{
	resp.append_header( restinio::http_field::server, "RESTinio sample server /v.0.6" );
	resp.append_header_date_field();

	return resp;
}

auto create_auth_handler()
{
	auto auth_checker = []( const auto & req, const auto & ) {
		// Try to parse Authorization header.
		using namespace restinio::http_field_parsers::basic_auth;

		const auto result = try_extract_params( *req,
				restinio::http_field::authorization );
		if( result )
		{
			// Authorization header is present and contains some value.
			if( result->username == "user" &&
					result->password == "1234" )
			{
				// User authentificated. The further processing can be
				// delegated to the next handler in the chain.
				return restinio::request_not_handled();
			}
		}

		// Otherwise we ask for credentials.
		init_resp( req->create_response( restinio::status_unauthorized() ) )
			.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
			.append_header( restinio::http_field::www_authenticate,
					R"(Basic realm="Username/Password required", charset="utf-8")" )
			.set_body( "Unauthorized access forbidden")
			.done();

		// Mark the request as processed.
		return restinio::request_accepted();
	};

	auto router = std::make_shared< express_router_t >();

	router->http_get( "/stats", auth_checker );
	router->http_get( "/admin", auth_checker );

	return [handler = std::move(router)]( const auto & req ) {
		return (*handler)( req );
	};
}

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

	return [handler = std::move(router)]( const auto & req ) {
		return (*handler)( req );
	};
}

int main()
{
	using namespace std::chrono;

	try
	{
		using traits_t =
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				restinio::single_threaded_ostream_logger_t,
				restinio::sync_chain::fixed_size_chain_t<2> >;

		restinio::run(
			restinio::on_this_thread<traits_t>()
				.port( 8080 )
				.address( "localhost" )
				.request_handler(
					create_auth_handler(),
					create_request_handler() ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

