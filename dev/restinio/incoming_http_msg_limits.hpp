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
//FIXME: document this!
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

