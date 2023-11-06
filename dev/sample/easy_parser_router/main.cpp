#include <iostream>

#include <restinio/core.hpp>
#include <restinio/router/easy_parser_router.hpp>

namespace sample
{

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

[[nodiscard]]
book_t
deserialize( std::string_view body )
{
	constexpr std::string_view author_tag{ "author:" };
	constexpr std::string_view title_tag{ "title:" };
	constexpr std::string_view separator{ ";;;" };

	book_t result;

	// Very, very simple parsing. Just to avoid the use of external
	// libraries or writing complex code.
	if( author_tag != body.substr( 0u, author_tag.size() ) )
		throw std::runtime_error{ "Unable to parse (no 'author:' tag)" };
	else
		body = body.substr( author_tag.size() );

	if( auto n = body.find( separator ); n == std::string_view::npos )
		throw std::runtime_error{ "Unable to parse (no value separator #1)" };
	else if( 0u == n )
		throw std::runtime_error{ "Unable to parse (no author name)" };
	else
	{
		result.m_author = body.substr( 0u, n );
		body = body.substr( n + separator.size() );
	}

	if( title_tag != body.substr( 0u, title_tag.size() ) )
		throw std::runtime_error{ "Unable to parse (no 'title:' tag)" };
	else
		body = body.substr( title_tag.size() );

	if( body.empty() )
		throw std::runtime_error{ "Unable to parse (no title)" };

	if( auto n = body.find( separator ); n == std::string_view::npos )
		// The remaining part is the title.
		result.m_title = body;
	else if( 0u == n )
		throw std::runtime_error{ "Unable to parse (no title)" };
	else
	{
		result.m_title = body.substr( 0u, n );

		body = body.substr( n + separator.size() );
		if( !body.empty() )
		{
			// There is some additional data. It's not expected nor allowed.
			throw std::runtime_error{ "Unable to parse (additional data found)" };
		}
	}

	return result;
}

using book_collection_t = std::vector< book_t >;

namespace rr = restinio::router;
namespace epr = restinio::router::easy_parser_router;

using router_t = rr::easy_parser_router_t;

using book_number_t = std::uint32_t;
using author_name_t = std::string;

class books_handler_t
{
public :
	explicit books_handler_t( book_collection_t & books )
		:	m_books( books )
	{}

	books_handler_t( const books_handler_t & ) = delete;
	books_handler_t( books_handler_t && ) = delete;

	auto on_books_list(
		const restinio::request_handle_t& req ) const
	{
		auto resp = init_resp( req->create_response() );

		resp.set_body(
			"Book collection (book count: " +
				std::to_string( m_books.size() ) + ")\n" );

		for( std::size_t i = 0; i < m_books.size(); ++i )
		{
			resp.append_body( std::to_string( i + 1 ) + ". " );
			const auto & b = m_books[ i ];
			resp.append_body( b.m_title + "[" + b.m_author + "]\n" );
		}

		return resp.done();
	}

	auto on_book_get(
		const restinio::request_handle_t& req,
		const book_number_t booknum )
	{
		auto resp = init_resp( req->create_response() );

		if( is_valid_booknum( booknum ) )
		{
			const auto & b = m_books[ booknum - 1 ];
			resp.set_body(
				"Book #" + std::to_string( booknum ) + " is: " +
					b.m_title + " [" + b.m_author + "]\n" );
		}
		else
		{
			resp.set_body(
				"No book with #" + std::to_string( booknum ) + "\n" );
		}

		return resp.done();
	}

