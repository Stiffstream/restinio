/*
	restinio
*/

/*!
	Tests for express router PCRE.
*/

#include <catch2/catch.hpp>

#include <iterator>

#include <restinio/all.hpp>
#include <restinio/router/pcre_regex_engine.hpp>

using namespace restinio;

using express_router_t = restinio::router::express_router_t< restinio::router::pcre_regex_engine_t<> >;
using restinio::router::route_params_t;

#include "../express_router/tests.ipp"
