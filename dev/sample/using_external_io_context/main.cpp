#include <iostream>

#include <restinio/all.hpp>

// Create request handler.
auto create_request_handler( const std::string& tag )
{
	return [tag]( auto req ) {
			if( restinio::http_method_get() == req->header().method() &&
				req->header().request_target() == "/" )
			{
				req->create_response()
					.append_header( restinio::http_field::server, "RESTinio hello world server" )
					.append_header_date_field()
					.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
					.set_body( "Server tag: " + tag )
					.done();

				return restinio::request_accepted();
			}

			return restinio::request_rejected();
		};
}

int main()
{
	// Since RESTinio supports both stand-alone ASIO and boost::ASIO
	// we specify an alias for a concrete asio namesace.
	// That's makes it possible to compile the code in both cases.
	// Typicaly only one of ASIO variants would be used,
	// and so only asio::* or only boost::asio::* would be applied.
	namespace asio_ns = restinio::asio_ns;

	try
	{
		// External io_context.
		asio_ns::io_context io_context;

		using server_t = restinio::http_server_t<>;
		using settings_t = restinio::server_settings_t<>;

		server_t srv1{
			restinio::external_io_context( io_context ),
			settings_t{}
				.port( 8080 )
				.address( "localhost" )
				.request_handler( create_request_handler( "server1" ) ) };

		server_t srv2{
			restinio::external_io_context( io_context ),
			settings_t{}
				.port( 8081 )
				.address( "localhost" )
				.request_handler( create_request_handler( "server2" ) ) };

		asio_ns::signal_set break_signals{ io_context, SIGINT };
		break_signals.async_wait(
			[&]( const asio_ns::error_code & ec, int ){
				if( !ec )
				{
					srv1.close_sync();
					srv2.close_sync();
				}
			} );

		asio_ns::post( io_context, [&]{
			srv1.open_sync();
			srv2.open_sync();
		} );

		io_context.run();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
