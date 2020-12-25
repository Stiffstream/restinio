/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/basic_auth.hpp>
#include <restinio/helpers/http_field_parsers/try_parse_field.hpp>

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

TEST_CASE( "No Authorization field", "[basic_auth]" )
{
	using namespace restinio::http_field_parsers::basic_auth;

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

TEST_CASE( "Empty Authorization field", "[basic_auth]" )
{
	using namespace restinio::http_field_parsers::basic_auth;

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

TEST_CASE( "Different encoding scheme", "[basic_auth]" )
{
	using namespace restinio::http_field_parsers::basic_auth;

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
	REQUIRE( extraction_error_t::not_basic_auth_scheme == result.error() );
}

TEST_CASE( "Wrong Basic Authentification params", "[basic_auth]" )
{
	using namespace restinio::http_field_parsers::basic_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Basic param=value, anotherparam=anothervalue"s );

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
	REQUIRE( extraction_error_t::invalid_basic_auth_param == result.error() );
}

TEST_CASE( "No semicolon in username:password pair", "[basic_auth]" )
{
	using namespace restinio::http_field_parsers::basic_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Basic dXNlcnBhc3N3b3Jk"s );

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
	REQUIRE( extraction_error_t::invalid_username_password_pair == result.error() );
}

TEST_CASE( "Empty username in username:password pair", "[basic_auth]" )
{
	using namespace restinio::http_field_parsers::basic_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Basic OnBhc3N3b3Jk"s );

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
	REQUIRE( extraction_error_t::empty_username == result.error() );
}

TEST_CASE( "Valid Authorization field", "[basic_auth]" )
{
	using namespace restinio::http_field_parsers::basic_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Basic dXNlcjoxMjM0"s );

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
	REQUIRE( "user" == result->username );
	REQUIRE( "1234" == result->password );
}

TEST_CASE( "Valid Authorization field with empty password", "[basic_auth]" )
{
	using namespace restinio::http_field_parsers::basic_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Basic dXNlcjo="s );

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
	REQUIRE( "user" == result->username );
	REQUIRE( "" == result->password );
}

TEST_CASE( "Valid X-My-Authorization field", "[basic_auth]" )
{
	using namespace restinio::http_field_parsers::basic_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Basic dXNlcjoxMjM0"s );
	dummy_header.set_field(
			"X-My-Authorization",
			"Basic bXktdXNlcjpteS0xMjM0"s );

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
	REQUIRE( "my-user" == result->username );
	REQUIRE( "my-1234" == result->password );
}

TEST_CASE( "Extract from parsed authorization_value_t", "[basic_auth]" )
{
	using namespace restinio::http_field_parsers;
	using namespace restinio::http_field_parsers::basic_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Basic dXNlcjoxMjM0"s );
	dummy_header.set_field(
			"X-My-Authorization",
			"Basic bXktdXNlcjpteS0xMjM0"s );

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
	REQUIRE( "basic" == field_parse_result->auth_scheme );

	const auto result = try_extract_params( *field_parse_result );

	REQUIRE( result );
	REQUIRE( "my-user" == result->username );
	REQUIRE( "my-1234" == result->password );
}

TEST_CASE( "Extract from parsed authorization_value_t "
		"(via try_parse_field(id))",
		"[basic_auth][try_parse_field]" )
{
	using namespace restinio::http_field_parsers;
	using namespace restinio::http_field_parsers::basic_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Basic bXktdXNlcjpteS0xMjM0"s );

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

	const auto field_parse_result = try_parse_field< authorization_value_t >(
			*req,
			restinio::http_field::authorization );

	REQUIRE( 0 == field_parse_result.index() );
	const auto & auth = restinio::get< authorization_value_t >(
			field_parse_result );
	REQUIRE( "basic" == auth.auth_scheme );

	const auto result = try_extract_params( auth );

	REQUIRE( result );
	REQUIRE( "my-user" == result->username );
	REQUIRE( "my-1234" == result->password );
}

TEST_CASE( "Extract from parsed authorization_value_t "
		"(via try_parse_field(name))",
		"[basic_auth][try_parse_field]" )
{
	using namespace restinio::http_field_parsers;
	using namespace restinio::http_field_parsers::basic_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			"X-My-Authorization",
			"Basic bXktdXNlcjpteS0xMjM0"s );

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

	const auto field_parse_result = try_parse_field< authorization_value_t >(
			*req,
			"x-my-authorization" );

	REQUIRE( 0 == field_parse_result.index() );
	const auto & auth = restinio::get< authorization_value_t >(
			field_parse_result );
	REQUIRE( "basic" == auth.auth_scheme );

	const auto result = try_extract_params( auth );

	REQUIRE( result );
	REQUIRE( "my-user" == result->username );
	REQUIRE( "my-1234" == result->password );
}

