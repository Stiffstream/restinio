# Traits

## List of types that must be defined be *Traits*

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

## timer_factory_t
`timer_factory_t` - defines a timeout controller logic.
It must define a nested type `timer_guard_t` with the following interface:

~~~~~
::c++
class timer_guard_t
{
  public:
    // Set new timeout guard.
    template <
        typename Executor,
        typename Callback_Func >
    void
    schedule_operation_timeout_callback(
      const Executor & executor,
      std::chrono::steady_clock::duration timeout,
      Callback_Func && f );

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
See [restinio/null_timer_factory.hpp](../dev/restinio/null_timer_factory.hpp);
* `asio_timer_factory_t` -- timer guards implemented with asio timers.
See [restinio/asio_timer_factory.hpp](../dev/restinio/asio_timer_factory.hpp);
* `so5::so_timer_factory_t` -- timer guards implemented with *SObjectizer* timers.
See [restinio/so5/so_timer_factory.hpp](../dev/restinio/so5/so_timer_factory.hpp)
Note that `restinio/so5/so_timer_factory.hpp` header file is not included
by `restinio/all.hpp`, so it needs to be included separately.

## logger_t

`logger_t` - defines a logger implementation.
It must support the following interface:
~~~~~
::c++
class null_logger_t
{
  public:
    template< typename Msg_Builder >
    void trace( Msg_Builder && mb );

    template< typename Msg_Builder >
    void info( Msg_Builder && mb );

    template< typename Msg_Builder >
    void warn( Msg_Builder && mb );

    template< typename Msg_Builder >
    void error( Msg_Builder && mb );
};
~~~~~

`Msg_Builder` is lambda that returns a message to log out.
This approach allows compiler to optimize logging when it is possible,
see [null_logger_t](../dev/restinio/loggers.hpp).

For implementation example see [ostream_logger_t](../dev/restinio/ostream_logger.hpp).

## request_handler_t

`request_handler_t` - is a key type for request handling process.
It must be a function-object with the following invocation interface:
~~~~~
::c++
restinio::request_handling_status_t
handler( restinio::request_handle_t req );
~~~~~

The `req` parameter defines request data and stores some data necessary for creating responses.
Parameter is passed by value and thus can be passed to another processing flow
(that is where an async handling becomes possible).

Handler must return handling status via `request_handling_status_t` enum.
If handler handles request it must return `accepted`.
If handler refuses to handle request it must return `rejected`.
There are two helper functions:
`restinio::request_accepted()` and `restinio::request_rejected()`
for refering an itemes of enum.

## strand_t

`strand_t` provides serialized callback invocation for events of a specific connection.
There are two option for `strand_t`: `asio::strand< asio::executor >` or `asio::executor`.

By default `asio::strand< asio::executor >` is used,
it guarantees serialized chain of callback invocation.
But if `asio::io_context` runs on a single thread there is no need
to use `asio::strand` because there is no way to run callbacks in parallel.
So in such cases it is enough to use `asio::executor` directly and
eliminate overhead of `asio::strand`.

## stream_socket_t

`stream_socket_t` allows to customize underlying socket type,
so it possible to create https server using identical interface (see [TLS support](./tls_support.md)).
