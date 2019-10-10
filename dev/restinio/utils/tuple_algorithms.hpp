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

template< typename Predicate >
RESTINIO_NODISCARD
bool
all_of_impl( Predicate && /*p*/ )
{
	return true;
}

template< typename Predicate, typename T, typename... Vs >
RESTINIO_NODISCARD
bool
all_of_impl( Predicate && p, T && current, Vs &&... rest )
{
	return p( std::forward<T>(current) ) &&
			all_of_impl( std::forward<Predicate>(p), std::forward<Vs>(rest)... );
}

template< typename Predicate, typename Tuple, std::size_t... I >
RESTINIO_NODISCARD
bool
perform_all_of(
	Predicate && p,
	Tuple && t,
	std::index_sequence<I...> )
{
	return all_of_impl(
			std::forward<Predicate>(p),
			std::get<I>(std::forward<Tuple>(t))... );
}

template< typename Predicate >
RESTINIO_NODISCARD
bool
any_of_impl( Predicate && /*p*/ )
{
	return false;
}

template< typename Predicate, typename T, typename... Vs >
RESTINIO_NODISCARD
bool
any_of_impl( Predicate && p, T && current, Vs &&... rest )
{
	return p( std::forward<T>(current) ) ||
			any_of_impl( std::forward<Predicate>(p), std::forward<Vs>(rest)... );
}

template< typename Predicate, typename Tuple, std::size_t... I >
RESTINIO_NODISCARD
bool
perform_any_of(
	Predicate && p,
	Tuple && t,
	std::index_sequence<I...> )
{
	return any_of_impl(
			std::forward<Predicate>(p),
			std::get<I>(std::forward<Tuple>(t))... );
}

} /* namespace impl */

//
// all_of
//
template< typename Tuple, typename Predicate >
RESTINIO_NODISCARD
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
RESTINIO_NODISCARD
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

