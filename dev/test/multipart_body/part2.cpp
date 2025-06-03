/*
	restinio
*/

#include "common.ipp"

TEST_CASE( "No Content-Type field", "[content-type]" )
{
	using namespace restinio::multipart_body;

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			restinio::http_request_header_t{},
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

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

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

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

		restinio::no_extra_data_factory_t extra_data_factory;
		auto req = std::make_shared< restinio::request_t >(
				restinio::request_id_t{1},
				std::move(dummy_header),
				"Body"s,
				dummy_connection_t::make(1u),
				make_dummy_endpoint(),
				extra_data_factory );

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

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

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

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"--1234567890\r\n"
			"Content-Disposition: form-data; name=\"file\"; filename=\"t.txt\"\r\n"
			"\r\n"
			"Hello, World!\r\n"
			"--1234567890--\r\n"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

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

	restinio::no_extra_data_factory_t extra_data_factory;
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
			make_dummy_endpoint(),
			extra_data_factory );

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

