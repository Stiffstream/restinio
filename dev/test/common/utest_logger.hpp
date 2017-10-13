/*
	restinio
*/

/*!
	Helper definition for logger used by unittests.
*/
#pragma once

#include <restinio/ostream_logger.hpp>

using utest_logger_t = restinio::shared_ostream_logger_t;
// using utest_logger_t = restinio::null_logger_t;
