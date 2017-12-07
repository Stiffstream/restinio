/*
	restinio
*/

/*!
	Tests for express router.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <iterator>

#include <restinio/all.hpp>
#include <restinio/router/pcre_regex_engine.hpp>

using namespace restinio;

using express_router_t = restinio::router::express_router_t< restinio::router::pcre_regex_engine_t<> >;
using restinio::router::route_params_t;

#include "../express_router/tests.inl"
