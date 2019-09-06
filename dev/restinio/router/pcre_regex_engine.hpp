/*
	restinio
*/

/*!
	Regex engine for using std::regex.
*/

#pragma once

#include <pcre.h>

#include <restinio/impl/include_fmtlib.hpp>

#include <restinio/exception.hpp>

namespace restinio
{

namespace router
{

namespace pcre_details
{


//
// match_results_t
//

//! A wrapper class for working with pcre match results.
template < typename Traits >
struct match_results_t final
{
	struct matched_item_descriptor_t final
	{
		matched_item_descriptor_t(
			int begin,
			int end )
			:	m_begin{ begin }
			,	m_end{ end }
		{}

		int m_begin;
		int m_end;
	};

	matched_item_descriptor_t
	operator [] ( std::size_t i ) const
	{
		if( m_submatches[ 2 * i ] >= 0 )
		{
			// Submatch has non-empty value.
			return matched_item_descriptor_t{
					m_submatches[ 2 * i ],
					m_submatches[ 1 + 2 * i ] };
		}

		// This submatch group is empty.
		return matched_item_descriptor_t{ 0, 0 };
	}

	std::size_t size() const { return m_size; }

	std::size_t m_size{ 0 };
	std::array< int, 3 * Traits::max_capture_groups > m_submatches;
};

//
// regex_t
//

//! A wrapper for using pcre regexes in express_router.
class regex_t final
{
	public:
		regex_t() = default;
		regex_t( string_view_t r, int options )
		{
			compile( r, options );
		}

		regex_t( const regex_t & ) = delete;
		regex_t & operator = ( const regex_t & ) = delete;

		regex_t( regex_t && rw ) noexcept
			:	m_route_regex{ rw.m_route_regex }
		{
			rw.m_route_regex = nullptr;
		}

		regex_t & operator = ( regex_t && rw ) noexcept
		{
			if( this != &rw )
			{
				m_route_regex = rw.m_route_regex;
				rw.m_route_regex = nullptr;
			}

			return *this;
		}

		~regex_t()
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
		compile( string_view_t r, int options )
		{
			const char* compile_error;
			int eoffset;

			// We need zero-terminated string.
			const std::string route{ r.data(), r.size() };

			m_route_regex = pcre_compile( route.c_str(), options, &compile_error, &eoffset, nullptr );

			if( nullptr == m_route_regex )
			{
				throw exception_t{
						fmt::format(
							"unable to compile regex \"{}\": {}",
							route,
							compile_error ) };
			}
		}
};

} /* namespace pcre_details */

//
// pcre_traits_t
//

//! PCRE traits.
template < std::size_t Max_Capture_Groups = 20, int Compile_Options = 0, int Match_Options = 0 >
struct pcre_traits_t
{
	static constexpr std::size_t max_capture_groups = Max_Capture_Groups;
	static constexpr int compile_options = Compile_Options;
	static constexpr int match_options = Match_Options;
};

//
// pcre_regex_engine_t
//

//! Regex engine implementation for PCRE.
template < typename Traits = pcre_traits_t<> >
struct pcre_regex_engine_t
{
	using compiled_regex_t = pcre_details::regex_t;
	using match_results_t = pcre_details::match_results_t< Traits >;
	using matched_item_descriptor_t = typename match_results_t::matched_item_descriptor_t;

	// Max itemes that can be captured be pcre engine.
	static constexpr std::size_t
	max_capture_groups()
	{
		return Traits::max_capture_groups;
	}

	//! Create compiled regex object for a given route.
	static auto
	compile_regex(
		//! Regular expression (the pattern).
		string_view_t r,
		//! Option for case sensativity.
		bool is_case_sensative )
	{
		int options = Traits::compile_options;

		if( !is_case_sensative )
		{
			options |= PCRE_CASELESS;
		}

		return compiled_regex_t{ r, options };
	}

	//! Wrapper function for matching logic invokation.
	static auto
	try_match(
		string_view_t target_path,
		const compiled_regex_t & r,
		match_results_t & match_results )
	{
		const int rc =
			pcre_exec(
				r.pcre_regex(),
				nullptr,
				target_path.data(),
				static_cast< int >( target_path.size() ),
				0, // startoffset
				Traits::match_options,
				match_results.m_submatches.data(),
				static_cast< int >( match_results.m_submatches.size() ) );

		if( rc > 0 )
		{
			match_results.m_size = static_cast<std::size_t>(rc);
			return true;
		}
		else if( rc == 0 )
		{
			// This should not happen,
			// because the number of groups is checked when creating route matcher.
			throw exception_t{ "unexpected: not enough submatch vector size" };
		}
		if( PCRE_ERROR_NOMATCH != rc )
		{
			throw exception_t{ fmt::format("pcre error: {}", rc ) };
		}
		// else PCRE_ERROR_NOMATCH -- no match for this route

		return false;
	}

	//! Get the beginning of a submatch.
	static auto
	submatch_begin_pos( const matched_item_descriptor_t & m )
	{
		return static_cast< std::size_t >( m.m_begin );
	}

	//! Get the end of a submatch.
	static auto
	submatch_end_pos( const matched_item_descriptor_t & m )
	{
		return static_cast< std::size_t >( m.m_end );
	}
};

} /* namespace router */

} /* namespace restinio */
