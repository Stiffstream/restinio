/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/bearer_auth.hpp>

#include <test/common/dummy_connection.hpp>

using namespace std::string_literals;

RESTINIO_NODISCARD
auto
make_dummy_endpoint()
{
	return restinio::endpoint_t{
			restinio::asio_ns::ip::address::from_string("127.0.0.1"),
			12345u
	};
}

TEST_CASE( "No Authorization field", "[bearer_auth]" )
{
	using namespace restinio::http_field_parsers::bearer_auth;

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			restinio::http_request_header_t{},
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

	const auto result = try_extract_params( *req,
			restinio::http_field::authorization );

	REQUIRE( !result );
	REQUIRE( extraction_error_t::no_auth_http_field == result.error() );
}

TEST_CASE( "Empty Authorization field", "[bearer_auth]" )
{
	using namespace restinio::http_field_parsers::bearer_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			""s );

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

	const auto result = try_extract_params( *req,
			restinio::http_field::authorization );

	REQUIRE( !result );
	REQUIRE( extraction_error_t::illegal_http_field_value == result.error() );
}

TEST_CASE( "Different encoding scheme", "[bearer_auth]" )
{
	using namespace restinio::http_field_parsers::bearer_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"MyScheme param=value, anotherparam=anothervalue"s );

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

	const auto result = try_extract_params( *req,
			restinio::http_field::authorization );

	REQUIRE( !result );
	REQUIRE( extraction_error_t::not_bearer_auth_scheme == result.error() );
}

TEST_CASE( "Wrong Bearer Authentification params", "[bearer_auth]" )
{
	using namespace restinio::http_field_parsers::bearer_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Bearer param=value, anotherparam=anothervalue"s );

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

	const auto result = try_extract_params( *req,
			restinio::http_field::authorization );

	REQUIRE( !result );
	REQUIRE( extraction_error_t::invalid_bearer_auth_param == result.error() );
}

TEST_CASE( "Valid Authorization field", "[bearer_auth]" )
{
	using namespace restinio::http_field_parsers::bearer_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Bearer dXNlcjoxMjM0"s );

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

	const auto result = try_extract_params( *req,
			restinio::http_field::authorization );

	REQUIRE( result );
	REQUIRE( "dXNlcjoxMjM0" == result->token );
}

TEST_CASE( "Valid X-My-Authorization field", "[bearer_auth]" )
{
	using namespace restinio::http_field_parsers::bearer_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Bearer dXNlcjoxMjM0"s );
	dummy_header.set_field(
			"X-My-Authorization",
			"Bearer bXktdXNlcjpteS0xMjM0"s );

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

	const auto result = try_extract_params( *req,
			"x-my-authorization" );

	REQUIRE( result );
	REQUIRE( "bXktdXNlcjpteS0xMjM0" == result->token );
}

TEST_CASE( "Extract from parsed authorization_value_t", "[bearer_auth]" )
{
	using namespace restinio::http_field_parsers;
	using namespace restinio::http_field_parsers::bearer_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Bearer dXNlcjoxMjM0"s );
	dummy_header.set_field(
			"X-My-Authorization",
			"Bearer bXktdXNlcjpteS0xMjM0"s );

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

	const auto field = req->header().opt_value_of( "x-my-authorization" );
	REQUIRE( field );

	const auto field_parse_result = authorization_value_t::try_parse( *field );
	REQUIRE( field_parse_result );
	REQUIRE( "bearer" == field_parse_result->auth_scheme );

	const auto result = try_extract_params( *field_parse_result );

	REQUIRE( result );
	REQUIRE( "bXktdXNlcjpteS0xMjM0" == result->token );
}

