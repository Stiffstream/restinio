/*
 * RESTinio
 */

/*!
 * @file
 * @brief Various meta-functions for operating the content of a tuple.
 *
 * @since v.0.6.1
 */

#pragma once

#include <restinio/compiler_features.hpp>

#include <utility>
#include <tuple>

namespace restinio
{

namespace utils
{

namespace tuple_algorithms
{

namespace impl
{

template< typename T >
using index_sequence_for_tuple =
		std::make_index_sequence< std::tuple_size<T>::value >;

template< typename Predicate, typename Tuple, std::size_t... I >
[[nodiscard]]
bool
perform_all_of(
	Predicate && p,
	Tuple && t,
	std::index_sequence<I...> )
{
	// Use fold expression after switching to C++17.
	return (p( std::get<I>(std::forward<Tuple>(t)) ) && ...);
}

template< typename Predicate, typename Tuple, std::size_t... I >
[[nodiscard]]
bool
perform_any_of(
	Predicate && p,
	Tuple && t,
	std::index_sequence<I...> )
{
	// Use fold expression after switching to C++17.
	return (p( std::get<I>(std::forward<Tuple>(t)) ) || ...);
}

} /* namespace impl */

//
// all_of
//
template< typename Tuple, typename Predicate >
[[nodiscard]]
bool
all_of( Tuple && tuple, Predicate && predicate )
{
	return impl::perform_all_of(
			std::forward<Predicate>(predicate),
			std::forward<Tuple>(tuple),
			typename impl::index_sequence_for_tuple<std::decay_t<Tuple>>{} );
}

//
// any_of
//
template< typename Tuple, typename Predicate >
[[nodiscard]]
bool
any_of( Tuple && tuple, Predicate && predicate )
{
	return impl::perform_any_of(
			std::forward<Predicate>(predicate),
			std::forward<Tuple>(tuple),
			typename impl::index_sequence_for_tuple<std::decay_t<Tuple>>{} );
}

} /* namespace tuple_algorithms */

} /* namespace utils */

} /* namespace restinio */

