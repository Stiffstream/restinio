/*
	restinio
*/

/*!
 * @file
 * @brief Information about chunked encoded body.
 * @since v.0.6.9
 */

#pragma once

#include <restinio/exception.hpp>
#include <restinio/http_headers.hpp>

#include <restinio/impl/include_fmtlib.hpp>

#include <vector>
#include <memory>

namespace restinio
{

//
// chunk_info_t
//
//FIXME: document this!
class chunk_info_t
{
	std::size_t m_started_at;
	std::size_t m_size;

public:
	chunk_info_t(
		std::size_t started_at,
		std::size_t size )
		:	m_started_at{ started_at }
		,	m_size{ size }
	{}

	RESTINIO_NODISCARD
	std::size_t
	started_at() const noexcept { return m_started_at; }

	RESTINIO_NODISCARD
	std::size_t
	size() const noexcept { return m_size; }

	RESTINIO_NODISCARD
	string_view_t
	make_string_view_nonchecked( string_view_t full_body ) const noexcept
	{
		return full_body.substr( m_started_at, m_size );
	}

	RESTINIO_NODISCARD
	string_view_t
	make_string_view( string_view_t full_body ) const
	{
		if( m_started_at >= full_body.size() ||
				m_started_at + m_size > full_body.size() )
		{
			throw exception_t{
				fmt::format( "unable to make a chunk (started_at:{}, size: {}) "
						"from a body with length:{}",
						m_started_at,
						m_size,
						full_body.size() )
			};
		}

		return make_string_view_nonchecked( full_body );
	}
};

namespace impl
{

//
// chunked_encoding_info_block_t
//
//FIXME: document this!
struct chunked_encoding_info_block_t
{
	std::vector< chunk_info_t > m_chunks;

	http_header_fields_t m_trailing_fields;
};

} /* namespace impl */

//
// chunked_encoding_info_t
//
//FIXME: document this!
class chunked_encoding_info_t
{
	//! Actual data.
	impl::chunked_encoding_info_block_t m_info;

public:
	chunked_encoding_info_t() = default;
	chunked_encoding_info_t(
		impl::chunked_encoding_info_block_t info )
		:	m_info{ std::move(info) }
	{}

	RESTINIO_NODISCARD
	std::size_t
	chunk_count() const noexcept { return m_info.m_chunks.size(); }

	RESTINIO_NODISCARD
	const chunk_info_t &
	chunk_at_nochecked( std::size_t index ) const noexcept
	{
		return m_info.m_chunks[ index ];
	}

	RESTINIO_NODISCARD
	const chunk_info_t &
	chunk_at( std::size_t index ) const
	{
		return m_info.m_chunks.at( index );
	}

	RESTINIO_NODISCARD
	const auto &
	chunks() const noexcept
	{
		return m_info.m_chunks;
	}

	RESTINIO_NODISCARD
	const http_header_fields_t &
	trailing_fields() const noexcept
	{
		return m_info.m_trailing_fields;
	}
};

//
// chunked_encoding_info_unique_ptr_t
//
/*!
 * @brief Alias of unique_ptr for chunked_encoding_info.
 *
 * @since v.0.6.9
 */
using chunked_encoding_info_unique_ptr_t =
	std::unique_ptr< chunked_encoding_info_t >;

} /* namespace restinio */

