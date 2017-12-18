/*
	restinio
*/

/*!
	Cast.
*/

#pragma once

#include <string>

#include <restinio/string_view.hpp>

#include <restinio/utils/from_string.hpp>

namespace restinio
{

//
// cast_to()
//

//! Cast string representation to a given type.
template < typename Value_Type >
Value_Type
cast_to( string_view_t str_representation )
{
	return utils::from_string< Value_Type >( str_representation );
}

} /* namespace restinio */

