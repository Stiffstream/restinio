/*
 * RESTinio
 */

/**
 * @file
 * @brief Implementation of target_path_holder helper class.
 *
 * @since v.0.6.6
 */

#pragma once

#include <restinio/utils/percent_encoding.hpp>

#include <restinio/string_view.hpp>

#include <memory>

namespace restinio
{

namespace router
{

namespace impl
{

//
// target_path_holder_t
//
/*!
 * @brief Helper class for holding a unique instance of char array with
 * target_path value.
 *
 * This class is a kind of std::unique_ptr<char[]> but it performs
 * the normalization of target_path value in the constructor.
 * All percent-encoded characters from unreserved set will be
 * decoded into their normal representation. It means that
 * target_path `/%7Etest` will be automatically transformed into
 * `/~test`.
 *
 * @since v.0.6.2
 */
class target_path_holder_t
{
	public:
		using data_t = std::unique_ptr<char[]>;

		//! Initializing constructor.
		/*!
		 * Copies the value of @a original_path into a unique and 
		 * dynamically allocated array of chars.
		 *
		 * Basic URI normalization procedure is automatically performed
		 * if necessary.
		 *
		 * @note
		 * Can throws if allocation of new data buffer fails or if
		 * @a original_path has an invalid format.
		 */
		target_path_holder_t( string_view_t original_path )
			:	m_size{ restinio::utils::uri_normalization::
					unreserved_chars::estimate_required_capacity( original_path ) }
		{
			m_data.reset( new char[ m_size ] );

			if( m_size != original_path.size() )
				// Transformation is actually needed.
				restinio::utils::uri_normalization::unreserved_chars::
						normalize_to( original_path, m_data.get() );
			else
				// Just copy original value to the destination.
				std::copy(
						original_path.begin(), original_path.end(),
						m_data.get() );
		}

		//! Get access to the value of target_path.
		/*!
		 * @attention
		 * This method should not be called after a call to giveout_data().
		 */
		RESTINIO_NODISCARD
		string_view_t
		view() const noexcept
		{
			return { m_data.get(), m_size };
		}

		//! Give out the value from holder.
		/*!
		 * @attention
		 * The holder becomes empty after the return from that method and
		 * should not be used anymore.
		 */
		RESTINIO_NODISCARD
		data_t
		giveout_data() noexcept
		{
			return std::move(m_data);
		}

	private:
		//! Actual data with target_path.
		/*!
		 * @note
		 * It becomes empty after a call to giveout_data().
		 */
		data_t m_data;
		//! The length of target_path.
		std::size_t m_size;
};

} /* namespace impl */

} /* namespace router */

} /* namespace restinio */

