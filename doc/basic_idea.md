# Basic idea

When describing  *RESTinio* http server there are three abstractions
vital for understanding of how to use it.

* `http_server_t<Traits>` - a class representing http server.
* `http_server_settings_t<Traits>` - a class representing server settings.
* `Traits` - a customization type for previous abstractions,
that makes it possible to tune concrete server implementaion.

Not less of importance is the fact that *RESTinio* runs on stand-alone
version of [asio](https://github.com/chriskohlhoff/asio.git).
We picked a tag from master branch to be closer to
[Networking TS](https://github.com/chriskohlhoff/networking-ts-impl)
that is aimed to be part of a standard.
An instance of `asio::io_context` is used to run the server.
*RESTinio* can use its own privately created `asio::io_context` or
it can make use of external io_context instance.
The last case requires some assumptions to be fulfilled
(see [using external io_context](./using_external_io_context.md)).

## Traits

Main class `http_server_t` is a template class parameterized with a single
template parameter: `Traits`.
Traits class must specify a set of types used inside *RESTinio*, they are:
~~~~
::c++
timer_factory_t;
logger_t;
request_handler_t;
strand_t;
stream_socket_t;
~~~~~

There is a helper classes for working with traits:
~~~~~
::c++
template <
    typename Timer_Factory,
    typename Logger,
    typename Request_Handler = default_request_handler_t,
    typename Strand = asio::strand< asio::executor >,
    typename Socket = asio::ip::tcp::socket >
struct traits_t; // Implementation omitted.

template <
    typename Timer_Factory,
    typename Logger,
    typename Request_Handler = default_request_handler_t >
using single_thread_traits_t =
  traits_t< Timer_Factory, Logger, Request_Handler, noop_strand_t >; // Implementation omitted.
~~~~~

Refer to [Traits](./traits.md) and [restinio/traits.hpp](../dev/restinio/traits.hpp) for details.

## Class *http_server_t<Traits>*

Class `http_server_t<Traits>` is a template class parameterized with a single template parameter: `Traits`.
Its meaning is directly depicted in its name, `http_server_t<Traits>`
represents http-server.
It is handy to consider `http_server_t<Traits>` class as a root class
for the rest of *RESTinio* ecosystem running behind it, because pretty much all of them are
also template types parameterized with the same `Traits` parameter.

Class `http_server_t<Traits>` has two constructors (simplified to omit verbose template stuff):
~~~~~
::c++
http_server_t(
  io_context_holder_t io_context,
  server_settings_t<Traits> settings );

template < typename Configurator >
http_server_t(
  io_context_holder_t io_context,
  Configurator && configurator );
~~~~~

The first is the main one. It obtains `io_context` as an `asio::io_context` back-end
and server settings with the bunch of params.

The second constructor can simplify setting of parameters via generic lambda like this:
~~~~~
::c++
http_server_t< my_traits_t > http_server{
  restinio::own_io_context(),
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

But in the end it delegates construction to the first custructor.

*RESTinio* runs its logic on `asio::io_context`, but its internal logic
is separated from maintaining io_context directly, hence allowing to
run restinio on external (specified by user) instance of `asio::io_context`
(see [using external io context](./using_external_io_context.md)).
So there is a special class for wrapping io_context instance and pass it to
`http_server_t` constructor: `io_context_holder_t`.
To create the such holder use on of the following functions:

* `restinio::own_io_context()` -- create and use its own instance of io_context;
* `restinio::external_io_context()` -- use external instance of io_context.

### Running server

To run server there are open()/close() methods :
~~~~~
::c++
class http_server_t
{
  // ...
  public:
    template <
        typename Server_Open_Ok_CB,
        typename Server_Open_Error_CB >
    void
    open_async(
      Server_Open_Ok_CB && open_ok_cb,
      Server_Open_Error_CB && open_err_cb )

    void
    open_sync();

    template <
        typename Server_Close_Ok_CB,
        typename Server_Close_Error_CB >
    void
    close_async(
      Server_Close_Ok_CB && close_ok_cb,
      Server_Close_Error_CB && close_err_cb );

    void
    close_sync();
    //...
}
~~~~~

There are sync methods for starting/stoping server and async.
To choose the right method it is necessary to understand
that *RESTinio* doesn't start and run io_context that it runs on.
So user is responsible for running io_context.
Sync versions of `open()/close()` methods assume they are called on
the context of a running io_context. For example:
~~~~~
::c++
// Create and initialize object.
restinio::http_server_t< my_traits_t > server{
  restinio::own_io_context(),
  [&]( auto & settings ){
    //
    settings
      .port( args.port() )
      // .set_more_params( ... )
      .request_handler(
        []( restinio::request_handle_t req ){
            // Handle request.
        } );
  } };

// Post initial action to asio event loop.
asio::post( server.io_context(),
  [&] {
    // Starting the server in a sync way.
    server.open_sync();
  } );

// Running server.
server.io_context().run();
~~~~~

Async versions of `open()/close()` methods can be used from any thread.
But it is not guaranteed that server is already  started when method finishes.
When using async_open() user provides two callbacks, the first one is called if server starts
successfully, and the second one is for handling error.
For example:
~~~~~
::c++
asio::io_context io_ctx;
restinio::http_server_t< my_traits_t > server{
    restinio::external_io_context(io_ctx),
    [&]( auto & settings ) { ... } };

// Launch thread on which server will work.
std::thread server_thread{ [&] {
    io_ctx.run();
  } };

// Start server in async way. Actual start will be performed
// on the context of server_thread.
server.open_async(
    // Ok callback. Nothing to do.
    []{},
    // Error callback. Rethrow an exception.
    []( auto ex_ptr ) {
      std::rethrow_exception( ex_ptr );
    } );
...
// Wait while server_thread finishes its work.
server_thread.join();
~~~~~

Refer to ([restinio/http_server.hpp](../dev/restinio/http_server.hpp)) for details.

## Class *server_settings_t<Traits>*

Class `server_settings_t<Traits>` serves to pass settings to `http_server_t<Traits>`.
It is defined in [restinio/settings.hpp](../dev/restinio/settings.hpp);

For each parameter a setter/getter pair is provided.
While setting most of parameters is pretty straightforward,
there are some parameters with a bit tricky setter/getter semantics.
They are request_handler, timer_factory, logger, acceptor_options_setter,
socket_options_setter and cleanup_func.

For example setter for request_handler looks like this:
~~~~~
::c++
template< typename... PARAMS >
server_settings_t &
request_handler( PARAMS &&... params );
~~~~~

When called an instance of `std::unique_ptr<Traits::request_handler_t>`
will be created with specified `params`.
If no constructor with such parameters is available, then compilation error will occur.
If `request_handler_t` has a default constructor then it is not
mandatory to call setter -- the default constructed instance will be used.
But there is an exception for `std::function` type,
because even though it has a default constructor
it will be useless when constructed in such a way.

Request handler is constructed as unique_ptr, then getter
returns unique_ptr value with ownership, so while manipulating
`server_settings_t` object don't use it.

The same applies to timer_factory,logger parameters, acceptor_options_setter,
socket_options_setter and cleanup_func.

When `http_server_t` instance is created all settings are checked to be properly instantiated.

Refer to [server settings](./server_settings.md) and[restinio/settings.hpp](../dev/restinio/settings.hpp for details.
