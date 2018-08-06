#include <iostream>

#include <restinio/all.hpp>

std::atomic< int > g_failed_responses{ 0 };

// Create request handler.
restinio::request_handling_status_t handler( restinio::request_handle_t req )
{
	if( restinio::http_method_get() == req->header().method() &&
		req->header().request_target() == "/" )
	{
		std::thread handler_thread{ [ req = std::move( req ) ]{

			// Simulate some delay.
			std::this_thread::sleep_for( std::chrono::seconds( 5 ) );

			req->create_response()
				.append_header( restinio::http_field::server, "RESTinio hello world server" )
				.append_header_date_field()
				.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
				.set_body( "Notificators: Hello world !")
				.done( []( const auto & ec ){
					if( ec )
					{
						// Increment the number of failed requests.
						++g_failed_responses;

					}
				} );
		} };

		handler_thread.detach();

		return restinio::request_accepted();
	}

	return restinio::request_rejected();
}

int main()
{
	try
	{
		std::thread failed_responses_printer{
			[]{
				int last_printed_failed_responses{ 0 };

				while( true )
				{
					std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

					if( -1 == g_failed_responses ) return;

					if( last_printed_failed_responses < g_failed_responses )
					{
						last_printed_failed_responses = g_failed_responses;

						std::cout << "Failed responses count: "
							<< last_printed_failed_responses << std::endl;
					}
				}
			} };

		restinio::run(
			restinio::on_thread_pool( std::thread::hardware_concurrency() )
				.port( 8080 )
				.address( "localhost" )
				.max_pipelined_requests( 4 )
				.request_handler( handler ) );

		std::cout << "Final failed responses count: " << g_failed_responses << std::endl;

		g_failed_responses = -1;
		failed_responses_printer.join();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
