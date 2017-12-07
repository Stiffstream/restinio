#pragma once

#include <restinio/router/pcre_regex_engine.hpp>

using namespace restinio;
using namespace restinio::router;
using regex_engine_t = restinio::router::pcre_regex_engine_t<>;
using route_matcher_t = restinio::router::impl::route_matcher_t< regex_engine_t >;
