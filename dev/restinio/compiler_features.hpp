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

/*!
 * @brief A wrapper around static_assert for checking that an expression
 * is noexcept and execution of that expression
 *
 * Usage example:
 * @code
 * some_class::~some_class() noexcept {
 * 	// We should have a guarantee that this call doesn't throw.
 * 	RESTINIO_ENSURE_NOEXCEPT_CALL(m_some_resouce.release());
 * 	...
 * }
 * @endcode
 *
 * @attention
 * This macro is a part of RESTinio and is not intended to be uses as
 * a part of public API. It can be changed or remove in some future version
 * without any prior notice.
 *
 * @since v.0.6.0
 */
#define RESTINIO_ENSURE_NOEXCEPT_CALL(expr) \
	static_assert(noexcept(expr), "this call is expected to be noexcept: " #expr); \
	expr

/*!
 * @brief A wrapper around static_assert for checking that an expression is
 * noexcept
 *
 * Usage example:
 * @code
 * void remove_appropriate_items_at_front(some_container_t & cnt) noexcept {
 * 	RESTINIO_STATIC_ASSERT_NOEXCEPT(cnt.empty());
 * 	RESTINIO_STATIC_ASSERT_NOEXCEPT(cnt.front());
 * 	RESTINIO_STATIC_ASSERT_NOEXCEPT(cnt.pop_front());
 *
 * 	while(!cnt.empty() && some_confitions(cnt.front()) {
 * 		// We don't expect exceptions here.
 * 		cnt.pop_front();
 * 	}
 * }
 * @endcode
 *
 * @attention
 * This macro is a part of RESTinio and is not intended to be uses as
 * a part of public API. It can be changed or remove in some future version
 * without any prior notice.
 *
 * @since v.0.6.0
 */
#define RESTINIO_STATIC_ASSERT_NOEXCEPT(expr) \
	static_assert(noexcept(expr), #expr " is expected to be noexcept" )

/*!
 * @brief A wrapper around static_assert for checking that an expression is
 * not noexcept.
 *
 * Usage example:
 * @code
 * some_class::~some_class() noexcept {
 * 	// If that call throws then we have to use try-catch block.
 * 	RESTINIO_STATIC_ASSERT_NOT_NOEXCEPT(m_some_resouce.release());
 * 	try {
 * 		m_some_resouce.release();
 * 	}
 * 	catch(...) {}
 * 	...
 * }
 * @endcode
 *
 * @attention
 * This macro is a part of RESTinio and is not intended to be uses as
 * a part of public API. It can be changed or remove in some future version
 * without any prior notice.
 *
 * @since v.0.6.0
 */
#define RESTINIO_STATIC_ASSERT_NOT_NOEXCEPT(expr) \
	static_assert(!noexcept(expr), #expr " is not expected to be noexcept" )

namespace restinio
{

namespace static_if_details
{

template< bool Condition >
struct static_if_impl;

template<>
struct static_if_impl<true>
{
	template<typename If_Part, typename Else_Part>
	static decltype(auto)
	call( If_Part && if_part, Else_Part && )
	{
		return if_part();
	}
};

template<>
struct static_if_impl<false>
{
	template<typename If_Part, typename Else_Part>
	static decltype(auto)
	call( If_Part &&, Else_Part && else_part )
	{
		return else_part();
	}
};

} /* namespace static_if_details */

//
// static_if_else
//
/*!
 * @brief An emulation of if constexpr for C++14.
 *
 * Usage example:
 * @code
 * static_if_else< noexcept(some-expression) >(
 * 	[]() noexcept {
 * 		... // Some action that doesn't throw.
 * 	},
 * 	[] {
 * 		try {
 * 			... // Some action that throws.
 * 		}
 * 		catch(...) {}
 * 	});
 * @endcode
 *
 * @since v.0.6.1.1
 */
template< bool Condition, typename If_Part, typename Else_Part >
decltype(auto)
static_if_else( If_Part && if_part, Else_Part && else_part )
{
	return static_if_details::static_if_impl<Condition>::call(
			std::forward<If_Part>(if_part),
			std::forward<Else_Part>(else_part) );
}

} /* namespace restinio */

