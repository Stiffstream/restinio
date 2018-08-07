#include <iostream>

#include <restinio/all.hpp>

using resp_parts_container_t = std::vector< std::string >;

resp_parts_container_t g_response_parts{
	R"-STR-(<!doctype html>
	<html>)-STR-",
	R"-STR-(
<head>
	<meta charset="utf-8">
	<meta name="viewport" content="width=device-width">
	<meta name="author" content="RESTinio">
	<meta name="description" content="RESTinio notificators">
	<title>RESTinio notificators sample</title>
	<style type="text/css">body {font-size: 1.1em; line-height: 1.5em; max-width: 45em; margin: auto; padding: 0 2%;} img {max-width: 100%; display: block; margin: .75em auto;}</style>
</head>)-STR-",
	R"-STR-(
<body>
	<p>This page is provided by RESTinio sample using notificators.</p>
</body>)-STR-",
	R"-STR-(
</html>)-STR-" };


using resp_builder_t = restinio::response_builder_t< restinio::chunked_output_t >;
using resp_builder_shared_ptr_t = std::shared_ptr< resp_builder_t >;

void
stream_response_chunks(
	resp_builder_shared_ptr_t resp,
	resp_parts_container_t::const_iterator it )
{
	if( end( g_response_parts ) == it )
	{
		resp->done();
	}
	else
	{
		resp->append_chunk( restinio::const_buffer( it->data(), it->size() ) );

		resp->flush( [ r = resp,
				it = ++it ]( const restinio::asio_ns::error_code & ec ) mutable {

				if( !ec )
				{
					stream_response_chunks( std::move( r ), it );
				}
			} );
	}
}

// Create request handler.
restinio::request_handling_status_t handler( restinio::request_handle_t req )
{
	if( restinio::http_method_get() == req->header().method() &&
		req->header().request_target() == "/" )
	{
		std::thread handler_thread{ [ req = std::move( req ) ]{

			auto resp = req->create_response< restinio::chunked_output_t >()
				.append_header( restinio::http_field::server, "RESTinio notificators sample" )
				.append_header_date_field()
				.append_header(
					restinio::http_field::content_type,
					"text/html; charset=utf-8" );

			stream_response_chunks(
				std::make_shared< resp_builder_t >( std::move( resp ) ),
				begin( g_response_parts ) );
		} };

		handler_thread.detach();

		return restinio::request_accepted();
	}

	return restinio::request_rejected();
}

int main()
{
	try
	{
		restinio::run(
			restinio::on_thread_pool( std::thread::hardware_concurrency() )
				.port( 8080 )
				.address( "localhost" )
				.max_pipelined_requests( 4 )
				.request_handler( handler ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
