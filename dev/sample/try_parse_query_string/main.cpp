#include <iostream>
#include <string>

#include <restinio/all.hpp>

using namespace std::string_literals;

// Type of query-string parser.
using query_string_parser_type =
	restinio::expected_t<
		restinio::query_string_params_t,
		restinio::parse_query_failure_t >
	(*)( restinio::string_view_t );

// Create an appropriate query-string parser.
query_string_parser_type create_parser( int argc, char ** argv )
{
	if( argc < 2 || "restinio_defaults"s == argv[ 1 ] )
		return []( restinio::string_view_t what ) {
			return restinio::try_parse_query<
				restinio::parse_query_traits::restinio_defaults >( what );
		};

	if( 2 == argc )
	{
		if( "javascript_compatible"s == argv[ 1 ] )
			return []( restinio::string_view_t what ) {
				return restinio::try_parse_query<
					restinio::parse_query_traits::javascript_compatible >( what );
			};
		if( "x_www_form_urlencoded"s == argv[ 1 ] )
			return []( restinio::string_view_t what ) {
				return restinio::try_parse_query<
					restinio::parse_query_traits::x_www_form_urlencoded >( what );
			};
		if( "relaxed"s == argv[ 1 ] ) {
			return []( restinio::string_view_t what ) {
				return restinio::try_parse_query<
					restinio::parse_query_traits::relaxed >( what );
			};
		}
	}

	throw std::runtime_error{
			"one optional argument expected: "
			"restinio_defaults, javascript_compatible, x_www_form_urlencoded or "
			"relaxed" 
		};
}

// Create request handler.
restinio::request_handling_status_t handler(
	const restinio::request_handle_t& req,
	query_string_parser_type parser )
{
	if( restinio::http_method_get() == req->header().method() )
	{
		fmt::basic_memory_buffer< char, 1u > response_body;
		fmt::format_to( response_body, "GET request to '{}'\n",
				req->header().request_target() );

		// Request header fields.
		fmt::format_to( response_body, "HTTP-fields ({}):\n",
				req->header().fields_count() );
		for( const auto & f : req->header() )
		{
			fmt::format_to( response_body, "{}: {}\n",
					f.name(), f.value() );
		}

		// An attempt to parse query-string.
		const auto qs_parse_result = parser( req->header().query() );
		if( !qs_parse_result )
		{
			// Return a negative response.
			req->create_response( restinio::status_bad_request() )
				.append_header( restinio::http_field::server, "RESTinio query string params server" )
				.append_header_date_field()
				.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
				.set_body( "Unable to parse query-string: " +
						qs_parse_result.error().description() )
				.done();

			return restinio::request_accepted();
		}

		// Handle query-string params.
		const auto & qp = *qs_parse_result;
		if( qp.empty() )
		{
			fmt::format_to( response_body, "No query parameters." );
		}
		else
		{
			fmt::format_to( response_body, "Query params ({}):\n", qp.size() );

			for( const auto p : qp )
			{
				fmt::format_to( response_body, "'{}' => '{}'\n",
						p.first, p.second );
			}
		}

		if( qp.has( "debug" ) && qp[ "debug" ] == "true" )
		{
			std::cout.write( response_body.data(), response_body.size() );
			std::cout << std::flush;
		}

		req->create_response()
			.append_header( restinio::http_field::server, "RESTinio query string params server" )
			.append_header_date_field()
			.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
			.set_body( std::move(response_body) )
			.done();

		return restinio::request_accepted();
	}

	return restinio::request_rejected();
}

int main( int argc, char ** argv )
{
	try
	{
		restinio::run(
			restinio::on_thread_pool( std::thread::hardware_concurrency() )
				.port( 8080 )
				.address( "localhost" )
				.request_handler(
					[parser = create_parser( argc, argv )]( const auto & req ) {
						return handler( req, parser );
					} )
			);
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

