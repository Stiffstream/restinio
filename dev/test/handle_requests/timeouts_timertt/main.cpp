/*
	restinio
*/

/*!
	Test triggering timeouts.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <asio.hpp>

#include <restinio/all.hpp>
#include <restinio/timertt_timer_factory.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

#if defined(__GNUG__)
#pragma GCC diagnostic ignored "-Wparentheses"
#endif

const std::string RESP_BODY{ "-=UNIT-TEST=-" };

#define TIMERTT_TYPE restinio::st_timertt_wheel_timer_factory_t
#define TIMERTT_TYPE_STR "ST timertt_wheel"

#include "tests.inl"

#undef TIMERTT_TYPE_STR
#undef TIMERTT_TYPE

#define TIMERTT_TYPE restinio::mt_timertt_wheel_timer_factory_t
#define TIMERTT_TYPE_STR "MT timertt_wheel"

#include "tests.inl"

#undef TIMERTT_TYPE_STR
#undef TIMERTT_TYPE

#define TIMERTT_TYPE restinio::st_timertt_list_timer_factory_t
#define TIMERTT_TYPE_STR "ST timertt_list"

#include "tests.inl"

#undef TIMERTT_TYPE_STR
#undef TIMERTT_TYPE

#define TIMERTT_TYPE restinio::mt_timertt_list_timer_factory_t
#define TIMERTT_TYPE_STR "MT timertt_list"

#include "tests.inl"

#undef TIMERTT_TYPE_STR
#undef TIMERTT_TYPE

#define TIMERTT_TYPE restinio::st_timertt_heap_timer_factory_t
#define TIMERTT_TYPE_STR "ST timertt_heap"

#include "tests.inl"

#undef TIMERTT_TYPE_STR
#undef TIMERTT_TYPE

#define TIMERTT_TYPE restinio::mt_timertt_heap_timer_factory_t
#define TIMERTT_TYPE_STR "MT timertt_heap"

#include "tests.inl"

#undef TIMERTT_TYPE_STR
#undef TIMERTT_TYPE
