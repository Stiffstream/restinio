/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to connection state listeners.
 *
 * @since v.0.5.1
 */

#pragma once

#include <restinio/compiler_features.hpp>
#include <restinio/common_types.hpp>
#include <restinio/variant.hpp>
#include <restinio/tls_fwd.hpp>

namespace restinio
{

namespace connection_state
{

/*!
 * @brief Type of object that tells that new connection has been accepted.
 *
 * If a new connection is a TLS-connection then is_tls_connection()
 * returns `true` and the information about TLS-socket can be inspected
 * via try_inspect_tls(), inspect_tls_or_throw() and
 * inspect_tls_or_default() methods.
 *
 * @since v.0.6.0
 */
class accepted_t final
{
	/*!
	 * \brief An optional pointer to TLS-related connection.
	 *
	 * Will be nullptr for non-TLS connections.
	 *
	 * \since
	 * v.0.6.0
	 */
	tls_socket_t * m_tls_socket;

public:
	accepted_t(
		tls_socket_t * tls_socket )
		:	m_tls_socket{ tls_socket }
	{}

	/*!
	 * @brief Checks if the accepted connection is a TLS-connection.
	 *
	 * \retval true if the accepted connection is a TLS-connection.
	 * \retval false if the accepted connection doesn't use TLS.
	 */
	RESTINIO_NODISCARD
	bool
	is_tls_connection() const noexcept { return nullptr != m_tls_socket; }

	/*!
	 * @brief Calls the specified lambda-function if the accepted
	 * connection is a TLS-connection.
	 *
	 * Do nothing if the accepted connection doens't use TLS.
	 *
	 * Lambda function should accept one argument of a type
	 * restinio::connection_state::tls_accessor_t (by value of by const
	 * reference).
	 *
	 * Usage example:
	 * \code
	 * class my_cause_visitor_t {
	 * 	void operator()(const restinio::connection_state::accepted_t & cause) const {
	 * 		... // Some application-logic.
	 * 		cause.try_inspect_tls([&](const restinio::connection_state::tls_accessor_t & tls_info) {
	 * 			... // Some application-specific work with TLS-params.
	 * 		});
	 * 		...
	 * 	}
	 * 	...
	 * };
	 * void some_state_listener_t::state_changed(
	 * 	const restinio::connection_state::notice_t & notice) {
	 * 	...
	 * 	restinio::visit(my_cause_visitor_t{...}, notice.cause());
	 * }
	 * \endcode
	 */
	template< typename Lambda >
	void
	try_inspect_tls( Lambda && lambda ) const;

	/*!
	 * @brief Calls the specified lambda-function if the accepted
	 * connection is a TLS-connection.
	 *
	 * Throws an instance of exception_t if the accepted connection doens't use
	 * TLS.
	 *
	 * Lambda function should accept one argument of a type
	 * restinio::connection_state::tls_accessor_t (by value of by const
	 * reference).
	 *
	 * \return the value returned by \a lambda.
	 *
	 * Usage example:
	 * \code
	 * class my_cause_visitor_t {
	 * 	void operator()(const restinio::connection_state::accepted_t & cause) const {
	 * 		... // Some application-logic.
	 * 		cause.inspect_tls_or_throw([&](const restinio::connection_state::tls_accessor_t & tls_info) {
	 * 			... // Some application-specific work with TLS-params.
	 * 		});
	 * 		...
	 * 	}
	 * 	...
	 * };
	 * void some_state_listener_t::state_changed(
	 * 	const restinio::connection_state::notice_t & notice) {
	 * 	...
	 * 	restinio::visit(my_cause_visitor_t{...}, notice.cause());
	 * }
	 * \endcode
	 */
	template< typename Lambda >
	decltype(auto)
	inspect_tls_or_throw( Lambda && lambda ) const;

