/*
 * RESTinio
 */

/*!
 * @file 
 * @brief Detection of compiler version and absence of various features.
 *
 * @since v.0.6.0
 */

#pragma once

// Try to use __has_cpp_attribute if it is supported.
#if defined(__has_cpp_attribute)
	// clang-4 and clang-5 produce warnings when [[nodiscard]]
	// is used with -std=c++11 and -std=c++14.
	#if __has_cpp_attribute(nodiscard) && \
			!(defined(__clang__) && __cplusplus < 201703L)
		#define RESTINIO_NODISCARD [[nodiscard]]
	#endif
#endif

// Handle the result of __has_cpp_attribute.
#if !defined( RESTINIO_NODISCARD )
	#define RESTINIO_NODISCARD
#endif

//FIXME: document this!
#define RESTINIO_ENSURE_NOEXCEPT_CALL(expr) \
	static_assert(noexcept(expr), "this call is expected to be noexcept: " #expr); \
	expr

//FIXME: document this!
#define RESTINIO_STATIC_ASSERT_NOEXCEPT(expr) \
	static_assert(noexcept(expr), #expr " is expected to be noexcept" )

//FIXME: document this!
#define RESTINIO_STATIC_ASSERT_NOT_NOEXCEPT(expr) \
	static_assert(!noexcept(expr), #expr " is expected to be not noexcept" )

