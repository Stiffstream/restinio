#include <iostream>
#include <restinio/all.hpp>

int main()
{
	restinio::run(
		1,
		restinio::server_settings_t<>{}
			.port(8080)
			.address("localhost")
			.request_handler([](auto req) {
				req->create_response().set_body("Hello, World!").done();
				return restinio::request_accepted();
			}));

	return 0;
}
