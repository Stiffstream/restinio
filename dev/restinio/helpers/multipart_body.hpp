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

#include <restinio/impl/string_caseless_compare.hpp>

#include <restinio/helpers/string_algo.hpp>
#include <restinio/helpers/easy_parser.hpp>
#include <restinio/helpers/http_field_parsers/basics.hpp>
#include <restinio/helpers/http_field_parsers/content-type.hpp>

#include <restinio/http_headers.hpp>
#include <restinio/request_handler.hpp>
#include <restinio/expected.hpp>

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
struct parsed_part_t
{
	http_header_fields_t fields;
	string_view_t body;
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
	expected_t< string_view_t, easy_parser::parse_error_t >
	try_parse( easy_parser::impl::source_t & from ) const noexcept
	{
		// Return the whole content from the current position.
		return from.fragment( from.current_position() );
	}
};

//
// field_value_producer_t
//
struct field_value_producer_t
	:	public easy_parser::impl::producer_tag< std::string >
{
	RESTINIO_NODISCARD
	expected_t< std::string, easy_parser::parse_error_t >
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
			return make_unexpected( easy_parser::parse_error_t{
					from.current_position(),
					easy_parser::error_reason_t::unexpected_eof
			} );

		// CR or LF symbol should be returned back.
		from.putback();

		return std::move(accumulator);
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
			) >> &parsed_part_t::fields,
			symbol(CR), symbol(LF),
			body_producer_t{} >> &parsed_part_t::body );
}

} /* namespace impl */

RESTINIO_NODISCARD
expected_t< parsed_part_t, restinio::easy_parser::parse_error_t >
try_parse_part( string_view_t part )
{
	namespace easy_parser = restinio::easy_parser;

	easy_parser::impl::source_t source{ part };

	auto actual_producer = impl::make_parser();

	return easy_parser::impl::top_level_clause_t< decltype(actual_producer) >{
				std::move(actual_producer)
			}.try_process( source );
}

//
// handling_result_t
//
//FIXME: document this!
enum class handling_result_t
{
	continue_enumeration,
	stop_enumeration,
	terminate_enumeration
};

//
// enumeration_error_t
//
//FIXME: document this!
enum class enumeration_error_t
{
	content_type_field_not_found,
	content_type_field_parse_error,
	content_type_field_inappropriate_value,
	illegal_boundary_value,
	no_parts_found,
	terminated_by_handler,
	unexpected_error
};

namespace impl
{

namespace boundary_value_checkers
{

// From https://tools.ietf.org/html/rfc1521:
//
// boundary := 0*69<bchars> bcharsnospace
//
// bchars := bcharsnospace / " "
//
// bcharsnospace :=  DIGIT / ALPHA / "'" / "(" / ")" / "+" /"_"
//                 / "," / "-" / "." / "/" / ":" / "=" / "?"
//
RESTINIO_NODISCARD
constexpr bool
is_bcharnospace( char ch )
{
	return (ch >= '0' && ch <= '9') // DIGIT
		|| ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) // ALPHA
		|| ch == '\''
		|| ch == '('
		|| ch == ')'
		|| ch == '+'
		|| ch == '_'
		|| ch == ','
		|| ch == '-'
		|| ch == '.'
		|| ch == '/'
		|| ch == ':'
		|| ch == '='
		|| ch == '?';
}

RESTINIO_NODISCARD
constexpr bool
is_bchar( char ch )
{
	return is_bcharnospace(ch) || ch == ' ';
}

} /* namespace boundary_value_checkers */

RESTINIO_NODISCARD
inline optional_t< enumeration_error_t >
check_bondary_value( string_view_t value )
{
	using namespace boundary_value_checkers;

	if( value.size() >= 1u && value.size() <= 70u )
	{
		const std::size_t last_index = value.size() - 1u;
		for( std::size_t i = 0u; i != last_index; ++i )
			if( !is_bchar( value[i] ) )
				return enumeration_error_t::illegal_boundary_value;

		if( !is_bcharnospace( value[ last_index ] ) )
			return enumeration_error_t::illegal_boundary_value;
	}
	else
		return enumeration_error_t::illegal_boundary_value;

	return nullopt;
}

