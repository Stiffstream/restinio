#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

#include <restinio/all.hpp>

// Class for connection state listener and IP-blocker.
class blocker_t
{
	// Because method of blocker_t can be called from
	// different threads at the same time we have to protect connection
	// listener.
	std::mutex m_lock;

	// Type of container for storing information about existing connections.
	using connections_t = std::map<
			restinio::asio_ns::ip::address,
			std::vector< restinio::connection_id_t > >;

	// Info about actual connections.
	connections_t m_connections;

public:
	// This method will be calleb by RESTinio when a new connection
	// is accepted by HTTP-server.
	restinio::ip_blocker::inspection_result_t
	inspect(
		const restinio::ip_blocker::incoming_info_t & info ) noexcept
	{
		std::lock_guard<std::mutex> l{ m_lock };

		const auto it = m_connections.find( info.remote_endpoint().address() );
		if( it == m_connections.end() || it->second.size() < 3u )
			// New connection can be allowed.
			return restinio::ip_blocker::allow();
		else
			// There are 3 connections from that IP.
			// New connection should be disabled.
			return restinio::ip_blocker::deny();
	}

	// This method will be called by RESTinio when state of connection
	// is changed.
	void state_changed(
		const restinio::connection_state::notice_t & notice )
	{
		using namespace restinio::connection_state;

		std::lock_guard<std::mutex> l{ m_lock };

		auto & connections = m_connections[ notice.remote_endpoint().address() ];
		const auto cause = notice.cause();
		if( restinio::holds_alternative< accepted_t >( cause ) )
		{
			// Info about a new connection must be stored.
			connections.push_back( notice.connection_id() );
		}
		else if( restinio::holds_alternative< closed_t >( cause ) )
		{
			// Info about closed connection must be removed.
			connections.erase(
					std::find( std::begin(connections), std::end(connections),
							notice.connection_id() ) );

			// There is no need to hold IP address if there is no
			// more connections for it.
			if( connections.empty() )
				m_connections.erase( notice.remote_endpoint().address() );
		}
		// All other cases just ignored in that example.
	}
};

// Actual request handler.
restinio::request_handling_status_t handler(
	restinio::asio_ns::io_context & ioctx,
	const restinio::request_handle_t& req)
{
	if( restinio::http_method_get() == req->header().method() &&
		req->header().request_target() == "/" )
	{
		// Delay request processing to 15s.
		auto timer = std::make_shared<restinio::asio_ns::steady_timer>( ioctx );
		timer->expires_after( std::chrono::seconds{15} );
		timer->async_wait( [timer, req](const auto & ec) {
				if( !ec ) {
					req->create_response()
						.append_header( restinio::http_field::server, "RESTinio hello world server" )
						.append_header_date_field()
						.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
						.set_body( "Hello world!")
						.done();
				}
			} );

		return restinio::request_accepted();
	}

	return restinio::request_rejected();
}

int main()
{
	try
	{
		// Special traits for our server.
		struct my_traits_t : public restinio::default_traits_t
		{
			using logger_t = restinio::shared_ostream_logger_t;

			// Use the same type as connection state listener and IP-blocker.
			using connection_state_listener_t = blocker_t;
			using ip_blocker_t = blocker_t;
		};

		// External io_context is necessary because we should have access to it.
		restinio::asio_ns::io_context ioctx;

		// IP-blocker should be created and passed to HTTP-server manually.
		auto blocker = std::make_shared<blocker_t>();

		restinio::run(
			ioctx,
			restinio::on_thread_pool<my_traits_t>( std::thread::hardware_concurrency() )
				.port( 8080 )
				.address( "localhost" )
				.connection_state_listener( blocker )
				.ip_blocker( blocker )
				.max_pipelined_requests( 4 )
				.handle_request_timeout( std::chrono::seconds{20} )
				.request_handler( [&ioctx](auto req) {
						return handler( ioctx, std::move(req) );
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

