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

using namespace std::chrono_literals;

int main()
{
	try
	{
		struct my_server_traits_t : public restinio::default_traits_t
		{
			using logger_t = restinio::shared_ostream_logger_t;
		};

		using server_t = restinio::http_server_t< my_server_traits_t >;

		server_t server{
			restinio::own_io_context(),
			restinio::server_settings_t< my_server_traits_t >{}
				.port( 8080 )
				.address( "localhost" )
				.request_handler( handler )
		};

		// Run server on a separate thread_pool.
		restinio::on_pool_runner_t< server_t > runner{
			std::thread::hardware_concurrency(),
			server
		};
		runner.start();

		// Initiate a countdown.
		auto finish_time = std::chrono::steady_clock::now() + 1min;
		do {
			const auto now = std::chrono::steady_clock::now();
			if( now < finish_time )
			{
				std::cout << std::chrono::duration_cast<std::chrono::seconds>(
						finish_time - now).count() << "s left" << std::endl;

				std::this_thread::sleep_for( 1s );
			}
		}
		while( finish_time > std::chrono::steady_clock::now() );

		// The server will be stopped automatically.
		// Or it can be stopped manually by:
		// runner.stop();
		// runner.wait();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