RESTINIO_NODISCARD
inline expected_t< std::string, enumeration_error_t >
detect_boundary_for_multipart_body(
	const request_t & req,
	string_view_t expected_media_type,
	optional_t< string_view_t > expected_media_subtype )
{
	namespace hfp = restinio::http_field_parsers;
	using restinio::impl::is_equal_caseless;

	// Content-Type header file should be present.
	const auto content_type = req.header().opt_value_of(
			restinio::http_field::content_type );
	if( !content_type )
		return make_unexpected(
				enumeration_error_t::content_type_field_not_found );

	// Content-Type field should successfuly parsed and should
	// contain value that correspond to expected media-type.
	const auto parse_result = hfp::content_type_value_t::try_parse(
			*content_type );
	if( !parse_result )
		return make_unexpected(
				enumeration_error_t::content_type_field_parse_error );

	const auto & media_type = parse_result->media_type;
	if( !is_equal_caseless( expected_media_type, media_type.type ) )
	{
		return make_unexpected(
				enumeration_error_t::content_type_field_inappropriate_value );
	}
	if( expected_media_subtype &&
			!is_equal_caseless( *expected_media_subtype, media_type.subtype ) )
	{
		return make_unexpected(
				enumeration_error_t::content_type_field_inappropriate_value );
	}

	// `boundary` param should be present in parsed Content-Type value.
	const auto boundary = hfp::find_first(
			parse_result->media_type.parameters,
			"boundary" );
	if( !boundary )
		return make_unexpected(
				enumeration_error_t::content_type_field_inappropriate_value );
	
	// `boundary` should have valid value.
	const auto boundary_check_result = check_bondary_value( *boundary );
	if( boundary_check_result )
		return make_unexpected( *boundary_check_result );

	// Actual value of boundary mark can be created.
	std::string actual_boundary_mark;
	actual_boundary_mark.reserve( 2 + boundary->size() );
	actual_boundary_mark.append( "--" );
	actual_boundary_mark.append( boundary->data(), boundary->size() );

	return std::move(actual_boundary_mark);
}

template< typename Handler >
RESTINIO_NODISCARD
expected_t< std::size_t, enumeration_error_t >
enumerate_parts_of_request_body(
	const std::vector< string_view_t > & parts,
	Handler && handler )
{
	std::size_t parts_processed{ 0u };
	optional_t< enumeration_error_t > error;

	for( auto current_part : parts )
	{
		// The current part should be parsed to headers and the body.
		auto part_parse_result = try_parse_part( current_part );
		if( !part_parse_result )
			return make_unexpected( enumeration_error_t::unexpected_error );

		// NOTE: parsed_part is passed as rvalue reference!
		const handling_result_t handler_ret_code = handler(
				std::move(*part_parse_result) );
		if( handling_result_t::continue_enumeration != handler_ret_code )
		{
			if( handling_result_t::terminate_enumeration == handler_ret_code )
				error = enumeration_error_t::terminated_by_handler;

			break;
		}
		else
			++parts_processed;
	}

	if( error )
		return make_unexpected( *error );

	return parts_processed;
}

} /* namespace impl */

//
// enumerate_parts
//
//FIXME: document this!
template< typename Handler >
RESTINIO_NODISCARD
expected_t< std::size_t, enumeration_error_t >
enumerate_parts(
	const request_t & req,
	Handler && handler,
	string_view_t expected_media_type = string_view_t{ "multipart" },
	optional_t< string_view_t > expected_media_subtype = nullopt )
{
	//FIXME: there should be some static_assert that checks the possibility
	//to call the handler. It means the right argument type and the result
	//type should be checked.

	const auto boundary = impl::detect_boundary_for_multipart_body(
			req,
			expected_media_type,
			expected_media_subtype );
	if( boundary )
	{
		const auto parts = split_multipart_body( req.body(), *boundary );

		if( parts.empty() )
			return make_unexpected(
					enumeration_error_t::no_parts_found );

		return impl::enumerate_parts_of_request_body(
				parts,
				std::forward<Handler>(handler) );
	}

	return make_unexpected( boundary.error() );
}

} /* namespace multipart_body */

} /* namespace restinio */

