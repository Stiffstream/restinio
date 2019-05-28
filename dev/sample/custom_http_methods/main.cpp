#include <iostream>
#include <sstream>

#include <restinio/all.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>


namespace rr = restinio::router;
using my_router_t = rr::express_router_t<>;

template < typename RESP >
RESP
init_resp( RESP resp )
{
	resp.append_header( "Server", "RESTinio sample server /v.0.4" );
	resp.append_header_date_field()
	.append_header( "Content-Type", "text/plain; charset=utf-8" );

	return resp;
};

// Custom HTTP-methods.
constexpr const restinio::http_method_id_t method_listen{
		HTTP_LISTEN, "LISTEN" };
constexpr const restinio::http_method_id_t method_stats{
		HTTP_STATS, "STATS" };
constexpr const restinio::http_method_id_t method_sign{
		HTTP_SIGN, "SIGN" };
constexpr const restinio::http_method_id_t method_encrypt{
		HTTP_ENCRYPT, "ENCRYPT" };

// Customization for RESTinio.
struct custom_http_methods_t
{
	static constexpr restinio::http_method_id_t
	from_nodejs( int m ) noexcept
	{
		if( m == method_listen.raw_id() )
			return method_listen;
		else if( m == method_stats.raw_id() )
			return method_stats;
		else if( m == method_sign.raw_id() )
			return method_sign;
		else if( m == method_encrypt.raw_id() )
			return method_encrypt;
		else
			return restinio::default_http_methods_t::from_nodejs( m );
	}
};

auto server_handler()
{
	auto router = std::make_unique< my_router_t >();

	// GET request to homepage.
	router->http_get( "/", []( auto req, auto ){
		return
			init_resp( req->create_response() )
				.set_body( "GET request to the homepage.")
				.done();
	} );

	// POST request to homepage.
	router->http_post( "/", []( auto req, auto ){
		return
			init_resp( req->create_response() )
				.set_body( "POST request to the homepage.\nbody: " + req->body() )
				.done();
	} );

	// LISTEN request to homepage
	router->add_handler(
			method_listen,
			"/",
			[]( auto req, auto ){
		return
			init_resp( req->create_response() )
				.set_body( "LISTEN request to the homepage.")
				.done();
	} );

	// STATS request to homepage
	router->add_handler(
			method_stats,
			"/",
			[]( auto req, auto ){
		return
			init_resp( req->create_response() )
				.set_body( "STATS request to the homepage.")
				.done();
	} );

	// SIGN request to homepage
	router->add_handler(
			method_sign,
			"/",
			[]( auto req, auto ){
		return
			init_resp( req->create_response() )
				.set_body( "SIGN request to the homepage.")
				.done();
	} );

	// ENCRYPT request to homepage
	router->add_handler(
			method_encrypt,
			"/",
			[]( auto req, auto ){
		return
			init_resp( req->create_response() )
				.set_body( "ENCRYPT request to the homepage.")
				.done();
	} );

	return router;
}

int main()
{
	using namespace std::chrono;

	try
	{
		struct traits_t : public restinio::default_single_thread_traits_t
		{
			using http_methods_t = custom_http_methods_t;
			using logger_t = restinio::single_threaded_ostream_logger_t;
			using request_handler_t = my_router_t;
		};

		restinio::run(
			restinio::on_this_thread< traits_t >()
				.address( "localhost" )
				.request_handler( server_handler() )
				.read_next_http_message_timelimit( 10s )
				.write_http_response_timelimit( 1s )
				.handle_request_timeout( 1s ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

