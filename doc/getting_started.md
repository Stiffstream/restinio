# Getting started

To start using *RESTinio* make sure that all dependencies are available.
The tricky one is [nodejs/http-parser](https://github.com/nodejs/http-parser),
because it is a to be compiled unit, which can be built as a static library and
linked to your target or can be a part of your target.
And it is worth to mention that *asio* that is used by *RESTinio* is the one from
github [repository](https://github.com/chriskohlhoff/asio.git).

To start using *RESTinio* simply include `<restinio/all.hpp>` header.
It includes all necessary headers of the library.

It easy to learn how to use *RESTinio* by example.

## Minimalistic hello world

Here is a minimal hello world http server
([see full example](../dev/sample/hello_world_minimal/main.cpp)):
~~~~~
::c++
#include <iostream>
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
~~~~~

Here a helper function `restinio::run()` is used to create and run the server.
It is the easiest way to start the server,
it hides some boilerplate code for simple common cases.
*RESTinio* considers the following two typical cases:

* run server on current thread;
* run server on thread pool.

See details [here](../dev/restinio/http_server_run.hpp).

Each `restinio::run()` function creates http server instance with specified settings
and runs it. And for also it subscribes to breakflag signals to stop the server
when ctrl+c is hit.

*RESTinio* does its networking stuff with
[asio](https://github.com/chriskohlhoff/asio.git) library, so to run server
it must have an `asio::io_context` instance to run on.
Each `restinio::run()` function creates an instance of `asio::io_context`
and then runs it (via `asio::io_context::run()` function) on a current thread or
on a thread pool.

`restinio::run()` functions receive server settings as an argument.
Server settings is a [fluent interface](https://en.wikipedia.org/wiki/Fluent_interface)
class for various server options. Most of the settings have reasonable default values.
And one setting is especially important: it is `request_handler`.
It is a function-object that handles http requests.
In the sample above it is lambda that serves all request with "Hello, World!" response.
Other two option in the sample specify to run server on localhost
and listen for connections on port 8080.

## Enhance request handler

Lets see a more complicated request handler and look at what it does.

As request handler is a function-object we will use a function to create one:
~~~~~
::c++
// Create request handler.
auto create_request_handler()
{
	return []( auto req ) {
			if( restinio::http_method_get() == req->header().method() &&
				req->header().request_target() == "/" )
			{
				req->create_response()
					.append_header( restinio::http_field::server, "RESTinio hello world server" )
					.append_header_date_field()
					.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
					.set_body( "Hello world!")
					.done();

				return restinio::request_accepted();
			}

			return restinio::request_rejected();
		};
}
~~~~~

Function `create_request_handler()` creates a lambda function
with the signature that later becomes the following (with concrete types):
~~~~~
::c++
restinio::request_handling_status_t func( restinio::request_handle_t req );
~~~~~

Function has a single parameter that holds a hanle on actual request - `restinio::request_t`
that has the following API (details are omitted):
~~~~~
::c++
/! HTTP Request data.
/*!
	Provides acces to header and body, and creates response builder
	for a given request.
*/
class request_t // Base class omitted.
{
	// Internals and implementation omitted.
	public:
		//! Get request header.
		const http_request_header_t &
		header() const;

		//! Get request body.
		const std::string &
		body() const;

		//! Create response.
		template < typename Output = restinio_controlled_output_t >
		auto
		create_response(
			std::uint16_t status_code = 200,
			std::string reason_phrase = "OK" );
};
~~~~~

* `request_t::header()` gives access to `http_request_header_t` object that describes
http header of a given request;
* `request_t::body()` gives access to body of a given request;
* `request_t::create_response()` creates an object for cunstructing and sending request.

First two functions are rather straightforward,
so for them there is no much sense to go into more details.
A more intriguing function is `request_t::create_response()` -
it is a template function that is customized with `Output` type.
For now it would be enough to use the default customization.
`request_t::create_response()` has two parameter *status code* (http response code)
and *reason phrase* (http response reason phrase).
By default these params are set for ` HTTP/1.1 200 OK` response.
If called for the first time `request_t::create_response()`
returns an instance of type `response_builder_t<Output>`,
it is an object for constructing and sending response on a given request.
To avoid the possibility of creating multiple responses on a single request
`request_t::create_response()` will return an object only for the first call
all further call would throw.

For setting response in the sample above we use some functions  of a response builder:

* `append_header( field, value )` - for setting header fields;
* `append_header_date_field()` - for setting `Date` field value with current timmestamp;
* `set_body()` - for setting response body;

All of mentioned functions return the reference to a response builder tha gives
some syntactic sugar and allows to set response in a nice way.

For sending response to peer a `done()` function is used.
It stops constructing response
(no setters will have an effect on the response) and initiates sending
a response via underlying tcp connection.

A one thing left to mention is what request handler returns.
Any request handler must return a value of `request_handling_status_t` enum:
~~~~~
::c++
enum class request_handling_status_t : std::uint8_t
{
	//! Request accepted for handling.
	accepted,

	//! Request wasn't accepted for handling.
	rejected
};
~~~~~

There are two helper functions:
`restinio::request_accepted()` and `restinio::request_rejected()`
for refering an itemes of enum. Both of them are used in the sample.

See also a full ([sample](../dev/sample/hello_world_basic/main.cpp)).

## Enhance request handler even more

One of the reasons to create *RESTinio* was an ability to have
[express](https://expressjs.com/)-like request handler router.

Since v 0.2.1 *RESTinio* has a router based on idea borrowed
from [express](https://expressjs.com/) - a JavaScript framework.

Routers acts as a request handler (it means it is a function-object
that can be called as a request handler).
But router aggregates several handlers and picks one or none of them to handle the request.
The choice of the handler to execute depends on request target and HTTP method.
If router finds no handler matching request then it rejects it.
There is a difference between ordinary restinio request handler
and the one that is used with experss router and is bound to concrete endpoint.
The signature of a handlers that can be put in router
has an additional parameter -- a container with parameters extracted from URI.

Express router is defined by `express_router_t` class.
Its implementation is inspired by
[express-router](https://expressjs.com/en/starter/basic-routing.html).
It allows to define route path with injection of parameters that become available for handlers.
For example the following code sets a handler with 2 parameters:
~~~~~
::c++
  router.http_get(
    R"(/article/:article_id/:page(\d+))",
    []( auto req, auto params ){
      const auto article_id = params[ "article_id" ];
      auto page = std::to_string( params[ "page" ] );
      // ...
    } );
~~~~~

The first parameter is still request handle, and the second parameter is
an instance of `route_params_t` that holds parameters of the request.

Lets enhance how we can use express router in a sample request handle:
~~~~~
::c++
using router_t = restinio::router::express_router_t;

auto create_server_handler()
{
	auto router = std::make_unique< router_t >();

	router->http_get(
		"/",
		[]( auto req, auto ){
				init_resp( req->create_response() )
					.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
					.set_body( "Hello world!")
					.done();

				return restinio::request_accepted();
		} );

	router->http_get(
		"/json",
		[]( auto req, auto ){
				init_resp( req->create_response() )
					.append_header( restinio::http_field::content_type, "text/json; charset=utf-8" )
					.set_body( R"-({"message" : "Hello world!"})-")
					.done();

				return restinio::request_accepted();
		} );

	router->http_get(
		"/html",
		[]( auto req, auto ){
				init_resp( req->create_response() )
						.append_header( restinio::http_field::content_type, "text/html; charset=utf-8" )
						.set_body(
							"<html>\r\n"
							"  <head><title>Hello from RESTinio!</title></head>\r\n"
							"  <body>\r\n"
							"    <center><h1>Hello world</h1></center>\r\n"
							"  </body>\r\n"
							"</html>\r\n" )
						.done();

				return restinio::request_accepted();
		} );


	return router;
}
~~~~~

Function `create_server_handler()` creates an instance of
`restinio::router::express_router_t` with thre endpoints:

* '/' - default path: reply with text hello message;
* '/json': reply with json hello message;
* '/html': reply with html hello message.

Note in the sample above we do not use route parameters.

Considering implementation of `create_server_handler()` above,
we can notice that it return a unique pointer on a router class.
And it is not function object. So how *RESTinio* can use it?
To receive an accurate answer one should read
[basic idea](./doc/basic_idea.md) page first.

A brief and non accurate answer will be that
"RESTinio" is customizable for concrete types of handler,
and if it knows a concrete type of a handler it can receive
it wrapped in unique pointer. To set this (and not only this) customization
a traits calss is used.
In samples in prevous sections a default traits were used, so they were hidden.
In this sample we explicitly define a traits to use:
~~~~~
::c++
using router_t = restinio::router::express_router_t;

using traits_t =
	restinio::traits_t<
		restinio::asio_timer_factory_t,
		restinio::single_threaded_ostream_logger_t,
		router_t >;
~~~~~

Here we use a helper class `restinio::traits_t`
that has some customizations defined by default.
Request handler type is on the third place.

And to run the server we need to point the traits we are using:
~~~~~
::c++
restinio::run(
	restinio::on_this_thread<traits_t>() // Use custom traits.
		.port( 8080 )
		.address( "localhost" )
		.request_handler( create_request_handler() );
~~~~~

Function `restinio::on_this_thread<Traits>()` involves a creation of a special type
and from this type `restinio::run()` function deduces the trats for its server.

See also a full ([sample](../dev/sample/hello_world_basic/main.cpp)).

