/*
 * RESTinio
 */

/*!
 * @file
 * @brief Various tools for working with multipart bodies.
 *
 * @since v.0.6.1
 */

#pragma once

#include <restinio/helpers/string_algo.hpp>
#include <restinio/helpers/easy_parser.hpp>
#include <restinio/helpers/http_field_parsers/basics.hpp>

#include <restinio/http_headers.hpp>

#include <iostream>

namespace restinio
{

namespace multipart_body
{

//
// split_multipart_body
//
//FIXME: document this!
RESTINIO_NODISCARD
//FIXME: it is better to give an alias for std::vector<string_view_t>.
inline std::vector< string_view_t >
split_multipart_body(
	string_view_t body,
	string_view_t boundary )
{
	using namespace restinio::string_algo;

	std::vector< string_view_t > result;
	std::vector< string_view_t > tmp_result;

	const string_view_t eol{ "\r\n" };
	const string_view_t last_separator{ "--\r\n" };

	// Find the first boundary.
	auto boundary_pos = body.find( boundary );
	if( string_view_t::npos == boundary_pos )
		// There is no initial separator in the body.
		return result;

	// The first body can be at the very begining of the body or
	// there should be CRLF before the initial boundary.
	if( boundary_pos != 0u &&
			(boundary_pos < eol.size() ||
			body.substr( boundary_pos - eol.size(), eol.size() ) != eol) )
		return result;

	auto remaining_body = body.substr( boundary_pos + boundary.size() );
	if( starts_with( remaining_body, last_separator ) )
		// The start boundary is the last boundary.
		return result;

	while( starts_with( remaining_body, eol ) )
	{
		remaining_body = remaining_body.substr( eol.size() );

		boundary_pos = remaining_body.find( boundary );
		if( string_view_t::npos == boundary_pos )
			return result;

		// There should be CRLF before the next boundary.
		if( boundary_pos < eol.size() ||
				remaining_body.substr( boundary_pos - eol.size(), eol.size() ) != eol )
			return result;

		tmp_result.push_back(
				remaining_body.substr( 0u, boundary_pos - eol.size() ) );

		remaining_body = remaining_body.substr( boundary_pos + boundary.size() );
		// Is this boundary the last one?
		if( starts_with( remaining_body, last_separator ) )
		{
			// Yes, our iteration can be stopped and we can return the result.
			swap( tmp_result, result );
			return result;
		}
	}

	// We didn't find the last boundary. Or some error encountered in the format
	// of the body.
	//
	// Empty result should be returned.
	return result;
}

//
// parsed_part_t
//
//FIXME: public members of that struct shouldn't have m_ prefix!
struct parsed_part_t
{
	http_header_fields_t m_fields;
	string_view_t m_body;
};

namespace impl
{

namespace parser_details
{

using namespace restinio::http_field_parsers;

namespace easy_parser = restinio::easy_parser;

constexpr char CR = '\r';
constexpr char LF = '\n';

//
// body_producer_t
//
struct body_producer_t
	:	public easy_parser::impl::producer_tag< string_view_t >
{
	RESTINIO_NODISCARD
	std::pair< bool, string_view_t >
	try_parse( easy_parser::impl::source_t & from ) const noexcept
	{
		// Return the whole content from the current position.
		return std::make_pair( true, from.fragment( from.current_position() ) );
	}
};

//
// field_value_producer_t
//
struct field_value_producer_t
	:	public easy_parser::impl::producer_tag< std::string >
{
	RESTINIO_NODISCARD
	std::pair< bool, std::string >
	try_parse( easy_parser::impl::source_t & from ) const
	{
		std::string accumulator;
		auto ch = from.getch();
		while( !ch.m_eof && ch.m_ch != CR && ch.m_ch != LF )
		{
			accumulator += ch.m_ch;
			ch = from.getch();
		}

		if( ch.m_eof )
			return std::make_pair( false, std::string{} );

		// CR or LF symbol should be returned back.
		from.putback();

		return std::make_pair( true, std::move(accumulator) );
	}
};

} /* namespace parser_details */

//
// make_parser
//
RESTINIO_NODISCARD
auto
make_parser()
{
	using namespace parser_details;

	return produce< parsed_part_t >(
			produce< http_header_fields_t >(
				repeat( 0, N,
					produce< http_header_field_t >(
						token_producer() >> to_lower() >> custom_consumer(
								[](auto & f, std::string && v) {
									f.name(std::move(v));
								} ),
						symbol(':'),
						ows(),
						field_value_producer_t{} >> custom_consumer(
								[](auto & f, std::string && v) {
									f.value(std::move(v));
								} ),
						symbol(CR), symbol(LF)
					) >> custom_consumer(
							[](auto & to, http_header_field_t && v) {
								to.set_field( std::move(v) );
							} )
				)
			) >> &parsed_part_t::m_fields,
			symbol(CR), symbol(LF),
			body_producer_t{} >> &parsed_part_t::m_body );
}

} /* namespace impl */

RESTINIO_NODISCARD
std::pair< bool, parsed_part_t >
try_parse_part( string_view_t part )
{
	namespace easy_parser = restinio::easy_parser;

	easy_parser::impl::source_t source{ part };

	auto actual_producer = impl::make_parser();

	return easy_parser::impl::top_level_clause_t< decltype(actual_producer) >{
				std::move(actual_producer)
			}.try_process( source );
}

} /* namespace multipart_body */

} /* namespace restinio */

