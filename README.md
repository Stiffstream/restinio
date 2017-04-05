[TOC]

# What Is It?
*RESTinio* is a header-only library for creating REST applications in c++.
It helps to create http server that can handle requests asynchronously.
Currently it is in alpha state and represents
the first view on the solution of the problem of
being able to handle request asynchronously.

*RESTinio* is a free software and is distributed under GNU Affero GPL v.3 license.

## Why creating yet another library of that kind?

Well, there are lots of libraries and frameworks
of all sorts of complexity and maturety for
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
handling request and writing resposne to socket.
And a header-only design is a plus.

And it happens that under such conditions it you don't have a lot of options.
So we have come up with *RESTinio*...

# Obtain And Build

## Prerequisites

To use *RESTinio* it is necessary to have:

* Reasonably modern C++14 compiler (VC++14.0, GCC 5.2 or above, clang 3.6 or above);
* [asio](http://think-async.com/Asio) 1.11.0;
* [nodejs/http-parser](https://github.com/nodejs/http-parser) 2.7.1 or above;
* [fmtlib](http://fmtlib.net/latest/index.html) 3.0.1 or above.
* Optional: [SObjectizer](https://sourceforge.net/projects/sobjectizer/) 5.5.18 and above;

For building samples, benchmarks and tests:

* [Mxx_ru](https://sourceforge.net/projects/mxxru/) 1.6.11 or above;
* [rapidjson](https://github.com/miloyip/rapidjson) 1.1.0;
* [json_dto](https://bitbucket.org/sobjectizerteam/json_dto-0.1) 0.1.2.1 or above;
* [args](https://github.com/Taywee/args) 6.0.4;
* [CATCH](https://github.com/philsquared/Catch) 1.8.2.

## Obtaining

### Cloning of hg repository

```
hg clone https://bitbucket.org/sobjectizerteam/restinio-0.1
```

And then:
```
cd restinio-0.1
mxxruexternals
```
to download and extract *RESTinio*'s dependencies.

### MxxRu::externals recipe

See MxxRu::externals recipies for *RESTinio*
[here](./doc/MxxRu_externals_recipe.md).

## Build

### CMake

Building with CMake currently is confined with two samples.
To build them run the following commands:
```
hg clone https://bitbucket.org/sobjectizerteam/restinio-0.1
cd restinio-0.1
mxxruexternals
cd dev
mkdir cmake_build
cd cmake_build
cmake -DCMAKE_INSTALL_PREFIX=target -DCMAKE_BUILD_TYPE=Release ..
make
make install
```

### Mxx_ru
While *RESTinio* is header-only library, samples, tests and benches require a build.

Compiling with Mxx_ru:
```
hg clone https://bitbucket.org/sobjectizerteam/restinio-0.1
cd restinio-0.1
mxxruexternals
cd dev
ruby build.rb
```

For release or debug builds use the following commands:
```
ruby build.rb --mxx-cpp-release
ruby build.rb --mxx-cpp-debug
```

*NOTE.* It might be necessary to set up `MXX_RU_CPP_TOOLSET` environment variable,
see Mxxx_ru documentation for further details.

### Dependencies default settings

External libraries used by *RESTinio* have the following default settings:

* A standalone version of *asio* is used and a chrono library is used,
so `ASIO_STANDALONE` and 'ASIO_HAS_STD_CHRONO' defines are necessary;
* a less strict version of *nodejs/http-parser* is used, so the following
definition `HTTP_PARSER_STRICT=0` is used;
* *fmtlib* is used as a header-only library, hence a `FMT_HEADER_ONLY`
define is necessary;

# How To Use It?

## Getting started

To start using *RESTinio* make sure that all dependencies are available.
The tricky one is [nodejs/http-parser](https://github.com/nodejs/http-parser),
because it is a to be compiled unit, which can be built as a static library and
linked to your target or can be a part of your target.

To start using *RESTinio* simply include `<restinio/all.hpp>` header.
It includes all necessary headers of the library.

It easy to learn how to use *RESTinio* by example.
Here is a hello world application
([see full example](./dev/sample/hello_world_basic/main.cpp)):
~~~~~
::c++
using http_server_t = restinio::http_server_t<>;

http_server_t http_server{
  restinio::create_child_io_service( 1 ),
  []( auto & settings ){
    settings.port( 8080 ).request_handler( request_handler() );
  }
};

// Start server.
http_server.open();

// ...

// Stop server.
http_server.close();
~~~~~

Template class `restinio::http_server_t<TRAITS>` encapsulates server
logic. It has two parameters: the first one is a wrapper for
`asio::ioservice` instance passed as
`io_service_wrapper_unique_ptr_t`, and the second one is a
`server_settings_t<TRAITS>` object that defines
server port, protocol (ipv4/ipv6), timeouts etc.

*RESTinio* does its networking stuff with
[asio](http://think-async.com/Asio) library, so to run server
it must have an `asio::io_service` instance to run on.
Internal logic of *RESTinio* is separated from
maintaining `asio::ioservice` directly by a wrapper class.
In most cases it would be enough to use one of standard
wrappers. The first one is provided by
`create_proxy_io_service( asio::io_service & )`
and is a proxy for user managed `asio::io_service` instance.
And the second is the one provided by
`create_child_io_service( unsigned int thread_pool_size )`
that creates an object with `asio::io_service` inside
that runs on a thread pool and is managed by server object.
Child io_service running on a single thread is used in example.

Server settings are set with lambda.
It is more convenient to use generic lambda to omit
naming the concrete type of `server_settings_t<TRAITS>`.
View `server_settings_t` class to see all available params
([settings.hpp](./dev/restinio/settings.hpp)).

To run a server `open()` and `close()` methods are used.
When open method is called server posts a callback on
`asio::ioservice` to bind on a server port and listen for new connections.
If callback executes without errors, open returns, in case of error
it throws. Note that in general open and close operations are executed
asynchronously so to make them sync a future-promise is used.

To make server complete we must set request handler.
Request handler as defined by default `TRATS` is a
`std::function< bool ( request_data, conn ) >`:
~~~~~
::c++
std::function< request_handling_status_t (http_request_handle_t, connection_handle_t) >
~~~~~

To create handler `request_handler()` function is used:
~~~~~
::c++
auto request_handler()
{
  return []( auto req, auto conn ) {
      if( restinio::http_method_get() == req->m_header.method() &&
        req->m_header.request_target() == "/" )
      {
        restinio::response_builder_t{ req->m_header, std::move( conn ) }
          .append_header( "Server", "RESTinio hello world server" )
          .append_header_date_field()
          .append_header( "Content-Type", "text/plain; charset=utf-8" )
          .set_body( "Hello world!")
          .done();

        return restinio::request_accepted();
      }

      return restinio::request_rejected();
    };
}
~~~~~

Request handler here is a lambda-function, it check if request method is `GET`
and target is `/` and if so makes a response and returns `restinio::request_handling_status_t::accepted`
meaning that request had been taken for processing.
Otherwise handler returns `restinio::request_handling_status_t::rejected` value meaning that request was not accepted for handling and *RESTinio* must take care of it.

## Basic idea

In general *RESTinio*  runs its logic on `asio::io_service`.
There are two major object types running:

* acceptor -- receives new connections and creates connection objects that
performs session logic;
* connection -- does tcp io-operations, http-parsing and calls handler.

There is a single instance of acceptor and as much connections as needed.

Acceptors life cycle is trivial and is the following:

1. Start listening for new connection;
2. Receive new tcp-connection;
3. Create connection handler object and start running it;
4. Back to step 1.

When the server is closed cycle breaks up.

Connections life cycle is more complicated and
without error handling and timeouts control looks like this:

1. Start reading from socket;
2. Receive a portion of data from socket and parse http request out of it;
3. If http message parsing is incomplete then go back to step 1;
4. If http message parsing is complete
pass request and connection to request handler;
5. If request handler reject request, then write not-implemented (status 501)
response and close connection;
6. Wait for response initiated from user domain either directly inside of handler call
or from other context where response actually is being built;
7. Write response to socket;
8. When write operation is complete and response was marked as connection-keep-alive
then go back to step 1;
9. If response was marked to connection-close then connection is closed and destroyed.

Of course implementation has error checks. Also implementation controls timeouts of
operations that are spread in time:

* reading the request: from starting reading bytes from socket up to
parsing a complete http-message;
* handling the request: from passing request data and connection handle
to request handler up to getting response to be written to socket;
* writing response to socket.

When handling a request there are two possible cases:

* response is created inside the request handlers call;
* request handler delegates handling job to other context via
some kind of async API.

The first case is trivial and response is simply begins to be written.

The second case and its possibility is a key point of *RESTinio* being created for.
As request data and connection handle are wrapped in shared pointers
so they can be moved to other context.
So it is possible to create handlers that can interact with async API.
When response data is ready response can be built and sent using connection handle.
After response building is complete connection handle
will post the necessary job to run on host `asio::io_service`.
So one can perform asynchronous request handling and
not to block worker threads.

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
~~~~~

It is easier to use `restinio::traits_t<>` helper type:
~~~~~
::c++
template <
    typename TIMER_FACTORY,
    typename LOGGER,
    typename REQUEST_HANDLER = default_request_handler_t,
    typename STRAND = asio::strand< asio::executor > >
struct traits_t;
~~~~~

Here is what `TRAITS` member type stand for:

* `timer_factory_t` defines the logic of how timeouts are managed;
* `logger_t` defines logger that is used by *RESTinio* to track
its inner logic;
* `request_handler_t` defines a function-like type to be used as
request handler;
* `strand_t` - defines a class that is used by connection as a wrapper
for its callback-handlers running on `asio::io_service` thread(s)
in order to guarantee serialized callbacks invocation
(see [asio doc](http://think-async.com/Asio/asio-1.11.0/doc/asio/reference/strand.html)).
Actually there are two options for the strand type:
`asio::strand< asio::executor >` and `asio::executor`.

It is handy to consider `http_server_t<TRAITS>` class as a root class
for the rest of *RESTinio* ecosystem, because pretty much all of them are
also template types parameterized with the same `TRAITS` parameter.

Class `http_server_t<TRAITS>` has two constructors:
~~~~~
::c++
http_server_t(
  io_service_wrapper_unique_ptr_t io_service_wrapper,
  server_settings_t<TRAITS> settings );

template < typename CONFIGURATOR >
http_server_t(
  io_service_wrapper_unique_ptr_t io_service_wrapper,
  CONFIGURATOR && configurator )
~~~~~

The first is the main one. It obtains `io_service_wrapper` as an
`asio::io_service` back-end and server settings with the bunch of params.

The second constructor simplifies setting of parameters via generic lambda:
~~~~~
::c++
http_server{
  restinio::create_child_io_service( 1 ),
  []( auto & settings ){ // Omit concrete name of settings type.
    settings
      .port( 8080 )
      .read_next_http_message_timelimit( std::chrono::seconds( 1 ) )
      .handle_request_timeout( std::chrono::milliseconds( 3900 ) )
      .write_http_response_timelimit( std::chrono::milliseconds( 100 ) )
      .logger( /* logger params */ )
      .request_handler( /* request handler params */ )
      .request_handler( request_handler() );
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

Async versions post callback on `asio::io_service` thread
and once executed one of specified callbacks is called.
The first one in case of success, and the second - in case of error.

Sync version work through async version getting synchronized by
future-promise.

`http_server_t<TRAITS>` also has methods to start/stop running
`asio::io_service`:
~~~~~
::c++
void
start_io_service();

void
stop_io_service();
~~~~~

They are helpful when using async versions of open()/close() methods,
when running `asio::io_service` and running server must be managed separately.

## Settings of *http_server_t*

Class `server_settings_t<TRAITS>` serves to pass settings to `http_server_t`.
It is defined in [restinio/settings.hpp](./dev/restinio/settings.hpp);

For each parameter a setter/getter pair is provided.
While setting most of params is pretty straightforward,
there are some params with a bit tricky setter/getter semantics.
They are request_handler, timer_factory, logger.

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
      CALLBACK_FUNC && cb ) const;

    // Cancel timeout guard if any.
    void
    cancel() const;
};
~~~~~

The first method starts guarding timeout of a specified duration,
and if it occurs some how the specified callback must be posted on
`asio::io_service` executor.

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
    create_timer_guard( asio::io_service & );
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

For implementation examples see
[`ostream_logger_t`](./dev/sample/common/ostream_logger.hpp).

### request_handler_t

`request_handler_t` - is a key type for request handling process.
It must be a function-object with the following invocation interface:
~~~~~
::c++
request_handling_status_t
handler(
  restinio::http_request_handle_t req,
  restinio::connection_handle_t conn );
~~~~~

The first parameter defines request data,
and the second one provides connection handle.
Both parameters passed by value and thus can be passed to another
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
But if `asio::ioservice` runs on a single thread there is no need
to use `asio::strand` because there is no way to run callbacks
in parallel. So in such cases it is enough to use `asio::executor`
directly.

## Request handling

Lets consider that we are at the point when response
on a particular request is ready to be created and send.
The key here is to use a given connection handle and
[response_builder_t](./dev/restinio/message_builders.hpp):

~~~~~
::c++
// Construct response builder.
restinio::response_builder_t resp{ req->m_header, std::move( conn ) };

// Set header fields:
resp.append_header( "Server", "RESTinio server" );
resp.append_header_date_field();
resp.append_header( "Content-Type", "text/plain; charset=utf-8" );

// Set body:
resp.set_body( "Hello world!" );

// Response is ready, send it:
resp.done();
~~~~~

# Road Map

Features for next releases:

* Routers for message handlers.
Support for a URI dependent routing to a set of handlers.
* Support for chunked transfer encoding. Separate responses on header and body chunks,
so it will be possible to send header and then body divided on chunks.
* True [HTTP pipelining](https://en.wikipedia.org/wiki/HTTP_pipelining) support.
Read, parse and call a handler for incoming requests independently.
When responses become available send them to client in order of corresponding requests.
* Non trivial benchmarks. Comparison with other libraries with similar features
on the range of various scenarios.
* HTTP client. Introduce functionality for building and sending requests and
receiving and parsing results.
* TLS support.
* Full CMake support.
* Want more features?
Please send us feedback and we will take your opinion into account.

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
