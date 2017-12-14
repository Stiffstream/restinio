/*
	restinio
*/

/*!
	Parameter bind.
*/

#pragma once

#include <string>

#include <restinio/string_view.hpp>

#include <restinio/utils/from_string.hpp>
#include <restinio/utils/percent_encoding.hpp>

namespace restinio
{

class query_string_params_t;

//
// Forwrd declaration.
//
namespace router
{
class route_params_t;
} /* namespace router */
class parameter_bind_t;

template < typename Value_Type >
Value_Type get( const parameter_bind_t & pb );

//
// parameter_bind_t
//

//! A bind to paramater.
/*!
	This class incapsulates parameter value which is string_view.
	It gives an intention to use parameter with respect to lifetime issues.
	It makes it easier to work with parameter as with read-only stack variable.
	While lifetime of a stack variable is binded to the context of a given function
	paramater is binded to some parameter container (its  internals),
	such as router::route_params_t.
	And it makes it harder to access the underlying string_view,
	thus preventing errors with lifetime of a buffer referred with that string_view.
*/
class parameter_bind_t final
{
		friend class router::route_params_t;
		friend class query_string_params_t;
		template < typename Value_Type > friend Value_Type get( const parameter_bind_t & pb );

	public:
		//! Some usefull opertors for typical use cases.
		//! \{
		bool operator == ( const char * str ) const
		{
			return m_parameter_data == string_view_t{ str };
		}

		bool operator != ( const char * str ) const
		{
			return !( *this == str );
		}

		bool operator == ( const std::string & str ) const
		{
			return m_parameter_data == string_view_t{ str.data(), str.size() };
		}

		bool operator != ( const std::string & str ) const
		{
			return !( *this == str );
		}
		//! \}

		parameter_bind_t( const parameter_bind_t & ) = delete;
		const parameter_bind_t & operator = ( const parameter_bind_t & ) = delete;
		parameter_bind_t & operator = ( parameter_bind_t && ) = delete;

	private:
		parameter_bind_t( string_view_t parameter_data )
			:	m_parameter_data{ parameter_data }
		{}

		parameter_bind_t( parameter_bind_t && ) = default;

		//! Data of a captured parameter.
		string_view_t m_parameter_data;
};

//! Cast parameter value to type.
/*!
	Gets the string_view object of a given parameter.

	\note When getting parameter value as string_view
	a copy of internal string_view object is returned.
	And it is valid only during lifetime of
	a given parameters-incapsulator.
	If parameters-incapsulator is moved then all string views
	remain valid during life time of a newly created parameters-incapsulator.
*/
template < typename Value_Type >
Value_Type
get( const parameter_bind_t & pb )
{
	return utils::from_string< Value_Type >( pb.m_parameter_data );
}

namespace utils
{

//! Overload for escape_percent_encoding with parameter_bind_t as parameter.
inline std::string
escape_percent_encoding( const parameter_bind_t & p )
{
	return escape_percent_encoding( get< string_view_t >( p ) );
}

//! Overload for unescape_percent_encoding with parameter_bind_t as parameter.
inline std::string
unescape_percent_encoding( const parameter_bind_t & p )
{
	return unescape_percent_encoding( get< string_view_t >( p ) );
}
//! \}

} /* namespace utils */

} /* namespace restinio */
