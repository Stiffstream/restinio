# Basic idea

When describing  *RESTinio* http server there are three major abstractions
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
(see [using external io_context](./doc/using_external_io_context.md)).

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
to support TLS connection (see [TLS support](./doc/tls_support.md)).
