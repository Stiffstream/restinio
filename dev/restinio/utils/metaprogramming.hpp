/*
 * RESTinio
 */

/*!
 * \file
 * \brief Various tools for C++ metaprogramming.
 *
 * \since
 * v.0.6.1
 */

#pragma once

#include <type_traits>

namespace restinio
{

namespace utils
{

namespace metaprogramming
{

// See https://en.cppreference.com/w/cpp/types/void_t for details.
template<typename... Ts> struct make_void { using type = void; };
template<typename... Ts> using void_t = typename make_void<Ts...>::type;

namespace impl
{

//
// debug_print
//
/*
 * NOTE: this type is intended to be used just for debugging
 * metaprogramming stuff. That is why it hasn't the definition.
 */
template<typename T>
struct debug_print;

} /* namespace impl */

//
// type_list
//
/*!
 * @brief The basic building block: a type for representation of a type list.
 *
 * @since v.0.6.1
 */
template<typename... Types>
struct type_list {};

namespace impl
{

//
// head_of
//
template<typename T, typename... Rest>
struct head_of
{
	using type = T;
};

template<typename T>
struct head_of<T>
{
	using type = T;
};

} /* namespace impl */

//
// head_of_t
//
/*!
 * @brief Metafunction to get the first item from a list of types.
 *
 * Usage example:
 * @code
 * using T = restinio::utils::metaprogramming::head_of_t<int, float, double>;
 * static_assert(std::is_same_v<T, int>, "T isn't int");
 * @endcode
 *
 * @since v.0.6.1
 */
template<typename... L>
using head_of_t = typename impl::head_of<L...>::type;

namespace impl
{

//
// tail_of
//
template<typename T, typename... Rest>
struct tail_of
{
	using type = type_list<Rest...>;
};

template<typename L>
struct tail_of<L>
{
	using type = type_list<>;
};

} /* namespace impl */

/*!
 * @brief Metafunction to get the tail of a list of types in a form of type_list.
 *
 * Returns all types expect the first one. If input list of types contains
 * just one type then `type_list<>` is returned.
 *
 * Usage example:
 * @code
 * using T = restinio::utils::metaprogramming::tail_of_t<int, float, double>;
 * static_assert(std::is_same_v<T,
 * 		restinio::utils::metaprogramming::typelist<float, double> >, "!Ok");
 * @endcode
 *
 * @since v.0.6.1
 */
template<typename... L>
using tail_of_t = typename impl::tail_of<L...>::type;

namespace impl
{

//
// put_front
//
template<typename T, typename Rest>
struct put_front;

template<typename T, template<class...> class L, typename... Rest>
struct put_front< T, L<Rest...> >
{
	using type = L<T, Rest...>;
};

} /* namespace impl */

//
// put_front_t
//
/*!
 * @brief Metafunction to insert a type to the front of a type_list.
 *
 * Usage example:
 * @code
 * using namespace restinio::utils::metaprogramming;
 *
 * using T = put_front_t<int, type_list<float, double>>;
 * static_assert(std::is_same_v<T, typelist<int, float, double> >, "!Ok");
 * @endcode
 *
 * @since v.0.6.1
 */
template<typename T, typename Rest>
using put_front_t = typename impl::put_front<T, Rest>::type;

namespace impl
{

//
// rename
//
template<typename From, template<class...> class To>
struct rename;

template<
	template<class...> class From,
	typename... Types,
	template<class...> class To>
struct rename<From<Types...>, To>
{
	using type = To<Types...>;
};

} /* namespace impl */

//
// rename_t
//
/*!
 * @brief Allows to pass all template arguments from one type to another.
 *
 * Usage example:
 * @code
 * using namespace restinio::utils::metaprogramming;
 * using T = rename_t<typelist<int, float, double>, std::tuple>;
 * static_assert(std::is_same_v<T, std::tuble<int, float, double>>, "!Ok");
 * @endcode
 *
 * @since v.0.6.1
 */
template<typename From, template<class...> class To>
using rename_t = typename impl::rename<From, To>::type;

namespace impl
{

//
// transform
//

template<
	template<class...> class Transform_F,
	typename To,
	typename From >
struct transform;

template<
	template<class...> class Transform_F,
	template<class...> class From,
	typename... Sources,
	template<class...> class To,
	typename... Results >
struct transform< Transform_F, From<Sources...>, To<Results...> >
{
	using type = typename transform<
			Transform_F,
			tail_of_t<Sources...>,
			To<Results..., typename Transform_F< head_of_t<Sources...> >::type>
		>::type;
};

template<
	template<class...> class Transform_F,
	template<class...> class From,
	template<class...> class To,
	typename... Results >
struct transform< Transform_F, From<>, To<Results...> >
{
	using type = To<Results...>;
};

} /* namespace impl */

/*!
 * @brief Applies a specified meta-function to every item from
 * a specified type-list and return a new type-list.
 *
 * Usage example:
 * @code
 * using namespace restinio::utils::metaprogramming;
 * using T = transform_t<std::decay, type_list<int, char &, const long &>>;
 * static_assert(std::is_same<T, type_list<int, char, long>>::value, "!Ok");
 * @endcode
 *
 * @since v.0.6.6
 */
template< template<class...> class Transform_F, typename From >
using transform_t = typename impl::transform<
		Transform_F,
		From,
		type_list<>
	>::type;

namespace impl
{

//
// all_of
//
template<
	template<class...> class Predicate,
	typename H,
	typename... Tail >
struct all_of
{
	static constexpr bool value = Predicate<H>::value &&
			all_of<Predicate, Tail...>::value;
};

template<
	template<class...> class Predicate,
	typename H >
struct all_of< Predicate, H >
{
	static constexpr bool value = Predicate<H>::value;
};

// Specialization for the case when types are represented as type_list.
//
// Since v.0.6.6.
template<
	template<class...> class Predicate,
	typename... Types >
struct all_of< Predicate, type_list<Types...> >
{
	static constexpr bool value = all_of<Predicate, Types...>::value;
};

} /* namespace impl */

//
// all_of
//
/*!
 * @brief Applies the predicate to all types from the list and
 * return true only if all types satisty that predicate.
 *
 * Usage example:
 * @code
 * using namespace restinio::utils::metaprogramming;
 * static_assert(all_of_v<std::is_integral, int, long, unsigned short>, "!Ok");
 * @endcode
 *
 * Since v.0.6.6 can be used with type_list:
 * @code
 * using namespace restinio::utils::metaprogramming;
 * static_assert(all_of_v<std::is_integral,
 * 	transform_t<std::decay, type_list<int &, long &, unsigned short&>>>, "!Ok");
 * @endcode
 *
 * @since v.0.6.1
 */
template< template<class...> class Predicate, typename... List >
constexpr bool all_of_v = impl::all_of<Predicate, List...>::value;

} /* namespace metaprogramming */

} /* namespace utils */

} /* namespace restinio */

