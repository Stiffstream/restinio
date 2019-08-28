#include <iostream>

#include <restinio/all.hpp>
#include <restinio/tls.hpp>

#include <fmt/format.h>

class user_connections_t
{
public :
	user_connections_t() = default;
	user_connections_t( const user_connections_t & ) = delete;
	user_connections_t( user_connections_t && ) = delete;

	void add(
		restinio::connection_id_t conn_id,
		std::string user_name )
	{
		fmt::print( "*** Adding information about a user '{}', conn_id={} ***\n",
				user_name, conn_id );

		m_data.emplace( conn_id, std::move(user_name) );
	}

	const std::string & query( restinio::connection_id_t conn_id ) const
	{
		const auto it = m_data.find( conn_id );
		if( it == m_data.end() )
			throw std::runtime_error(
					fmt::format( "unable to find info for connection with id={}",
							conn_id ) );
		return it->second;
	}

	void remove( restinio::connection_id_t conn_id ) noexcept
	{
		fmt::print( "*** Removing information about conn_id={} ***\n",
				conn_id );

		m_data.erase( conn_id );
	}

private :
	std::map< restinio::connection_id_t, std::string > m_data;
};

using user_connections_shptr_t = std::shared_ptr< user_connections_t >;

template < typename RESP >
RESP
init_resp( RESP resp )
{
	resp.append_header( restinio::http_field::server, "RESTinio sample server /v.0.2" );
	resp.append_header_date_field();

	return resp;
}

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

auto server_handler( const user_connections_shptr_t& user_connections )
{
	auto router = std::make_unique< router_t >();

	router->http_get(
		"/",
		[]( auto req, auto ){
				init_resp( req->create_response() )
					.append_header(
							restinio::http_field::content_type,
							"text/plain; charset=utf-8" )
					.set_body( "Try to access /all or /limited resources on that server\n")
					.done();

				return restinio::request_accepted();
		} );

	router->http_get(
		"/all",
		[user_connections]( auto req, auto ){
				init_resp( req->create_response() )
					.append_header(
							restinio::http_field::content_type,
							"text/plain; charset=utf-8" )
					.set_body(
							fmt::format(
								"There is no any restrictions "
										"for user '{}' on that resource\n",
								user_connections->query( req->connection_id() ) ) )
					.done();

				return restinio::request_accepted();
		} );

	router->http_get(
		"/limited",
		[user_connections]( auto req, auto ){
				auto resp = init_resp( req->create_response() )
					.append_header(
							restinio::http_field::content_type,
							"text/plain; charset=utf-8" );

				const auto & user = user_connections->query( req->connection_id() );
				if( "alice" == user )
					resp.set_body(
							fmt::format(
									"User '{}' have access to limited resource\n",
									user ) );
				else
					resp.set_body(
							fmt::format(
									"User '{}' haven't access to limited resource\n",
									user ) );

				resp.done();

				return restinio::request_accepted();
		} );

	return router;
}

struct openssl_free_t {
	void operator()(void * ptr) const noexcept
	{
		OPENSSL_free( ptr );
	}
};

std::string extract_user_name_from_client_certificate(
	const restinio::connection_state::tls_accessor_t & info )
{
	auto nhandle = info.native_handle();

	std::unique_ptr<X509, decltype(&X509_free)> client_cert{
			SSL_get_peer_certificate(nhandle),
			X509_free
	};
	if( !client_cert )
		throw std::runtime_error( "Unable to get client certificate!" );

	X509_NAME * subject_name = X509_get_subject_name( client_cert.get() );

	int last_pos = -1;
	last_pos = X509_NAME_get_index_by_NID(
			subject_name,
			NID_commonName,
			last_pos );
	if( last_pos < 0 )
		throw std::runtime_error( "commonName is not found!" );

	unsigned char * common_name_utf8{};
	if( ASN1_STRING_to_UTF8(
			&common_name_utf8,
			X509_NAME_ENTRY_get_data(
					X509_NAME_get_entry( subject_name, last_pos ) ) ) < 0 )
		throw std::runtime_error( "ASN1_STRING_to_UTF8 failed!" );

	std::unique_ptr<unsigned char, openssl_free_t > common_name_deleter{
			common_name_utf8
		};

	return { reinterpret_cast<char *>(common_name_utf8) };
}

class my_tls_inspector_t
{
public:
	explicit my_tls_inspector_t(
		user_connections_shptr_t user_connections )
		:	m_user_connections{ std::move(user_connections) }
	{}

	void state_changed(
		const restinio::connection_state::notice_t & notice)
	{
		restinio::visit(
				notice_visitor_t{ *m_user_connections, notice },
				notice.cause() );
   }

public:
	user_connections_shptr_t m_user_connections;

	struct notice_visitor_t {
		user_connections_t & m_user_connections;
		const restinio::connection_state::notice_t & m_notice;

		void operator()(
			const restinio::connection_state::accepted_t & cause ) const
		{
			cause.inspect_tls_or_throw(
				[&]( const restinio::connection_state::tls_accessor_t & tls ) {
					m_user_connections.add(
							m_notice.connection_id(),
							extract_user_name_from_client_certificate( tls ) );
				} );
		}

		void operator()(
			const restinio::connection_state::closed_t & ) const noexcept
		{
			m_user_connections.remove( m_notice.connection_id() );
		}

		void operator()(
			const restinio::connection_state::upgraded_to_websocket_t & ) const noexcept
		{
			m_user_connections.remove( m_notice.connection_id() );
		}
	};
};

int main( int argc, const char * argv[] )
{
	using namespace std::chrono;

	std::string certs_dir = ".";

	if( 2 == argc )
	{
		certs_dir = argv[ 1 ];
	}

	try
	{
		struct traits_t : public restinio::single_thread_tls_traits_t<
				restinio::asio_timer_manager_t,
				restinio::single_threaded_ostream_logger_t,
				router_t >
		{
			using connection_state_listener_t = my_tls_inspector_t;
		};

		// Since RESTinio supports both stand-alone ASIO and boost::ASIO
		// we specify an alias for a concrete asio namesace.
		// That's makes it possible to compile the code in both cases.
		// Typicaly only one of ASIO variants would be used,
		// and so only asio::* or only boost::asio::* would be applied.
		namespace asio_ns = restinio::asio_ns;

		asio_ns::ssl::context tls_context{ asio_ns::ssl::context::sslv23 };
		tls_context.set_options(
			asio_ns::ssl::context::default_workarounds
			| asio_ns::ssl::context::no_sslv2
			| asio_ns::ssl::context::single_dh_use );

		tls_context.use_certificate_chain_file( certs_dir + "/server.cer" );
		tls_context.use_private_key_file(
			certs_dir + "/server.key",
			asio_ns::ssl::context::pem );
		tls_context.set_verify_mode(
			asio_ns::ssl::verify_peer
			| asio_ns::ssl::verify_fail_if_no_peer_cert );
		tls_context.load_verify_file( certs_dir + "/ca.cer" );
		tls_context.use_tmp_dh_file( certs_dir + "/dh2048.pem" );

		auto user_connections = std::make_shared< user_connections_t >();
		auto tls_inspector = std::make_shared< my_tls_inspector_t >(
				user_connections );

		restinio::run(
			restinio::on_this_thread< traits_t >()
				.address( "localhost" )
				.request_handler( server_handler( user_connections ) )
				.read_next_http_message_timelimit( 10s )
				.write_http_response_timelimit( 1s )
				.handle_request_timeout( 1s )
				.tls_context( std::move( tls_context ) )
				.connection_state_listener( tls_inspector ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
