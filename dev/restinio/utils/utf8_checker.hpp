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
 *
 * Note: this class is moved to restinio::utils namespace in v.0.6.5.
 */
class utf8_checker_t
{
public:
	utf8_checker_t() = default;

	RESTINIO_NODISCARD
	bool
	process_byte( std::uint8_t byte ) noexcept
	{
		check_overlong( byte );

		if( m_current_symbol_rest_bytes > 0 )
		{
			// check byte is 10xxxxxx.
			if( (byte  & 0xC0) == 0x80 )
			{
				m_current_symbol <<= 6;
				byte &= 0x3F;

				m_current_symbol |= byte;

				if( --m_current_symbol_rest_bytes == 0 )
				{
					validate_current_symbol();
				}
			}
			else
			{
				m_state = state_t::invalid;
			}
		}
		else
		{
			m_current_symbol = 0;

			if( (byte & 0x80) == 0x00)
			{
				// mask 0xxxxxxx
				m_current_symbol_rest_bytes = 0;
			}
			else if( (byte & 0xE0) == 0xC0)
			{
				// mask 110xxxxx
				m_current_symbol_rest_bytes = 1;
				byte &= 0x1F;
			}
			else if( (byte & 0xF0) == 0xE0)
			{
				// mask 1110xxxx
				m_current_symbol_rest_bytes = 2;
				byte &= 0xF;
			}
			else if( (byte & 0xF8) == 0xF0)
			{
				// mask 11110xxx
				m_current_symbol_rest_bytes = 3;
				byte &= 0x7;
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

			m_current_symbol = byte;
		}

		return m_state == state_t::valid || m_state == state_t::may_be_overlong;
	}

	/*!
	 * @return true if the current sequence finalized.
	 */
	RESTINIO_NODISCARD
	bool
	finalized() const noexcept
	{
		return m_current_symbol_rest_bytes == 0;
	}

	void
	reset() noexcept
	{
		m_current_symbol = 0;
		m_current_symbol_rest_bytes = 0;
	}

	RESTINIO_NODISCARD
	std::uint32_t
	current_symbol() const noexcept { return m_current_symbol; }

private:

	void
	validate_current_symbol() noexcept
	{
		if( (m_current_symbol >= 0xD800 && m_current_symbol <= 0xDFFF) ||
			(m_current_symbol >= 0x110000) )
		{
			m_state = state_t::invalid;
		}
	}

	void
	check_overlong( std::uint8_t byte ) noexcept
	{
		if( m_current_symbol_rest_bytes > 0 &&
				m_state == state_t::may_be_overlong )
		{
			if( m_current_symbol_rest_bytes == 2 &&
				(byte & 0xE0) == 0x80 )
				m_state = state_t::overlong;
			else if( m_current_symbol_rest_bytes == 3 &&
				(byte & 0xF0) == 0x80 )
				m_state = state_t::overlong;
			else
				m_state = state_t::valid;
		}
		else
		{
			if( byte == 0xC0 || byte == 0xC1 )
			{
				m_state = state_t::overlong;
			}
			else if( byte == 0xE0 || byte == 0xF0
					|| byte == 0xF8 || byte == 0xFC )
			{
				m_state = state_t::may_be_overlong;
			}
		}
	}

	std::uint32_t m_current_symbol = 0u;

	std::size_t m_current_symbol_rest_bytes = 0u;

	enum class state_t
	{
		valid,
		invalid,
		may_be_overlong,
		overlong
	};

	state_t m_state = state_t::valid;
};

class utf8_checker2_t
{
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

	std::uint32_t m_current_symbol = 0u;
	state_t m_state{ state_t::wait_first_byte };

	void
	ensure_valid_result_value() noexcept
	{
		if( (m_current_symbol >= 0xD800 && m_current_symbol <= 0xDFFF) ||
			(m_current_symbol >= 0x110000) )
		{
			m_state = state_t::invalid;
		}
		else
		{
			m_state = state_t::wait_first_byte;
		}
	}

	void
	on_first_byte( std::uint8_t byte ) noexcept
	{
		if( 0xC0u == byte || 0xC1u == byte )
		{
			// It's beginning of overlong sequence.
			m_state = state_t::invalid;
		}
		else if( byte <= 0x7Fu )
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
		//NOTE: there is no need to check for overlong, because
		//it's already done in on_first_byte for the case of two
		//bytes per character.
		if( 0x80u == (byte & 0xC0u) )
		{
			m_current_symbol <<= 6;
			m_current_symbol |= (byte & 0x3Fu);
			ensure_valid_result_value();
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
				ensure_valid_result_value();
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
				ensure_valid_result_value();
		}
		else
		{
			m_state = state_t::invalid;
		}
	}

public:
	utf8_checker2_t() = default;

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

	void
	reset() noexcept
	{
		m_current_symbol = 0u;
		m_state = state_t::wait_first_byte;
	}

	RESTINIO_NODISCARD
	std::uint32_t
	current_symbol() const noexcept { return m_current_symbol; }
};

} /* namespace utils */

} /* namespace restinio */

