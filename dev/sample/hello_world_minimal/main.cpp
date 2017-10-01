#include <iostream>
#include <restinio/all.hpp>

int main()
{
	restinio::http_server_t<> http_server{
		restinio::create_child_io_context(1),
		restinio::server_settings_t<>{}
			.port(8080)
			.address("localhost")
			.request_handler([](auto req) {
				req->create_response().set_body("Hello, World!").done();
				return restinio::request_accepted();
			})
		};

	http_server.open();
	std::cin.ignore();
	http_server.close();

	return 0;
}
