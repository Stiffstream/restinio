/*
 * RESTinio
 */

/*!
 * @file
 * @brief Lambda-related type traits.
 *
 * @note
 * This file is borrowed from SObjectizer project:
 * https://github.com/Stiffstream/sobjectizer
 *
 * @since v.0.6.6
 */

//FIXME: this file should be added to RESTinio's CMakeLists.txt.

#pragma once

#include <type_traits>

namespace restinio
{

namespace utils
{

namespace lambda_traits
{

/*!
 * \brief Detector of plain type without const/volatile modifiers.
 */
template< typename M >
struct plain_argument_type
	{
		using type =
				typename std::remove_cv<
						typename std::remove_reference< M >::type >::type;
	};

/*!
 * \brief Detector of lambda result and argument type.
 */
template< typename L >
struct traits
	: 	public traits< decltype(&L::operator()) >
	{
		//! Type to be used to pass lambda as argument to another function.
		using pass_by_type = L;
	};

/*!
 * \brief Specialization of lambda traits for const-lambda.
 */
template< class L, class R, class M >
struct traits< R (L::*)(M) const >
	{
		//! Type to be used to pass lambda as argument to another function.
		using pass_by_type = R(L::*)(M) const;

		//! Type of lambda result value.
		using result_type = R;
		//! Type of lambda argument.
		using argument_type = typename plain_argument_type< M >::type;

		//! Helper for calling a lambda.
		static R call_with_arg( L l, M m )
			{
				return l(m);
			}
	};

/*!
 * \brief Specialization of lambda traits for mutable lambda.
 */
template< class L, class R, class M >
struct traits< R (L::*)(M) >
	{
		//! Type to be used to pass lambda as argument to another function.
		using pass_by_type = R(L::*)(M);

		//! Type of lambda result value.
		using result_type = R;
		//! Type of lambda argument.
		using argument_type = typename plain_argument_type< M >::type;

		//! Helper for calling a lambda.
		static R call_with_arg( L l, M m )
			{
				return l(m);
			}
	};

/*!
 * \brief Specialization of lambda traits for const-lambda without argument.
 */
template< class L, class R >
struct traits< R (L::*)() const >
	{
		//! Type to be used to pass lambda as argument to another function.
		using pass_by_type = R(L::*)() const;

		//! Type of lambda result value.
		using result_type = R;

		//! Helper for calling a lambda.
		static R call_without_arg( L l )
			{
				return l();
			}
	};

/*!
 * \brief Specialization of lambda traits for mutable lambda without argument.
 */
template< class L, class R >
struct traits< R (L::*)() >
	{
		//! Type to be used to pass lambda as argument to another function.
		using pass_by_type = R(L::*)();
		
		//! Type of lambda result value.
		using result_type = R;

		//! Helper for calling a lambda.
		static R call_without_arg( L l )
			{
				return l();
			}
	};

/*!
 * \brief Specialization of lambda traits for ordinary function pointer.
 */
template< class R, class M >
struct traits< R(*)(M) >
	{
		//! Type to be used to pass lambda as argument to another function.
		using pass_by_type = R(*)(M);

		//! Type of lambda result value.
		using result_type = R;
		//! Type of lambda argument.
		using argument_type = typename plain_argument_type< M >::type;

		//! Helper for calling a lambda.
		static R call_with_arg( R (*l)(M), M m )
			{
				return l(m);
			}
	};

/*!
 * \brief Specialization of lambda traits for reference to ordinary function.
 */
template< class R, class M >
struct traits< R(&)(M) >
	{
		//! Type to be used to pass lambda as argument to another function.
		using pass_by_type = R(*)(M);

		//! Type of lambda result value.
		using result_type = R;
		//! Type of lambda argument.
		using argument_type = typename plain_argument_type< M >::type;

		//! Helper for calling a lambda.
		static R call_with_arg( R (*l)(M), M m )
			{
				return l(m);
			}
	};

// Please note: there are no traits for R(*)() and R(&)().
// It is because argument-less handlers is going to be disabled in
// the next major version of SObjectizer.

namespace impl
{

/*!
 * \brief A checker for lambda likeness.
 */
template< typename L >
class has_func_call_operator
	{
	private :
		template< typename P, P > struct checker;

		template< typename D > static std::true_type test(
				checker< decltype(&D::operator()), &D::operator()> * );

		template< typename D > static std::false_type test(...);

		using actual_type = typename std::decay<L>::type;

	public :
		static constexpr const bool value =
			std::is_same< std::true_type, decltype(test<actual_type>(nullptr)) >::value;
	};

/*!
 * \brief A detector of lambda argument type if the checked type is lambda.
 */
template< bool is_lambda, class L >
struct argument_if_lambda
	{};

/*!
 * \brief A specialization of lambda argument detector for the case
 * when the checked type is a lambda.
 */
template< class L >
struct argument_if_lambda< true, L >
	{
		using type = typename traits< L >::argument_type;
	};

} /* namespace impl */

/*!
 * \brief A detector of lambda argument type if the checked type is lambda.
 */
template< class L >
struct argument_type_if_lambda
	:	public impl::argument_if_lambda<
			impl::has_func_call_operator< L >::value, L >
	{};

/*!
 * \brief A detector that type is a lambda or functional object.
 */
template<typename L>
struct is_lambda
	{
		static constexpr const bool value = impl::has_func_call_operator<L>::value;
	};

} /* namespace lambda_traits */

} /* namespace utils */

} /* namespace restinio */

