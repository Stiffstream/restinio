#include <iostream>

#include <restinio/all.hpp>

// Create request handler.
restinio::request_handling_status_t handler( const restinio::request_handle_t& req )
{
	if( restinio::http_method_get() == req->header().method() )
	{
		fmt::basic_memory_buffer< char, 1u > response_body;
		fmt::format_to( response_body, "GET request to '{}'\n",
				req->header().request_target() );

		// Request header fields.
		fmt::format_to( response_body, "HTTP-fields ({}):\n",
				req->header().fields_count() );
		for( const auto & f : req->header() )
		{
			fmt::format_to( response_body, "{}: {}\n",
					f.name(), f.value() );
		}

		// Query params.
		const auto qp = restinio::parse_query( req->header().query() );

		if( qp.empty() )
		{
			fmt::format_to( response_body, "No query parameters." );
		}
		else
		{
			fmt::format_to( response_body, "Query params ({}):\n", qp.size() );

			for( const auto p : qp )
			{
				fmt::format_to( response_body, "'{}' => '{}'\n",
						p.first, p.second );
			}
		}

		if( qp.has( "debug" ) && qp[ "debug" ] == "true" )
		{
			std::cout.write( response_body.data(), response_body.size() );
			std::cout << std::flush;
		}

		req->create_response()
			.append_header( restinio::http_field::server, "RESTinio query string params server" )
			.append_header_date_field()
			.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
			.set_body( std::move(response_body) )
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
