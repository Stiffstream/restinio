/*
	restinio
*/

/*!
	Adoption for std::string_view (c++17).
*/

#pragma once

#if defined( __has_include )
	//
	// Check for std::string_view or std::experimental::string_view
	//
	#define RESTINIO_CHECK_FOR_STRING_VIEW
	#if defined( _MSC_VER ) && !_HAS_CXX17
		// Visual C++ 14.* allows to include <string_view> only in c++17 mode.
		#undef RESTINIO_CHECK_FOR_STRING_VIEW
	#endif

	#if defined( RESTINIO_CHECK_FOR_STRING_VIEW )

		#if __has_include(<experimental/string_view>)
			#include <experimental/string_view>
			#define RESTINIO_HAS_EXPERIMENTAL_STRING_VIEW
		#elif __has_include(<string_view>)
			#include <string_view>
			#define RESTINIO_HAS_STRING_VIEW
		#endif

		#if defined( RESTINIO_HAS_STRING_VIEW ) || defined( RESTINIO_HAS_EXPERIMENTAL_STRING_VIEW )
			#define RESTINIO_SUPPORTS_STRING_VIEW
		#endif
	#endif
#endif

namespace restinio
{

#if defined( RESTINIO_SUPPORTS_STRING_VIEW )
	#if defined( RESTINIO_HAS_STRING_VIEW )
		using string_view_t = std::string_view;
	#elif defined( JSON_DTO_HAS_EXPERIMENTAL_OPTIONAL )
		using string_view_t = std::experimental::string_view;
	#else
		// In case nothing standard is available use string.
		using string_view_t = std::string;
	#endif
#endif

} /* namespace restinio */
