
/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/content-disposition.hpp>

TEST_CASE( "Content-Disposition", "[content-disposition]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	{
		const auto result = content_disposition_value_t::try_parse(
				"form-data" );

		REQUIRE( result );

		REQUIRE( "form-data" == result->value );
		REQUIRE( result->parameters.empty() );
	}

	{
		const auto result = content_disposition_value_t::try_parse(
				"form-data; name=some-name" );

		REQUIRE( result );

		REQUIRE( "form-data" == result->value );

		content_disposition_value_t::parameter_container_t expected{
			{ "name"s, "some-name"s },
		};
		REQUIRE( expected == result->parameters );
	}

	{
		const auto result = content_disposition_value_t::try_parse(
				"form-data; name=some-name  ;  filename=\"file\"" );

		REQUIRE( result );

		REQUIRE( "form-data" == result->value );

		content_disposition_value_t::parameter_container_t expected{
			{ "name"s, "some-name"s },
			{ "filename"s, "file"s },
		};
		REQUIRE( expected == result->parameters );
	}

	{
		const char * what = "form-data; name=some-name  ;  filename=\"file\""
				";filename*=utf-8''another-name";
		const auto result = content_disposition_value_t::try_parse( what );

		REQUIRE( result );

		REQUIRE( "form-data" == result->value );

		content_disposition_value_t::parameter_container_t expected{
			{ "name"s, "some-name"s },
			{ "filename"s, "file"s },
			{ "filename*"s, "utf-8''another-name"s },
		};
		REQUIRE( expected == result->parameters );
	}

	{
		const char * what = "form-data; name=some-name"
				";filename*=utf-8'en-US'another-name";
		const auto result = content_disposition_value_t::try_parse( what );

		REQUIRE( result );

		REQUIRE( "form-data" == result->value );

		content_disposition_value_t::parameter_container_t expected{
			{ "name"s, "some-name"s },
			{ "filename*"s, "utf-8'en-US'another-name"s },
		};
		REQUIRE( expected == result->parameters );
	}

	{
		const char * what = "form-data; name=some-name"
				";filename*=utf-8'en-US'Yet%20another%20name";
		const auto result = content_disposition_value_t::try_parse( what );

		REQUIRE( result );

		REQUIRE( "form-data" == result->value );

		content_disposition_value_t::parameter_container_t expected{
			{ "name"s, "some-name"s },
			{ "filename*"s, "utf-8'en-US'Yet%20another%20name"s },
		};
		REQUIRE( expected == result->parameters );
	}
}

