# Express router

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
It allows to define route path with injection
of parameters that become available for handlers.
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
~~~~~
::c++

  // GET request with single parameter.
  router->http_get( "/single/:param", []( auto req, auto params ){
    return
      init_resp( req->create_response() )
        .set_body( "GET request with single parameter: " + params[ "param" ] )
        .done();
  } );
~~~~~


The following requests will be routed to that handler:

* http://localhost/single/123 param="123"
* http://localhost/single/parameter/ param="parameter"
* http://localhost/single/another-param param="another-param"

But the following will not:

* http://localhost/single/123/and-more
* http://localhost/single/
* http://localhost/single-param/123

Let's use more parameters and assign a capture regex for them:
~~~~~
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
~~~~~

The following requests will be routed to that handler:

* http://localhost/many/2017.01.01 year="2017", month="01", day="01"
* http://localhost/many/2018.06.03 year="2018", month="06", day="03"
* http://localhost/many/2017.12.22 year="2017", month="12", day="22"

But the following will not:

* http://localhost/many/2o17.01.01
* http://localhost/many/2018.06.03/events
* http://localhost/many/17.12.22

Using indexed parameters is practically the same, just omit parameters names:
~~~~~
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
~~~~~

The following requests will be routed to that handler:

* http://localhost/indexed/xyz-007/one #0="xyz", #1="007", #2="one"
* http://localhost/indexed/ABCDE-2017/two #0="ABCDE", #1="2017", #2="two"
* http://localhost/indexed/sobjectizer-5/three #0="sobjectizer", #1="5", #2="three"

But the following will not:

* http://localhost/indexed/xyz-007/zero
* http://localhost/indexed/173-xyz/one
* http://localhost/indexed/ABCDE-2017/one/two/three

See full [example](../dev/sample/express_router_tutorial/main.cpp)

For details on `route_params_t` and `express_router_t` see
[express.hpp](../dev/restinio/router/express.cpp).
