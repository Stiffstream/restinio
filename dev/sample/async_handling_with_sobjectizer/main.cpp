#include <restinio/all.hpp>

#include <so_5/all.hpp>

#include <random>

// Message for transfer requests from RESTinio's thread to processing thread.
struct handle_request
{
	restinio::request_handle_t m_req;
};

// Message for delaying actual requests processing.
struct timeout_elapsed
{
	restinio::request_handle_t m_req;
	std::chrono::milliseconds m_pause;
};

void processing_thread_func(const so_5::mchain_t& req_ch)
{
	// The stuff necessary for random pause generation.
	std::random_device rd;
	std::mt19937 generator{rd()};
	std::uniform_int_distribution<> pause_generator{350, 3500};

	// The channel for delayed timeout_elapsed messages.
	auto delayed_ch = so_5::create_mchain(req_ch->environment());

	// This flag will be set to 'true' when some of channels will be closed.
	bool stop = false;
	select(
		so_5::from_all()
			// If some channel become closed we should set out 'stop' flag.
			.on_close([&stop](const auto &) { stop = true; })
			// A predicate for stopping select() function.
			.stop_on([&stop]{ return stop; }),

		// Read and handle handle_request messages from req_ch.
		case_(req_ch,
			[&](const handle_request& cmd) {
				// Generate a random pause for processing of that request.
				const std::chrono::milliseconds pause{pause_generator(generator)};

				// Delay processing of that request by using delayed message.
				so_5::send_delayed<timeout_elapsed>(delayed_ch,
						// This the pause for send_delayed.
						pause,
						// The further arguments are going to timeout_elapsed's
						// constructor.
						cmd.m_req,
						pause);
			}),

		// Read and handle timeout_elapsed messages from delayed_ch.
		case_(delayed_ch,
			[](const timeout_elapsed& cmd) {
				// Now we can create an actual response to the request.
				cmd.m_req->create_response()
						.set_body("Hello, World! (pause:"
								+ std::to_string(cmd.m_pause.count())
								+ "ms)")
						.done();
			})
	);
}

int main()
{
	// Launching SObjectizer on a separate thread.
	// There is no need to start and shutdown SObjectize:
	// the wrapped_env_t instance does it automatically.
	so_5::wrapped_env_t sobj;

	// Thread object for processing thread.
	std::thread processing_thread;
	// This thread should be automatically joined at exit
	// (this is necessary for exception safety).
	auto processing_thread_joiner = so_5::auto_join(processing_thread);

	// A channel for sending requests from RESTinio's thread to
	// separate processing thread.
	auto req_ch = so_5::create_mchain(sobj);
	// This channel should be automatically closed at scope exit
	// (this is necessary for exception safety).
	auto ch_closer = so_5::auto_close_drop_content(req_ch);

	// Now we can start processing thread.
	// If some exception will be thrown somewhere later the thread
	// will be automatically stopped and joined.
	processing_thread = std::thread{
			processing_thread_func, req_ch
	};

	// Traits for our simple server.
	struct traits_t : public restinio::default_traits_t
	{
		using logger_t = restinio::shared_ostream_logger_t;
	};

	restinio::run(
		restinio::on_this_thread<traits_t>()
			.port(8080)
			.address("localhost")
			.request_handler([req_ch](auto req) {
				// Handle only HTTP GET requests for the root.
				if(restinio::http_method_get() == req->header().method() &&
						"/" == req->header().path())
				{
					// Request will be delegated to the actual processing.
					so_5::send<handle_request>(req_ch, req);
					return restinio::request_accepted();
				}
				else
					return restinio::request_rejected();
			})
			.cleanup_func([&] {
				// Processing thread needs to be closed.
				// It is better to do it manually because there can
				// be requests waiting in req_ch.
				so_5::close_drop_content(req_ch);
			}));

	return 0;
}

