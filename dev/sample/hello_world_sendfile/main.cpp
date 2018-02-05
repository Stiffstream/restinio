#include <iostream>

#include <restinio/all.hpp>

// Create request handler.
restinio::request_handling_status_t handler(restinio::request_handle_t req)
{
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
}

using http_server_traits_t =
	restinio::traits_t<
		restinio::asio_timer_manager_t,
		restinio::shared_ostream_logger_t >;

int main( int argc, const char * argv[] )
{
	try
	{
		if( 2 == argc )
		{
			restinio::run(
				restinio::on_thread_pool< http_server_traits_t >(
							std::thread::hardware_concurrency() )
					.port( 8080 )
					.address( "localhost" )
					.request_handler(
						[&]( auto req ){
							if( restinio::http_method_get() == req->header().method() &&
								req->header().request_target() == "/" )
							{
								req->create_response()
									.append_header( restinio::http_field::server, "RESTinio hello world server" )
									.append_header_date_field()
									.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
									.set_body(
										restinio::sendfile( argv[ 1 ] )
											.offset_and_size( 0, 20 )
											.chunk_size( 1024 ) )
									.done();

								return restinio::request_accepted();
							}

							return restinio::request_rejected();
					} ) );
		}
		else
		{
			std::cout << "Usage: hello_world_sendfile filepath" << std::endl;
			return 1;
		}
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
