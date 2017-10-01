#include <iostream>

#include <restinio/all.hpp>

// Create request handler.
auto request_handler()
{
	return []( auto req ) {
			if( restinio::http_method_get() == req->header().method() &&
				req->header().request_target() == "/" )
			{
				req->create_response()
					.append_header( restinio::http_field::server, "RESTinio hello world server" )
					.append_header_date_field()
					.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
					.set_body( "Hello world!")
					.done();

				return restinio::request_accepted();
			}

			return restinio::request_rejected();
		};
}

int main()
{
	try
	{
		restinio::run(
			1,
			restinio::server_settings_t<>{}
				.port( 8080 )
				.address( "localhost" )
				.request_handler( request_handler() ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
