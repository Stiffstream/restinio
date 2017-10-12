# Response builder

Lets consider that we are at the point when response
on a particular request is ready to be created and send.
The key here is to use a given connection handle and
[response_builder_t](../dev/restinio/message_builders.hpp) that is created by
this connection handle:

Basic example of default response builder:
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
Each builder type is a specialization of a template class `response_builder_t< Output >`
with a specific output-type:

* Output `restinio_controlled_output_t`. Simple standard response builder.
* Output `user_controlled_output_t`. User controlled response output builder.
* Output `chunked_output_t`. Chunked transfer encoding output builder.

## RESTinio controlled output response builder

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

## User controlled output response builder

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

## Chunked transfer encoding output builder

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
