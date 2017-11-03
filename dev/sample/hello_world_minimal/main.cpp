#include <restinio/all.hpp>

int main()
{
	restinio::run(
		restinio::on_this_thread<>()
			.port(8080)
			.address("localhost")
			.request_handler([](auto req) {
				return req->create_response().set_body("Hello, World!").done();
			}));

	return 0;
}
