/*
	restinio
*/

/*!
	Regex engine for using std::regex.
*/

#pragma once

#include <regex>

namespace restinio
{

namespace router
{

//
// std_regex_engine_t
//

//! Regex engine implementation for using with standard regex implementation.
struct std_regex_engine_t
{
	using compiled_regex_t = std::regex;
	using match_results_t = std::cmatch;
	using matched_item_descriptor_t = match_results_t::value_type;

	//! Wrapper function for matching logic invokation.
	static auto
	try_match(
		const std::string & target,
		const compiled_regex_t & r,
		match_results_t & match_results )
	{
		return
			std::regex_search(
				target.data(),
				target.data() + target.size(),
				match_results,
				r );
	}

	//! Get the beginning of a submatch.
	static auto
	start_str_piece( const matched_item_descriptor_t & m )
	{
		return m.first;
	}

	//! Get the size of a submatch.
	static auto
	size_str_piece( const matched_item_descriptor_t & m )
	{
		return m.second - m.first;
	}
};

} /* namespace router */

} /* namespace restinio */
