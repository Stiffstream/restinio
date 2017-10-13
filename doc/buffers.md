# Buffers

RESTinio has a capability to receive not only string buffers but also
constant and custom buffers. Message builders has
body setters methods (set_body(), append_body(), append_chunk())
with an argument of a type `buffer_storage_t`
(see [buffers.hpp](../dev/restinio/buffers.hpp) for more details).
`buffer_storage_t` is a wrapper for different type of buffers
that creates `asio::const_buffer` out of different implementations:

* const buffers based on data pointer and data size;
* string buffers based on `std::string`;
* shared buffer - a shared_ptr on an object with data-size interface:
 `std::shared_ptr< Buf >` where `Buf` has `data()` and `size()`
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
`std::shared_ptr< Buffer >`. `Buffer` type is  required to have data()/size()
member functions, so it is possible to obtain a pointer to data and data size.
For example `std::shared_ptr< std::string >` can be used.
Such form of buffers was introduced for dealing with the cases
when there are lots of parallel requests that must be served with the same response
(or partly the same, so identical parts can be wrapped in shared buffers).
