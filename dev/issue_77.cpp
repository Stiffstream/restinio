#include <cstdint>
#include <ostream>

#include <restinio/all.hpp>

std::string content()
{
  const static int WORDS_COUNT = 20000;  // Change to 10000+ will cause an incomplete body

  std::string text;
  for (auto i = 0; i < WORDS_COUNT; ++i)
  {
    text += fmt::format( "Some text {}\n", i );
  }
  return text;
}

class Body : public std::string
{
public:
  Body(std::function<void()> callback) : std::string(content()), m_callback(callback)
  {
  }

  ~Body()
  {
    m_callback();
  }

private:
  std::function<void()> m_callback;
};

int main()
{
  using Traits = restinio::default_single_thread_traits_t;
  using Server = restinio::http_server_t<Traits>;

  std::unique_ptr<Server> server;

  auto shutdown = [&server]() {
    server->close_sync();
    server->io_context().stop();
  };

  server = std::make_unique<restinio::http_server_t<Traits>>(restinio::own_io_context(),
    restinio::run_on_this_thread_settings_t<Traits>().port(8888).address("0.0.0.0").request_handler([shutdown](auto req) {
      req->create_response().set_body(std::make_shared<Body>([shutdown]() { shutdown(); })).done();
      return restinio::request_accepted();
    }));

  server->open_sync();
  server->io_context().run();

  return 0;
}
