/*
	restinio
*/

/*!
	Tests for express router.
*/

#include <catch2/catch.hpp>

#include <iterator>

#include <restinio/all.hpp>

using namespace restinio;

using express_router_t = restinio::router::express_router_t<>;
using restinio::router::route_params_t;

#include "tests.ipp"
