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

#include <utility>

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

