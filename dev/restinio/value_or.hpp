/*
	restinio
*/

/*!
	Helper functions for parameter extraction with default values.

	@since v.0.4.4
*/

#pragma once

#include <type_traits>

#include <restinio/cast_to.hpp>
#include <restinio/uri_helpers.hpp>
#include <restinio/router/express.hpp>

namespace restinio
{

//! Get parameter value or a given default.
/*!
	Get the value of a parameter specified by \a key if parameter exists.
	If parameter exists in \a params it is obtained as \c string_view_t object and
	casted to a necessary type and then returns.
	If \a params has no such parameters then the \a default_value is returned.

	@since v.0.4.4
*/
template < typename Value_Type, typename Parameter_Container >
typename std::enable_if<
		std::is_same< Parameter_Container, query_string_params_t >::value ||
			std::is_same< Parameter_Container, router::route_params_t >::value,
		Value_Type >::type
value_or( const Parameter_Container & params, string_view_t key, Value_Type default_value )
{
	const auto value = params.get_param( key );
	if( value )
	{
		return cast_to< Value_Type >( *value );
	}

	return default_value;
}

/*!
	\brief Gets the value of a parameter specified by \a key wrapped in `optional_t<Value_Type>`
	if parameter exists and empty `optional_t<Value_Type>`
	if parameter with a given key value doesn't exist.

	If parameter exists in \a params it is obtained as \c string_view_t object and
	casted to a necessary type and then is wrapped in `optional_t<Value_Type>`.
	If \a params has no such parameters then the empty `optional_t<Value_Type>`
	is returned.

	@since v.0.4.5.1
*/
template < typename Value_Type, typename Parameter_Container >
typename std::enable_if<
		std::is_same< Parameter_Container, query_string_params_t >::value ||
			std::is_same< Parameter_Container, router::route_params_t >::value,
		optional_t< Value_Type > >::type
opt_value( const Parameter_Container & params, string_view_t key )
{
	optional_t< Value_Type > result{};

	const auto value = params.get_param( key );
	if( value )
	{
		result = cast_to< Value_Type >( *value );
	}

	return result;
}

} /* namespace restinio */
