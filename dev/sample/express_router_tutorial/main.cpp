#include <iostream>

#include <restinio/all.hpp>

#include <fmt/format.h>


namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

template < typename RESP >
RESP
init_resp( RESP resp )
{
	resp.append_header( "Server", "RESTinio sample server /v.0.4" );
	resp.append_header_date_field()
	.append_header( "Content-Type", "text/plain; charset=utf-8" );

	return resp;
}

auto server_handler()
{
	auto router = std::make_unique< router_t >();

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

	// GET request with single parameter.
	router->http_get( "/single/:param", []( auto req, auto params ){
		return
			init_resp( req->create_response() )
				.set_body(
					fmt::format(
						"GET request with single parameter: '{}'",
						params[ "param" ] ) )
				.done();
	} );

	// POST request with several parameters.
	router->http_post( R"(/many/:year(\d{4}).:month(\d{2}).:day(\d{2}))",
		[]( auto req, auto params ){
			return
				init_resp( req->create_response() )
					.set_body(
						fmt::format(
							"POST request with many parameters:\n"
							"year: {}\nmonth: {}\nday: {}\nbody: {}",
							params[ "year" ],
							params[ "month" ],
							params[ "day" ],
							req->body() ) )
					.done();
		} );

	// GET request with indexed parameters.
	router->http_get( R"(/indexed/([a-z]+)-(\d+)/(one|two|three))",
		[]( auto req, auto params ){
			return
				init_resp( req->create_response() )
					.set_body(
						fmt::format(
							"GET request with indexed parameters:\n"
							"#0: '{}'\n#1: {}\n#2: '{}'",
							params[ 0 ],
							params[ 1 ],
							params[ 2 ] ) )
					.done();
		} );

	// GET request with indexed parameters.
	router->http_get( R"(/query/params)",
		[]( auto req, auto ){

			std::ostringstream sout;
			sout << "GET request with query params:\n";

			// Query params.
			const auto qp = restinio::parse_query( req->header().query() );

			if( qp.empty() )
			{
				sout << "No query parameters.";
			}
			else
			{
				sout << "Query params ("<< qp.size() << "):\n";
				// p is a pair of string_view_t, so copy is cheap.
				for( const auto p : qp )
				{
					sout << "'"<< p.first << "' => "<<  p.second << "'\n";
				}
			}

			return
				init_resp( req->create_response() )
					.set_body( sout.str() )
					.done();
		} );

	router->non_matched_request_handler(
		[]( auto req ){
			return
				req->create_response( restinio::status_not_found() )
					.append_header_date_field()
					.connection_close()
					.done();
		} );

	return router;
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
				router_t >;

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
