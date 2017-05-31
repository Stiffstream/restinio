#include <type_traits>
#include <iostream>
#include <chrono>
#include <memory>
#include <vector>

#include <asio.hpp>
#include <asio/ip/tcp.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <restinio/all.hpp>

struct book_t
{
	book_t() = default;

	book_t(
		std::string author,
		std::string title )
		:	m_author{ std::move( author ) }
		,	m_title{ std::move( title ) }
	{}

	std::string m_author;
	std::string m_title;
};

using book_collection_t = std::vector< book_t >;

namespace rr = restinio::router;
using router_t = rr::express_router_t;

template < typename RESP >
RESP
init_resp( RESP resp )
{
	resp
		.append_header( "Server", "RESTinio sample server /v.0.2" )
		.append_header_date_field()
		.append_header( "Content-Type", "text/plain; charset=utf-8" );

	return resp;
};

auto server_handler( book_collection_t & book_collection )
{
	auto router = std::make_unique< router_t >();

	router->http_get(
		"/",
		[ &book_collection ]( auto req, auto ){
				auto resp = init_resp( req->create_response() );

				resp.set_body(
					"Book collection (book count: " +
						std::to_string( book_collection.size() ) + ")\n" );

				for( std::size_t i = 0; i < book_collection.size(); ++i )
				{
					resp.append_body( std::to_string( i + 1 ) + ". " );
					const auto & b = book_collection[ i ];
					resp.append_body( b.m_title + "[" + b.m_author + "]\n" );
				}

				return resp.done();
		} );

	router->http_get(
		R"(/:booknum(\d+))",
		[ &book_collection ]( auto req, auto params ){
				const std::size_t booknum = std::atoi( params[ "booknum" ].c_str() );

				auto resp = init_resp( req->create_response() );

				if( 0 != booknum && booknum < book_collection.size() )
				{
					const auto & b = book_collection[ booknum - 1 ];
					resp.set_body(
						"Book #" + std::to_string( booknum ) + " is: " +
							b.m_title + "[" + b.m_author + "]\n" );
				}
				else
				{
					resp.set_body(
						"No book with #" + std::to_string( booknum ) + "\n" );
				}

				return resp.done();
		} );

	router->http_get(
		"/author/:author",
		[ &book_collection ]( auto req, auto params ){
				const auto & author = params[ "author" ];

				auto resp = init_resp( req->create_response() );
				resp.set_body( "Books of " + author + ":\n" );

				for( std::size_t i = 0; i < book_collection.size(); ++i )
				{
					const auto & b = book_collection[ i ];
					if( author == b.m_author )
					{
						resp.append_body( std::to_string( i + 1 ) + ". " );
						resp.append_body( b.m_title + "[" + b.m_author + "]\n" );
					}
				}

				return resp.done();
		} );

	router->http_post(
		"/",
		[ &book_collection ]( auto req, auto ){
			// TODO

			auto resp = init_resp( req->create_response() );

			return resp.done();
		} );

	router->http_put(
		R"(/:booknum(\d+))",
		[ &book_collection ]( auto req, auto params ){
			// TODO
			const std::size_t booknum = std::atoi( params[ "booknum" ].c_str() );


			auto resp = init_resp( req->create_response() );

			return resp.done();
		} );

	router->http_delete(
		R"(/:booknum(\d+))",
		[ &book_collection ]( auto req, auto params ){
			const std::size_t booknum = std::atoi( params[ "booknum" ].c_str() );

			auto resp = init_resp( req->create_response() );

			if( 0 != booknum && booknum < book_collection.size() )
			{
				const auto & b = book_collection[ booknum - 1 ];
				resp.set_body(
					"Delete book #" + std::to_string( booknum ) + ": " +
						b.m_title + "[" + b.m_author + "]\n" );

				book_collection.erase( book_collection.begin() + ( booknum - 1 ) );
			}
			else
			{
				resp.set_body(
					"No book with #" + std::to_string( booknum ) + "\n" );
			}

			return resp.done();
		} );

	return router;
}

int main()
{
	using namespace std::chrono;

	try
	{
		using http_server_t =
			restinio::http_server_t<
				restinio::traits_t<
					restinio::asio_timer_factory_t,
					restinio::single_threaded_ostream_logger_t,
					router_t > >;

		book_collection_t book_collection;
		book_collection.emplace_back( "Agatha Christie", "Murder on the Orient Express" );
		book_collection.emplace_back( "Agatha Christie", "Sleeping Murder" );
		book_collection.emplace_back( "B. Stroustrup", " The C++ Programming Language" );

		http_server_t http_server{
			restinio::create_child_io_service( 1 ),
			[&]( auto & settings ){
				settings
					.request_handler( server_handler( book_collection ) )
					.read_next_http_message_timelimit( 10s )
					.write_http_response_timelimit( 1s )
					.handle_request_timeout( 1s );
			} };

		http_server.open();

		// Wait for quit command.
		std::cout << "Type \"quit\" or \"q\" to quit." << std::endl;

		std::string cmd;
		do
		{
			std::cin >> cmd;
		} while( cmd != "quit" && cmd != "q" );

		http_server.close();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
