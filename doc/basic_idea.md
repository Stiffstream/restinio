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

Refer to ([traits.hpp](../dev/restinio/traits.hpp)) for details.

### List of types that must be defined be *Traits*

* `timer_factory_t` defines the logic of how timeouts are managed;
* `logger_t` defines logger that is used by *RESTinio* to track its inner logic;
* `request_handler_t` defines a function-like type to be used as request handler;
* `strand_t` - defines a class that is used by connection as a wrapper
for its callback-handlers running on `asio::io_context` thread(s)
in order to guarantee serialized callbacks invocation
(see [asio doc](https://chriskohlhoff.github.io/networking-ts-doc/doc/networking_ts/reference/strand.html)).
Actually there are two options for the strand type:
`asio::strand< asio::executor >` and `asio::executor`.
The first is a real strand that guarantees serialized invocation and
the second one is simply a default executor to eliminate unnecessary overhead
when running `asio::io context` on a single thread;
* `stream_socket_t` is a customization point that tells restinio
what type of socket used for connections. This parameter allows restinio
to support TLS connection (see [TLS support](./tls_support.md)).

## Class *http_server_t<Traits>*

Class `http_server_t<Traits>` is a template class parameterized with a single template parameter: `Traits`.
Its meaning is directly depicted in its name, `http_server_t<Traits>`
represents http-server.
It is handy to consider `http_server_t<TRAITS>` class as a root class
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

Refer to ([http_server.hpp](../dev/restinio/http_server.hpp)) for details.

## Class *http_server_settings_t<Traits>*

Refer to ([settings.hpp](./dev/restinio/settings.hpp)) for details.
