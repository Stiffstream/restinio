#include <iostream>

#include <restinio/all.hpp>
#include <so_5/all.hpp>

using resp_parts_container_t = std::vector< std::string >;

// Response divided into parts.
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

// Request handler object.
// Iterates over g_response_parts sending portions of response data
// waiting for a previous portion to be written before sending next porion.
class a_req_handler_t final : public so_5::agent_t
{
		using so_base_type_t = so_5::agent_t;

	public:
		a_req_handler_t(
			context_t ctx,
			const restinio::request_handle_t& req )
			:	so_base_type_t{ std::move( ctx ) }
			,	m_resp{ req->create_response< restinio::chunked_output_t >() }
			,	m_part_it{ begin( g_response_parts ) }
		{
			so_subscribe_self().event( &a_req_handler_t::evt_stream_next_chunk );
		};

		void so_evt_start() override
		{
			// Set header and start response sending circle.

			m_resp
				.append_header(
					restinio::http_field::server,
					"RESTinio notificators sample" )
				.append_header_date_field()
				.append_header(
					restinio::http_field::content_type,
					"text/html; charset=utf-8" );

			evt_stream_next_chunk( restinio::asio_ns::error_code{} );
		}

	private:
		// An iteration of sending data to client.
		void evt_stream_next_chunk( const restinio::asio_ns::error_code & ec )
		{
			if( !ec )
			{
				// If previous part finished with seccess status
				// We can move to the next part.

				if( end( g_response_parts ) != m_part_it )
				{
					// Send next part.
					m_resp.append_chunk(
						restinio::const_buffer(
							m_part_it->data(),
							m_part_it->size() ) );

					m_resp.flush( [ mbox = so_direct_mbox()]( const auto & ec ){
							so_5::send< restinio::asio_ns::error_code >( mbox, ec );
						} );

					++m_part_it;

					// No dereg must be invoked
					return;
				}
				else
				{
					// Response was served.
					// Complete the response:
					m_resp.done();
				}
			}

			// In case of error or response complete - deregister the agent.
			so_deregister_agent_coop( so_5::dereg_reason::normal );
		}

		restinio::response_builder_t< restinio::chunked_output_t > m_resp;
		resp_parts_container_t::const_iterator m_part_it;
};

int main()
{
	try
	{
		so_5::wrapped_env_t sobj{};

		restinio::run(
			restinio::on_thread_pool( std::thread::hardware_concurrency() )
				.port( 8080 )
				.address( "localhost" )
				.max_pipelined_requests( 4 )
				.request_handler( [&]( const restinio::request_handle_t& req ){
					sobj.environment()
						.register_agent_as_coop(
							so_5::autoname,
							std::make_unique< a_req_handler_t >(
								sobj.environment(),
								req ) );

					return restinio::request_accepted();
				} ) );

		sobj.stop_then_join();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
