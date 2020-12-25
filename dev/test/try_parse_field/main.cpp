/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/try_parse_field.hpp>
#include <restinio/helpers/http_field_parsers/content-encoding.hpp>

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

TEST_CASE( "No Content-Encoding field", "[try_parse_field]" )
{
	using namespace restinio::http_field_parsers;

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			restinio::http_request_header_t{},
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

	struct handler_t
	{
		void operator()(const content_encoding_value_t &) const {
			REQUIRE( false );
		}

		void operator()(field_not_found_t) const {
			REQUIRE( true );
		}

		void operator()(restinio::easy_parser::parse_error_t) const {
			REQUIRE( false );
		}
	};

	restinio::visit( handler_t{},
			try_parse_field< content_encoding_value_t >(
					*req,
					restinio::http_field::content_encoding) );
}

TEST_CASE( "Empty Content-Encoding field", "[try_parse_field]" )
{
	using namespace restinio::http_field_parsers;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::content_encoding,
			""s );

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

	struct handler_t
	{
		void operator()(const content_encoding_value_t &) const {
			REQUIRE( false );
		}

		void operator()(field_not_found_t) const {
			REQUIRE( false );
		}

		void operator()(restinio::easy_parser::parse_error_t err) const {
			REQUIRE( restinio::easy_parser::error_reason_t::unexpected_eof
					== err.reason() );
		}
	};

	restinio::visit( handler_t{},
			try_parse_field< content_encoding_value_t >(
					*req,
					restinio::http_field::content_encoding) );
}

TEST_CASE( "Normal Content-Encoding field", "[try_parse_field]" )
{
	using namespace restinio::http_field_parsers;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::content_encoding,
			"UTF-8"s );

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

	struct handler_t
	{
		void operator()(const content_encoding_value_t & v) const {
			REQUIRE( std::vector<std::string>{ "utf-8"s } == v.values);
		}

		void operator()(field_not_found_t) const {
			REQUIRE( false );
		}

		void operator()(restinio::easy_parser::parse_error_t) const {
			REQUIRE( false );
		}
	};

	restinio::visit( handler_t{},
			try_parse_field< content_encoding_value_t >(
					*req,
					restinio::http_field::content_encoding) );
}

TEST_CASE( "Default value for Content-Encoding", "[try_parse_field]" )
{
	using namespace restinio::http_field_parsers;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

	struct handler_t
	{
		void operator()(const content_encoding_value_t & v) const {
			REQUIRE( std::vector<std::string>{ "utf-8"s } == v.values);
		}

		void operator()(field_not_found_t) const {
			REQUIRE( false );
		}

		void operator()(restinio::easy_parser::parse_error_t) const {
			REQUIRE( false );
		}
	};

	restinio::visit( handler_t{},
			try_parse_field< content_encoding_value_t >(
					*req,
					restinio::http_field::content_encoding,
					"UTF-8") );
}

TEST_CASE( "Normal Content-Encoding field with custom name", "[try_parse_field]" )
{
	using namespace restinio::http_field_parsers;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			"My-Content-Encoding",
			"UTF-8"s );

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

	struct handler_t
	{
		void operator()(const content_encoding_value_t & v) const {
			REQUIRE( std::vector<std::string>{ "utf-8"s } == v.values);
		}

		void operator()(field_not_found_t) const {
			REQUIRE( false );
		}

		void operator()(restinio::easy_parser::parse_error_t) const {
			REQUIRE( false );
		}
	};

	restinio::visit( handler_t{},
			try_parse_field< content_encoding_value_t >(
					*req,
					"my-content-encoding") );
}

TEST_CASE( "Default value for Content-Encoding with custom name", "[try_parse_field]" )
{
	using namespace restinio::http_field_parsers;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

	struct handler_t
	{
		void operator()(const content_encoding_value_t & v) const {
			REQUIRE( std::vector<std::string>{ "utf-8"s } == v.values);
		}

		void operator()(field_not_found_t) const {
			REQUIRE( false );
		}

		void operator()(restinio::easy_parser::parse_error_t) const {
			REQUIRE( false );
		}
	};

	restinio::visit( handler_t{},
			try_parse_field< content_encoding_value_t >(
					*req,
					"My-Content-Encoding",
					"UTF-8") );
}

TEST_CASE( "Normal Content-Encoding field with get_if", "[try_parse_field]" )
{
	using namespace restinio::http_field_parsers;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::content_encoding,
			"UTF-8"s );

	restinio::no_extra_data_factory_t extra_data_factory;
	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint(),
			extra_data_factory );

	const auto parse_result = try_parse_field< content_encoding_value_t >(
			*req,
			restinio::http_field::content_encoding );
	if( const auto * v = restinio::get_if< content_encoding_value_t >(
			&parse_result ) ) 
	{
		REQUIRE( std::vector<std::string>{ "utf-8"s } == v->values);
	}
	else
	{
		REQUIRE( false );
	}
}

