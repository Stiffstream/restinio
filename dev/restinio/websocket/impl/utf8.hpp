/*!
	UTF-8 .
*/

#pragma once

#include <iostream> // std::cout, debug

#include <restinio/string_view.hpp>
#include <restinio/utils/utf8_checker.hpp>

namespace restinio
{

namespace websocket
{

namespace basic
{

namespace impl
{

//
// check_utf8_is_correct
//

inline bool
check_utf8_is_correct( string_view_t sv ) noexcept
{
	restinio::utils::utf8_checker_t checker;

	for( const auto & ch : sv )
	{
		if( !checker.process_byte( static_cast<std::uint8_t>(ch) ) )
		{
			return false;
		}
	}

	return checker.finalized();
}

} /* namespace impl */

} /* namespace basic */

} /* namespace websocket */

} /* namespace restinio */
