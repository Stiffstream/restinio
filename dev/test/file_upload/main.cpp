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

TEST_CASE( "No Content-Type field", "[content-type]" )
{
	using namespace restinio::file_upload;

	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			restinio::http_request_header_t{},
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	REQUIRE( enumeration_error_t::content_type_field_not_found ==
			enumerate_parts_with_files(
					*req,
					[]( const part_description_t & ) {
						return handling_result_t::continue_enumeration;
					} ).error() );
}

TEST_CASE( "Empty Content-Type field", "[content-type]" )
{
	using namespace restinio::file_upload;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::content_type,
			""s );

	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	REQUIRE( enumeration_error_t::content_type_field_parse_error ==
			enumerate_parts_with_files(
					*req,
					[]( const part_description_t & ) {
						return handling_result_t::continue_enumeration;
					} ).error() );
}

TEST_CASE( "Inappropriate Content-Type media-type", "[content-type]" )
{
	using namespace restinio::file_upload;

	std::vector< std::string > types{
		"text/plain",
		"multipart/*",
		"*/form-data",
		"*/*"
	};

	for( const auto & t : types )
	{
		restinio::http_request_header_t dummy_header{
				restinio::http_method_post(),
				"/"
		};
		dummy_header.set_field(
				restinio::http_field::content_type,
				t );

		auto req = std::make_shared< restinio::request_t >(
				restinio::request_id_t{1},
				std::move(dummy_header),
				"Body"s,
				dummy_connection_t::make(1u),
				make_dummy_endpoint() );

		REQUIRE( enumeration_error_t::content_type_field_inappropriate_value ==
				enumerate_parts_with_files(
						*req,
						[]( const part_description_t & ) {
							return handling_result_t::continue_enumeration;
						} ).error() );
	}
}

TEST_CASE( "Empty body", "[empty-body]" )
{
	using namespace restinio::file_upload;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::content_type,
			"multipart/form-data; boundary=1234567890" );

	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	REQUIRE( enumeration_error_t::no_parts_found ==
			enumerate_parts_with_files(
					*req,
					[]( const part_description_t & ) {
						return handling_result_t::continue_enumeration;
					} ).error() );
}

TEST_CASE( "Just one part", "[body]" )
{
	using namespace restinio::file_upload;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::content_type,
			"multipart/form-data; boundary=1234567890" );

	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"--1234567890\r\n"
			"Content-Disposition: form-data; name=\"file\"; filename=\"t.txt\"\r\n"
			"\r\n"
			"Hello, World!\r\n"
			"--1234567890--\r\n"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	const auto result = enumerate_parts_with_files(
			*req,
			[]( const part_description_t & part ) {
				REQUIRE( "Hello, World!" == part.body );
				REQUIRE( "file" == part.name );
				REQUIRE( "t.txt" == part.filename );
				REQUIRE( !part.filename_star );

				return handling_result_t::continue_enumeration;
			} );

	REQUIRE( result );
	REQUIRE( 1u == *result );
}

TEST_CASE( "Several parts in the body", "[body]" )
{
	using namespace restinio::file_upload;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::content_type,
			"multipart/form-data; boundary=1234567890" );

	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"--1234567890\r\n"
			"Content-Disposition: form-data; name=\"file1\"; filename=\"t1.txt\"\r\n"
			"\r\n"
			"Hello, World!\r\n"
			"--1234567890\r\n"
			"Content-Disposition: form-data; name=\"text\"\r\n"
			"\r\n"
			"Hello, World!\r\n"
			"--1234567890\r\n"
			"Content-Disposition: form-data; name=\"file2\"; filename*=utf-8''t2.txt\r\n"
			"\r\n"
			"Bye, World!\r\n"
			"\r\n"
			"--1234567890\r\n"
			"Content-Disposition: form-data; name=\"another-text\"\r\n"
			"\r\n"
			"Hello, World!\r\n"
			"--1234567890\r\n"
			"Content-Disposition: form-data; name=\"file3\"; "
					"filename=\"t31.txt\"; "
					"filename*=utf-8''t32.txt\r\n"
			"\r\n"
			"Bye, Bye!\r\n"
			"--1234567890--\r\n"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	int ordinal{0};
	const auto result = enumerate_parts_with_files(
			*req,
			[&ordinal]( part_description_t part ) {
				REQUIRE( ordinal < 3 );
				if( 0 == ordinal )
				{
					REQUIRE( "Hello, World!" == part.body );
					REQUIRE( "file1" == part.name );
					REQUIRE( "t1.txt" == part.filename );
					REQUIRE( !part.filename_star );
				}
				else if( 1 == ordinal )
				{
					REQUIRE( "Bye, World!\r\n" == part.body );
					REQUIRE( "file2" == part.name );
					REQUIRE( "utf-8''t2.txt" == part.filename_star );
					REQUIRE( !part.filename );
				}
				else if( 2 == ordinal )
				{
					REQUIRE( "Bye, Bye!" == part.body );
					REQUIRE( "file3" == part.name );
					REQUIRE( "t31.txt" == part.filename );
					REQUIRE( "utf-8''t32.txt" == part.filename_star );
				}

				++ordinal;

				return handling_result_t::continue_enumeration;
			} );

	REQUIRE( result );
	REQUIRE( 3u == *result );
	REQUIRE( 3 == ordinal );
}

