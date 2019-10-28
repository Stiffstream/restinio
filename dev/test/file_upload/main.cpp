/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/file_upload.hpp>

using namespace std::string_literals;

class dummy_connection_t : public restinio::impl::connection_base_t
{
public:
	using restinio::impl::connection_base_t::connection_base_t;

	void
	write_response_parts(
		restinio::request_id_t /*request_id*/,
		restinio::response_output_flags_t /*response_output_flags*/,
		restinio::write_group_t /*wg*/ ) override
	{ /* Nothing to do! */ }

	void
	check_timeout(
		std::shared_ptr< restinio::tcp_connection_ctx_base_t > & /*self*/ ) override
	{ /* Nothing to do! */ }

	RESTINIO_NODISCARD
	static auto
	make( restinio::connection_id_t id )
	{
		return std::make_shared< dummy_connection_t >( id );
	}

};

RESTINIO_NODISCARD
auto
make_dummy_endpoint()
{
	return restinio::endpoint_t{
			restinio::asio_ns::ip::address::from_string("127.0.0.1"),
			12345u
	};
}

TEST_CASE( "Simple", "[simple]" )
{
	using namespace restinio::file_upload;

	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			restinio::http_request_header_t{},
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	REQUIRE( enumeration_result_t::unexpected_error ==
			enumerate_parts_with_files( *req, [](){} ) );
}

