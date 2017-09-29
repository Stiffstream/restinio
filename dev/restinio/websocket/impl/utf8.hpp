/*!
	UTF-8 .
*/

#pragma once

#include <string>
#include <array>
#include <exception>
#include <iostream> // std::cout, debug
#include <algorithm>

namespace restinio
{

namespace websocket
{

namespace impl
{

class utf8_checker_t
{
	public:

		utf8_checker_t() = default;

		bool
		process_byte( std::uint8_t byte )
		{
			if( m_current_symbol_rest_bytes > 0 )
			{
				// check byte is 10xxxxxx.
				if( (bytehg  & 0xC0) == 0x80 )
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

				if( byte == 0xC0 || byte == 0xC1 )
				{
					m_state = state_t::invalid;
				}

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
				else if( (byte & 0xFC) == 0xF8)
				{
					// mask 111110xx
					m_current_symbol_rest_bytes = 4;
					byte &= 0x3;
				}
				else if( (byte & 0xFE) == 0xFC)
				{
					// mask 1111110x
					m_current_symbol_rest_bytes = 5;
					byte &= 0x1;
				}
				else
				{
					m_state = state_t::invalid;
				}

				m_current_symbol = byte;
			}

			return m_state == state_t::valid;
		}

		bool
		final() const
		{
			return m_current_symbol_rest_bytes == 0;
		}

	private:

		void
		validate_current_symbol()
		{
			// std::cout << "CURRENT SYMBOL: " << std::hex << m_current_symbol << "\n";
			if( (m_current_symbol >= 0xD800 && m_current_symbol <= 0xDFFF) ||
				(m_current_symbol >= 0x110000) )
			{
				m_state = state_t::invalid;
			}
		}

		uint32_t m_current_symbol = 0;

		size_t m_current_symbol_rest_bytes = 0;

		enum class state_t
		{
			valid,
			invalid
		};

		state_t m_state = state_t::valid;
};

inline bool
check_utf8_is_correct( const std::string & str )
{
	utf8_checker_t checker;

	for( const auto & ch : str )
	{
		if( ! checker.process_byte( static_cast<std::uint8_t>(ch) ))
		{
			return false;
		}
	}

	return checker.final();
}
} /* namespace impl */

} /* namespace websocket */

} /* namespace restinio */