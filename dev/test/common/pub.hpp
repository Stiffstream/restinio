#pragma once

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <restinio/asio_include.hpp>

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>


constexpr std::uint16_t
utest_default_port()
{
// Make it possible to run unit-tests for g++/clang in parallel.
#if defined(__clang__)
	return 8086;
#else
	return 8085;
#endif
}

template < typename LAMBDA >
void
do_with_socket(
	LAMBDA && lambda,
	const std::string & addr = "127.0.0.1",
	std::uint16_t port = utest_default_port() )
{
	restinio::asio_ns::io_context io_context;
	restinio::asio_ns::ip::tcp::socket socket{ io_context };

	restinio::asio_ns::ip::tcp::resolver resolver{ io_context };
	restinio::asio_ns::ip::tcp::resolver::query
		query{ restinio::asio_ns::ip::tcp::v4(), addr, std::to_string( port ) };
	restinio::asio_ns::ip::tcp::resolver::iterator iterator = resolver.resolve( query );

	restinio::asio_ns::connect( socket, iterator );

	lambda( socket, io_context );
	socket.close();
}

inline std::string
do_request(
	const std::string & request,
	const std::string & addr = "127.0.0.1",
	std::uint16_t port = utest_default_port() )
{
	std::string result;
	do_with_socket(
		[ & ]( auto & socket, auto & /*io_context*/ ){

			restinio::asio_ns::streambuf b;
			std::ostream req_stream(&b);
			req_stream << request;
			restinio::asio_ns::write( socket, b );

			std::ostringstream sout;
			restinio::asio_ns::streambuf response_stream;
			restinio::asio_ns::read_until( socket, response_stream, "\r\n\r\n" );

			sout << &response_stream;

			// Read until EOF, writing data to output as we go.
			restinio::asio_ns::error_code error;
			while( restinio::asio_ns::read( socket, response_stream, restinio::asio_ns::transfer_at_least(1), error) )
				sout << &response_stream;

			if ( !restinio::error_is_eof( error ) )
				throw std::runtime_error{ fmt::format( "read error: {}", error ) };

			result = sout.str();
		},
		addr,
		port );

	return result;
}

template<typename Http_Server>
class other_work_thread_for_server_t
{
	Http_Server & m_server;

	std::thread m_thread;
public:
	other_work_thread_for_server_t(
		Http_Server & server )
		: m_server(server)
	{}
	~other_work_thread_for_server_t()
	{
		if( m_thread.joinable() )
			stop_and_join();
	}

	void
	run()
	{
		m_thread = std::thread( [this] {
			m_server.open_sync();
			m_server.io_context().run();
		} );

		// Ensure server was started:
		std::promise< void > p;
		restinio::asio_ns::post(
			m_server.io_context(),
			[&]{
				p.set_value();
			} );
		p.get_future().get();
	}

	void
	stop_and_join()
	{
		restinio::asio_ns::post(
			m_server.io_context(),
			[&]{
				m_server.close_sync();
			} );

		m_thread.join();
	}
};

