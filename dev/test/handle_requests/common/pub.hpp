#pragma once

#include <string>

#include <asio.hpp>

constexpr std::uint16_t
utest_default_port()
{
	return 8085;
}


template < typename LAMBDA >
void
do_with_socket(
	LAMBDA && lambda,
	const std::string & addr = "127.0.0.1",
	std::uint16_t port = utest_default_port() )
{
	asio::io_service io_service;
	asio::ip::tcp::socket socket{ io_service };

	asio::ip::tcp::resolver resolver{ io_service };
	asio::ip::tcp::resolver::query
		query{ asio::ip::tcp::v4(), addr, std::to_string( port ) };
	asio::ip::tcp::resolver::iterator iterator = resolver.resolve( query );

	asio::connect( socket, iterator );

	lambda( socket, io_service );
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
		[ & ]( auto & socket, auto & /*io_service*/ ){

			asio::streambuf b;
			std::ostream req_stream(&b);
			req_stream << request;
			asio::write( socket, b );

			std::ostringstream sout;
			asio::streambuf response_stream;
			asio::read_until( socket, response_stream, "\r\n\r\n" );

			sout << &response_stream;

			// Read until EOF, writing data to output as we go.
			asio::error_code error;
			while( asio::read( socket, response_stream, asio::transfer_at_least(1), error) )
				sout << &response_stream;

			if (error != asio::error::eof)
				throw asio::system_error(error);

			result = sout.str();
		},
		addr,
		port );

	return result;
}

