#include <iostream>

#include <restinio/all.hpp>

// Create request handler.
restinio::request_handling_status_t handler( restinio::request_handle_t req )
{
	if( restinio::http_method_get() == req->header().method() )
	{
		std::ostringstream sout;
		sout << "GET request to '" << req->header().request_target() << "'\n";

		// Query params.
		const auto qp = restinio::parse_query( req->header().query() );

		if( 0 == qp.size() )
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

		if( qp.has( "debug" ) && qp[ "debug" ] == "true" )
		{
			std::cout << sout.str() << std::endl;
		}

		req->create_response()
			.append_header( restinio::http_field::server, "RESTinio query string params server" )
			.append_header_date_field()
			.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
			.set_body( sout.str() )
			.done();

		return restinio::request_accepted();
	}

	return restinio::request_rejected();
}

int main()
{
	try
	{
		restinio::run(
			restinio::on_thread_pool( std::thread::hardware_concurrency() )
				.port( 8080 )
				.address( "localhost" )
				.request_handler( handler ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
