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
		typename Timer_Factory,
		typename Logger,
		typename Request_Handler = default_request_handler_t,
		typename Strand = asio::strand< asio::executor >,
		typename Socket = asio::ip::tcp::socket >
struct traits_t
{
	using timer_factory_t = Timer_Factory;
	using logger_t = Logger;
	using request_handler_t = Request_Handler;
	using strand_t = Strand;
	using stream_socket_t = Socket;
};

using noop_strand_t = asio::executor;

//
// single_thread_traits_t
//

template <
		typename Timer_Factory,
		typename Logger,
		typename Request_Handler = default_request_handler_t >
using single_thread_traits_t =
	traits_t< Timer_Factory, Logger, Request_Handler, noop_strand_t >;

//
// default_traits_t
//

using default_traits_t =
		traits_t<
			asio_timer_factory_t,
			null_logger_t >;

} /* namespace restinio */

