#include <restinio/all.hpp>
#include <restinio/router/pcre2_regex_engine.hpp>

#define RESTINIO_EXPRESS_ROUTER_BENCH_APP_TITLE "Express router (pcre2) benchmark"
#define RESTINIO_EXPRESS_ROUTER_BENCH_REGEX_ENGINE restinio::router::pcre2_regex_engine_t<>

#include "../express_router_bench/main.cpp"
