/*
	restinio
*/

/*!
	Regex engine for using std::regex.
*/

#pragma once

#include <array>

#include <pcre2.h>
#include <fmt/format.h>

#include <restinio/exception.hpp>

namespace restinio
{

namespace router
{

// Max itemes that can be captured be pcre engine.
#ifndef RESTINIO_PCRE2_REGEX_ENGINE_MAX_CAPTURE_GROUPS
	#define RESTINIO_PCRE2_REGEX_ENGINE_MAX_CAPTURE_GROUPS 20
#endif

//
// pcre2_match_results_wrapper_t
//

//! A wrapper class for working with pcre match results.
struct pcre2_match_results_wrapper_t final
{
	pcre2_match_results_wrapper_t()
	{
		m_match_data = pcre2_match_data_create(
			RESTINIO_PCRE2_REGEX_ENGINE_MAX_CAPTURE_GROUPS,
			nullptr );
	}

	~pcre2_match_results_wrapper_t()
	{
		if( nullptr != m_match_data )
			pcre2_match_data_free( m_match_data );
	}

	pcre2_match_results_wrapper_t( const pcre2_match_results_wrapper_t & ) = delete;
	pcre2_match_results_wrapper_t( pcre2_match_results_wrapper_t && ) = delete;
	const pcre2_match_results_wrapper_t & operator = ( const pcre2_match_results_wrapper_t & ) = delete;
	pcre2_match_results_wrapper_t & operator = ( pcre2_match_results_wrapper_t && ) = delete;

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
		PCRE2_SIZE * submatches = pcre2_get_ovector_pointer( m_match_data );

		return matched_item_descriptor_t{
				m_target + submatches[ 2 * i ],
				static_cast< std::size_t >(
					submatches[ 1 + 2 * i ] - submatches[ 2 * i ] ) };
	}

	std::size_t size() const { return m_size; }

	//! The beginning of the matched string.
	/*!
		Used for creating matched_item_descriptor_t instances.
	*/
	const char * m_target;
	std::size_t m_size{ 0 };

	pcre2_match_data * m_match_data;
};

//
// pcre2_regex_wrapper_t
//

//! A wrapper for using pcre regexes in express_router.
class pcre2_regex_wrapper_t
{
	public:
		pcre2_regex_wrapper_t() = default;
		pcre2_regex_wrapper_t( const std::string & r, int options )
		{
			compile( r, options );
		}

		pcre2_regex_wrapper_t( const pcre2_regex_wrapper_t & ) = delete;
		const pcre2_regex_wrapper_t & operator = ( const pcre2_regex_wrapper_t & ) = delete;

		pcre2_regex_wrapper_t( pcre2_regex_wrapper_t && rw )
		{
			(*this) = std::move( rw );
		}

		pcre2_regex_wrapper_t & operator = ( pcre2_regex_wrapper_t && rw )
		{
			if( this != &rw )
			{
				m_route_regex = rw.m_route_regex;
				rw.m_route_regex = nullptr;
			}

			return *this;
		}

		~pcre2_regex_wrapper_t()
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
		compile( const std::string & r, int options )
		{
			PCRE2_SIZE erroroffset;
			int errorcode;

			m_route_regex = pcre2_compile(
				(const unsigned char*)r.data(),
				r.size(),
				options,
				&errorcode,
				&erroroffset,
				NULL );

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

//
// pcre2_regex_engine_t
//

//! Regex engine implementation for using with standard regex implementation.
struct pcre2_regex_engine_t
{
	using compiled_regex_t = pcre2_regex_wrapper_t;
	using match_results_t = pcre2_match_results_wrapper_t;
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
			options |= PCRE2_CASELESS;
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
			pcre2_match(
				r.pcre2_regex(),
				(const unsigned char*)target.data(),
				target.size(),
				0, // startoffset
				0, // options
				match_results.m_match_data,
				nullptr );

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
