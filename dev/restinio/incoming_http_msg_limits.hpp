/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to limits of an incoming HTTP message.
 *
 * @since v.0.6.12
 */

#pragma once

#include <restinio/compiler_features.hpp>

#include <cstdint>
#include <limits>

namespace restinio
{

//
// incoming_http_msg_limits_t
//
/*!
 * @brief A type of holder of limits related to an incoming HTTP message.
 *
 * Since v.0.6.12 RESTinio supports various limits for incoming HTTP messages.
 * If some part of message (like the length of HTTP field name) exceeds
 * the specified limit then that message will be ignored by RESTinio.
 *
 * For the compatibility with the previous versions such limits are optional.
 * The default constructor of incoming_http_msg_limits_t sets the limits
 * to the maximum values that cannot be exceeded.
 *
 * In v.0.6.12 a user has to set appropriate values for limits by his/herself.
 * For example:
 *
 * @code
 * restinio::run(
 * 	restinio::on_this_thread<>()
 * 		.port(8080)
 * 		.address("localhost")
 * 		.incoming_http_msg_limits(
 * 			restinio::incoming_http_msg_limits_t{}
 * 				.max_url_size(8000u)
 * 				.max_field_name_size(1024u)
 * 				.max_field_value_size(4096u)
 * 				.max_body_size(10240u)
 * 		)
 * 		.request_handler(...)
 * );
 * @endcode
 *
 * @attention
 * Setters of incoming_http_msg_limits_t doesn't checks values.
 * It means that it is possible to set 0 as a limit for the length
 * of field name size. That will lead to ignorance of every incoming
 * request.
 *
 * @note
 * Almost all limits except the limit for the body size are std::size_t.
 * It means that those limits can have different borders in 32- and 64-bit
 * mode. The limit for the body size is always std::uint64_t.
 *
 * @since v.0.6.12
 */
class incoming_http_msg_limits_t
{
	std::size_t m_max_url_size{ std::numeric_limits<std::size_t>::max() };
	std::size_t m_max_field_name_size{ std::numeric_limits<std::size_t>::max() };
	std::size_t m_max_field_value_size{ std::numeric_limits<std::size_t>::max() };
	std::size_t m_max_field_count{ std::numeric_limits<std::size_t>::max() };
	std::uint64_t m_max_body_size{ std::numeric_limits<std::uint64_t>::max() };

public:
	incoming_http_msg_limits_t() noexcept = default;

	RESTINIO_NODISCARD
	std::size_t
	max_url_size() const noexcept { return m_max_url_size; }

	incoming_http_msg_limits_t &
	max_url_size( std::size_t value ) & noexcept
	{
		m_max_url_size = value;
		return *this;
	}

	incoming_http_msg_limits_t &&
	max_url_size( std::size_t value ) && noexcept
	{
		return std::move(max_url_size(value));
	}

	RESTINIO_NODISCARD
	std::size_t
	max_field_name_size() const noexcept { return m_max_field_name_size; }

	incoming_http_msg_limits_t &
	max_field_name_size( std::size_t value ) & noexcept
	{
		m_max_field_name_size = value;
		return *this;
	}

	incoming_http_msg_limits_t &&
	max_field_name_size( std::size_t value ) && noexcept
	{
		return std::move(max_field_name_size(value));
	}

	RESTINIO_NODISCARD
	std::size_t
	max_field_value_size() const noexcept { return m_max_field_value_size; }

	incoming_http_msg_limits_t &
	max_field_value_size( std::size_t value ) & noexcept
	{
		m_max_field_value_size = value;
		return *this;
	}

	incoming_http_msg_limits_t &&
	max_field_value_size( std::size_t value ) && noexcept
	{
		return std::move(max_field_value_size(value));
	}

	RESTINIO_NODISCARD
	std::size_t
	max_field_count() const noexcept { return m_max_field_count; }

	incoming_http_msg_limits_t &
	max_field_count( std::size_t value ) & noexcept
	{
		m_max_field_count = value;
		return *this;
	}

	incoming_http_msg_limits_t &&
	max_field_count( std::size_t value ) && noexcept
	{
		return std::move(max_field_count(value));
	}

	RESTINIO_NODISCARD
	std::uint64_t
	max_body_size() const noexcept { return m_max_body_size; }

	incoming_http_msg_limits_t &
	max_body_size( std::uint64_t value ) & noexcept
	{
		m_max_body_size = value;
		return *this;
	}

	incoming_http_msg_limits_t &&
	max_body_size( std::uint64_t value ) && noexcept
	{
		return std::move(max_body_size(value));
	}
};

} /* namespace restinio */

