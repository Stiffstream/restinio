# *RESTinio* context entities running on asio::io_context

*RESTinio* runs its logic on `asio::io_context`.
*RESTinio* works with asio on the base of callbacks,
that means tha some context is always passed from one callback to another.
There are two main entities the contexts of which is passed between callbacks:

* acceptor -- receives new connections and creates connection objects that
performs session logic;
* connection -- does tcp io-operations, http-parsing and calls handler.


### Acceptor

There is a single instance of acceptor and as much connections as needed.

Acceptors life cycle is trivial and is the following:

1. Start listening for new connection.
2. Receive new tcp-connection.
3. Create connection handler object and start running it.
4. Back to step 1'.

When the server is closed cycle breaks up.

To set custom options for acceptor use `server_settings_t::acceptor_options_setter()`.

By default *RESTinio* accepts connections one-by-one,
so a big number of clients initiating simultaneous connections might be a problem
even when running `asio::io_context` on a pool of threads.
There are a number of options to tune *RESTinio* for such cases.

* Increase the number of concurrent accepts. By default *RESTinio*
initiates only one accept operation, but when running server on
N threads then up to N accepts can be handled concurrently.
See `server_settings_t::concurrent_accepts_count()`.
* After accepting new connection on socket *RESTinio* creates
internal connection wrapper object. The creation of such object can
be done separately (in another callback posted on asio).
So creating connection instance that involves allocations
and initialization can be done in a context that is independent to acceptors one.
It makes on-accept callback to run faster, thus more connections can be
accepted in the same time interval.
See `server_settings_t::separate_accept_and_create_connect()`

Example of using acceptor options:
~~~~~
::c++
// using traits_t = ...
restinio::http_server_t< traits_t >
  server{
    restinio::create_child_io_context( 4 ),
    restinio::server_settings_t< traits_t >{}
      .port( port )
      .buffer_size( 1024 )
      .max_pipelined_requests( 4 )
      .request_handler( db )
      // Using acceptor options:
      .acceptor_options_setter(
        []( auto & options ){
          options.set_option( asio::ip::tcp::acceptor::reuse_address( true ) );
        } )
      .concurrent_accepts_count( 4 )
      .separate_accept_and_create_connect( true ) };
~~~~~

### Connection

Connections life cycle is more complicated and cannot be expressed lineary.
Simultaneously connection runs two logical objectives. The first one is
responsible for receiving requests and passing them to handler (read part) and
the second objective is streaming resulting responses back to client (write part).
Such logical separation comes from HTTP pipelining support and
various types of response building strategies.

Without error handling and timeouts control Read part looks like this:

1. Start reading from socket.
2. Receive a portion of data from socket and parse HTTP request out of it.
3. If HTTP message parsing is incomplete then go back to step 1.
4. If HTTP message parsing is complete pass request and connection to request handler.
5. If request handler rejects request, then push not-implemented response (status 501)
to outgoing queue and stop reading from socket.
5. If request was accepted and the number of requests in process is less than
`max_pipelined_requests` then go back to step 1.
6. Stop reading socket until awaken by the write part.

And the Write part looks like this:

1. Wait for response pieces initiated from user domain
either directly inside of handler call or from other context where
response actually is being built.
2. Push response data to outgoing queue with consideration of associated response position
(multiple request can be in process, and response for a given request
cannot be written to socket before writing all previous responses to it).
3. Check if there is outgoing data ready to send.
4. If there is no ready data available then go back to step 1.
5. Send ready data.
6. Wait for write operation to complete. If more response pieces comes while
write operation runs it is simply received (steps 1-2 without any further go).
7. After write operation completes:
if last committed response was marked to close connection
then connection is closed and destroyed.
8. If it appears that the room for more pipeline requests became available again
then awake the read part.
9. Go back to step 3.

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
When response data is ready response can be built and sent using request handle.
After response building is complete connection handle
will post the necessary job to run on host `asio::io_context`.
So one can perform asynchronous request handling and
not to block worker threads.

To set custom options for acceptor use `server_settings_t::socket_options_setter()`:
~~~~~
::c++
// using traits_t = ...
restinio::http_server_t< traits_t >
  server{
    restinio::create_child_io_context( 4 ),
    restinio::server_settings_t< traits_t >{}
      .port( port )
      .buffer_size( 1024 )
      .max_pipelined_requests( 4 )
      .request_handler( db )
      // Using custom socket options:
      .socket_options_setter(
        []( auto & options ){
          options.set_option( asio::ip::tcp::no_delay{ true } );
        } ) };
~~~~~
