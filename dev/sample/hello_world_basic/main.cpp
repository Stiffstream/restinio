#include <iostream>

#include <restinio/all.hpp>

// Create request handler.
restinio::request_handling_status_t handler( const restinio::request_handle_t& req )
{
	if( restinio::http_method_get() == req->header().method() &&
		req->header().request_target() == "/" )
	{
		req->create_response()
			.append_header( restinio::http_field::server, "RESTinio hello world server" )
			.append_header_date_field()
			.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
			.set_body( fmt::format( "{}: Hello world!", req->remote_endpoint() ) )
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
