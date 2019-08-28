/*
	restinio
*/

/*!
	Include all necessary header files in one.
*/

#pragma once

#include <restinio/version.hpp>

#include <restinio/asio_include.hpp>
#include <restinio/settings.hpp>
#include <restinio/http_headers.hpp>
#include <restinio/message_builders.hpp>
#include <restinio/http_server.hpp>
#include <restinio/http_server_run.hpp>
#include <restinio/asio_timer_manager.hpp>
#include <restinio/null_timer_manager.hpp>
#include <restinio/null_logger.hpp>
#include <restinio/ostream_logger.hpp>
#include <restinio/uri_helpers.hpp>
#include <restinio/cast_to.hpp>
#include <restinio/value_or.hpp>

#include <restinio/router/express.hpp>
