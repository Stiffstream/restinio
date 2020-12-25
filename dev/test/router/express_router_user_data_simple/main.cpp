/*
	restinio
*/

/*!
	Tests for express router.
*/

#include <catch2/catch.hpp>

#include <iterator>

#include <restinio/all.hpp>

#include "../../common/test_extra_data_factory.ipp"

using namespace restinio;

using express_router_t = restinio::router::generic_express_router_t<
		restinio::router::std_regex_engine_t,
		test::ud_factory_t >;

using restinio::router::route_params_t;

#include "../express_router/tests.ipp"

