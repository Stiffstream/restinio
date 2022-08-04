/*
 * RESTinio
 */

/*!
 * @file
 * @brief An implementation of checker for UTF-8 sequences.
 *
 * @since v.0.6.5
 */

#pragma once

#include <restinio/compiler_features.hpp>

#include <cstdint>

namespace restinio
{

namespace utils
{

//
// utf8_checker_t
//

/*!
 * @brief Helper class for checking UTF-8 byte sequence during parsing
 * URI or incoming byte stream.
 */
class utf8_checker_t
{
	//! Enumeration of all possible checker states.
	enum class state_t
	{
		wait_first_byte,
		wait_second_of_two,
		wait_second_of_three,
		wait_second_of_four,
		wait_third_of_three,
		wait_third_of_four,
		wait_fourth_of_four,
		invalid,
	};

	//! The current UNICODE symbol.
	/*!
	 * Contains a valid value only if some bytes were successfully
	 * processed by process_byte() and the current state is
	 * wait_first_byte.
	 */
	std::uint32_t m_current_symbol = 0u;

	//! The current state of the checker.
	state_t m_state{ state_t::wait_first_byte };

	void
	on_first_byte( std::uint8_t byte ) noexcept
	{
		if( byte <= 0x7Fu )
		{
			m_state = state_t::wait_first_byte;
			m_current_symbol = byte;
		}
		else if( 0xC0u == (byte & 0xE0u) )
		{
			m_state = state_t::wait_second_of_two;
			m_current_symbol = (byte & 0x1Fu);
		}
		else if( 0xE0u == (byte & 0xF0u) )
		{
			m_state = state_t::wait_second_of_three;
			m_current_symbol = (byte & 0x0Fu);
		}
		else if( 0xF0u == (byte & 0xF8u) )
		{
			m_state = state_t::wait_second_of_four;
			m_current_symbol = (byte & 0x07u);
		}
		else
		{
			// Because UTF-8 can represent only ranges from:
			//
			// 0000 0000-0000 007F
			// 0000 0080-0000 07FF
			// 0000 0800-0000 FFFF
			// 0001 0000-0010 FFFF
			//
			// There is no need to check masks like 0b111110xx and so on.
			//
			// See https://datatracker.ietf.org/doc/html/rfc3629
			//
			m_state = state_t::invalid;
		}
	}

	void
	on_second_of_two( std::uint8_t byte ) noexcept
	{
		if( 0x80u == (byte & 0xC0u) )
		{
			m_current_symbol <<= 6;
			m_current_symbol |= (byte & 0x3Fu);

			// Check for overlong sequence.
			// The valid range for two bytes representation is 0x0080..0x07FF.
			if( m_current_symbol < 0x0080u )
			{
				// The value is too small, it's overlong.
				m_state = state_t::invalid;
			}
			else
				// Three is no need to check the result value against
				// invalid ranges (0xD800..0xDFFF and 0x110000..)
				// because two bytes only represents 0x0080..0x07FF.
				m_state = state_t::wait_first_byte;
		}
		else
		{
			m_state = state_t::invalid;
		}
	}

	void
	on_second_of_three( std::uint8_t byte ) noexcept
	{
		if( 0x80u == (byte & 0xC0u) )
		{
			m_current_symbol <<= 6;
			m_current_symbol |= (byte & 0x3Fu);

			m_state = state_t::wait_third_of_three;
		}
		else
		{
			m_state = state_t::invalid;
		}
	}

	void
	on_second_of_four( std::uint8_t byte ) noexcept
	{
		if( 0x80u == (byte & 0xC0u) )
		{
			m_current_symbol <<= 6;
			m_current_symbol |= (byte & 0x3Fu);

			m_state = state_t::wait_third_of_four;
		}
		else
		{
			m_state = state_t::invalid;
		}
	}

