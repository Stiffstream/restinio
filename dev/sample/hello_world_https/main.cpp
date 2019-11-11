#include <iostream>

#include <restinio/all.hpp>
#include <restinio/tls.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>


template < typename RESP >
RESP
init_resp( RESP resp )
{
	resp.append_header( restinio::http_field::server, "RESTinio sample server /v.0.2" );
	resp.append_header_date_field();

	return resp;
}

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

auto server_handler()
{
	auto router = std::make_unique< router_t >();

	router->http_get(
		"/",
		[]( auto req, auto ){
				init_resp( req->create_response() )
					.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
					.set_body( "Hello world!")
					.done();

				return restinio::request_accepted();
		} );

	router->http_get(
		"/json",
		[]( auto req, auto ){
				init_resp( req->create_response() )
					.append_header( restinio::http_field::content_type, "application/json" )
					.set_body( R"-({"message" : "Hello world!"})-")
					.done();

				return restinio::request_accepted();
		} );

	router->http_get(
		"/html",
		[]( auto req, auto ){
				init_resp( req->create_response() )
						.append_header( restinio::http_field::content_type, "text/html; charset=utf-8" )
						.set_body(
R"-(<html>
<head><title>Hello from RESTinio!</title></head>
<body>
<center><h1>Hello world</h1></center>
</body>
</html>)-" )
						.done();

				return restinio::request_accepted();
		} );

	return router;
}

int main( int argc, const char * argv[] )
{
	using namespace std::chrono;

	std::string certs_dir = ".";

	if( 2 == argc )
	{
		certs_dir = argv[ 1 ];
	}

	try
	{
		using traits_t =
			restinio::single_thread_tls_traits_t<
				restinio::asio_timer_manager_t,
				restinio::single_threaded_ostream_logger_t,
				router_t >;

		// Since RESTinio supports both stand-alone ASIO and boost::ASIO
		// we specify an alias for a concrete asio namesace.
		// That's makes it possible to compile the code in both cases.
		// Typicaly only one of ASIO variants would be used,
		// and so only asio::* or only boost::asio::* would be applied.
		namespace asio_ns = restinio::asio_ns;

		asio_ns::ssl::context tls_context{ asio_ns::ssl::context::sslv23 };
		tls_context.set_options(
			asio_ns::ssl::context::default_workarounds
			| asio_ns::ssl::context::no_sslv2
			| asio_ns::ssl::context::single_dh_use );

		tls_context.use_certificate_chain_file( certs_dir + "/server.pem" );
		tls_context.use_private_key_file(
			certs_dir + "/key.pem",
			asio_ns::ssl::context::pem );
		tls_context.use_tmp_dh_file( certs_dir + "/dh2048.pem" );

		restinio::run(
			restinio::on_this_thread< traits_t >()
				.address( "localhost" )
				.request_handler( server_handler() )
				.read_next_http_message_timelimit( 10s )
				.write_http_response_timelimit( 1s )
				.handle_request_timeout( 1s )
				.tls_context( std::move( tls_context ) ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
