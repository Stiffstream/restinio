/*
	restinio
*/

/*!
	Regex engine for using std::regex.
*/

#pragma once

#include <array>

#include <pcre2.h>

#include <restinio/impl/include_fmtlib.hpp>

#include <restinio/exception.hpp>

namespace restinio
{

namespace router
{

namespace pcre2_details
{

//
// match_results_t
//

//! A wrapper class for working with pcre match results.
template < typename Traits >
struct match_results_t final
{
	match_results_t()
	{
		m_match_data = pcre2_match_data_create( Traits::max_capture_groups, nullptr );
	}

	~match_results_t()
	{
		if( nullptr != m_match_data )
			pcre2_match_data_free( m_match_data );
	}

	match_results_t( const match_results_t & ) = delete;
	match_results_t( match_results_t && ) = delete;
	match_results_t & operator = ( const match_results_t & ) = delete;
	match_results_t & operator = ( match_results_t && ) = delete;

	struct matched_item_descriptor_t final
	{
		matched_item_descriptor_t(
			PCRE2_SIZE begin,
			PCRE2_SIZE end )
			:	m_begin{ begin }
			,	m_end{ end }
		{}

		PCRE2_SIZE m_begin;
		PCRE2_SIZE m_end;
	};

	matched_item_descriptor_t
	operator [] ( std::size_t i ) const
	{
		PCRE2_SIZE * submatches = pcre2_get_ovector_pointer( m_match_data );

		return matched_item_descriptor_t{
				submatches[ 2 * i ],
				submatches[ 1 + 2 * i ] };
	}

	std::size_t size() const { return m_size; }

	std::size_t m_size{ 0 };
	pcre2_match_data * m_match_data;
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
			: m_route_regex{ rw.m_route_regex }
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
				pcre2_code_free( m_route_regex );
			}
		}

		const pcre2_code *
		pcre2_regex() const
		{
			return m_route_regex;
		}

	private:
		pcre2_code * m_route_regex{ nullptr };

		void
		compile( string_view_t r, int options )
		{
			PCRE2_SIZE erroroffset;
			int errorcode;

			m_route_regex = pcre2_compile(
				reinterpret_cast< const unsigned char*>( r.data() ),
				r.size(),
				static_cast<unsigned int>(options),
				&errorcode,
				&erroroffset,
				nullptr );

			if( nullptr == m_route_regex )
			{
				std::array< unsigned char, 256 > buffer;
				(void)pcre2_get_error_message( errorcode, buffer.data(), buffer.size() );
				throw exception_t{
						fmt::format(
							"unable to compile regex \"{}\": {}",
							r,
							reinterpret_cast< const char * >( buffer.data() ) ) };
			}
		}
};

} /* namespace pcre2_details */

//
// pcre_traits_t
//

//! PCRE traits.
template < std::size_t Max_Capture_Groups = 20, int Compile_Options = 0, int Match_Options = 0 >
struct pcre2_traits_t
{
	static constexpr std::size_t max_capture_groups = Max_Capture_Groups;
	static constexpr int compile_options = Compile_Options;
	static constexpr int match_options = Match_Options;
};

//
// pcre2_regex_engine_t
//

//! Regex engine implementation for PCRE2.
template < typename Traits = pcre2_traits_t<> >
struct pcre2_regex_engine_t
{
	using compiled_regex_t = pcre2_details::regex_t;
	using match_results_t = pcre2_details::match_results_t< Traits >;
	using matched_item_descriptor_t = typename  match_results_t::matched_item_descriptor_t;

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
			options |= PCRE2_CASELESS;
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
			pcre2_match(
				r.pcre2_regex(),
				reinterpret_cast< const unsigned char* >( target_path.data() ),
				target_path.size(),
				0, // startoffset
				Traits::match_options,
				match_results.m_match_data,
				nullptr );

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
		if( PCRE2_ERROR_NOMATCH != rc )
		{
			throw exception_t{ fmt::format("pcre2 error: {}", rc ) };
		}
		// else PCRE2_ERROR_NOMATCH -- no match for this route

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
