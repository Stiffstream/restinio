/*
	restinio
*/

/*!
	Regex engine for using std::regex.
*/

#pragma once

#include <pcre.h>
#include <fmt/format.h>

namespace restinio
{

namespace router
{

// Max itemes that can be captured be pcre engine.
#ifndef RESTINIO_PCRE_REGEX_INGINE_MAX_CAPTURE_GROUPS
	#define RESTINIO_PCRE_REGEX_INGINE_MAX_CAPTURE_GROUPS 20
#endif

//
// pcre_match_results_wrapper_t
//

//! A wrapper class for working with pcre match results.
struct pcre_match_results_wrapper_t
{
	struct matched_item_descriptor_t
	{
		matched_item_descriptor_t(
			const char * str,
			std::size_t size )
			:	m_str{ str }
			,	m_size{ size }
		{}

		const char * const m_str;
		const std::size_t m_size;
	};

	matched_item_descriptor_t
	operator [] ( std::size_t i ) const
	{
		if( m_submatches[ 2 * i ] >= 0 )
		{
			// Submatch has non-empty value.
			return matched_item_descriptor_t{
					m_target + m_submatches[ 2 * i ],
					static_cast< std::size_t >(
						m_submatches[ 1 + 2 * i ] - m_submatches[ 2 * i ] ) };
		}

		// This submatch group is empty.
		return matched_item_descriptor_t{ nullptr, 0 };
	}

	std::size_t size() const { return m_size; }

	//! The beginning of the matched string.
	/*!
		Used for creating matched_item_descriptor_t instances.
	*/
	const char * m_target;
	std::size_t m_size{ 0 };
	std::array< int, 3 * RESTINIO_PCRE_REGEX_INGINE_MAX_CAPTURE_GROUPS > m_submatches;
};


//
// pcre_regex_wrapper_t
//

//! A wrapper for using pcre regexes in express_router.
class pcre_regex_wrapper_t
{
	public:
		pcre_regex_wrapper_t() = default;
		pcre_regex_wrapper_t( const std::string & r, int options )
		{
			compile( r, options );
		}

		pcre_regex_wrapper_t( const pcre_regex_wrapper_t & ) = delete;
		const pcre_regex_wrapper_t & operator = ( const pcre_regex_wrapper_t & ) = delete;

		pcre_regex_wrapper_t( pcre_regex_wrapper_t && rw )
		{
			(*this) = std::move( rw );
		}

		pcre_regex_wrapper_t & operator = ( pcre_regex_wrapper_t && rw )
		{
			m_route_regex = rw.m_route_regex;
			rw.m_route_regex = nullptr;
		}

		~pcre_regex_wrapper_t()
		{
			if( nullptr != m_route_regex )
			{
				pcre_free( m_route_regex );
			}
		}

		const pcre *
		pcre_regex() const
		{
			return m_route_regex;
		}

	private:
		pcre * m_route_regex{ nullptr };

		void
		compile( const std::string & r, int options )
		{
			const char* compile_error;
			int eoffset;

			m_route_regex = pcre_compile( r.c_str(), options, &compile_error, &eoffset, NULL );
			if( nullptr == m_route_regex )
			{
				throw std::runtime_error{
						fmt::format("unable to compile regex: {}", compile_error ) };
			}
		}
};

//
// pcre_regex_engine_t
//

//! Regex engine implementation for using with standard regex implementation.
struct pcre_regex_engine_t
{
	using compiled_regex_t = pcre_regex_wrapper_t;
	using match_results_t = pcre_match_results_wrapper_t;
	using matched_item_descriptor_t = match_results_t::matched_item_descriptor_t;

	//! Create compiled regex object for a given route.
	static auto
	compile_regex(
		//! Regular expression (the pattern).
		const std::string & r,
		//! Option for case sensativity.
		bool is_case_sensative )
	{
		int options = 0;

		if( !is_case_sensative )
		{
			options |= PCRE_CASELESS;
		}

		return compiled_regex_t{ r, options };
	}

	//! Wrapper function for matching logic invokation.
	static auto
	try_match(
		const std::string & target,
		const compiled_regex_t & r,
		match_results_t & match_results )
	{
		const int rc =
			pcre_exec(
				r.pcre_regex(),
				0,
				target.data(),
				target.size(),
				0, // startoffset
				0, // options
				match_results.m_submatches.data(),
				match_results.m_submatches.size() );

		if( rc > 0 )
		{
			match_results.m_size = rc;
			match_results.m_target = target.data();
			return true;
		}
		return false;
	}

	//! Get the beginning of a submatch.
	static auto
	start_str_piece( const matched_item_descriptor_t & m )
	{
		return m.m_str;
	}

	//! Get the size of a submatch.
	static auto
	size_str_piece( const matched_item_descriptor_t & m )
	{
		return m.m_size;
	}
};

} /* namespace router */

} /* namespace restinio */
