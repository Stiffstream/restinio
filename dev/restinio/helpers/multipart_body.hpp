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
#include <restinio/helpers/http_field_parsers/content-type.hpp>

#include <restinio/http_headers.hpp>
#include <restinio/request_handler.hpp>
#include <restinio/expected.hpp>

#include <restinio/impl/string_caseless_compare.hpp>

#include <restinio/utils/metaprogramming.hpp>

#include <iostream>

namespace restinio
{

namespace multipart_body
{

//
// split_multipart_body
//
/*!
 * @brief Helper function for spliting a multipart body into a serie of
 * separate parts.
 *
 * @return A list of separate parts. This list will be empty if no parts
 * are found or if there is some error in the body format (for example if
 * some part is opened by @a boundary but is not closed properly).
 *
 * @note
 * A user should extract the value of @a boundary should from Content-Type
 * field and modify it proper way (two leading hypens should be added to the
 * value of "boundary" parameter) by him/herself. Helper function
 * detect_boundary_for_multipart_body() can be used for that purpose.
 *
 * Usage example:
 * @code
 * using namespace restinio::multipart_body;
 *
 * const auto boundary = detect_boundary_for_multipart_body(
 * 		req, "multipart", "form-data" );
 * if( boundary )
 * {
 * 	const auto parts = split_multipart_body( req.body(), *boundary );
 * 	for( restinio::string_view_t one_part : parts )
 * 	{
 * 		... // Handling of a part.
 * 	}
 * }
 * @endcode
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
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
/*!
 * @brief A description of parsed content of one part of a multipart body.
 *
 * @since v.0.6.1
 */
struct parsed_part_t
{
	//! HTTP-fields local for that part.
	/*!
	 * @note
	 * It can be empty if no HTTP-fields are found for that part.
	 */
	http_header_fields_t fields;
	//! The body of that part.
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
/*!
 * @brief A special producer that consumes the whole remaining
 * content from the input stream.
 *
 * @attention
 * This producer can be seen as a hack. It can't be used safely
 * outside the context for that this producer was created. It's because
 * body_producer_t doesn't shift the current position in the input
 * stream.
 *
 * @since v.0.6.1
 */
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
/*!
 * @brief A special producer that consumes the rest of the current
 * line in the input stream until CR/LF will be found.
 *
 * @note
 * CR and LF symbols are not consumed from the input stream.
 *
 * @since v.0.6.1
 */
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
/*!
 * @brief A factory function for a parser of a part of multipart message.
 *
 * Handles the following rule:
@verbatim
part := *( token ':' OWS field-value CR LF ) CR LF body
@endverbatim
 *
 * Produces parsed_part_t instance.
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
auto
make_parser()
{
	using namespace parser_details;

	return produce< parsed_part_t >(
			produce< http_header_fields_t >(
				repeat( 0, N,
					produce< http_header_field_t >(
						token_p() >> to_lower() >> custom_consumer(
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

//
// try_parse_part
//
/*!
 * @brief Helper function for parsing content of one part of a multipart body.
 *
 * This function is intended to be used with split_multipart_body():
 * @code
 * using namespace restinio::multipart_body;
 *
 * const auto boundary = detect_boundary_for_multipart_body(
 * 		req, "multipart", "form-data" );
 * if( boundary )
 * {
 * 	const auto parts = split_multipart_body( req.body(), *boundary );
 * 	for( restinio::string_view_t one_part : parts )
 * 	{
 * 		const auto parsed_part = try_parse_part( one_part );
 * 		if( parsed_part )
 * 		{
 * 			... // Handle the content of the parsed part.
 * 		}
 * 	}
 * }
 * @endcode
 *
 * @since v.0.6.1
 */
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
/*!
 * @brief The result to be returned from user-provided handler of
 * parts of multipart body.
 *
 * @since v.0.6.1
 */
enum class handling_result_t
{
	//! Enumeration of parts should be continued.
	//! If there is another part the user-provided handler will
	//! be called for it.
	continue_enumeration,
	//! Enumeration of parts should be stopped.
	//! All remaining parts of multipart body will be skipped.
	//! But the result of the enumeration will be successful.
	stop_enumeration,
	//! Enumeration of parts should be ignored.
	//! All remaining parts of multipart body will be skipped and
	//! the result of the enumeration will be a failure.
	terminate_enumeration
};

//
// enumeration_error_t
//
/*!
 * @brief The result of an attempt to enumerate parts of a multipart body.
 *
 * @since v.0.6.1
 */
enum class enumeration_error_t
{
	//! Content-Type field is not found.
	//! If Content-Type is absent there is no way to detect 'boundary'
	//! parameter.
	content_type_field_not_found,
	//! Unable to parse Content-Type field value.
	content_type_field_parse_error,
	//! Content-Type field value parsed but doesn't contain an appropriate
	//! value. For example there can be media-type different from 'multipart'
	//! or 'boundary' parameter can be absent.
	content_type_field_inappropriate_value,
	//! Value of 'boundary' parameter is invalid (for example it contains
	//! some illegal characters).
	illegal_boundary_value,
	//! No parts of a multipart body actually found.
	no_parts_found,
	//! Enumeration of parts was aborted by user-provided handler.
	//! This code is returned when user-provided handler returns
	//! handling_result_t::terminate_enumeration.
	terminated_by_handler,
	//! Some unexpected error encountered during the enumeration.
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

} /* namespace impl */

//
// check_boundary_value
//
/*!
 * @brief A helper function for checking the validity of 'boundary' value.
 *
 * The allowed format for 'boundary' value is defined here:
 * https://tools.ietf.org/html/rfc2046
@verbatim
     boundary := 0*69<bchars> bcharsnospace

     bchars := bcharsnospace / " "

     bcharsnospace := DIGIT / ALPHA / "'" / "(" / ")" /
                      "+" / "_" / "," / "-" / "." /
                      "/" / ":" / "=" / "?"
@endverbatim
 *
 * @return enumeration_error_t::illegal_boundary_value if @a value has
 * illegal value or an empty optional if there is no errros detected.
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline optional_t< enumeration_error_t >
check_boundary_value( string_view_t value )
{
	using namespace impl::boundary_value_checkers;

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

//
// detect_boundary_for_multipart_body
//
/*!
 * @brief Helper function for parsing Content-Type field and extracting
 * the value of 'boundary' parameter.
 *
 * It finds Content-Type field, then parses it, then checks the value
 * of media-type, then finds 'boundary' parameter, the checks the validity
 * of 'boundary' value and then adds two leading hypens to the value of
 * 'boundary' parameter.
 *
 * The returned value (if there is no error) can be used for spliting
 * a multipart body to separate parts.
 *
 * @since v.0.6.1
 */
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
	const auto boundary_check_result = check_boundary_value( *boundary );
	if( boundary_check_result )
		return make_unexpected( *boundary_check_result );

