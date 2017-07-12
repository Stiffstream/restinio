/*
	restinio
*/

/*!
	HTTP server traits.
*/

#pragma once

#include <restinio/request_handler.hpp>
#include <restinio/asio_timer_factory.hpp>
#include <restinio/null_logger.hpp>

namespace restinio
{

//
// traits_t
//

template <
		typename TIMER_FACTORY,
		typename LOGGER,
		typename REQUEST_HANDLER = default_request_handler_t,
		typename STRAND = asio::strand< asio::executor >,
		typename STREAM_SOCKET = asio::ip::tcp::socket >
struct traits_t
{
	using timer_factory_t = TIMER_FACTORY;
	using logger_t = LOGGER;
	using request_handler_t = REQUEST_HANDLER;
	using strand_t = STRAND;
	using stream_socket_t = STREAM_SOCKET;
};

using noop_strand_t = asio::executor;

//
// single_thread_traits_t
//

template <
		typename TIMER_FACTORY,
		typename LOGGER,
		typename REQUEST_HANDLER = default_request_handler_t >
using single_thread_traits_t =
	traits_t< TIMER_FACTORY, LOGGER, REQUEST_HANDLER, noop_strand_t >;

//
// default_traits_t
//

using default_traits_t =
		traits_t<
			asio_timer_factory_t,
			null_logger_t >;

} /* namespace restinio */