	auto on_author_get(
		const restinio::request_handle_t& req,
		const author_name_t & author )
	{
		auto resp = init_resp( req->create_response() );
		try
		{
			resp.set_body( "Books of " + author + ":\n" );

			for( std::size_t i = 0; i < m_books.size(); ++i )
			{
				const auto & b = m_books[ i ];
				if( author == b.m_author )
				{
					resp.append_body( std::to_string( i + 1 ) + ". " );
					resp.append_body( b.m_title + "[" + b.m_author + "]\n" );
				}
			}
		}
		catch( const std::exception & )
		{
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_new_book(
		const restinio::request_handle_t& req )
	{
		auto resp = init_resp( req->create_response() );

		try
		{
			m_books.emplace_back( deserialize( req->body() ) );
		}
		catch( const std::exception & ex )
		{
			std::cerr << "*** on_new_book: exception: " << ex.what() << std::endl;
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_book_update(
		const restinio::request_handle_t& req,
		const book_number_t booknum )
	{
		auto resp = init_resp( req->create_response() );

		try
		{
			auto b = deserialize( req->body() );

			if( is_valid_booknum( booknum ) )
			{
				m_books[ booknum - 1 ] = b;
			}
			else
			{
				mark_as_bad_request( resp );
				resp.set_body( "No book with #" + std::to_string( booknum ) + "\n" );
			}
		}
		catch( const std::exception & ex )
		{
			std::cerr << "*** on_book_update: exception: " << ex.what() << std::endl;
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_book_delete(
		const restinio::request_handle_t& req,
		const book_number_t booknum )
	{
		auto resp = init_resp( req->create_response() );

		if( is_valid_booknum( booknum ) )
		{
			const auto & b = m_books[ booknum - 1 ];
			resp.set_body(
				"Delete book #" + std::to_string( booknum ) + ": " +
					b.m_title + "[" + b.m_author + "]\n" );

			m_books.erase( m_books.begin() + ( booknum - 1 ) );
		}
		else
		{
			resp.set_body(
				"No book with #" + std::to_string( booknum ) + "\n" );
		}

		return resp.done();
	}

private :
	book_collection_t & m_books;

	[[nodiscard]]
	bool
	is_valid_booknum( book_number_t booknum ) const noexcept
	{
		return ( booknum > 0u && booknum <= m_books.size() );
	}

	template < typename RESP >
	static RESP
	init_resp( RESP resp )
	{
		resp
			.append_header( "Server", "RESTinio sample server /v.0.7" )
			.append_header_date_field()
			.append_header( "Content-Type", "text/plain; charset=utf-8" );

		return resp;
	}

	template < typename RESP >
	static void
	mark_as_bad_request( RESP & resp )
	{
		resp.header().status_line( restinio::status_bad_request() );
	}
};

auto server_handler( book_collection_t & book_collection )
{
	auto router = std::make_unique< router_t >();
	auto handler = std::make_shared< books_handler_t >( std::ref(book_collection) );

	auto book_num = epr::non_negative_decimal_number_p< book_number_t >();

	auto by = [&]( auto method ) {
		using namespace std::placeholders;
		return std::bind( method, handler, _1, _2 );
	};
	auto by0 = [&]( auto method ) {
		using namespace std::placeholders;
		return std::bind( method, handler, _1 );
	};
	auto method_not_allowed = []( const auto & req, const auto & ) {
		return req->create_response( restinio::status_method_not_allowed() )
				.connection_close()
				.done();
	};

	// Handlers for '/' path.
	router->http_get(
			epr::path_to_params( "/" ),
			by0( &books_handler_t::on_books_list ) );
	router->http_post(
			epr::path_to_params( "/" ),
			by0( &books_handler_t::on_new_book ) );

	// Disable all other methods for '/'.
	router->add_handler(
			restinio::router::none_of_methods(
					restinio::http_method_get(), restinio::http_method_post() ),
			epr::path_to_tuple( "/" ),
			method_not_allowed );

	// Handlers for '/:booknum' path.
	router->http_get(
			epr::path_to_params( "/", book_num ),
			by( &books_handler_t::on_book_get ) );

	router->http_put(
			epr::path_to_params( "/", book_num ),
			by( &books_handler_t::on_book_update ) );

	router->http_delete(
			epr::path_to_params( "/", book_num ),
			by( &books_handler_t::on_book_delete ) );

	// Disable all other methods for '/:booknum'.
	router->add_handler(
			restinio::router::none_of_methods(
					restinio::http_method_get(),
					restinio::http_method_post(),
					restinio::http_method_delete() ),
			epr::path_to_tuple( "/", book_num ),
			method_not_allowed );

	// Handler for '/author/:author' path.
	router->http_get(
			epr::path_to_params(
					"/author/",
					epr::path_fragment_p() >> epr::unescape() ),
			by( &books_handler_t::on_author_get ) );

	// Disable all other methods for '/author/:author'.
	router->add_handler(
			restinio::router::none_of_methods( restinio::http_method_get() ),
			epr::path_to_tuple(
					"/author/",
					epr::path_fragment_p() >> epr::unescape() ),
			method_not_allowed );

	router->non_matched_request_handler(
		[]( auto req ){
			return req->create_response( restinio::status_not_found() )
					.append_header_date_field()
					.connection_close()
					.done();
		} );

	return router;
}

} /* namespace sample */

int main()
{
	using namespace sample;
	using namespace std::chrono;

	try
	{
		using traits_t =
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				restinio::single_threaded_ostream_logger_t,
				router_t >;

		book_collection_t book_collection{
			{ "Agatha Christie", "Murder on the Orient Express" },
			{ "Agatha Christie", "Sleeping Murder" },
			{ "B. Stroustrup", "The C++ Programming Language" }
		};

		restinio::run(
			restinio::on_this_thread< traits_t >()
				.address( "localhost" )
				.request_handler( server_handler( book_collection ) )
				.read_next_http_message_timelimit( 10s )
				.write_http_response_timelimit( 1s )
				.handle_request_timeout( 1s ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
