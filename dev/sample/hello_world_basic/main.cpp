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
					.append_header( "Server", "RESTinio hello world server" )
					.append_header_date_field()
					.append_header( "Content-Type", "text/plain; charset=utf-8" )
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
		using http_server_t = restinio::http_server_t<>;

		http_server_t http_server{
			restinio::create_child_io_context( 1 ),
			[]( auto & settings ){
				settings
					.port( 8080 )
					.address( "localhost" )
					.request_handler( request_handler() );
			}
		};

		// Start server.
		http_server.open();

		// Wait for quit command.
		std::cout << "Type \"quit\" or \"q\" to quit." << std::endl;
		std::string cmd;
		do
		{
			std::cin >> cmd;
		} while( cmd != "quit" && cmd != "q" );

		// Stop server.
		http_server.close();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
