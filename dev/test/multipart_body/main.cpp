/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/multipart_body.hpp>

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

TEST_CASE( "Valid boundary value", "[boundary]" )
{
	using namespace restinio::multipart_body;

	REQUIRE( !check_boundary_value( "1" ) );

	{
		const auto r = check_boundary_value( "" );
		REQUIRE( r );
		REQUIRE( enumeration_error_t::illegal_boundary_value == *r );
	}

	{
		const auto r = check_boundary_value(
				"123456789_123456789_123456789_"
				"123456789_123456789_123456789_"
				"123456789_1" );
		REQUIRE( r );
		REQUIRE( enumeration_error_t::illegal_boundary_value == *r );
	}

	{
		const auto r = check_boundary_value(
				"123456789_123456789_123456789_"
				"123456789_123456789_123456789_"
				"123456789_" );
		REQUIRE( !r );
	}

	{
		const auto r = check_boundary_value(
				" _" );
		REQUIRE( !r );
	}

	{
		const auto r = check_boundary_value( "1 " );
		REQUIRE( r );
		REQUIRE( enumeration_error_t::illegal_boundary_value == *r );
	}

	{
		const auto r = check_boundary_value( "123[45]678" );
		REQUIRE( r );
		REQUIRE( enumeration_error_t::illegal_boundary_value == *r );
	}
}

TEST_CASE( "Basics", "[basics]" )
{
	using namespace restinio::multipart_body;

	const restinio::string_view_t boundary{ "--boundary" };

	{
		auto result = split_multipart_body( "", boundary );
		REQUIRE( result.empty() );
	}

	{
		auto result = split_multipart_body(
				"Some text with boundary word inside",
				boundary );
		REQUIRE( result.empty() );
	}

	{
		auto result = split_multipart_body(
				"Some text with --boundary inside",
				boundary );
		REQUIRE( result.empty() );
	}

	{
		auto result = split_multipart_body(
				"--boundary\r\n"
				"The first part"
				"--boundary--",
				boundary );
		REQUIRE( result.empty() );
	}

	{
		auto result = split_multipart_body(
				"--boundary\r\n"
				"The first part\r\n",
				boundary );
		REQUIRE( result.empty() );
	}

	{
		auto result = split_multipart_body(
				"--boundary\r\n"
				"The first part\r\n\r\n"
				"--boundary--",
				boundary );
		REQUIRE( result.empty() );
	}

	{
		auto result = split_multipart_body(
				"--boundary\r\n"
				"The first part\r\n\r\n"
				"--boundary--\r\n",
				boundary );
		REQUIRE( 1u == result.size() );
		REQUIRE( "The first part\r\n" == result[0] );
	}

	{
		auto result = split_multipart_body(
				"--boundary\r\n"
				"The first part\r\n"
				"--boundary--\r\n",
				boundary );
		REQUIRE( 1u == result.size() );
		REQUIRE( "The first part" == result[0] );
	}

	{
		auto result = split_multipart_body(
				"Some preamble\r\n"
				"--boundary\r\n"
				"The first part\r\n"
				"--boundary--\r\n",
				boundary );
		REQUIRE( 1u == result.size() );
		REQUIRE( "The first part" == result[0] );
	}

	{
		auto result = split_multipart_body(
				"Some preamble\r\n"
				"--boundary\r\n"
				"The first part\r\n"
				"--boundary--\r\n"
				"Some epilog\r\n"
				"--boundary\r\n"
				"This should be ignored\r\n",
				boundary );
		REQUIRE( 1u == result.size() );
		REQUIRE( "The first part" == result[0] );
	}

	{
		auto result = split_multipart_body(
				"Some preamble\r\n"
				"--boundary\r\n"
				"The first part\r\n"
				"--boundary\r\n"
				"The second part\r\n"
				"\r\n"
				"With empty string inside\r\n"
				"--boundary--\r\n"
				"Some epilog\r\n"
				"--boundary\r\n"
				"This should be ignored\r\n",
				boundary );
		REQUIRE( 2u == result.size() );
		REQUIRE( "The first part" == result[0] );
		REQUIRE( "The second part\r\n"
				"\r\n"
				"With empty string inside" == result[1] );
	}

	{
		auto result = split_multipart_body(
				"Some preamble\r\n"
				"--boundary\r\n"
				"The first part\r\n"
				"--boundary\r\n"
				"The second part\r\n"
				"\r\n"
				"With empty string inside and wrong --boundary mark\r\n"
				"--boundary--\r\n"
				"Some epilog\r\n"
				"--boundary\r\n"
				"This should be ignored\r\n",
				boundary );

		REQUIRE( result.empty() );
	}
}

