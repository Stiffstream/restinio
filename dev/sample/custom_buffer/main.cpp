#include <iostream>

#include <restinio/all.hpp>

class custom_buffer_t
{
	public:
		custom_buffer_t(
			std::unique_ptr< char[] > buf,
			std::size_t buf_size )
			:	m_buf{ std::move( buf ) }
			,	m_buf_size{ buf_size }
		{}

		// No copy, only moves.
		custom_buffer_t( const custom_buffer_t & ) = delete;
		custom_buffer_t & operator = ( const custom_buffer_t & ) = delete;

		custom_buffer_t( custom_buffer_t && ) = default;
		custom_buffer_t & operator = ( custom_buffer_t && ) = default;

		auto data() const noexcept { return m_buf.get(); }
		auto size() const noexcept { return m_buf_size; }

	private:
		std::unique_ptr< char[] > m_buf;
		std::size_t m_buf_size;
};

// Create a custom buffer.
custom_buffer_t make_resp_body()
{
	constexpr std::size_t buf_size{ 12 };
	std::unique_ptr< char[] > buf{ new char[ buf_size ] };

	std::memcpy( buf.get(), "Hello world!", buf_size );

	return custom_buffer_t{ std::move( buf ), buf_size };
}

// Create request handler.
restinio::request_handling_status_t handler( const restinio::request_handle_t& req )
{
	if( restinio::http_method_get() == req->header().method() &&
		req->header().request_target() == "/" )
	{
		req->create_response()
			.append_header( restinio::http_field::server, "RESTinio hello world server" )
			.append_header_date_field()
			.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
			.set_body( make_resp_body() )
			.done();

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
				.request_handler( handler ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
