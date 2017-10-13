# TLS support

Restinio support HTTS using ASIO ssl facilities (based on OpenSSL).

To create https server it is needed to include extra header file `restinio/tls.hpp`.
This file contains necessary customization classes and structs
that make `restinio::http_server_t` usable as https server.
For specializing `restinio::http_server_t` to work as https server
one should use `restinio::tls_traits_t` (or `restinio::single_thread_tls_traits_t`)
for it and also it is vital to set TLS context using `asio::ssl::context`.
That setting is added to `server_settings_t` class instantiated with TLS traits.

Lets look through an example:
~~~~~
::c++
// ...
using traits_t =
  restinio::single_thread_tls_traits_t<
    restinio::asio_timer_factory_t,
    restinio::single_threaded_ostream_logger_t,
    router_t >;

asio::ssl::context tls_context{ asio::ssl::context::sslv23 };
tls_context.set_options(
  asio::ssl::context::default_workarounds
  | asio::ssl::context::no_sslv2
  | asio::ssl::context::single_dh_use );

tls_context.use_certificate_chain_file( certs_dir + "/server.pem" );
tls_context.use_private_key_file(
  certs_dir + "/key.pem",
  asio::ssl::context::pem );
tls_context.use_tmp_dh_file( certs_dir + "/dh2048.pem" );

restinio::run(
  restinio::on_this_thread< traits_t >()
    .address( "localhost" )
    .request_handler( server_handler() )
    .read_next_http_message_timelimit( 10s )
    .write_http_response_timelimit( 1s )
    .handle_request_timeout( 1s )
    .tls_context( std::move( tls_context ) ) );

// ...
~~~~~

See full [sample](../dev/sample/hello_world_https/main.cpp) for details.
