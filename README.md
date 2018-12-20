# What Is RESTinio?

[![CodeFactor](https://www.codefactor.io/repository/github/moeryomenko/restinio/badge)](https://www.codefactor.io/repository/github/moeryomenko/restinio)

RESTinio is a header-only C++14 library that gives you an embedded
HTTP/Websocket server. It is based on standalone version of ASIO
and targeted primarily for asynchronous processing of HTTP-requests.
Since v.0.4.1 Boost::ASIO (1.66 or higher) is also supported
(see [notes on building with Boost::ASIO](https://stiffstream.com/en/docs/restinio/0.4/obtaining.html#building-with-boost)).

# A Very Basic Example Of RESTinio

Consider the task of writing a C++ application that must support some REST API,
RESTinio represents our solution for that task. Currently it is in stable beta state.
Lets see how it feels like in the simplest case:

```C++
#include <restinio/all.hpp>
int main()
{
    restinio::run(
        restinio::on_this_thread()
        .port(8080)
        .address("localhost")
        .request_handler([](auto req) {
            return req->create_response().set_body("Hello, World!").done();
        }));
    return 0;
}
```

Server runs on the main thread, and respond to all requests with hello-world
message. Of course you've got an access to the structure of a given HTTP request,
so you can apply a complex logic for handling requests.

# Features

* Async request handling. Cannot get the response data immediately? That's ok,
  store request handle somewhere and/or pass it to another execution context
  and get back to it when the data is ready.
* HTTP pipelining. Works well with async request handling.
  It might increase your server throughput dramatically.
* Timeout control. RESTinio can take care of bad connection that are like: send
  "GET /" and then just stuck.
* Response builders. Need chunked-encoded body - then RESTinio has a special
  response builder for you (obviously it is not the only builder).
* ExpressJS-like request routing (see an example below).
* Working with query string parameters.
* Supports sending files and its parts (with sendfile on linux/unix and TransmitFile on windows).
* Supports compression (deflate, gzip).
* Supports TLS (HTTPS).
* Basic websocket support. Simply restinio::websocket::basic::upgrade() the
  request handle and start websocket session on a corresponding connection.
* Can run on external asio::io_context. RESTinio is separated from execution
  context.
* Some tune options. One can set acceptor and socket options. When running
  RESTinio on a pool of threads connections can be accepted in parallel.

# Enhanced Example With Express Router

```C++
#include <restinio/all.hpp>

using namespace restinio;

template<typename T>
std::ostream & operator<<(std::ostream & to, const optional_t<T> & v) {
    if(v) to << *v;
    return to;
}

int main() {
    // Create express router for our service.
    auto router = std::make_unique<router::express_router_t<>>();
    router->http_get(
            R"(/data/meter/:meter_id(\d+))",
            [](auto req, auto params) {
                const auto qp = parse_query(req->header().query());
                return req->create_response()
                        .set_body(
                                fmt::format("meter_id={} (year={}/mon={}/day={})",
                                        cast_to<int>(params["meter_id"]),
                                        opt_value<int>(qp, "year"),
                                        opt_value<int>(qp, "mon"),
                                        opt_value<int>(qp, "day")))
                        .done();
            });

    router->non_matched_request_handler(
            [](auto req){
                return req->create_response( 404, "Not found").connection_close().done();
            });

    // Launching a server with custom traits.
    struct my_server_traits : public default_single_thread_traits_t {
        using request_handler_t = restinio::router::express_router_t<>;
    };

    restinio::run(
            restinio::on_this_thread<my_server_traits>()
                    .address("localhost")
                    .request_handler(std::move(router)));

    return 0;
}
```

# License

RESTinio is distributed under BSD-3-CLAUSE license.

# How To Use It?

The full documentation for RESTinio can be found [here](https://stiffstream.com/en/docs/restinio/0.4).

# More

* Issues and bugs:
[Issue Tracker on BitBucket](https://bitbucket.org/sobjectizerteam/restinio-0.4/issues).
