
/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/authorization.hpp>

namespace restinio
{

namespace http_field_parsers
{

namespace authorization_details
{

bool
operator==( const token68_t & a, const token68_t & b ) noexcept
{
	return a.value == b.value;
}

} /* namespace authorization */

bool
operator==(
	const authorization_value_t::param_value_t & a,
	const authorization_value_t::param_value_t & b ) noexcept
{
	return std::tie(a.value, a.form) == std::tie(b.value, b.form);
}

bool
operator==(
	const authorization_value_t::param_t & a,
	const authorization_value_t::param_t & b ) noexcept
{
	return std::tie(a.name, a.value) == std::tie(b.name, b.value);
}

} /* namespace http_field_parsers */

} /* namespace restinio */

TEST_CASE( "Authorization", "[authorization]" )
{
	using namespace restinio::http_field_parsers;
	using namespace std::string_literals;

	using param_t = authorization_value_t::param_t;
	using param_value_t = authorization_value_t::param_value_t;
	using value_form_t = authorization_value_t::value_form_t;

	{
		const auto result = authorization_value_t::try_parse(
				"" );

		REQUIRE( !result );
	}

	{
		const auto what = "Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ#"s;
		const auto result = authorization_value_t::try_parse( what );

		REQUIRE( !result );
	}

	{
		// # can't be used in token68, but can be used in token.
		const auto what = "Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ#="s;
		const auto result = authorization_value_t::try_parse( what );

		REQUIRE( !result );
	}

	{
		const auto what = "Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ=="s;
		const auto result = authorization_value_t::try_parse( what );

		if(!result)
			std::cerr << "*** "
				<< make_error_description(result.error(), what) << std::endl;

		REQUIRE( result );

		auto expected_param = authorization_value_t::auth_param_t{
				authorization_value_t::token68_t{ "QWxhZGRpbjpvcGVuIHNlc2FtZQ=="s }
		};

		REQUIRE( "basic" == result->auth_scheme );
		REQUIRE( expected_param == result->auth_param );
	}

	{
		// NOTE: trailing space in the value.
		const auto what = "Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==  "s;
		const auto result = authorization_value_t::try_parse( what );

		if(!result)
			std::cerr << "*** "
				<< make_error_description(result.error(), what) << std::endl;

		REQUIRE( result );

		auto expected_param = authorization_value_t::auth_param_t{
				authorization_value_t::token68_t{ "QWxhZGRpbjpvcGVuIHNlc2FtZQ=="s }
		};

		REQUIRE( "basic" == result->auth_scheme );
		REQUIRE( expected_param == result->auth_param );
	}

	{
		// # can't be used in token68, but can be used in token.
		const auto what = "Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ#=#"s;
		const auto result = authorization_value_t::try_parse( what );

		if(!result)
			std::cerr << "*** "
				<< make_error_description(result.error(), what) << std::endl;

		REQUIRE( result );

		auto expected_param = authorization_value_t::auth_param_t{
			authorization_value_t::param_container_t{
				param_t{ "qwxhzgrpbjpvcgvuihnlc2ftzq#",
					param_value_t{ "#",
						value_form_t::token } }
			}
		};

		REQUIRE( "basic" == result->auth_scheme );
		REQUIRE( expected_param == result->auth_param );
	}

	{
		const auto what = R"(Digest username="Mufasa",)"
				R"(realm="http-auth@example.org", )"
				R"(URI="/dir/index.html", )"
				R"(algorithm    =MD5, )"
				R"(nonce=    "7ypf/xlj9XXwfDPEoM4URrv/xwf94BcCAzFZH4GiTo0v", )"
				R"(nc=00000001, )"
				R"(cnonce = "f2/wE4q74E6zIJEtWaHKaf5wv/H5QzzpXusqGemxURZJ", )"
				R"(qop=auth , )"
				R"(response= "8ca523f5e9506fed4657c9700eebdbec", )"
				R"(opaque ="FQhe/qaU925kfnzjCev0ciny7QMkPqMAFRtzCUYo5tdS")"
				;

		const auto result = authorization_value_t::try_parse( what );

		if(!result)
			std::cerr << "*** "
				<< make_error_description(result.error(), what) << std::endl;

		REQUIRE( result );

		auto expected_param = authorization_value_t::auth_param_t{
			authorization_value_t::param_container_t{
				param_t{ "username",
					param_value_t{ "Mufasa",
						value_form_t::quoted_string } }
				,param_t{ "realm",
					param_value_t{ "http-auth@example.org",
						value_form_t::quoted_string } }
				,param_t{ "uri",
					param_value_t{ "/dir/index.html",
						value_form_t::quoted_string } }
				,param_t{ "algorithm",
					param_value_t{ "MD5",
						value_form_t::token } }
				,param_t{ "nonce",
					param_value_t{ "7ypf/xlj9XXwfDPEoM4URrv/xwf94BcCAzFZH4GiTo0v",
						value_form_t::quoted_string } }
				,param_t{ "nc",
					param_value_t{ "00000001",
						value_form_t::token } }
				,param_t{ "cnonce",
					param_value_t{ "f2/wE4q74E6zIJEtWaHKaf5wv/H5QzzpXusqGemxURZJ",
						value_form_t::quoted_string } }
				,param_t{ "qop",
					param_value_t{ "auth",
						value_form_t::token } }
				,param_t{ "response",
					param_value_t{ "8ca523f5e9506fed4657c9700eebdbec",
						value_form_t::quoted_string } }
				,param_t{ "opaque",
					param_value_t{ "FQhe/qaU925kfnzjCev0ciny7QMkPqMAFRtzCUYo5tdS",
						value_form_t::quoted_string } }
			}
		};

		REQUIRE( "digest" == result->auth_scheme );
		REQUIRE( expected_param == result->auth_param );
	}

	{
		const auto what = R"(Digest , ,username="Mufasa",)"
				R"(realm="http-auth@example.org", )"
				R"(URI="/dir/index.html" )"
				;

		const auto result = authorization_value_t::try_parse( what );

		if(!result)
			std::cerr << "*** "
				<< make_error_description(result.error(), what) << std::endl;

		REQUIRE( result );

		auto expected_param = authorization_value_t::auth_param_t{
			authorization_value_t::param_container_t{
				param_t{ "username",
					param_value_t{ "Mufasa",
						value_form_t::quoted_string } }
				,param_t{ "realm",
					param_value_t{ "http-auth@example.org",
						value_form_t::quoted_string } }
				,param_t{ "uri",
					param_value_t{ "/dir/index.html",
						value_form_t::quoted_string } }
			}
		};

		REQUIRE( "digest" == result->auth_scheme );
		REQUIRE( expected_param == result->auth_param );
	}

	{
		const auto what = R"(VeryNonstandard)"
				;

		const auto result = authorization_value_t::try_parse( what );

		if(!result)
			std::cerr << "*** "
				<< make_error_description(result.error(), what) << std::endl;

		REQUIRE( result );

		auto expected_param = authorization_value_t::auth_param_t{};

		REQUIRE( "verynonstandard" == result->auth_scheme );
		REQUIRE( expected_param == result->auth_param );
	}

	{
		const auto what = R"(Digest , ,username="Mufasa",)"
				R"(realm="http-auth@example.org", )"
				R"(URI="/dir/index.html" )"
				;

		const auto result = authorization_value_t::try_parse( what );

		if(!result)
			std::cerr << "*** "
				<< make_error_description(result.error(), what) << std::endl;

		REQUIRE( result );

		auto expected_param = authorization_value_t::auth_param_t{
			authorization_value_t::param_container_t{
				param_t{ "username",
					param_value_t{ "Mufasa",
						value_form_t::quoted_string } }
				,param_t{ "realm",
					param_value_t{ "http-auth@example.org",
						value_form_t::quoted_string } }
				,param_t{ "uri",
					param_value_t{ "/dir/index.html",
						value_form_t::quoted_string } }
			}
		};

		REQUIRE( "digest" == result->auth_scheme );
		REQUIRE( expected_param == result->auth_param );
	}

}