	/*!
	 * @brief Calls the specified lambda-function if the accepted
	 * connection is a TLS-connection.
	 *
	 * Returns the value of \a default_value if the accepted connection doens't
	 * use TLS.
	 *
	 * Lambda function should accept one argument of a type
	 * restinio::connection_state::tls_accessor_t (by value of by const
	 * reference).
	 *
	 * \return the value returned by \a lambda if it is TLS-connection or
	 * \a default_value otherwise. Note that \a lambda can return a value
	 * of a different type and in that case the returned value will be used
	 * for constructing of a new value of type \a T.
	 *
	 * Usage example:
	 * \code
	 * class my_cause_visitor_t {
	 * 	void operator()(const restinio::connection_state::accepted_t & cause) const {
	 * 		... // Some application-logic.
	 * 		auto user_name = cause.inspect_tls_or_default(
	 * 				[&](const restinio::connection_state::tls_accessor_t & tls_info) {
	 *		 			... // Some application-specific work with TLS-params.
	 *		 			},
	 *		 			std::string{"unknown-user"});
	 * 		...
	 * 	}
	 * 	...
	 * };
	 * void some_state_listener_t::state_changed(
	 * 	const restinio::connection_state::notice_t & notice) {
	 * 	...
	 * 	restinio::visit(my_cause_visitor_t{...}, notice.cause());
	 * }
	 * \endcode
	 */
	template< typename Lambda, typename T >
	T
	inspect_tls_or_default( Lambda && lambda, T && default_value ) const;
};

/*!
 * @brief Type of object that tells that the connection has been closed.
 *
 * @note
 * This type is empty now, but it can be extended in some of future versions.
 *
 * @since v.0.6.0
 */
class closed_t final
{
};

/*!
 * @brief Type of object that tells that the connection has been upgraded
 * to WebSocket.
 *
 * @note
 * This type is empty now, but it can be extended in some of future versions.
 *
 * @since v.0.6.0
 */
class upgraded_to_websocket_t final
{
};

/*!
 * @brief A type for the representation of the current state of a connection.
 *
 * Please note that in C++17 and above it is just a `std::variant` and
 * all tools from the C++ standard library (like `std::holds_alternative`,
 * `std::get`, `std::get_if`, `std::visit`) can be used.
 *
 * But for C++14 a version of those tools from restinio namespace should
 * be used (e.g. `restinio::holds_alternative`, `restinio::get`,
 * `restinio::get_if`, `restinio::visit`).
 *
 * @since v.0.6.0
 */
using cause_t = variant_t< accepted_t, closed_t, upgraded_to_websocket_t >;

/*!
 * @brief An object with info about connection to be passed to state listener.
 *
 * That object contains available information of the connection for that
 * state listener is called.
 *
 * NOTE. Content of this type can be changed in future versions of RESTinio.
 *
 * @since v.0.5.1
 */
class notice_t
{
	connection_id_t m_conn_id;
	endpoint_t m_remote_endpoint;
	cause_t m_cause;

public :
	//! Initializing constructor.
	notice_t(
		connection_id_t conn_id,
		endpoint_t remote_endpoint,
		cause_t cause )
		:	m_conn_id{ conn_id }
		,	m_remote_endpoint{ remote_endpoint }
		,	m_cause{ cause }
	{}

	//! Get the connection id.
	RESTINIO_NODISCARD
	connection_id_t
	connection_id() const noexcept { return m_conn_id; }

	//! Get the remote endpoint for the connection.
	RESTINIO_NODISCARD
	endpoint_t
	remote_endpoint() const noexcept { return m_remote_endpoint; }

	//! Get the cause for the notification.
	/*!
	 * @attention
	 * Since v.0.6.0 the type cause_t is a variant, not a simple
	 * enumeration as in v.0.5.
	 */
	RESTINIO_NODISCARD
	cause_t
	cause() const noexcept { return m_cause; }
};

/*!
 * @brief The default no-op state listener.
 *
 * This type is used for connection_state_listener_t trait by default.
 * 
 * NOTE. When this type if used no calls to state listener will be generated.
 * It means that there won't be any performance penalties related to
 * invoking of state listener's state_changed() method.
 *
 * @since v.0.5.1
 */
struct noop_listener_t
{
	// empty type by design.
};

} /* namespace connection_state */

} /* namespace restinio */