	void
	on_third_of_three( std::uint8_t byte ) noexcept
	{
		if( 0x80u == (byte & 0xC0u) )
		{
			m_current_symbol <<= 6;
			m_current_symbol |= (byte & 0x3Fu);

			// Check for overlong sequence.
			// The valid range for three bytes representation is 0x0800..0xFFFF.
			if( m_current_symbol < 0x0800u )
			{
				// The value is too small, it's overlong.
				m_state = state_t::invalid;
			}
			else
			{
				// It's necessary to check illigal points 0xD800..0xDFFF.
				if( m_current_symbol >= 0xD800 && m_current_symbol <= 0xDFFF )
					m_state = state_t::invalid;
				else
					m_state = state_t::wait_first_byte;
			}
		}
		else
		{
			m_state = state_t::invalid;
		}
	}

	void
	on_third_of_four( std::uint8_t byte ) noexcept
	{
		if( 0x80u == (byte & 0xC0u) )
		{
			m_current_symbol <<= 6;
			m_current_symbol |= (byte & 0x3Fu);

			m_state = state_t::wait_fourth_of_four;
		}
		else
		{
			m_state = state_t::invalid;
		}
	}

	void
	on_fourth_of_four( std::uint8_t byte ) noexcept
	{
		if( 0x80u == (byte & 0xC0u) )
		{
			m_current_symbol <<= 6;
			m_current_symbol |= (byte & 0x3Fu);

			// Check for overlong sequence.
			// The valid range for three bytes representation is 0x10000..0x10FFFF.
			if( m_current_symbol < 0x10000u )
			{
				// The value is too small, it's overlong.
				m_state = state_t::invalid;
			}
			else
			{
				// It's necessary to check for values above 0x10FFFF.
				// There is no need to check 0xD800..0xDFFF range because
				// it was already handled by overlong check.
				if( m_current_symbol >= 0x110000 )
					m_state = state_t::invalid;
				else
					m_state = state_t::wait_first_byte;
			}
		}
		else
		{
			m_state = state_t::invalid;
		}
	}

public:
	utf8_checker_t() = default;

	/*!
	 * Checks another byte.
	 *
	 * @note
	 * The actual value of the current symbol can be obtained only if
	 * process_byte() returns `true` and the subsequent call to
	 * finalized() returns `true`:
	 *
	 * @code
	 * utf8checker_t checker;
	 * for( const auto ch : some_string )
	 * {
	 * 	if( checker.process_byte() )
	 * 	{
	 * 		if( checker.finalized() )
	 * 			process_unicode_symbol( checker.current_symbol() );
	 * 	}
	 * 	else
	 * 	{
	 * 		... // Invalid sequence found!
	 * 		break;
	 * 	}
	 * }
	 * @endcode
	 *
	 * @retval true if the sequence is still valid and the next byte
	 * can be given to the next call to process_byte().
	 *
	 * @retval false if the sequence is invalid an there is no sense
	 * to continue call process_byte().
	 */
	RESTINIO_NODISCARD
	bool
	process_byte( std::uint8_t byte ) noexcept
	{
		switch( m_state )
		{
			case state_t::wait_first_byte:
				on_first_byte( byte );
			break;

			case state_t::wait_second_of_two:
				on_second_of_two( byte );
			break;

			case state_t::wait_second_of_three:
				on_second_of_three( byte );
			break;

			case state_t::wait_second_of_four:
				on_second_of_four( byte );
			break;

			case state_t::wait_third_of_three:
				on_third_of_three( byte );
			break;

			case state_t::wait_third_of_four:
				on_third_of_four( byte );
			break;

			case state_t::wait_fourth_of_four:
				on_fourth_of_four( byte );
			break;

			case state_t::invalid:
				// Nothing to do.
			break;
		}

		return (state_t::invalid != m_state);
	}

	/*!
	 * @return true if the current sequence finalized.
	 */
	RESTINIO_NODISCARD
	bool
	finalized() const noexcept
	{
		return state_t::wait_first_byte == m_state;
	}

	/*!
	 * Return the object into the initial state.
	 */
	void
	reset() noexcept
	{
		m_current_symbol = 0u;
		m_state = state_t::wait_first_byte;
	}

	/*!
	 * Get the collected value of the current symbol.
	 *
	 * @note
	 * It returns the actual value only if:
	 *
	 * - some bytes were successfully feed into process_byte();
	 * - finalized() returns `true`.
	 */
	RESTINIO_NODISCARD
	std::uint32_t
	current_symbol() const noexcept { return m_current_symbol; }
};

} /* namespace utils */

} /* namespace restinio */

