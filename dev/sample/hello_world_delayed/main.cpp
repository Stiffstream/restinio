#include <iostream>

#include <restinio/all.hpp>

// Actual request handler.
restinio::request_handling_status_t handler(
	restinio::asio_ns::io_context & ioctx,
	const restinio::request_handle_t& req)
{
	if( restinio::http_method_get() == req->header().method() &&
		req->header().request_target() == "/" )
	{
		// Delay request processing to 2.5s.
		auto timer = std::make_shared<restinio::asio_ns::steady_timer>( ioctx );
		timer->expires_after( std::chrono::milliseconds(2500) );
		timer->async_wait( [timer, req](const auto & ec) {
				if( !ec ) {
					req->create_response()
						.append_header( restinio::http_field::server, "RESTinio hello world server" )
						.append_header_date_field()
						.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
						.set_body( "Hello world!")
						.done();
				}
			} );

		return restinio::request_accepted();
	}

	return restinio::request_rejected();
}

int main()
{
	try
	{
		// External io_context is necessary because we should have access to it.
		restinio::asio_ns::io_context ioctx;

		restinio::run(
			ioctx,
			restinio::on_thread_pool( std::thread::hardware_concurrency() )
				.port( 8080 )
				.address( "localhost" )
				.request_handler( [&ioctx](auto req) {
						return handler( ioctx, std::move(req) );
					} )
		);
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

