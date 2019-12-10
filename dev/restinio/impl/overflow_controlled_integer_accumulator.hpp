/*
 * RESTinio
 */

/*!
 * @file
 * @brief Helper for parsing integer values.
 *
 * @since v.0.6.2
 */

#pragma once

#include <restinio/compiler_features.hpp>

#include <type_traits>
#include <limits>

namespace restinio
{

namespace impl
{

namespace overflow_controlled_integer_accumulator_details
{

template< typename T, typename Storage_Type >
RESTINIO_NODISCARD
typename std::enable_if< std::is_signed<T>::value, bool >::type
is_greater_than_maximum( Storage_Type v, Storage_Type maximum )
{
	return v > maximum;
}

// If T is unsigned type then this comparison has no sense.
template< typename T, typename Storage_Type >
RESTINIO_NODISCARD
typename std::enable_if< !std::is_signed<T>::value, bool >::type
is_greater_than_maximum( Storage_Type, Storage_Type )
{
	return false;
}

} /* namespace overflow_controlled_integer_accumulator_details */

//
// overflow_controlled_integer_accumulator_t
//
/*!
 * @brief Helper class for accumulating integer value during parsing
 * it from string (with check for overflow).
 *
 * Usage example:
 * @code
	int parse_int(string_view_t str) {
		overflow_controlled_integer_accumulator_t<int> acc;
		for(const auto ch : str) {
			if(!is_digit(ch))
				throw not_a_digit(ch);
			acc.next_digit(ch);
			if(acc.overflow_detected())
				throw too_big_value();
		}
		return acc.value();
	}
 * @endcode
 *
 * @since v.0.6.2
 */
template<typename T>
class overflow_controlled_integer_accumulator_t
{
	//! Type to be used for holding intermediate value.
	using storage_type = std::make_unsigned_t<T>;

	//! The current value of the accumulator.
	storage_type m_current{};
	//! Overflow detection flag.
	bool m_overflow_detected{ false };

public :
	//! Try to add another digit to the accumulator.
	/*!
	 * Value of the accumulator will be changed only if there is no overflow.
	 */
	void
	next_digit( T digit ) noexcept
	{
		using namespace overflow_controlled_integer_accumulator_details;

		constexpr storage_type multiplier{10};
		constexpr storage_type maximum{ static_cast<storage_type>(
				std::numeric_limits<T>::max())
		};

		const storage_type updated_value = m_current * multiplier +
				static_cast<storage_type>(digit);

		if( updated_value < m_current ||
				is_greater_than_maximum<T>( updated_value, maximum ) )
			m_overflow_detected = true;
		else
			m_current = updated_value;
	}

	//! Is overflow detected during previous call to next_digit?
	bool
	overflow_detected() const noexcept
	{
		return m_overflow_detected;
	}

	//! Get the current accumulator value.
	T
	value() const noexcept
	{
		return static_cast<T>(m_current);
	}
};

} /* namespace restinio */

} /* namespace restinio */

