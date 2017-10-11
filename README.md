# What Is It?
*RESTinio* is a header-only library for creating REST applications in c++.
It helps to create http server that can handle requests asynchronously.
Currently it is in beta state and represents our solution for the problem of
being able to handle request asynchronously with additional features.

RESTinio is distributed on dual-license mode.
There is a GNU Affero GPL v.3 license for usage of RESTinio in OpenSource software.
There is also a commercial license for usage of RESTinio in proprietary projects
(contact "info at stiffstream dot com" for more information).

## Why creating yet another library of that kind?

Well, there are lots of libraries and frameworks
of all sorts of complexity and maturity for
building REST service in C++.
Isn't it really a [NIH syndrom](https://en.wikipedia.org/wiki/Not_invented_here)?

We've used some of already available libraries and have tried lots of them.
And we have found that pretty much of them lack an ability
to handle requests asynchronously.
Usually library design forces user to set the response
inside a handler call. So when handling needs some interactions
with async API such design results in blocking of a caller thread.
And that hurts when the rest of the application is built on async bases.

In addition to async handling feature we though it would be nice
for such library to keep track of what is going on with connections and
control timeouts on operations of reading request from socket,
handling request and writing response to socket.
And it would also be nice to have request handler router
(like in [express](https://expressjs.com/)).
And a header-only design is a plus.

And it happens that under such conditions you don't have a lot of options.
So we have come up with *RESTinio*...

# How to use it

* [Obtain And Build](./doc/obtain_and_build.md)
* [Getting started](./doc/getting_started.md)
* [Advanced doc](./doc/advanced_doc.md)

# Roadmap

The list of features for next releases

|       Feature        | description | release  |
|----------------------|-------------|----------|
| HTTP client | Introduce functionality for building and sending requests and receiving and parsing results. | ? |
| Improve router | Improve router with type conversion of parameters | ? |
| Compresion | Support compressed content | ? |
| Improve API | Make easy cases easy by hiding template stuff | ? |
| Raw response | Support raw response, when response message data is fully constructed in user domain. | ? |
| Client disconnect detection | In case client disconnects before response is ready, clean up connection. | ? |
| Full CMake support | Add missing cmake support for [SObjectizer](https://sourceforge.net/projects/sobjectizer/) samples | ? |

## Done features

|       Feature        | description | release  |
|----------------------|-------------|----------|
| Running server simplification | Add functions to deal with boilerplate code for running server in simple cases | 0.3.0 |
| Improve internal design | Redesign server start/stop logic | 0.3.0 |
| Web Sockets | Basic support for Web Sockets | 0.3.0 |
| Acceptor options | Custom options for socket can be set in settings. | 0.3.0 |
| Separate accept and create connection | creating connection instance that involves allocations and initialization can be done in a context that is independent to acceptors context. | 0.3.0 |
| Concurrent accept | Server can accept several client connections concurrently. | 0.3.0 |
| Add uri helpers | A number of functions to work with query string see [uri_helpers.hpp](./dev/restinio/uri_helpers.hpp). | 0.3.0 |
| Improve header fields API | Type/enum support for known header fields and their values. | 0.2.2 |
| TLS support | Sopport for HTTPS with OpenSSL | 0.2.2 |
| External buffers | Support external (constant) buffers support for body and/or body parts. | 0.2.2 |
| Benchmarks | Non trivial benchmarks. Comparison with other libraries with similar features on the range of various scenarios. | started independent [project](https://bitbucket.org/sobjectizerteam/restinio-benchmark-jun2017) |
| Routers for message handlers | Support for a URI dependent routing to a set of handlers (express-like router). | 0.2.1 |
| Bind localhost aliases | Accept "localhost" and "ip6-localhost" as address parameter for server to bound to.  | 0.2.1 |
| Chunked transfer encoding | Support for chunked transfer encoding. Separate responses on header and body chunks, so it will be possible to send header and then body divided on chunks. | 0.2.0 |
| HTTP pipelining | [HTTP pipelining](https://en.wikipedia.org/wiki/HTTP_pipelining) support. Read, parse and call a handler for incoming requests independently. When responses become available send them to client in order of corresponding requests. | 0.2.0 |
| Address binding | Bind server to specific ip address. | 0.1.0 |
| Timeout control | Enable timeout guards for operations of receiving request (read and parse complete request), handling request, write response | 0.1.0 |
| Logging | Support for logging of internal server work. | 0.1.0 |
| ASIO thread pool | Support ASIO running on a thread pool. | 0.1.0 |
| IPv6 | IPv6 support. | 0.1.0 |


# License

*RESTinio* is distributed under GNU Affero GPL v.3 license (see [LICENSE](./LICENSE) and [AGPL](./agpl-3.0.txt) files).

For the license of *asio* library see COPYING file in *asio* distributive.

For the license of *nodejs/http-parser* library
see LICENSE file in *nodejs/http-parser* distributive.

For the license of *fmtlib* see LICENSE file in *fmtlib* distributive.

For the license of *SObjectizer* library see LICENSE file in *SObjectizer* distributive.

For the license of *rapidjson* library see LICENSE file in *rapidjson* distributive.

For the license of *json_dto* library see LICENSE file in *json_dto* distributive.

For the license of *args* library see LICENSE file in *args* distributive.

For the license of *CATCH* library see LICENSE file in *CATCH* distributive.


IGNORE ALL THE FOLLOWING

IGNORE ALL THE FOLLOWING

IGNORE ALL THE FOLLOWING

IGNORE ALL THE FOLLOWING

IGNORE ALL THE FOLLOWING

IGNORE ALL THE FOLLOWING
IGNORE ALL THE FOLLOWING

## Getting started


Server settings are set with lambda.
It is more convenient to use generic lambda to omit
naming the concrete type of `server_settings_t<TRAITS>`,
but of course passing settings parameter by value is also possible.
View `server_settings_t` class to see all available params
([settings.hpp](./dev/restinio/settings.hpp)).

To run the server `open()` and `close()` methods are used.
When open method is called server posts a callback on
`asio::io_context` to bind on a server port and listen for new connections.
If callback executes without errors, open returns, in case of error
it throws. Note that in general open and close operations are executed
asynchronously so to make them sync a future-promise is used.

To make server complete we must set request handler.
Request handler as defined by default `TRATS` is a
`restinio::default_request_handler_t`:
~~~~~
::c++
using default_request_handler_t =
  std::function< request_handling_status_t ( request_handle_t ) >
~~~~~

To create handler a `request_handler()` function is used:
~~~~~
::c++
auto request_handler()
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

Request handler here is a lambda-function, it checks if request method is `GET`
and target is `/` and if so makes a response and returns
`restinio::request_handling_status_t::accepted`
meaning that request had been taken for processing.
Otherwise handler returns `restinio::request_handling_status_t::rejected` value
meaning that request was not accepted for handling and *RESTinio* must take care of it.

## Basic idea

## Class *http_server_t*

`http_server_t` is a template class parameterized with a single
template parameter: `TRAITS`.
Traits class must specify a set types used inside *RESTinio*,
they are:
~~~~~
::c++
timer_factory_t;
logger_t;
request_handler_t;
strand_t;
stream_socket_t;
~~~~~

It is easier to use `restinio::traits_t<>` helper type:
~~~~~
::c++
template <
    typename TIMER_FACTORY,
    typename LOGGER,
    typename REQUEST_HANDLER = default_request_handler_t,
    typename STRAND = asio::strand< asio::executor >,
    typename STREAM_SOCKET = asio::ip::tcp::socket >
struct traits_t;
~~~~~

Here is what `TRAITS` member type stand for:

* `timer_factory_t` defines the logic of how timeouts are managed;
* `logger_t` defines logger that is used by *RESTinio* to track
its inner logic;
* `request_handler_t` defines a function-like type to be used as
request handler;
* `strand_t` - defines a class that is used by connection as a wrapper
for its callback-handlers running on `asio::io_context` thread(s)
in order to guarantee serialized callbacks invocation
(see [asio doc](http://think-async.com/Asio/asio-1.11.0/doc/asio/reference/strand.html)).
Actually there are two options for the strand type:
`asio::strand< asio::executor >` and `asio::executor`;
* `stream_socket_t` is a customization point that tells restinio
what type of socket used for connections. This parameter allows restinio
to support TLS connection
(see [TLS support](#markdown-header-tls-support) section).

It is handy to consider `http_server_t<TRAITS>` class as a root class
for the rest of *RESTinio* ecosystem, because pretty much all of them are
also template types parameterized with the same `TRAITS` parameter.

Class `http_server_t<TRAITS>` has two constructors:
~~~~~
::c++
http_server_t(
  io_context io_context,
  server_settings_t<TRAITS> settings );

template < typename CONFIGURATOR >
http_server_t(
  io_context io_context,
  CONFIGURATOR && configurator )
~~~~~

The first is the main one. It obtains `io_context` as an
`asio::io_context` back-end and server settings with the bunch of params.

The second constructor simplifies setting of parameters via generic lambda:
~~~~~
::c++
http_server_t http_server{
  restinio::create_child_io_context( 1 ),
  []( auto & settings ){ // Omit concrete name of settings type.
    settings
      .port( 8080 )
      .read_next_http_message_timelimit( std::chrono::seconds( 1 ) )
      .handle_request_timeout( std::chrono::milliseconds( 3900 ) )
      .write_http_response_timelimit( std::chrono::milliseconds( 100 ) )
      .logger( /* logger params */ )
      .request_handler( /* request handler params */ );
  } };
~~~~~

### Running server

To run server there are open()/close() methods:
~~~~~
::c++
template <
    typename SRV_OPEN_OK_CALLBACK,
    typename SRV_OPEN_ERR_CALLBACK >
void
open_async(
  SRV_OPEN_OK_CALLBACK && open_ok_cb,
  SRV_OPEN_ERR_CALLBACK && open_err_cb );

void
open_sync();

// Shortcut for open_sync().
void
open();

template <
    typename SRV_CLOSE_OK_CALLBACK,
    typename SRV_CLOSE_ERR_CALLBACK >
void
close_async(
  SRV_CLOSE_OK_CALLBACK && close_ok_cb,
  SRV_CLOSE_ERR_CALLBACK && close_err_cb );

void
close_sync();

void
close();
~~~~~

Async versions post callback on `asio::io_context` thread
and once executed one of specified callbacks is called.
The first one in case of success, and the second - in case of error.

Sync version work through async version getting synchronized by
future-promise.

`http_server_t<TRAITS>` also has methods to start/stop running
`asio::io_context`:
~~~~~
::c++
void
start_io_context();

void
stop_io_context();
~~~~~

They are helpful when using async versions of open()/close() methods,
when running `asio::io_context` and running server must be managed separately.

## Settings of *http_server_t*

Class `server_settings_t<TRAITS>` serves to pass settings to `http_server_t`.
It is defined in [restinio/settings.hpp](./dev/restinio/settings.hpp);

For each parameter a setter/getter pair is provided.
While setting most of parameters is pretty straightforward,
there are some parameters with a bit tricky setter/getter semantics.
They are request_handler, timer_factory, logger, acceptor_options_setter and socket_options_setter.

For example setter for request_handler looks like this:
~~~~~
::c++
template< typename... PARAMS >
server_settings_t &
request_handler( PARAMS &&... params );
~~~~~

When called an instance of `std::unique_ptr<TRAITS::request_handler_t>`
will be created with specified `params`.
If no constructor with such parameters is available,
then compilation error will occur.
If `request_handler_t` has a default constructor then it is not
mandatory to call setter -- the default constructed instance will be used.

As request handler is constructed as unique_ptr, then getter
returns unique_ptr value with ownership, so while manipulating
`server_settings_t` object don't use it.

The same applies to timer_factory and logger parameters.

When `http_server_t` instance is created request_handler, timer_factory and logger
are checked to be instantiated.

## Traits of *http_server_t*

### timer_factory_t
`timer_factory_t` - defines a timeout controller logic.
It must define a nested type `timer_guard_t` with the following interface:

~~~~~
::c++
class timer_guard_t
{
  public:
    // Set new timeout guard.
    template <
        typename EXECUTOR,
        typename CALLBACK_FUNC >
    void
    schedule_operation_timeout_callback(
      const EXECUTOR & e,
      std::chrono::steady_clock::duration d,
      CALLBACK_FUNC && cb );

    // Cancel timeout guard if any.
    void
    cancel();
};
~~~~~

The first method starts guarding timeout of a specified duration,
and if it occurs some how the specified callback must be posted on
`asio::io_context` executor.

The second method must cancel execution of the previously scheduled timer.

An instance of `std::shared_ptr< timer_guard_t >` is stored in each connection
managed by *RESTinio* and to create it `timer_factory_t` must define
the following method:

~~~~~
::c++
class timer_factory_t
{
  public:
    // ...

    using timer_guard_instance_t = std::shared_ptr< timer_guard_t >;

    // Create guard for connection.
    timer_guard_instance_t
    create_timer_guard( asio::io_context & );
    // ...
};
~~~~~

*RESTinio* comes with a set of ready-to-use `timer_factory_t` implementation:

* `null_timer_factory_t` -- noop timer guards, they produce timer guards
that do nothing (when no control needed).
See [restinio/null_timer_factory.hpp](./dev/restinio/null_timer_factory.hpp);
* `asio_timer_factory_t` -- timer guards implemented with asio timers.
See [restinio/asio_timer_factory.hpp](./dev/restinio/asio_timer_factory.hpp);
* `so5::so_timer_factory_t` -- timer guards implemented with *SObjectizer* timers.
See [restinio/so5/so_timer_factory.hpp](./dev/restinio/so5/so_timer_factory.hpp)
Note that `restinio/so5/so_timer_factory.hpp` header file is not included
by `restinio/all.hpp`, so it needs to be included separately.

### logger_t
`logger_t` - defines a logger implementation.
It must support the following interface:
~~~~~
::c++
class null_logger_t
{
  public:
    template< typename MSG_BUILDER >
    void trace( MSG_BUILDER && mb );

    template< typename MSG_BUILDER >
    void info( MSG_BUILDER && mb );

    template< typename MSG_BUILDER >
    void warn( MSG_BUILDER && mb );

    template< typename MSG_BUILDER >
    void error( MSG_BUILDER && mb );
};
~~~~~

`MSG_BUILDER` is lambda that returns a message to log out.
This approach allows compiler to optimize logging when it is possible,
see [`null_logger_t`](./dev/restinio/loggers.hpp).

For implementation example see
[`ostream_logger_t`](./dev/restinio/ostream_logger.hpp).

### request_handler_t

`request_handler_t` - is a key type for request handling process.
It must be a function-object with the following invocation interface:
~~~~~
::c++
restinio::request_handling_status_t
handler( restinio::request_handle_t req );
~~~~~

The `req` parameter defines request data and stores some data
necessary for creating responses.
Parameter is passed by value and thus can be passed to another
processing flow (that is where an async handling becomes possible).

Handler must return handling status via `request_handling_status_t` enum.
If handler handles request it must return `accepted`.

If handler refuses to handle request it must return `rejected`.

### strand_t

`strand_t` provides serialized callback invocation
for events of a specific connection.
There are two option for `strand_t`:
`asio::strand< asio::executor >` or `asio::executor`.

By default `asio::strand< asio::executor >` is used,
it guarantees serialized chain of callback invocation.
But if `asio::io_context` runs on a single thread there is no need
to use `asio::strand` because there is no way to run callbacks
in parallel. So in such cases it is enough to use `asio::executor`
directly and eliminate overhead of `asio::strand`.

## Request handling

Lets consider that we are at the point when response
on a particular request is ready to be created and send.
The key here is to use a given connection handle and
[response_builder_t](./dev/restinio/message_builders.hpp) that is created by
this connection handle:

Basic example of response builder with default response builder:
~~~~~
::c++
// Construct response builder.
auto resp = req->create_response(); // 200 OK

// Set header fields:
resp.append_header( restinio::http_field::server, "RESTinio server" );
resp.append_header_date_field();
resp.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" );

// Set body:
resp.set_body( "Hello world!" );

// Response is ready, send it:
resp.done();
~~~~~

Currently there are three types of response builders.
Each builder type is a specialization of a template class `response_builder_t< TAG >`
with a specific tag-type:

* Tag `restinio_controlled_output_t`. Simple standard response builder.
* Tag `user_controlled_output_t`. User controlled response output builder.
* Tag `chunked_output_t`. Chunked transfer encoding output builder.

### Simple standard response builder

Requires user to set header and body.
Content length is automatically calculated.
Once the data is ready, the user calls done() method
and the resulting response is scheduled for sending.

~~~~~
::c++
 handler =
  []( auto req ) {
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
~~~~~

### User controlled response output builder

This type of output allows user
to send body divided into parts.
But it is up to user to set the correct
Content-Length field.

~~~~~
::c++
 handler =
  []( restinio::request_handle_t req ){
    using output_type_t = restinio::user_controlled_output_t;
    auto resp = req->create_response< output_type_t >();

    resp.append_header( restinio::http_field::server, "RESTinio" )
      .append_header_date_field()
      .append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
      .set_content_length( req->body().size() );

    resp.flush(); // Send only header

    for( const char c : req->body() )
    {
      resp.append_body( std::string{ 1, c } );
      if( '\n' == c )
      {
        resp.flush();
      }
    }

    return resp.done();
  }
~~~~~

### Chunked transfer encoding output builder

This type of output sets transfer-encoding to chunked
and expects user to set body using chunks of data.

~~~~~
::c++
 handler =
  []( restinio::request_handle_t req ){
    using output_type_t = restinio::chunked_output_t;
    auto resp = req->create_response< output_type_t >();

    resp.append_header( restinio::http_field::server, "RESTinio" )
      .append_header_date_field()
      .append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" );

    resp.flush(); // Send only header


    for( const char c : req->body() )
    {
      resp.append_chunk( std::string{ 1, c } );
      if( '\n' == c )
      {
        resp.flush();
      }
    }

    return resp.done();
  }
~~~~~

## Buffers

RESTinio has a capability to receive not only string buffers but also
constant and custom buffers. Since v.0.2.2 message builders has
body setters methods (set_body(), append_body(), append_chunk())
with an argument of a type `buffer_storage_t`
(see [buffers.hpp](./dev/restinio/buffers.hpp) for more details).
`buffer_storage_t` is a wrapper for different type of buffers
that creates `asio::const_buffer` out of different implementations:

* const buffers based on data pointer and data size;
* string buffers based on `std::string`;
* shared buffer - a shared_ptr on an object with data-size interface:
 `std::shared_ptr< BUF >` where `BUF` has `data()` and `size()`
 methods returning `void*` (or convertible to it) and
`size_t` (or convertible to).

Const buffers are intended for cases when the data is defined
as a constant char sequence and its lifetime is guaranteed to be long enough
(for example a c-strings defined globally).
To make the usage of const buffers safer `buffer_storage_t` constructors
don't accept pointer and size params directly, and to instantiate
a `buffer_storage_t` object that refers to const buffers a helper `const_buffer_t`
class must be used. There is a helper function `const_buffer()` that helps to create
`const_buffer_t`. Let's have a look on a clarifying example:

~~~~~
::c++

// Request handler:
[]( restinio::request_handle_t req ){
  // Create response builder.
  auro resp = req->create_response();

  const char * resp = "0123456789 ...";

  // Set response part as const buffer.
  resp.set_body( restinio::const_buffer( resp ) ); // OK, size will be calculated with std::strlen().
  resp.set_body( restinio::const_buffer( resp, 4 ) ); // OK, size will be 4.

  // When not using restinio::const_buffer() helper function
  // char* will be treated as a parameter for std::string constructor.
  resp.set_body( resp ); // OK, but std::string will be actually used.

  const std::string temp{ "watch the lifetime, please" };

  // Using a temporary source for const buffer.
  resp.set_body( restinio::const_buffer( temp.data(), temp.size() ) ); // BAD!

  // Though using a temporary source directly is OK.
  resp.set_body( temp ); // OK, will create a copy of the string.

  // Though using a temporary source directly is OK.
  resp.set_body( temp ); // OK, will create a copy of the string.

  // ...
}
~~~~~

The simplest option is to use std::string. Passed string is copied or moved if possible.

The third option is to use shared (custom) buffers wrapped in shared_ptr:
`std::shared_ptr< BUFFER >`. `BUFFER` type is  required to have data()/size()
member functions, so it is possible to obtain a pointer to data and data size.
For example `std::shared_ptr< std::string >` can be used.
Such form of buffers was introduced for dealing with the cases
when there are lots of parallel requests that must be served with the same response
(or partly the same, so identical parts can be wrapped in shared buffers).

## Routers

One of the reasons to create *RESTinio* was an ability to have
[express](https://expressjs.com/)-like request handler router.

Since v 0.2.1 *RESTinio* has a router based on idea borrowed
from [express](https://expressjs.com/) - a JavaScript framework.

Routers acts as a request handler (it means it is a function-object
that can be called as a request handler).
But router aggregates several handlers and picks one or none of them
to handle the request. The choice of the handler to execute depends on
request target and HTTP method. If router finds no handler matching request then it
rejects it.
Note that that the signature of the handlers put in router
are not the same as standard request handler.
It has an additional parameter -- a container with parameters extracted from URI.

Express router is defined by `express_router_t` class.
Its implementation is inspired by
[express-router](https://expressjs.com/en/starter/basic-routing.html).
It allows to define route path with injection
of parameters that become available for handlers.
For example the following code sets a handler with 2 parameters:
```
::c++
  router.http_get(
    R"(/article/:article_id/:page(\d+))",
    []( auto req, auto params ){
      const auto article_id = params[ "article_id" ];
      auto page = std::to_string( params[ "page" ] );
      // ...
    } );
```

Note that express handler receives 2 parameters not only request handle
but also `route_params_t` instance that holds parameters of the request:
```
::c++
using express_request_handler_t =
    std::function< request_handling_status_t( request_handle_t, route_params_t ) >;
```

Route path defines a set of named and indexed parameters.
Named parameters starts with `:`, followed by non-empty parameter name
(only A-Za-z0-9_ are allowed). After parameter name it is possible to
set a capture regex enclosed in brackets
(actually a subset of regex - none of the group types are allowed).
Indexed parameters are simply a capture regex in brackets.

Let's show how it works by example.
First let's assume that variable `router` is a pointer to express router.
So that is how we add a request handler with a single parameter:
```
::c++
  // GET request with single parameter.
  router->http_get( "/single/:param", []( auto req, auto params ){
    return
      init_resp( req->create_response() )
        .set_body( "GET request with single parameter: " + params[ "param" ] )
        .done();
  } );
```

The following requests will be routed to that handler:

* http://localhost/single/123 param="123"
* http://localhost/single/parameter/ param="parameter"
* http://localhost/single/another-param param="another-param"

But the following will not:

* http://localhost/single/123/and-more
* http://localhost/single/
* http://localhost/single-param/123

Let's use more parameters and assign a capture regex for them:
```
::c++
  // POST request with several parameters.
  router->http_post( R"(/many/:year(\d{4}).:month(\d{2}).:day(\d{2}))",
    []( auto req, auto params ){
      return
        init_resp( req->create_response() )
          .set_body( "POST request with many parameters:\n"
            "year: "+ params[ "year" ] + "\n" +
            "month: "+ params[ "month" ] + "\n" +
            "day: "+ params[ "day" ] + "\n"
            "body: " + req->body() )
          .done();
    } );
```
The following requests will be routed to that handler:

* http://localhost/many/2017.01.01 year="2017", month="01", day="01"
* http://localhost/many/2018.06.03 year="2018", month="06", day="03"
* http://localhost/many/2017.12.22 year="2017", month="12", day="22"

But the following will not:

* http://localhost/many/2o17.01.01
* http://localhost/many/2018.06.03/events
* http://localhost/many/17.12.22

Using indexed parameters is practically the same, just omit parameters names:
```
::c++
  // GET request with indexed parameters.
  router->http_get( R"(/indexed/([a-z]+)-(\d+)/(one|two|three))",
    []( auto req, auto params ){
      return
        init_resp( req->create_response() )
          .set_body( "POST request with indexed parameters:\n"
            "#0: "+ params[ 0 ] + "\n" +
            "#1: "+ params[ 1 ] + "\n" +
            "#2: "+ params[ 2 ] + "\n" )
          .done();
    } );
```
The following requests will be routed to that handler:

* http://localhost/indexed/xyz-007/one #0="xyz", #1="007", #2="one"
* http://localhost/indexed/ABCDE-2017/two #0="ABCDE", #1="2017", #2="two"
* http://localhost/indexed/sobjectizer-5/three #0="sobjectizer", #1="5", #2="three"

But the following will not:

* http://localhost/indexed/xyz-007/zero
* http://localhost/indexed/173-xyz/one
* http://localhost/indexed/ABCDE-2017/one/two/three

See full [example](./dev/sample/express_router_tutorial/main.cpp)

For details on `route_params_t` and `express_router_t` see
[express.hpp](./dev/restinio/router/express.cpp).

## TLS support

Restinio support HTTS using ASIO ssl facilities (based on OpenSSL).

To create https server it is needed to include extra header file `restinio/tls.hpp`.
This file contains necessary customization classes and structs
that make `restinio::http_server_t` usable as https server.
For specializing `restinio::http_server_t` to work as https server
one should use `restinio::tls_traits_t` (or `restinio::single_thread_tls_traits_t`)
for it and also it is vital to set TLS context using `asio::ssl::context`.
That setting is added to `server_settings_t` class instantiated with TLS traits.

Lets look through an example:
```
::c++
// ...

using traits_t =
  restinio::single_thread_tls_traits_t<
    restinio::asio_timer_factory_t,
    restinio::single_threaded_ostream_logger_t,
    router_t >;

using http_server_t = restinio::http_server_t< traits_t >;

const std::string certs_dir{ "." }; // Or another path.

http_server_t http_server{
  restinio::create_child_io_context( 1 ),
  [ & ]( auto & settings ){
    // Set TLS context.
    asio::ssl::context tls_context{ asio::ssl::context::sslv23 };
    tls_context.set_options(
      asio::ssl::context::default_workarounds
      | asio::ssl::context::no_sslv2
      | asio::ssl::context::single_dh_use );

    tls_context.use_certificate_chain_file( certs_dir + "/server.pem" );
    tls_context.use_private_key_file(
      certs_dir + "/key.pem",
      asio::ssl::context::pem );
    tls_context.use_tmp_dh_file( certs_dir + "/dh2048.pem" );

    settings
      .address( "localhost" )
      .request_handler( server_handler() )
      .read_next_http_message_timelimit( 10s )
      .write_http_response_timelimit( 1s )
      .handle_request_timeout( 1s )
      // Set the context:
      .tls_context( std::move( tls_context ) );
  } };

http_server.open();

// ...
```

See full [example](./dev/sample/hello_world_https/main.cpp) for details.