	// Actual value of boundary mark can be created.
	std::string actual_boundary_mark;
	actual_boundary_mark.reserve( 2 + boundary->size() );
	actual_boundary_mark.append( "--" );
	actual_boundary_mark.append( boundary->data(), boundary->size() );

	return std::move(actual_boundary_mark);
}

namespace impl
{

/*!
 * @brief A function that parses every part of a multipart body and
 * calls a user-provided handler for every parsed part.
 *
 * @return the count of parts successfuly handled by @a handler or
 * error code in the case if some error is detected.
 *
 * @since v.0.6.1
 */
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

		if( handling_result_t::terminate_enumeration != handler_ret_code )
			++parts_processed;
		else
			error = enumeration_error_t::terminated_by_handler;

		if( handling_result_t::continue_enumeration != handler_ret_code )
			break;
	}

	if( error )
		return make_unexpected( *error );

	return parts_processed;
}

//
// valid_handler_type
//
template< typename, typename = restinio::utils::metaprogramming::void_t<> >
struct valid_handler_type : public std::false_type {};

template< typename T >
struct valid_handler_type<
		T,
		restinio::utils::metaprogramming::void_t<
			std::enable_if_t<
				std::is_same<
					handling_result_t,
					decltype(std::declval<T>()(std::declval<parsed_part_t>()))
				>::value,
				bool
			>
		>
	> : public std::true_type
{};

} /* namespace impl */

//
// enumerate_parts
//
/*!
 * @brief A helper function for enumeration of parts of a multipart body.
 *
 * This function:
 *
 * - finds Content-Type field for @a req;
 * - parses Content-Type field, checks the media-type and extracts
 *   the value of 'boundary' parameter. The extracted 'boundary'
 *   parameter is checked for validity;
 * - splits the body of @a req using value of 'boundary' parameter;
 * - enumerates every part of body, parses every part and calls
 *   @handler for every parsed part.
 *
 * Enumeration stops if @a handler returns handling_result_t::stop_enumeration
 * or handling_result_t::terminate_enumeration. If @a handler returns
 * handling_result_t::terminate_enumeration the enumerate_parts() returns
 * enumeration_error_t::terminated_by_handler error code.
 *
 * A handler passed as @a handler argument should be a function or
 * lambda/functor with one of the following formats:
 * @code
 * handling_result_t(parsed_part_t part);
 * handling_result_t(parsed_part_t && part);
 * handling_result_t(const parsed_part_t & part);
 * @endcode
 * Note that enumerate_part() passes parsed_part_t instance to
 * @a handler as rvalue reference. And this reference will be invalidaded
 * after the return from @a handler.
 *
 * Usage example:
 * @code
 * auto on_post(const restinio::request_handle_t & req) {
 * 	using namespace restinio::multipart_body;
 * 	const auto result = enumerate_parts( *req,
 * 		[](parsed_part_t part) {
 * 			... // Some actions with the current part.
 * 			return handling_result_t::continue_enumeration;
 * 		},
 * 		"multipart", "form-data" );
 * 	if(result) {
 * 		... // Producing positive response.
 * 	}
 * 	else {
 * 		... // Producing negative response.
 * 	}
 * 	return restinio::request_accepted();
 * }
 * @endcode
 *
 * @return the count of parts successfuly handled by @a handler or
 * error code in the case if some error is detected.
 *
 * @since v.0.6.1
 */
template< typename Handler >
RESTINIO_NODISCARD
expected_t< std::size_t, enumeration_error_t >
enumerate_parts(
	//! The request to be handled.
	const request_t & req,
	//! The handler to be called for every parsed part.
	Handler && handler,
	//! The expected value of 'type' part of 'media-type' from Content-Type.
	//! If 'type' part is not equal to @a expected_media_type then
	//! enumeration won't be performed.
	//!
	//! @note
	//! The special value '*' is not handled here.
	string_view_t expected_media_type = string_view_t{ "multipart" },
	//! The optional expected value of 'subtype' part of 'media-type'
	//! from Content-Type. If @a expected_media_subtype is specified and
	//! missmatch with 'subtype' part then enumeration won't be performed.
	//!
	//! @note
	//! The special value '*' is not handled here.
	optional_t< string_view_t > expected_media_subtype = nullopt )
{
	static_assert(
			impl::valid_handler_type< std::decay_t<Handler> >::value,
			"Handler should be callable object, "
			"should accept parsed_part_t by value, const or rvalue reference, "
			"and should return handling_result_t" );

	const auto boundary = detect_boundary_for_multipart_body(
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