TEST_CASE( "try_parse_part", "[try_parse_part]" )
{
	using namespace restinio::multipart_body;

	{
		const auto r = try_parse_part( "" );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse_part( " " );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse_part( " body" );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse_part( "content-type: text/plain" );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse_part(
				"content-type: text/plain\r\n"
				"body." );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse_part(
				"content-type: text/plain;\r\n"
				"  boundary=12345567\r\n"
				"\r\n"
				"body." );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse_part(
				"content-type: text/plain;\r\n"
				" content-disposition: form-data;\r\n"
				"\r\n"
				"body." );

		REQUIRE( !r );
	}

	{
		const auto r = try_parse_part( "\r\n" );

		REQUIRE( r );
		REQUIRE( 0u == r->fields.fields_count() );
		REQUIRE( "" == r->body );
	}

	{
		const auto r = try_parse_part(
				"content-type: text/plain\r\n"
				"\r\n"
				"body." );

		REQUIRE( r );
		REQUIRE( 1u == r->fields.fields_count() );
		REQUIRE( r->fields.has_field( "content-type" ) );
		REQUIRE( "text/plain" == r->fields.value_of( "content-type" ) );
		REQUIRE( "body." == r->body );
	}

	{
		const auto r = try_parse_part(
				"content-type: text/plain\r\n"
				"content-disposition: form-data; name=value\r\n"
				"\r\n"
				"body." );

		REQUIRE( r );
		REQUIRE( 2u == r->fields.fields_count() );

		REQUIRE( r->fields.has_field( "content-type" ) );
		REQUIRE( "text/plain" == r->fields.value_of( "content-type" ) );

		REQUIRE( r->fields.has_field( "content-disposition" ) );
		REQUIRE( "form-data; name=value" ==
				r->fields.value_of( "content-disposition" ) );

		REQUIRE( "body." == r->body );
	}

	{
		const auto r = try_parse_part(
				"content-type: text/plain\r\n"
				"content-disposition: form-data; name=value\r\n"
				"\r\n"
				"\r\n"
				"\r\n"
				"body." );

		REQUIRE( r );
		REQUIRE( 2u == r->fields.fields_count() );

		REQUIRE( r->fields.has_field( "content-type" ) );
		REQUIRE( "text/plain" == r->fields.value_of( "content-type" ) );

		REQUIRE( r->fields.has_field( "content-disposition" ) );
		REQUIRE( "form-data; name=value" ==
				r->fields.value_of( "content-disposition" ) );

		REQUIRE( "\r\n\r\nbody." == r->body );
	}
}

TEST_CASE( "No Content-Type field", "[content-type]" )
{
	using namespace restinio::multipart_body;

	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			restinio::http_request_header_t{},
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	REQUIRE( enumeration_error_t::content_type_field_not_found ==
			enumerate_parts(
					*req,
					[]( parsed_part_t && ) {
						return handling_result_t::continue_enumeration;
					} ).error() );
}

TEST_CASE( "Empty Content-Type field", "[content-type]" )
{
	using namespace restinio::multipart_body;

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
			enumerate_parts(
					*req,
					[]( const parsed_part_t & ) {
						return handling_result_t::continue_enumeration;
					} ).error() );
}

TEST_CASE( "Inappropriate Content-Type media-type", "[content-type]" )
{
	using namespace restinio::multipart_body;

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
				enumerate_parts(
						*req,
						[]( const parsed_part_t & ) {
							return handling_result_t::continue_enumeration;
						},
						"MultiPart",
						"Form-Data" ).error() );
	}
}

TEST_CASE( "Empty body", "[empty-body]" )
{
	using namespace restinio::multipart_body;

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
			enumerate_parts(
					*req,
					[]( const parsed_part_t & ) {
						return handling_result_t::continue_enumeration;
					} ).error() );
}

TEST_CASE( "Just one part", "[body]" )
{
	using namespace restinio::multipart_body;

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

	const auto result = enumerate_parts(
			*req,
			[]( const parsed_part_t & part ) {
				REQUIRE( "Hello, World!" == part.body );

				return handling_result_t::continue_enumeration;
			},
			"MultiPart", "Form-Data" );

	REQUIRE( result );
	REQUIRE( 1u == *result );
}

TEST_CASE( "Several parts in the body", "[body]" )
{
	using namespace restinio::multipart_body;

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
			"Content-Disposition: form-data; name=\"file2\"; filename*=\"t2.txt\"\r\n"
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
					"filename*=\"t32.txt\"\r\n"
			"\r\n"
			"Bye, Bye!\r\n"
			"--1234567890--\r\n"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	int ordinal{0};
	const auto result = enumerate_parts(
			*req,
			[&ordinal]( parsed_part_t part ) {
				REQUIRE( ordinal < 5 );
				if( 0 == ordinal )
				{
					REQUIRE( "Hello, World!" == part.body );
				}
				else if( 1 == ordinal )
				{
					REQUIRE( "Hello, World!" == part.body );
				}
				else if( 2 == ordinal )
				{
					REQUIRE( "Bye, World!\r\n" == part.body );
				}
				else if( 3 == ordinal )
				{
					REQUIRE( "Hello, World!" == part.body );
				}
				else if( 4 == ordinal )
				{
					REQUIRE( "Bye, Bye!" == part.body );
				}

				++ordinal;

				return handling_result_t::continue_enumeration;
			} );

	REQUIRE( result );
	REQUIRE( 5u == *result );
	REQUIRE( 5 == ordinal );
}

