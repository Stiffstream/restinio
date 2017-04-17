/*
	restinio
*/

/*!
	Helper definition for logger used by unittests.
*/
#pragma once

#include <sample/common/ostream_logger.hpp>

using utest_logger_t = restinio::sample::single_threaded_ostream_logger_t;
// using utest_logger_t = restinio::null_logger_t;
