#include <iostream>
#include <restinio/all.hpp>

#include <restinio/timertt_timer_factory.hpp>

int main()
{
	using traits_t = restinio::single_thread_traits_t<
			restinio::st_timertt_wheel_timer_factory_t,
			restinio::null_logger_t >;

	restinio::run(
		restinio::on_this_thread< traits_t >()
			.port(8080)
			.address("localhost")
			.request_handler([](auto req) {
				return req->create_response().set_body("Hello, World!").done();
			}));

	return 0;
}
