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

# Version history

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
