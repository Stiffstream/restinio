/*
	restinio
*/

/*!
	Regex engine for using std::regex.
*/

#pragma once

#include <boost/regex.hpp>

namespace restinio
{

namespace router
{

//
// boost_regex_engine_t
//

//! Regex engine implementation for using with standard regex implementation.
struct boost_regex_engine_t
{
	using compiled_regex_t = boost::regex;
	using match_results_t = std::vector< std::pair< std::size_t, size_t > >;
	using matched_item_descriptor_t = match_results_t::value_type;

	static constexpr std::size_t
	max_capture_groups()
	{
		// The size of match results for standard regexes cannot be reserved
		// and grows as needed, so no limits beforehand.
		return std::numeric_limits< std::size_t >::max();
	}

	//! Create compiled regex object for a given route.
	static auto
	compile_regex(
		//! Regular expression (the pattern).
		string_view_t r,
		//! Option for case sensativity.
		bool is_case_sensative )
	{
		compiled_regex_t::flag_type regex_flags = boost::regex::ECMAScript;

		if( !is_case_sensative )
		{
			regex_flags |= boost::regex::icase;
		}

		return compiled_regex_t{ r.data(), r.size(), regex_flags };
	}

	//! Wrapper function for matching logic invokation.
	static auto
	try_match(
		string_view_t target_path,
		const compiled_regex_t & r,
		match_results_t & match_results )
	{
		boost::cmatch matches;
		if(
			boost::regex_search(
				target_path.data(),
				target_path.data() + target_path.size(),
				matches,
				r ) )
		{
			match_results.reserve( matches.size() );

			std::transform(
				matches.begin(),
				matches.end(),
				std::back_inserter( match_results ),
				[ begin = target_path.data() ]( const auto & m ){
					return matched_item_descriptor_t{ m.first - begin, m.second - begin };
				} );

			return true;
		}
		return false;
	}

	//! Get the beginning of a submatch.
	static auto
	submatch_begin_pos( const matched_item_descriptor_t & m )
	{
		return m.first;
	}

	//! Get the end of a submatch.
	static auto
	submatch_end_pos( const matched_item_descriptor_t & m )
	{
		return m.second;
	}
};

} /* namespace router */

} /* namespace restinio */
