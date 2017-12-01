/*
	restinio
*/

/*!
	Tests for express router.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <iterator>

#include <restinio/router/express.hpp>

using namespace restinio;

using express_router_t = restinio::router::express_router_t< >;
using restinio::router::route_params_t;

#include "../express_router/tests.inl"
